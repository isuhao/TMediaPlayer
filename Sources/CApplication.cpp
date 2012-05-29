
#include "CApplication.hpp"
#include "CSong.hpp"
#include "CSongTableModel.hpp"
#include "CStaticPlayList.hpp"
#include "CDynamicPlayList.hpp"
#include "CListFolder.hpp"
#include "CPlayListView.hpp"
#include "CDialogEditSong.hpp"
#include "CDialogEditSongs.hpp"
#include "CDialogEditStaticPlayList.hpp"
#include "CDialogEditDynamicList.hpp"
#include "CDialogPreferences.hpp"

// Qt
#include <QStandardItemModel>
#include <QSettings>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>
#include <QFileDialog>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDomDocument>
#include <QCryptographicHash>

// FMOD
#include <fmod/fmod.hpp>

// DEBUG
#include <QtDebug>
#include <QSqlDriver>


CApplication::CApplication(void) :
    QMainWindow            (NULL),
    m_uiWidget             (new Ui::TMediaPlayer()),
    m_soundSystem          (NULL),
    m_playListView         (NULL),
    m_settings             (NULL),
    m_timer                (NULL),
    m_listInfos            (NULL),
    m_currentSongItem      (NULL),
    m_currentSongTable     (NULL),
    m_library              (NULL),
    m_displayedSongTable   (NULL),
    m_state                (Stopped),
    m_isRepeat             (false),
    m_isShuffle            (false),
    m_isMute               (false),
    m_volume               (50),

    // Last.fm
    m_lastFmEnableScrobble       (false),
    m_delayBeforeNotification    (5000),
    m_percentageBeforeScrobbling (60),
    m_timerLastFm                (NULL),
    m_lastFmSessionRequest       (0),
    m_lastFmAPIKey               ("20478fcc23bae9e1e2396a2b1cc52338"),
    m_lastFMSecret               ("b2ed8ec840ec1995003bb99fb02ace44"),
    m_lastFmTimeListened         (0),
    m_lastFmLastPosition         (0),
    m_lastFmState                (NoScrobble)
{
    // Chargement des paramètres de l'application
    m_settings = new QSettings("Ted", "TMediaPlayer", this);

    // Initialisation de l'interface graphique
    m_uiWidget->setupUi(this);

    restoreGeometry(m_settings->value("Window/WindowGeometry").toByteArray());
    restoreState(m_settings->value("Window/WindowState").toByteArray());

    // Last.fm
    m_lastFmEnableScrobble = m_settings->value("LastFm/EnableScrobble", false).toBool();
    m_delayBeforeNotification = m_settings->value("LastFm/DelayBeforeNotification", 5000).toInt();
    m_percentageBeforeScrobbling = m_settings->value("LastFm/PercentageBeforeScrobbling", 60).toInt();
    m_lastFmKey = m_settings->value("LastFm/SessionKey", "").toByteArray();

    // Paramètres de lecture
    setVolume(m_settings->value("Preferences/Volume", 50).toInt());
    setShuffle(m_settings->value("Preferences/Shuffle", false).toBool());
    setRepeat(m_settings->value("Preferences/Repeat", false).toBool());

    // Barre d'état
    QTime duration(0, 0);
    m_listInfos = new QLabel(tr("%n song(s), %1", "", 0).arg(duration.toString()));
    statusBar()->addPermanentWidget(m_listInfos);

    m_uiWidget->btnStop->setVisible(m_settings->value("Preferences/ShowButtonStop", true).toBool());


    // Connexions des signaux et des slots
    connect(m_uiWidget->songInfos, SIGNAL(clicked()), this, SLOT(selectCurrentSong()));

    // Boutons
    connect(m_uiWidget->btnPlay, SIGNAL(clicked()), this, SLOT(togglePlay()));
    connect(m_uiWidget->btnStop, SIGNAL(clicked()), this, SLOT(stop()));

    connect(m_uiWidget->btnPrevious, SIGNAL(clicked()), this, SLOT(previousSong()));
    connect(m_uiWidget->btnNext, SIGNAL(clicked()), this, SLOT(nextSong()));

    connect(m_uiWidget->btnRepeat, SIGNAL(toggled(bool)), this, SLOT(setRepeat(bool)));
    connect(m_uiWidget->btnShuffle, SIGNAL(toggled(bool)), this, SLOT(setShuffle(bool)));
    connect(m_uiWidget->btnMute, SIGNAL(clicked()), this, SLOT(toggleMute()));

    // Sliders
    connect(m_uiWidget->sliderVolume, SIGNAL(sliderMoved(int)), this, SLOT(setVolume(int)));
    connect(m_uiWidget->sliderPosition, SIGNAL(sliderReleased()), this, SLOT(updatePosition()));

    // Menus
    connect(m_uiWidget->actionNewPlayList, SIGNAL(triggered()), this, SLOT(openDialogAddStaticPlayList()));
    connect(m_uiWidget->actionNewDynamicPlayList, SIGNAL(triggered()), this, SLOT(openDialogAddDynamicList()));
    connect(m_uiWidget->actionAddFiles, SIGNAL(triggered()), this, SLOT(openDialogAddSongs()));
    connect(m_uiWidget->actionAddFolder, SIGNAL(triggered()), this, SLOT(openDialogAddFolder()));
    connect(m_uiWidget->actionInformations, SIGNAL(triggered()), this, SLOT(openDialogSongInfos()));
    connect(m_uiWidget->actionOpenInExplorer, SIGNAL(triggered()), this, SLOT(openSongInExplorer()));

    connect(m_uiWidget->actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(m_uiWidget->actionSelectNone, SIGNAL(triggered()), this, SLOT(selectNone()));
    connect(m_uiWidget->actionPreferences, SIGNAL(triggered()), this, SLOT(openDialogPreferences()));

    connect(m_uiWidget->actionPlay, SIGNAL(triggered()), this, SLOT(play()));
    connect(m_uiWidget->actionPause, SIGNAL(triggered()), this, SLOT(pause()));
    connect(m_uiWidget->actionStop, SIGNAL(triggered()), this, SLOT(stop()));

    connect(m_uiWidget->actionPrevious, SIGNAL(triggered()), this, SLOT(previousSong()));
    connect(m_uiWidget->actionNext, SIGNAL(triggered()), this, SLOT(nextSong()));

    connect(m_uiWidget->actionRepeat, SIGNAL(triggered(bool)), this, SLOT(setRepeat(bool)));
    connect(m_uiWidget->actionShuffle, SIGNAL(triggered(bool)), this, SLOT(setShuffle(bool)));

    connect(m_uiWidget->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));


    connect(this, SIGNAL(songPlayStart(CSong *)), this, SLOT(updateSongDescription(CSong *)));
}


/**
 * Libère les ressources utilisées par l'application.
 */

CApplication::~CApplication()
{
    qDebug() << "CApplication::~CApplication()";

    if (m_timerLastFm)
    {
        m_timerLastFm->stop();
        delete m_timerLastFm;
    }

    if (m_timer)
    {
        m_timer->stop();
        delete m_timer;
    }

    // Largeur de la vue pour les listes de lecture
    int treeWidth = m_uiWidget->splitter->sizes()[0];
    m_settings->setValue("Window/PlayListViewWidth", treeWidth);

    // Enregistrement des paramètres
    m_settings->setValue("Preferences/Volume", m_volume);
    m_settings->setValue("Preferences/Shuffle", m_isShuffle);
    m_settings->setValue("Preferences/Repeat", m_isRepeat);

    //dumpObjectTree();

    foreach (CListFolder * folder, m_folders)
    {
        delete folder;
    }

    foreach (CPlayList * playList, m_playLists)
    {
        playList->updateDatabase();
        delete playList;
    }

    m_library->updateDatabase();
    m_library->deleteSongs();
    delete m_library;

    m_dataBase.close();

    m_soundSystem->release();
}


/**
 * Initialise l'interface graphique et charge les données.
 */

void CApplication::initWindow(void)
{
    static bool init = false;

    if (init)
    {
        qWarning() << "CApplication::initWindow() : l'application a déjà été initialisée";
        return;
    }

    m_playListView = new CPlayListView(this);
    m_uiWidget->splitter->addWidget(m_playListView);

    int treeWidth = m_settings->value("Window/PlayListViewWidth", 200).toInt();
    m_uiWidget->splitter->setSizes(QList<int>() << treeWidth);

    //m_listModel = new QStandardItemModel(this);
    //m_playListView->setModel(m_listModel);
    connect(m_playListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectPlayListFromTreeView(const QModelIndex&)));


    // Initialisation de FMOD
    if (!initSoundSystem())
    {
        QMessageBox::critical(this, QString(), tr("Failed to init sound system with FMOD."));
        QCoreApplication::exit();
    }


    // Chargement de la base de données
    m_dataBase = QSqlDatabase::addDatabase("QSQLITE");

    QString dbHostName = m_settings->value("Database/Host", QString("localhost")).toString();
    QString dbBaseName = m_settings->value("Database/Base", QString("library.sqlite")).toString();
    QString dbUserName = m_settings->value("Database/UserName", QString("root")).toString();
    QString dbPassword = m_settings->value("Database/Password", QString("")).toString();

    m_dataBase.setHostName(dbHostName);
    m_dataBase.setDatabaseName(dbBaseName);
    m_dataBase.setUserName(dbUserName);
    m_dataBase.setPassword(dbPassword);

    m_settings->setValue("Database/Host", dbHostName);
    m_settings->setValue("Database/Base", dbBaseName);
    m_settings->setValue("Database/UserName", dbUserName);
    m_settings->setValue("Database/Password", dbPassword);

    if (!m_dataBase.open())
    {
        QMessageBox::critical(this, QString(), tr("Failed to load database."));
        QCoreApplication::exit();
    }

    loadDatabase();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(400);

    displaySongTable(m_library);
    updateSongDescription(NULL);

    init = true;
}


void CApplication::showDatabaseError(const QString& msg, const QString& query, const QString& fileName, int line)
{

#ifdef _DEBUG
    QMessageBox::warning(this, tr("Database error"), tr("File: %1 (%2)\n\nQuery: %3\n\nError: %4").arg(fileName).arg(line).arg(query).arg(msg));
#endif

    qWarning() << "Database error (in file" << fileName << ", line" << line << "):" << msg;
    qWarning() << "  Query:" << query;
}


void CApplication::setRowHeight(int height)
{
    height = qBound(15, height, 50);
    m_settings->setValue("Preferences/RowHeight", height);

    // Mise à jour des vues
    m_library->verticalHeader()->setDefaultSectionSize(height);

    foreach (CPlayList * playList, getAllPlayLists())
    {
        playList->verticalHeader()->setDefaultSectionSize(height);
    }
}


int CApplication::getRowHeight(void) const
{
    return m_settings->value("Preferences/RowHeight", 19).toInt();
}


void CApplication::showButtonStop(bool show)
{
    m_settings->setValue("Preferences/ShowButtonStop", show);
    m_uiWidget->btnStop->setVisible(show);
}


void CApplication::enableScrobbling(bool enable)
{
    m_lastFmEnableScrobble = true;
    m_settings->setValue("LastFm/EnableScrobble", true);
}


void CApplication::setDelayBeforeNotification(int delay)
{
    delay = qBound(2000, delay, 20000);
    m_delayBeforeNotification = delay;
    m_settings->setValue("LastFm/DelayBeforeNotification", delay);
}


void CApplication::setPercentageBeforeScrobbling(int percentage)
{
    percentage = qBound(50, percentage, 100);
    m_percentageBeforeScrobbling = percentage;
    m_settings->setValue("LastFm/PercentageBeforeScrobbling", percentage);
}


/**
 * Change la liste de morceaux à afficher.
 * Si \a songTable est invalide, la médiathèque est affichée.
 *
 * \param songTable Liste de morceaux à afficher.
 */

void CApplication::setDisplayedSongTable(CSongTable * songTable)
{
    m_displayedSongTable = (songTable ? songTable : m_library);
}


/**
 * Retourne le pointeur sur un morceau à partir de son identifiant en base de données.
 *
 * \param id Identifiant du morceau.
 * \return Pointeur sur le morceau, ou NULL si l'identifiant est invalide.
 */

CSong * CApplication::getSongFromId(int id) const
{
    if (id <= 0)
    {
        return NULL;
    }

    foreach (CSong * song, m_library->getSongs())
    {
        if (song->getId() == id)
        {
            return song;
        }
    }

    return NULL;
}


/**
 * Retourne la liste des listes de lecture contenant un morceau.
 *
 * \todo Chercher dans les dossiers.
 *
 * \param song Morceau à rechercher.
 * \return Liste des listes de lecture.
 */

QList<CPlayList *> CApplication::getPlayListsWithSong(CSong * song) const
{
    Q_CHECK_PTR(song);

    QList<CPlayList *> playLists;

    foreach (CPlayList * playList, m_playLists)
    {
        if (playList->hasSong(song))
        {
            playLists.append(playList);
        }
    }

    return playLists;
}


/**
 * Retourne la liste des listes de lecture de la médiathèque.
 *
 * \todo Parcourir récursivement les dossiers.
 *
 * \return Listes de lecture.
 */

QList<CPlayList *> CApplication::getAllPlayLists(void) const
{
    QList<CPlayList *> playLists;

    foreach (CListFolder * folder, m_folders)
    {
        //...
    }

    foreach (CPlayList * playList, m_playLists)
    {
        playLists.append(playList);
    }

    return playLists;
}


/**
 * Indique si la répétition est active.
 *
 * \todo Définir le fonctionnement de la répétition.
 *
 * \return Booléen.
 */

bool CApplication::isRepeat(void) const
{
    return m_isRepeat;
}


/**
 * Indique si la lecture aléatoire est active.
 *
 * \return Booléen.
 */

bool CApplication::isShuffle(void) const
{
    return m_isShuffle;
}


/**
 * Indique si le son est coupé.
 *
 * \return Booléen.
 */

bool CApplication::isMute(void) const
{
    return m_isMute;
}


/**
 * Donne le volume sonore.
 *
 * \return Volume sonore, entre 0 et 100.
 */

int CApplication::getVolume(void) const
{
    return m_volume;
}


/**
 * Donne la position de lecture.
 *
 * \return Position de lecture, ou 0 si aucun morceau n'est en cours de lecture.
 */

int CApplication::getPosition(void) const
{
    return (m_currentSongItem ? m_currentSongItem->getSong()->getPosition() : 0);
}


/**
 * Récupère l'identifiant d'un artiste en base de données.
 *
 * \param name     Nom de l'artiste.
 * \param nameSort Nom de l'artiste pour le tri.
 * \return Identifiant de l'artiste, ou -1 en cas d'erreur.
 */

int CApplication::getArtistId(const QString& name, const QString& nameSort)
{
    Q_ASSERT(!name.isNull());
    Q_ASSERT(!nameSort.isNull());

    QSqlQuery query(m_dataBase);
    query.prepare("SELECT artist_id FROM artist WHERE artist_name = ? AND artist_name_sort = ?");
    query.bindValue(0, name);
    query.bindValue(1, nameSort);

    if (!query.exec())
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (query.next())
    {
        return query.value(0).toInt();
    }

    query.prepare("INSERT INTO artist (artist_name, artist_name_sort) VALUES (?, ?)");
    query.bindValue(0, name);
    query.bindValue(1, nameSort);

    if (!query.exec())
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    return query.lastInsertId().toInt();
}


/**
 * Récupère l'identifiant d'un album en base de données.
 *
 * \param title     Titre de l'album.
 * \param titleSort Titre de l'album pour le tri.
 * \return Identifiant de l'album, ou -1 en cas d'erreur.
 */

int CApplication::getAlbumId(const QString& title, const QString& titleSort)
{
    Q_ASSERT(!title.isNull());
    Q_ASSERT(!titleSort.isNull());

    QSqlQuery query(m_dataBase);
    query.prepare("SELECT album_id FROM album WHERE album_title = ? AND album_title_sort = ?");
    query.bindValue(0, title);
    query.bindValue(1, titleSort);

    if (!query.exec())
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (query.next())
    {
        return query.value(0).toInt();
    }

    query.prepare("INSERT INTO album (album_title, album_title_sort) VALUES (?, ?)");
    query.bindValue(0, title);
    query.bindValue(1, titleSort);

    if (!query.exec())
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    return query.lastInsertId().toInt();
}


/**
 * Récupère l'identifiant d'un genre en base de données.
 *
 * \param name Nom du genre.
 * \return Identifiant du genre, ou -1 en cas d'erreur.
 */

int CApplication::getGenreId(const QString& name)
{
    Q_ASSERT(!name.isNull());

    QSqlQuery query(m_dataBase);
    query.prepare("SELECT genre_id FROM genre WHERE genre_name = ?");
    query.bindValue(0, name);

    if (!query.exec())
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (query.next())
    {
        return query.value(0).toInt();
    }

    query.prepare("INSERT INTO genre (genre_name) VALUES (?)");
    query.bindValue(0, name);

    if (!query.exec())
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    return query.lastInsertId().toInt();
}


/**
 * Lance le processus d'authentication avec Last.fm.
 * Le navigateur doit s'ouvrir pour que l'utilisateur puisse se connecter.
 */

void CApplication::connectToLastFm(void)
{
qDebug() << "CApplication::connectToLastFm()";

    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyLastFmGetToken(QNetworkReply *)));

    QMap<QByteArray, QByteArray> args;
    args["method"]  = m_lastFmAPIKey;
    args["api_key"] = "auth.getToken";
    QByteArray signature = getLastFmSignature(args);

    manager->get(QNetworkRequest(QUrl(QString("http://ws.audioscrobbler.com/2.0/?method=auth.gettoken&api_key=%1&api_sig=%2").arg(QString(m_lastFmAPIKey)).arg(QString(signature)))));
}


void CApplication::replyLastFmGetToken(QNetworkReply * reply)
{
qDebug() << "CApplication::replyLastFmGetToken()";
    Q_CHECK_PTR(reply);

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "CApplication::replyLastFmGetToken() : erreur HTTP avec Last.fm (" << reply->error() << ")";
        //return;
    }

    QByteArray data = reply->readAll();
    logLastFmResponse(reply->error(), data);

    QDomDocument doc;
    
    QString error;
    if (!doc.setContent(data, &error))
    {
        qWarning() << "CApplication::replyLastFmGetToken() : document XML invalide (" << error << ")";
        return;
    }

    QDomElement racine = doc.documentElement();

    if (racine.tagName() != "lfm")
    {
        qWarning() << "CApplication::replyLastFmGetToken() : réponse XML incorrecte (élément 'lfm' attendu)";
        return;
    }

    if (racine.attribute("status", "failed") == "failed")
    {
        qWarning() << "CApplication::replyLastFmGetToken() : la requête Last.fm a echouée";
        return;
    }

    racine = racine.firstChildElement();

    if (racine.tagName() != "token")
    {
        qWarning() << "CApplication::replyLastFmGetToken() : réponse XML incorrecte (élément 'token' attendu)";
        return;
    }
    
    m_lastFmToken = racine.text().toLatin1();
qDebug() << "CApplication::replyLastFmGetToken() : token = " << m_lastFmToken;

    // Ouverture du navigateur
    QDesktopServices::openUrl(QUrl(QString("http://www.last.fm/api/auth/?api_key=%1&token=%2").arg(QString(m_lastFmAPIKey)).arg(QString(m_lastFmToken))));

    if (m_timerLastFm)
    {
        m_timerLastFm->stop();
        delete m_timerLastFm;
    }

    m_timerLastFm = new QTimer(this);
    connect(m_timerLastFm, SIGNAL(timeout()), this, SLOT(getLastFmSession()));
    m_timerLastFm->start(5000);

    reply->deleteLater();
}


void CApplication::getLastFmSession(void)
{
    Q_CHECK_PTR(m_timerLastFm);

    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyLastFmFinished(QNetworkReply *)));

    QMap<QByteArray, QByteArray> args;
    args["method"]  = m_lastFmAPIKey;
    args["api_key"] = "auth.getSession";
    args["token"]  = m_lastFmToken;
    QByteArray signature = getLastFmSignature(args);

    QString url = QString("http://ws.audioscrobbler.com/2.0/?method=auth.getSession&api_key=%1&token=%2&api_sig=%3").arg(QString(m_lastFmAPIKey)).arg(QString(m_lastFmToken)).arg(QString(signature));
    qDebug() << url;
    manager->get(QNetworkRequest(QUrl(url)));

    if (++m_lastFmSessionRequest >= 10)
    {
        m_timerLastFm->stop();
        delete m_timerLastFm;
        m_timerLastFm = NULL;
        m_lastFmSessionRequest = 0;
        return;
    }
}


void CApplication::replyLastFmFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);
    Q_CHECK_PTR(m_timerLastFm);

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "CApplication::replyLastFmFinished() : erreur HTTP avec Last.fm (" << reply->error() << ")";
        //return;
    }

    QByteArray data = reply->readAll();
    logLastFmResponse(reply->error(), data);

    QDomDocument doc;
    
    QString error;
    if (!doc.setContent(data, &error))
    {
        qWarning() << "CApplication::replyLastFmFinished() : document XML invalide (" << error << ")";
        return;
    }

    QDomElement racine = doc.documentElement();

    if (racine.tagName() != "lfm")
    {
        qWarning() << "CApplication::replyLastFmFinished() : réponse XML incorrecte (élément 'lfm' attendu)";
        return;
    }

    if (racine.attribute("status", "failed") == "failed")
    {
        qWarning() << "CApplication::replyLastFmFinished() : la requête Last.fm a echouée";
        return;
    }
    
    racine = racine.firstChildElement();

    if (racine.tagName() != "session")
    {
        qWarning() << "CApplication::replyLastFmGetToken() : réponse XML incorrecte (élément 'session' attendu)";
        return;
    }

    racine = racine.firstChildElement();
    racine = racine.nextSiblingElement("key");

    if (racine.isNull())
    {
        qWarning() << "CApplication::replyLastFmFinished() : réponse XML incorrecte (élément key attendu)";
        return;
    }

    m_lastFmKey = racine.text().toLatin1();
    qDebug() << "CApplication::replyLastFmFinished() : key = " << m_lastFmKey;

    // Enregistrement de la clé
    m_settings->setValue("LastFm/SessionKey", m_lastFmKey);

    reply->deleteLater();

    m_timerLastFm->stop();
    delete m_timerLastFm;
    m_timerLastFm = NULL;
    m_lastFmSessionRequest = 0;
}


void CApplication::replyLastFmUpdateNowPlaying(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "CApplication::replyLastFmUpdateNowPlaying() : erreur HTTP avec Last.fm (" << reply->error() << ")";
        //return;
    }

    QByteArray data = reply->readAll();
    logLastFmResponse(reply->error(), data);

    reply->deleteLater();
}


void CApplication::selectAll(void)
{
    Q_CHECK_PTR(m_displayedSongTable);
    m_displayedSongTable->selectAll();
}


void CApplication::selectNone(void)
{
    Q_CHECK_PTR(m_displayedSongTable);
    m_displayedSongTable->clearSelection();
}


/**
 * Démarre la lecture du morceau sélectionné.
 */

void CApplication::play(void)
{
    qDebug() << "CApplication::play()";

    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        if (m_state == Paused)
        {
            qDebug() << "CApplication::play() : chanson en pause";
            m_uiWidget->btnPlay->setIcon(QPixmap(":/icons/pause"));
            emit songResumed(m_currentSongItem->getSong());
            m_currentSongItem->getSong()->play();
        }

        m_state = Playing;
    }
    else
    {
        m_state = Stopped;

        qDebug() << "CApplication::play() : lancement du premier morceau";
        m_currentSongTable = m_displayedSongTable;

        // Recherche du morceau sélectionné
        m_currentSongItem = m_currentSongTable->getSelectedSongItem();

        // Lecture du premier morceau de la liste
        if (!m_currentSongItem)
        {
            m_currentSongItem = m_currentSongTable->getNextSong(NULL, m_isShuffle);
        }

        if (!m_currentSongItem)
        {
            m_currentSongTable = NULL;
            return;
        }

        if (m_currentSongItem->getSong()->loadSound())
        {
            startPlay();
        }
        else
        {
            nextSong();
        }
    }
}


/**
 * Arrête la lecture.
 */

void CApplication::stop(void)
{
    if (m_currentSongItem)
    {
        m_currentSongItem->getSong()->stop();
        emit songStopped(m_currentSongItem->getSong());
        updateSongDescription(NULL);
        m_currentSongItem = NULL;
    }

    m_currentSongTable = NULL;
    m_state = Stopped;
    m_uiWidget->btnPlay->setIcon(QPixmap(":/icons/play"));

    m_lastFmTimeListened = 0;
    m_lastFmState = NoScrobble;
}


/**
 * Met la lecture en pause.
 */

void CApplication::pause(void)
{
    if (m_currentSongItem)
    {
        m_uiWidget->btnPlay->setIcon(QPixmap(":/icons/play"));
        m_currentSongItem->getSong()->pause();
        emit songPaused(m_currentSongItem->getSong());
        m_state = Paused;
    }
}


void CApplication::togglePlay(void)
{
    if (m_state != Playing)
    {
        play();
    }
    else
    {
        pause();
    }
}


/**
 * Active le morceau précédent du morceau actuel.
 * Si le morceau actuel est le premier de la liste, ou que la position de lecture est
 * supérieure à 4 secondes, on revient au début du morceau.
 */

void CApplication::previousSong(void)
{
    qDebug() << "CApplication::previousSong()";

    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        int position = m_currentSongItem->getSong()->getPosition();

        m_currentSongItem->getSong()->stop();
        updateSongDescription(NULL);
        m_uiWidget->btnPlay->setIcon(QPixmap(":/icons/play"));

        // Retour au début du morceau
        if (position > 4000)
        {
            if (m_state == Paused)
            {
                startPlay();
                pause();
            }
            else
            {
                startPlay();
            }

            return;
        }

        CSongTableItem * songItem = m_currentSongTable->getPreviousSong(m_currentSongItem, m_isShuffle);

        // Premier morceau de la liste
        if (!songItem)
        {
            //m_currentSongTable = NULL;
            //m_state = Stopped;

            if (m_state == Paused)
            {
                startPlay();
                pause();
            }
            else
            {
                startPlay();
            }

            return;
        }

        m_currentSongItem = songItem;

        if (m_currentSongTable == m_displayedSongTable)
        {
            selectCurrentSong();
        }

        if (m_currentSongItem->getSong()->loadSound())
        {
            if (m_state == Paused)
            {
                startPlay();
                pause();
            }
            else
            {
                startPlay();
            }
        }
        else
        {
            m_currentSongItem = NULL;
            m_currentSongTable = NULL;
            m_state = Stopped;
            return;
        }
/*
        m_currentSongItem = m_currentSongTable->getPreviousSong(m_currentSongItem, m_isShuffle);

        if (m_currentSongItem && m_currentSongItem->getSong()->loadSound())
        {
            play();
        }
        else
        {
            m_currentSongItem = NULL;
            m_currentSongTable = NULL;
            m_state = Stopped;
            updateSongDescription(NULL);
        }
*/
    }
    else
    {
        m_state = Stopped;
    }
}


void CApplication::nextSong(void)
{
    qDebug() << "CApplication::nextSong()";

    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        m_currentSongItem->getSong()->stop();
        updateSongDescription(NULL);
        m_uiWidget->btnPlay->setIcon(QPixmap(":/icons/play"));

        m_currentSongItem = m_currentSongTable->getNextSong(m_currentSongItem, m_isShuffle);

        // Répétition de la liste
        if (!m_currentSongItem && m_isRepeat)
        {
            m_currentSongItem = m_currentSongTable->getNextSong(NULL, m_isShuffle);
        }

        if (!m_currentSongItem)
        {
            m_currentSongTable = NULL;
            m_state = Stopped;
            return;
        }

        if (m_currentSongTable == m_displayedSongTable)
        {
            selectCurrentSong();
        }

        if (m_currentSongItem->getSong()->loadSound())
        {
            if (m_state == Paused)
            {
                startPlay();
                pause();
            }
            else
            {
                startPlay();
            }
        }
        else
        {
            nextSong();
        }
    }
    else
    {
        m_state = Stopped;
    }
}


/**
 * Lance la lecture d'un morceau de la liste actuellement affichée.
 *
 * \param songItem Morceau à lire.
 */

void CApplication::playSong(CSongTableItem * songItem)
{
    Q_CHECK_PTR(songItem);

    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        m_currentSongItem->getSong()->stop();
        updateSongDescription(NULL);
    }

    m_currentSongItem = songItem;
    m_currentSongTable = m_displayedSongTable;

    if (m_currentSongItem->getSong()->loadSound())
    {
        m_uiWidget->btnPlay->setIcon(QPixmap(":/icons/pause"));
        startPlay();
    }
    else
    {
        m_uiWidget->btnPlay->setIcon(QPixmap(":/icons/play"));
        m_currentSongItem = NULL;
        m_currentSongTable = NULL;
    }
}


void CApplication::setRepeat(bool repeat)
{
    if (repeat != m_isRepeat)
    {
        m_isRepeat = repeat;
        m_uiWidget->btnRepeat->setChecked(repeat);
        m_uiWidget->actionRepeat->setChecked(repeat);
    }
}


void CApplication::setShuffle(bool shuffle)
{
    if (shuffle != m_isShuffle)
    {
        m_isShuffle = shuffle;
        m_uiWidget->btnShuffle->setChecked(shuffle);
        m_uiWidget->actionShuffle->setChecked(shuffle);
    }
}


void CApplication::setMute(bool mute)
{
    if (mute != m_isMute)
    {
        m_isMute = mute;

        if (m_currentSongItem)
        {
            m_currentSongItem->getSong()->setMute(mute);
        }

        m_uiWidget->btnMute->setIcon(QPixmap(mute ? ":/icons/muet" : ":/icons/volume"));
    }
}


void CApplication::toggleMute(void)
{
    setMute(!m_isMute);
}


void CApplication::setVolume(int volume)
{
    volume = qBound(0, volume, 100);

    if (volume != m_volume)
    {
        m_volume = volume;

        if (m_currentSongItem)
        {
            m_currentSongItem->getSong()->setVolume(volume);
        }

        m_uiWidget->sliderVolume->setValue(volume);
    }
}


void CApplication::setPosition(int position)
{
    Q_ASSERT(position >= 0);

    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        m_currentSongItem->getSong()->setPosition(position);
        const int songPosition = m_currentSongItem->getSong()->getPosition();

        if (songPosition >= 0)
        {
            // Last.fm
            if (m_lastFmEnableScrobble)
            {
                if (m_state == Playing)
                {
                    m_lastFmTimeListened += (songPosition - m_lastFmLastPosition);
                }

                m_lastFmLastPosition = songPosition;
            }

            m_uiWidget->sliderPosition->setValue(songPosition);

            QTime positionTime(0, 0);
            positionTime = positionTime.addMSecs(songPosition);
            m_uiWidget->lblPosition->setText(positionTime.toString("m:ss"));
        }
    }
}

/*
/// \todo Supprimer
void CApplication::openPlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    m_displayedSongTable = playList;
    //TODO: maj vue
}
*/
/*
void CApplication::renamePlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    //TODO: open dialog rename Playlist
    //TODO: maj DB
}
*/
/*
void CApplication::editDynamicPlayList(CDynamicPlayList * playList)
{
    if (!playList)
    {
        return;
    }

    //TODO: open dialog edit SmartPlaylist
    //TODO: maj DB
}
*/

/**
 * Supprime une liste de lecture.
 * Une boite de dialogue de confirmation est ouverte.
 *
 * \param playList Pointeur sur la liste de lecture à supprimer.
 */
/*
void CApplication::deletePlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    // Confirmation
    if (QMessageBox::question(this, QString(), tr("Are you sure you want to delete this playlist?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    if (m_currentSongTable == playList)
    {
        m_currentSongTable = NULL;
    }

    if (m_displayedSongTable == playList)
    {
        m_displayedSongTable = m_library;
        //TODO: maj vue
    }

    if (playList->getFolder())
    {
        //TODO: maj vue gauche
        playList->setFolder(NULL);
        delete playList;
        //TODO: maj DB
        return;
    }

    foreach (CPlayList * playListTmp, m_playLists)
    {
        if (playListTmp == playList)
        {
            //TODO: maj vue gauche
            m_playLists.removeOne(playList);
            delete playList;
            //TODO: maj DB
            return;
        }
    }
}
*/
/*
void CApplication::addListFolder(void)
{
    //TODO: open dialog new folder
    //TODO: maj DB
    //TODO: maj vue gauche
}
*/
/*
void CApplication::renameListFolder(CListFolder * folder)
{
    Q_CHECK_PTR(folder);

    //TODO: open dialog rename folder
    //TODO: maj DB
    //TODO: maj vue gauche
}
*/

/**
 * Supprime un dossier de liste de lectures.
 *
 * \param folder Pointeur sur le dossier à supprimer.
 */
/*
void CApplication::deleteListFolder(CListFolder * folder)
{
    Q_CHECK_PTR(folder);

    //TODO: confirmation
    //TODO: maj vue

    foreach (CPlayList * playList, folder->getPlayLists())
    {
        playList->setFolder(nullptr);
        m_playLists.append(playList);
    }

    m_folders.removeOne(folder);
    delete folder;

    //TODO: maj DB
}
*/


/// \todo Implémentation
void CApplication::openDialogPreferences(void)
{
    CDialogPreferences * dialog = new CDialogPreferences(this, m_settings);
    dialog->show();
}


/**
 * Affiche une boite de dialogue pour sélectionner des fichiers à ajouter à la médiathèque.
 */

void CApplication::openDialogAddSongs(void)
{
    QStringList fileList = QFileDialog::getOpenFileNames(this, QString(), QString(), tr("Media files (*.flac *.ogg *.mp3);;MP3 (*.mp3);;FLAC (*.flac);;OGG (*.ogg);;All files (*.*)"));

    foreach (QString fileName, fileList)
    {
        addSong(fileName);
    }
}


/**
 * Affiche une boite de dialogue pour ajouter un dossier à la médiathèque.
 */

void CApplication::openDialogAddFolder(void)
{
    QString folder = QFileDialog::getExistingDirectory(this);

    if (folder.isEmpty())
    {
        return;
    }

    QStringList fileList = addFolder(folder);

    if (!fileList.isEmpty())
    {
        QProgressDialog progress(tr("Add files..."), tr("Abort"), 0, fileList.size(), this);
        int i = 0;

        foreach (QString fileName, fileList)
        {
            progress.setValue(i++);
            addSong(fileName);
            qApp->processEvents();

            if (progress.wasCanceled())
            {
                return;
            }
        }
    }
}


/**
 * Affiche une boite de dialogue pour visualiser et éditer les informations du morceau sélectionné.
 *
 * \todo Afficher toutes les informations.
 * \todo Mettre-à-jour la base de données.
 * \todo Mettre-à-jour les tags du fichier.
 */

void CApplication::openDialogSongInfos(void)
{
    Q_CHECK_PTR(m_displayedSongTable);

    // Liste des morceaux sélectionnés
    QList<CSongTableItem *> songItemList = m_displayedSongTable->getSelectedSongItems();

    if (songItemList.size() > 1)
    {
qDebug() << songItemList;

        CDialogEditSongs * dialog = new CDialogEditSongs(songItemList, this);
        dialog->show();

        //...

        return;
    }

    // Recherche du morceau sélectionné
    CSongTableItem * songItem = m_displayedSongTable->getSelectedSongItem();

    if (songItem)
    {
        CDialogEditSong * dialog = new CDialogEditSong(songItem, m_displayedSongTable);
        dialog->show();

        //...
    }
}


/**
 * Affiche la boite de dialogue pour crée une nouvelle liste de lecture statique.
 *
 * \param folder Pointeur sur le dossier où créer la liste.
 */

void CApplication::openDialogAddStaticPlayList(CListFolder * folder)
{
    qDebug() << "Nouvelle liste de lecture...";

    CDialogEditStaticPlayList * dialog = new CDialogEditStaticPlayList(NULL, this);
    dialog->show();

    //...
}


/**
 * Affiche la boite de dialogue pour crée une nouvelle liste de lecture dynamique.
 *
 * \param folder Pointeur sur le dossier où créer la liste.
 */

void CApplication::openDialogAddDynamicList(CListFolder * folder)
{
    qDebug() << "Nouvelle liste de lecture dynamique...";

    CDialogEditDynamicList * dialog = new CDialogEditDynamicList(NULL, this);
    dialog->show();

    //...
}


/**
 * Ajoute une liste de lecture à la vue.
 *
 * \param playList Pointeur sur la liste de lecture à ajouter.
 */

void CApplication::addPlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    m_playLists.append(playList);

    // Ajout dans le panneau gauche
    m_uiWidget->splitter->addWidget(playList);
    playList->hide();

    connect(playList, SIGNAL(songStarted(CSongTableItem *)), this, SLOT(playSong(CSongTableItem *)));

    // Liste de lecture statique
    CStaticPlayList * staticList = qobject_cast<CStaticPlayList *>(playList);

    if (staticList)
    {
        connect(staticList, SIGNAL(songAdded(CSong *)), this, SLOT(updateListInformations()));
        connect(staticList, SIGNAL(songRemoved(CSong *)), this, SLOT(updateListInformations()));
    }

    // Liste de lecture dynamique
    CDynamicPlayList * dynamicList = qobject_cast<CDynamicPlayList *>(playList);

    if (dynamicList)
    {
        dynamicList->update();
    }

    m_playListView->addSongTable(playList);
}


/**
 * Ajoute une chanson à la médiathèque.
 * Le fichier doit être un son valide, et ne doit pas être déjà présent dans la médiathèque.
 *
 * \param fileName Fichier à charger.
 */

void CApplication::addSong(const QString& fileName)
{
    qDebug() << "Chargement du fichier " << fileName;

    CSong * song = CSong::loadFromFile(this, fileName);
    if (song)
    {
        m_library->addSong(song);
        updateListInformations();
        return;
    }
}


/**
 * Sélectionne le morceau en cours de lecture.
 */

void CApplication::selectCurrentSong(void)
{
    selectSong(m_currentSongTable, m_currentSongItem);
}


/**
 * Sélectionne un morceau dans une liste.
 *
 * \param songTable Liste de morceaux à afficher.
 * \param songItem  Morceau à sélectionner.
 */

void CApplication::selectSong(CSongTable * songTable, CSongTableItem * songItem)
{
    if (!songTable || !songItem)
    {
        return;
    }

    displaySongTable(songTable);
    songTable->selectSongItem(songItem);
    songTable->setFocus(Qt::OtherFocusReason);
}


/**
 * Liste les morceaux contenus dans un répertoire.
 *
 * \param pathName Nom du répertoire à parcourir récursivement.
 * \return Liste des fichiers du répertoire.
 */

QStringList CApplication::addFolder(const QString& pathName)
{
    QStringList fileList;
    QDir dir(pathName);

    foreach (QString fileName, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        fileList.append(addFolder(dir.absoluteFilePath(fileName)));
    }

    foreach (QString fileName, dir.entryList(QDir::Files | QDir::Readable, QDir::Name))
    {
        fileList.append(QDir::toNativeSeparators(dir.absoluteFilePath(fileName)));
    }

    return fileList;
}

/*
void CApplication::editSong(CSongTableItem * songItem)
{
    qDebug() << "CApplication::editSong()";
    if (!songItem)
    {
        return;
    }

    CDialogEditSong * dialog = new CDialogEditSong(songItem, m_displayedSongTable);
    dialog->show();
}
*/
/*
/// \todo Implémentation
void CApplication::removeSong(CSongTableItem * songItem)
{
    qDebug() << "CApplication::removeSong()";

    if (!songItem)
    {
        return;
    }

    //TODO: confirmation

    if (m_currentSongItem == songItem)
    {
        stop();
        m_currentSongItem = NULL;
    }

    //TODO: maj vue

    m_library->removeSong(songItem->getSong());

    foreach (CListFolder * folder, m_folders)
    {
        foreach (CPlayList * playList, folder->getPlayLists())
        {
            CStaticPlayList * staticPlayList = dynamic_cast<CStaticPlayList *>(playList);

            if (staticPlayList)
            {
                staticPlayList->removeSong(songItem->getSong());
            }
        }
    }

    foreach (CPlayList * playList, m_playLists)
    {
        CStaticPlayList * staticPlayList = dynamic_cast<CStaticPlayList *>(playList);

        if (staticPlayList)
        {
            staticPlayList->removeSong(songItem->getSong());
        }
    }

    //TODO: maj DB

    emit songRemoved(songItem->getSong());

    delete songItem->getSong();
}
*/

/**
 * Affiche le morceau sélectionné dans l'explorateur de fichiers.
 */

void CApplication::openSongInExplorer(void)
{
    // Recherche du morceau sélectionné
    CSongTableItem * songItem = m_displayedSongTable->getSelectedSongItem();

    if (!songItem)
    {
        songItem = m_currentSongItem;
    }

    if (songItem)
    {
        QDir songDir(songItem->getSong()->getFileName());
        songDir.cdUp();
        QDesktopServices::openUrl(QUrl::fromLocalFile(songDir.absolutePath()));
    }
}


/**
 * Méthode appelée quand la lecture d'un morceau se termine.
 */

void CApplication::onPlayEnd(void)
{
    if (m_currentSongItem)
    {
        qDebug() << "CApplication::onPlayEnd()";
        emit songPlayEnd(m_currentSongItem->getSong());
        updateSongDescription(NULL);
        //TODO: maj vue
        //m_currentSong->addOnePlay();

        if (m_isRepeat)
        {
            m_currentSongItem->getSong()->setPosition(0);
            play();
        }
        else
        {
            //Q_CHECK_PTR(m_displayedSongTable);

            //m_currentSongIndex = m_displayedSongTable->getNextSong(m_currentSongIndex, m_isShuffle);
            //m_currentSong = m_displayedSongTable->getSongForIndex(m_currentSongIndex);
            nextSong();
        }

        m_library->update();
/*
        if (m_currentSong)
        {
            m_currentSongTable = m_displayedSongTable;
            m_isPlaying = true;

            if (!m_currentSong->loadSound())
            {
                m_currentSong = NULL;
                m_currentSongIndex = -1;
                m_currentSongTable = NULL;

                m_isPlaying = false;
                m_isPaused = false;

                return;
            }

            m_currentSong->setVolume(m_volume);
            m_currentSong->setMute(m_isMute);
            m_currentSong->play();

            emit songPlayStart(m_currentSong);
            connect(m_currentSong, SIGNAL(playEnd()), this, SLOT(onPlayEnd()));
        }
        else
        {
            m_currentSongTable = nullptr;
            m_isPlaying = false;
        }
*/
    }
}


/**
 * Met à jour les informations sur le morceau en cours de lecture.
 *
 * \param song Morceau à utiliser, ou NULL pour n'afficher aucune information.
 */

void CApplication::updateSongDescription(CSong * song)
{
    if (song)
    {
        m_uiWidget->songInfos->setText(song->getTitle() + " - " + song->getArtistName() + " - " + song->getAlbumTitle());
        m_uiWidget->sliderPosition->setEnabled(true);

        const int duration = song->getDuration();
        if (duration >= 0)
        {
            m_uiWidget->sliderPosition->setRange(0, duration);

            QTime durationTime(0, 0);
            durationTime = durationTime.addMSecs(duration);
            m_uiWidget->lblTime->setText(durationTime.toString("m:ss")); /// \todo Stocker dans les settings
        }
    }
    else
    {
        m_uiWidget->songInfos->setText(""); // "Pas de morceau en cours de lecture..."
        m_uiWidget->sliderPosition->setEnabled(false);
        m_uiWidget->sliderPosition->setRange(0, 1000);
        m_uiWidget->lblPosition->setText("0:00");
        m_uiWidget->lblTime->setText("0:00");
    }

    m_uiWidget->sliderPosition->setValue(0);
}


/**
 * Met à jour les informations sur la liste de morceaux affichée.
 */

void  CApplication::updateListInformations(void)
{
    // Barre d'état
    QTime duration(0, 0);
    duration = duration.addMSecs(m_displayedSongTable->getTotalDuration());
    m_listInfos->setText(tr("%n song(s), %1", "", m_displayedSongTable->getNumSongs()).arg(duration.toString()));
}


/**
 * Met à jour la position de lecture depuis la position du curseur.
 */

void CApplication::updatePosition(void)
{
    setPosition(m_uiWidget->sliderPosition->value());
}


/**
 * Méthode appelée régulièrement pour mettre à jour les informations sur la lecture,
 * et pour passer au morceau suivant si nécessaire.
 */

void CApplication::update(void)
{
    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        //qDebug() << "CApplication::update()";
        const int position = m_currentSongItem->getSong()->getPosition();

        if (m_lastFmEnableScrobble && (m_lastFmState == Started || m_lastFmState == Notified) && m_state == Playing)
        {
            //m_lastFmLastPosition
            int elapsedTime = position - m_lastFmLastPosition;
            //qDebug() << "elapsed =" << elapsedTime;
            m_lastFmTimeListened += elapsedTime;
            m_lastFmLastPosition = position;

            //qDebug() << "Last.fm time = " << m_lastFmTimeListened;

            if (m_lastFmState == Started)
            {
                if (m_lastFmTimeListened > m_delayBeforeNotification)
                {
                    qDebug() << "Last.fm : update";
                    updateLastFmNowPlaying(m_currentSongItem->getSong());
                    m_lastFmState = Notified;
                }
            }
            else if (m_lastFmState == Notified)
            {
                if (m_lastFmTimeListened > 4 * 60000 || m_lastFmTimeListened > m_currentSongItem->getSong()->getDuration() * m_percentageBeforeScrobbling / 100)
                {
                    qDebug() << "Last.fm : scrobble";
                    m_lastFmState = Scrobbled;
                }
            }
        }

        if (m_currentSongItem->getSong()->isEnded())
        {
            qDebug() << "m_currentSong->isEnded()";
            m_currentSongItem->getSong()->emitPlayEnd();
            return;
        }

        if (position >= 0)
        {
            if (!m_uiWidget->sliderPosition->isSliderDown())
            {
                m_uiWidget->sliderPosition->setValue(position);
            }

            QTime positionTime(0, 0);
            positionTime = positionTime.addMSecs(position);
            m_uiWidget->lblPosition->setText(positionTime.toString("m:ss"));
        }
    }
}


void CApplication::selectPlayListFromTreeView(const QModelIndex& index)
{
    qDebug() << "Changement de liste...";

    CSongTable * songTable = m_playListView->getSongTable(index);
    displaySongTable(songTable);
}


/**
 * Change la liste de morceaux affichée.
 *
 * \param songTable Liste de morceaux à afficher.
 */

void CApplication::displaySongTable(CSongTable * songTable)
{
    Q_CHECK_PTR(songTable);

    if (songTable != m_displayedSongTable)
    {
        if (m_displayedSongTable)
        {
            m_displayedSongTable->hide();
        }

        m_playListView->selectionModel()->clearSelection();
        m_playListView->selectionModel()->setCurrentIndex(m_playListView->getModelIndex(songTable), QItemSelectionModel::Select | QItemSelectionModel::Rows);

        m_displayedSongTable = songTable;
        m_displayedSongTable->show();

        updateListInformations();
    }
}


/**
 * Initialise FMOD.
 *
 * \return Booléen indiquant le succès ou l'échec du chargement.
 */

bool CApplication::initSoundSystem(void)
{
    FMOD_RESULT res;

    res = FMOD::System_Create(&m_soundSystem);
    if (res != FMOD_OK) return false;

    unsigned int version;
    res = m_soundSystem->getVersion(&version);
    if (res != FMOD_OK) return false;

    if (version < FMOD_VERSION)
    {
        QMessageBox::critical(this, QString(), tr("This program requires FMOD %1 or superior.").arg(FMOD_VERSION));
        return false;
    }

    int numDrivers;
    res = m_soundSystem->getNumDrivers(&numDrivers);
    if (res != FMOD_OK) return false;

    if (numDrivers == 0)
    {
        res = m_soundSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        if (res != FMOD_OK) return false;
    }
    else
    {
        FMOD_CAPS caps;
        FMOD_SPEAKERMODE speakermode;
        res = m_soundSystem->getDriverCaps(0, &caps, NULL, &speakermode);
        if (res != FMOD_OK) return false;

        // Set the user selected speaker mode
        res = m_soundSystem->setSpeakerMode(speakermode);
        if (res != FMOD_OK) return false;

        if (caps & FMOD_CAPS_HARDWARE_EMULATED)
        {
            res = m_soundSystem->setDSPBufferSize(1024, 10);
            if (res != FMOD_OK) return false;
        }

        char name[256] = "";
        res = m_soundSystem->getDriverInfo(0, name, 256, 0);
        if (res != FMOD_OK) return false;

        if (strstr(name, "SigmaTel"))
        {
            res = m_soundSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
            if (res != FMOD_OK) return false;
        }
    }

    res = m_soundSystem->init(100, FMOD_INIT_NORMAL, 0);
    if (res == FMOD_ERR_OUTPUT_CREATEBUFFER)
    {
        res = m_soundSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        if (res != FMOD_OK) return false;
        res = m_soundSystem->init(2, FMOD_INIT_NORMAL, 0);
    }

    return (res == FMOD_OK);
}


/**
 * Charge la base de données.
 *
 * \todo Utiliser des méthodes dédiées dans les classes CSongTable et CListFolder.
 * \todo Charger les dossiers.
 */

void CApplication::loadDatabase(void)
{
    QSqlQuery query(m_dataBase);


    // Création de la médiathèque
    m_library = new CSongTable(this);
    m_library->m_idPlayList = 0;
    m_uiWidget->splitter->addWidget(m_library);
    connect(m_library, SIGNAL(songStarted(CSongTableItem *)), this, SLOT(playSong(CSongTableItem *)));

    if (!query.exec("SELECT list_columns FROM playlist WHERE playlist_id = 0"))
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    if (query.next())
    {
        m_library->m_idPlayList = 0;
        m_library->initColumns(query.value(0).toString());
    }

    m_playListView->setCurrentIndex(m_playListView->addSongTable(m_library));


    // Liste des morceaux
    QList<CSong *> songList = CSong::loadAllSongsFromDatabase(this);
    m_library->addSongsToTable(songList);


    // Création des dossiers
    if (!query.exec("SELECT folder_id, folder_name FROM folder ORDER BY folder_position"))
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    while (query.next())
    {
        //CListFolder * folder = new CListFolder();
        //folder->m_id   = query.value(0).toInt();
        //folder->m_name = query.value(1).toString();
        //...
    }


    // Création des listes de lecture statiques
    if (!query.exec("SELECT static_list_id, playlist_name, list_columns, playlist_id "
                    "FROM static_list NATURAL JOIN playlist ORDER BY list_position"))
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    while (query.next())
    {
        CStaticPlayList * playList = new CStaticPlayList(this, query.value(1).toString());
        playList->m_id = query.value(0).toInt();
        playList->m_idPlayList = query.value(3).toInt();
        playList->initColumns(query.value(2).toString());

        // Liste des morceaux de la liste de lecture
        QSqlQuery query2(m_dataBase);
        query2.prepare("SELECT song_id, song_position FROM static_list_song "
                       "WHERE static_list_id = ? ORDER BY song_position");
        query2.bindValue(0, playList->m_id);

        if (!query2.exec())
        {
            showDatabaseError(query2.lastError().text(), query2.lastQuery(), __FILE__, __LINE__);
            delete playList;
            continue;
        }

        while (query2.next())
        {
            CSong * song = getSongFromId(query2.value(0).toInt());

            if (song)
            {
                playList->addSongToTable(song, query2.value(1).toInt());
            }
        }

        addPlayList(playList);
    }


    // Création des listes de lecture dynamiques
    if (!query.exec("SELECT dynamic_list_id, playlist_name, list_columns, playlist_id "
                    "FROM dynamic_list NATURAL JOIN playlist ORDER BY list_position"))
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    while (query.next())
    {
        CDynamicPlayList * playList = new CDynamicPlayList(this, query.value(1).toString());
        playList->m_id = query.value(0).toInt();
        playList->m_idPlayList = query.value(3).toInt();
        playList->initColumns(query.value(2).toString());

        playList->loadFromDatabase();

        addPlayList(playList);
    }
}


void CApplication::startPlay(void)
{
    qDebug() << "CApplication::startPlay()";

    Q_CHECK_PTR(m_currentSongItem);
    Q_CHECK_PTR(m_currentSongTable);

    m_uiWidget->btnPlay->setIcon(QPixmap(":/icons/pause"));
    m_currentSongItem->getSong()->play();
    emit songPlayStart(m_currentSongItem->getSong());
    connect(m_currentSongItem->getSong(), SIGNAL(playEnd()), this, SLOT(onPlayEnd()), Qt::UniqueConnection);

    updateSongDescription(m_currentSongItem->getSong());

    m_state = Playing;

    // Last.fm
    if (m_lastFmEnableScrobble)
    {
        m_lastFmTimeListened = 0;
        m_lastFmLastPosition = 0;

        if (m_currentSongItem->getSong()->getDuration() >= 30000)
        {
            m_lastFmState = Started;
        }
        else
        {
            m_lastFmState = NoScrobble;
        }
    }
}


void CApplication::keyPressEvent(QKeyEvent * event)
{
    qDebug() << event->key();
    event->accept();
}


/**
 * Méthode appelée lors de la fermeture de la fenêtre.
 * Sauvegarde l'état de la fenêtre.
 *
 * \param event Évènement de fermeture.
 */

void CApplication::closeEvent(QCloseEvent * event)
{
    Q_CHECK_PTR(event);
    Q_CHECK_PTR(m_settings);

    m_settings->setValue("Window/WindowGeometry", saveGeometry());
    m_settings->setValue("Window/WindowState", saveState());

    QMainWindow::closeEvent(event);
}


/// \todo Implémentation
void CApplication::scrobbleLastFm(CSong * song)
{
    Q_CHECK_PTR(song);

    //...
}


void CApplication::updateLastFmNowPlaying(CSong * song)
{
    Q_CHECK_PTR(song);

    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyLastFmUpdateNowPlaying(QNetworkReply *)));

    // Arguments de la requête
    QMap<QByteArray, QByteArray> args;
    
    args["method"]   = "track.updateNowPlaying";
    args["artist"]   = song->getArtistName().toUtf8();
    args["track"]    = song->getTitle().toUtf8();
    args["api_key"]  = m_lastFmAPIKey;
    args["duration"] = QString::number(song->getDuration() / 1000).toUtf8();
    args["sk"]       = m_lastFmKey;

    QByteArray albumTitle = song->getAlbumTitle().toUtf8();

    if (!albumTitle.isEmpty())
    {
        args["album"] = albumTitle;
        QByteArray albumArtist = song->getAlbumArtist().toUtf8();

        if (!albumArtist.isEmpty() && albumArtist != args["artist"])
        {
            args["albumArtist"] = albumArtist;
        }
    }

    if (song->getTrackNumber() > 0)
    {
        args["trackNumber"] = QString::number(song->getTrackNumber()).toUtf8();
    }

    QByteArray content = getLastFmQuery(args);
    logLastFmRequest("http://ws.audioscrobbler.com/2.0/", content);

    QNetworkRequest request(QUrl("http://ws.audioscrobbler.com/2.0/"));
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    manager->post(request, content);
}


void CApplication::logLastFmRequest(const QString& url, const QString& content)
{
    qDebug() << "--------------------------------------------------";
    qDebug() << QDateTime::currentDateTime();
    qDebug() << url;
    qDebug() << content;
    qDebug() << "--------------------------------------------------";
}


void CApplication::logLastFmResponse(int code, const QString& content)
{
    qDebug() << "--------------------------------------------------";
    qDebug() << QDateTime::currentDateTime();
    qDebug() << code;
    qDebug() << content;
    qDebug() << "--------------------------------------------------";
}


QByteArray CApplication::getLastFmQuery(const QMap<QByteArray, QByteArray>& args) const
{
    QByteArray content;

    for (QMap<QByteArray, QByteArray>::const_iterator it = args.begin(); it != args.end(); ++it)
    {
        if (it != args.begin())
        {
            content.append("&");
        }

        content.append(it.key());
        content.append("=");
        content.append(encodeString(it.value()));
    }

    content.append("&api_sig=");
    content.append(getLastFmSignature(args));

    return content;
}


/**
 * Calcule la signature d'une méthode pour envoyer une requête à Last.fm.
 *
 * \param args       Tableau associatif des arguments (de la forme clé => valeur), avec la méthode.
 * \param methodName Nom de la méthode.
 */

QByteArray CApplication::getLastFmSignature(const QMap<QByteArray, QByteArray>& args) const
{
    QCryptographicHash crypto(QCryptographicHash::Md5);

    for (QMap<QByteArray, QByteArray>::const_iterator it = args.begin(); it != args.end(); ++it)
    {
        crypto.addData(it.key());
        crypto.addData(it.value());
    }

    crypto.addData(m_lastFMSecret);
    return crypto.result().toHex();
}


QByteArray CApplication::encodeString(const QByteArray& str)
{
    QByteArray res;

    // Encodage de la chaine
    for (int i = 0; i < str.size(); ++i)
    {
        if (str[i] == '&')
        {
            res.append("%26");
        }
        else if (str[i] == '=')
        {
            res.append("%3D");
        }
        else
        {
            res.append(str[i]);
        }
    }

    return res;
}

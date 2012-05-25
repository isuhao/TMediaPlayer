
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
#include <QUrl>

// FMOD
#include <fmod/fmod.hpp>

// TagLib
#include <fileref.h>
#include <tag.h>
#include <flacfile.h>
#include <xiphcomment.h>
#include <tmap.h>

// DEBUG
#include <QtDebug>
#include <QSqlDriver>


CApplication::CApplication(void) :
    QMainWindow          (NULL),
    m_uiWidget           (new Ui::TMediaPlayer()),
    m_soundSystem        (NULL),
    m_playListView       (NULL),
    m_settings           (NULL),
    m_timer              (NULL),
    m_currentSongItem    (NULL),
    m_currentSongTable   (NULL),
    m_library            (NULL),
    m_displayedSongTable (NULL),
    m_state              (Stopped),
    m_isRepeat           (false),
    m_isShuffle          (false),
    m_isMute             (false),
    m_volume             (50)
{
    // Chargement des paramètres de l'application
    m_settings = new QSettings("Ted", "TMediaPlayer", this);


    // Initialisation de l'interface graphique
    m_uiWidget->setupUi(this);

    restoreGeometry(m_settings->value("Window/WindowGeometry").toByteArray());
    restoreState(m_settings->value("Window/WindowState").toByteArray());


    // Connexions des signaux et des slots

    // Boutons
    connect(m_uiWidget->btnPlay, SIGNAL(clicked()), this, SLOT(togglePlay()));
    //connect(m_uiWidget->btnPause, SIGNAL(clicked()), this, SLOT(pause()));
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
    //connect(m_uiWidget->actionPreferences, SIGNAL(triggered()), this, SLOT(openDialogPreferences()));

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

    m_timer->stop();

    // Largeur de la vue pour les listes de lecture
    int treeWidth = m_uiWidget->splitter->sizes()[0];
    m_settings->setValue("Window/PlayListViewWidth", treeWidth);

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


void CApplication::initWindow(void)
{
    static bool init = false;

    if (!init)
    {
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


QList<CPlayList *> CApplication::getAllPlayLists(void) const
{
    QList<CPlayList *> playLists;

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
        QString error = query.lastError().text();
        QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
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
        QString error = query.lastError().text();
        QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
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
        QString error = query.lastError().text();
        QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
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
        QString error = query.lastError().text();
        QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
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
        QString error = query.lastError().text();
        QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
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
        QString error = query.lastError().text();
        QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
        return -1;
    }

    return query.lastInsertId().toInt();
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


void CApplication::previousSong(void)
{
    if (m_currentSongTable)
    {
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

        if (m_currentSongItem->getSong()->loadSound())
        {
            startPlay();
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
    Q_ASSERT(volume >= 0 || volume <= 100);

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
            m_uiWidget->sliderPosition->setValue(songPosition);

            QTime positionTime(0, 0);
            positionTime = positionTime.addMSecs(songPosition);
            m_uiWidget->lblPosition->setText(positionTime.toString("m:ss"));
        }
    }
}


/// \todo Supprimer
void CApplication::openPlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    m_displayedSongTable = playList;
    //TODO: maj vue
}


void CApplication::renamePlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    //TODO: open dialog rename Playlist
    //TODO: maj DB
}


void CApplication::editDynamicPlayList(CDynamicPlayList * playList)
{
    if (!playList)
    {
        return;
    }

    //TODO: open dialog edit SmartPlaylist
    //TODO: maj DB
}


/**
 * Supprime une liste de lecture.
 * Une boite de dialogue de confirmation est ouverte.
 *
 * \param playList Pointeur sur la liste de lecture à supprimer.
 */

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


void CApplication::addListFolder(void)
{
    //TODO: open dialog new folder
    //TODO: maj DB
    //TODO: maj vue gauche
}


void CApplication::renameListFolder(CListFolder * folder)
{
    Q_CHECK_PTR(folder);

    //TODO: open dialog rename folder
    //TODO: maj DB
    //TODO: maj vue gauche
}


/**
 * Supprime un dossier de liste de lectures.
 *
 * \param folder Pointeur sur le dossier à supprimer.
 */

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

    m_uiWidget->splitter->addWidget(playList);
    connect(playList, SIGNAL(songStarted(CSongTableItem *)), this, SLOT(playSong(CSongTableItem *)));
    playList->hide();
/*
    QStandardItem * playListItem = new QStandardItem(playList->getName());

    if (qobject_cast<CDynamicPlayList *>(playList))
    {
        playListItem->setIcon(QPixmap(":/icons/dynamic_list"));
    }
    else
    {
        playListItem->setIcon(QPixmap(":/icons/playlist"));
    }
*/
    //playListItem->setData(QVariant::fromValue(reinterpret_cast<CSongTable *>(playList)));
    m_playListView->addSongTable(playList);
    //m_listModel->appendRow(playListItem);
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
        return;
    }

qDebug() << "Erreur";
return;

    //---------- OLD ------------------------------------

    // On vérifie que le fichier n'est pas déjà dans la médiathèque
    if (CSong::getId(this, fileName) >= 0)
    {
        qDebug() << "Fichier déjà présent dans la médiathèque.";
        return;
    }

    FMOD_RESULT res;
    FMOD::Sound * sound;

    // Chargement du son
    res = m_soundSystem->createStream(qPrintable(fileName), FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, NULL, &sound);

    if (res == FMOD_OK && sound)
    {
        qDebug() << "Erreur lors du chargement du son avec FMOD";
        return;
    }

    song = new CSong(this);
    song->m_isModified = true;

    song->m_fileName = fileName;
    song->m_fileSize = 0; // TODO
    song->m_bitRate  = 0; // TODO

    FMOD_SOUND_TYPE type;
    FMOD_SOUND_FORMAT format;
    int bits;
    res = sound->getFormat(&type, &format, &(song->m_numChannels), &bits);

    switch (type)
    {
        default:
            song->m_format = CSong::FormatUnknown;
            break;

        case FMOD_SOUND_TYPE_MPEG:
            song->m_format = CSong::FormatMP3;
            break;

        case FMOD_SOUND_TYPE_OGGVORBIS:
            song->m_format = CSong::FormatOGG;
            break;

        case FMOD_SOUND_TYPE_FLAC:
            song->m_format = CSong::FormatFLAC;
            break;
    }

    FMOD_TAG tag;
    int numTags;
    res = sound->getNumTags(&numTags, NULL);
    qDebug() << "Nombre de tags = " << numTags;

    for (int i = 0; i < numTags; ++i)
    {
        res = sound->getTag(NULL, i, &tag);
        qDebug() << "Tag[" << i << "] : " << tag.name << " = " << (res == FMOD_OK ? reinterpret_cast<char *>(tag.data) : "?");
    }

    res = sound->getLength(reinterpret_cast<unsigned int *>(&(song->m_duration)), FMOD_TIMEUNIT_MS);

    res = sound->getTag("TITLE", 0, &tag);
    song->m_title       = (res == FMOD_OK ? reinterpret_cast<char *>(tag.data) : "");

    res = sound->getTag("ARTIST", 0, &tag);
    song->m_artistName  = (res == FMOD_OK ? reinterpret_cast<char *>(tag.data) : "");

    res = sound->getTag("ALBUM", 0, &tag);
    song->m_albumTitle  = (res == FMOD_OK ? reinterpret_cast<char *>(tag.data) : "");

    res = sound->getTag("YEAR", 0, &tag);
    if (res == FMOD_OK)
    {
        QString yearString = reinterpret_cast<char *>(tag.data);
        song->m_year = yearString.toInt();
    }

    song->m_trackNumber = 0;
    song->m_trackTotal  = 0;
    song->m_discNumber  = 0;
    song->m_discTotal   = 0;

    res = sound->getTag("GENRE", 0, &tag);
    song->m_genre       = "";

    song->m_rating      = 0;
    song->m_comments    = "";

    sound->release();

    TagLib::FileRef f(qPrintable(fileName));

    switch (song->m_format)
    {
        case CSong::FormatMP3:
            break;

        case CSong::FormatOGG:
            break;

        case CSong::FormatFLAC:
        {
            TagLib::FLAC::File * file = dynamic_cast<TagLib::FLAC::File *>(f.file());

            if (!file)
            {
                qWarning() << "Le fichier n'est pas au format FLAC selon TagLib";
                break;
            }

            TagLib::Ogg::XiphComment * xiphComment = file->xiphComment(true);

            QString tagTitle = QString::fromUtf8(xiphComment->title().toCString(true));
            QString tagArtist = QString::fromUtf8(xiphComment->artist().toCString(true));
            QString tagAlbum = QString::fromUtf8(xiphComment->album().toCString(true));
            QString tagComments = QString::fromUtf8(xiphComment->comment().toCString(true));
            QString tagGenre = QString::fromUtf8(xiphComment->genre().toCString(true));

            int fieldCount = xiphComment->fieldCount();

            if (fieldCount > 0)
            {
                const TagLib::Ogg::FieldListMap& fieldList = xiphComment->fieldListMap();
                /*
                for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldList.begin() ; it != fieldList.end(); ++it)
                {
                    qDebug() << QString::fromUtf8(it->first.toCString(true));
                    for (TagLib::StringList::ConstIterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                    {
                        qDebug() << QString::fromUtf8(it2->toCString(true));
                    }
                }
                */
            }
/*
virtual uint 	year () const
virtual uint 	track () const
*/

/*
ID3v2::Tag * 	ID3v2Tag (bool create=false)
ID3v1::Tag * 	ID3v1Tag (bool create=false)
xiphComment (bool create=false)
*/

            //...

            break;
        }
    }

    song->updateDatabase();
    m_library->addSong(song);

    //-------------------------------------------------

    //...

    //TODO: les ajouter...
    //TODO: ouvrir fichier
    //TODO: maj DB
    //TODO: maj library
    //TODO: maj vue
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


void CApplication::editSong(CSongTableItem * songItem)
{
    if (!songItem)
    {
        return;
    }

    CDialogEditSong * dialog = new CDialogEditSong(songItem, m_displayedSongTable);
    dialog->show();
}


void CApplication::removeSong(CSongTableItem * songItem)
{
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
        m_uiWidget->label->setText(song->getTitle() + " - " + song->getArtistName() + " - " + song->getAlbumTitle());
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
        m_uiWidget->label->setText(""); // "Pas de morceau en cours de lecture..."
        m_uiWidget->sliderPosition->setEnabled(false);
        m_uiWidget->sliderPosition->setRange(0, 1000);
        m_uiWidget->lblPosition->setText("0:00");
        m_uiWidget->lblTime->setText("0:00");
    }

    m_uiWidget->sliderPosition->setValue(0);
}


void CApplication::updatePosition(void)
{
    setPosition(m_uiWidget->sliderPosition->value());
}


void CApplication::update(void)
{
    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        qDebug() << "CApplication::update()";
        const int position = m_currentSongItem->getSong()->getPosition();

        if (m_currentSongItem->getSong()->isEnded()/* && m_state != Loading*/)
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

    //CSongTable * songTable = m_listModel->data(index, Qt::UserRole + 1).value<CSongTable *>();
    CSongTable * songTable = m_playListView->getSongTable(index);
    displaySongTable(songTable);
}


/**
 * Change la liste affichée.
 *
 * \todo Mettre à jour la barre d'état.
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

        m_displayedSongTable = songTable;
        m_displayedSongTable->show();

        // Barre d'état
        QTime duration(0, 0);
        duration = duration.addMSecs(m_displayedSongTable->getTotalDuration());
        statusBar()->addWidget(new QLabel(tr("%1 song(s), %2").arg(m_displayedSongTable->getNumSongs()).arg(duration.toString())));
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
 * \todo Utiliser les méthodes dédiées dans les classes CSongTable et CListFolder.
 */

void CApplication::loadDatabase(void)
{
    QSqlQuery query(m_dataBase);


    // Création de la librairie
    m_library = new CSongTable(this);
    m_library->m_idPlayList = 0;
    m_uiWidget->splitter->addWidget(m_library);
    connect(m_library, SIGNAL(songStarted(CSongTableItem *)), this, SLOT(playSong(CSongTableItem *)));

    if (!query.exec("SELECT list_columns FROM playlist WHERE playlist_id = 0"))
    {
        QString error = query.lastError().text();
        QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
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
        QString error = query.lastError().text();
        QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
    }

    while (query.next())
    {
        //CListFolder * folder = new CListFolder();
        //folder->m_id   = query.value(0).toInt();
        //folder->m_name = query.value(1).toString();
        //...
    }


    // Création des listes de lecture statiques
    if (!query.exec("SELECT static_list_id, playlist_name, list_columns, playlist_id FROM static_list NATURAL JOIN playlist ORDER BY list_position"))
    {
        QString error = query.lastError().text();
        QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
    }

    while (query.next())
    {
        CStaticPlayList * playList = new CStaticPlayList(this, query.value(1).toString());
        playList->m_id = query.value(0).toInt();
        playList->m_idPlayList = query.value(3).toInt();
        playList->initColumns(query.value(2).toString());

        // Liste des morceaux de la liste de lecture
        QSqlQuery query2(m_dataBase);
        query2.prepare("SELECT song_id, song_position FROM static_list_song WHERE static_list_id = ? ORDER BY song_position");
        query2.bindValue(0, playList->m_id);

        if (!query2.exec())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
            delete playList;
            continue;
        }

        while (query2.next())
        {
            playList->addSongToTable(getSongFromId(query2.value(0).toInt()), query2.value(1).toInt());
        }

        addPlayList(playList);
    }


    // Création des listes de lecture dynamiques
    if (!query.exec("SELECT dynamic_list_id, playlist_name, dynamic_list_union FROM dynamic_list NATURAL JOIN playlist ORDER BY list_position"))
    {
        QString error = query.lastError().text();
        QMessageBox::warning(this, QString(), tr("Database error:\n%1").arg(error));
    }

    while (query.next())
    {
        //...
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

/*
Copyright (C) 2012 Teddy Michel

This file is part of TMediaPlayer.

TMediaPlayer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TMediaPlayer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TMediaPlayer. If not, see <http://www.gnu.org/licenses/>.
*/

#include "CApplication.hpp"
#include "CSong.hpp"
#include "CSongTableModel.hpp"
#include "CStaticPlayList.hpp"
#include "CDynamicList.hpp"
#include "CFolder.hpp"
#include "CPlayListView.hpp"
#include "CListModel.hpp"
#include "Dialog/CDialogEditDynamicList.hpp"
#include "Dialog/CDialogEditFolder.hpp"
#include "Dialog/CDialogEditMetadata.hpp"
#include "Dialog/CDialogEditSong.hpp"
#include "Dialog/CDialogEditSongs.hpp"
#include "Dialog/CDialogEditStaticPlayList.hpp"
#include "Dialog/CDialogPreferences.hpp"
#include "Dialog/CDialogEqualizer.hpp"
#include "Dialog/CDialogRemoveFolder.hpp"
#include "Importer/CImporterITunes.hpp"
#include "CLibrary.hpp"

// Last.fm
#include "Last.fm/CAuthentication.hpp"
#include "Last.fm/CUpdateNowPlaying.hpp"
#include "Last.fm/CScrobble.hpp"

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
#include <QDockWidget>

// FMOD
#include <fmod/fmod.hpp>

// DEBUG
#include <QtDebug>
#include <QSqlDriver>


/**
 * Constructeur de la classe principale de l'application.
 */

CApplication::CApplication(void) :
    QMainWindow            (NULL),
    m_uiWidget             (new Ui::TMediaPlayer()),
    m_uiControl            (new Ui::WidgetControl()),
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
    m_lastFmTimeListened         (0),
    m_lastFmLastPosition         (0),
    m_lastFmState                (NoScrobble)
{
    // Chargement des paramètres de l'application
    m_settings = new QSettings(this);

    m_applicationPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QDir::separator();
    QDir(m_applicationPath).mkpath(".");

    // Initialisation de l'interface graphique
    m_uiWidget->setupUi(this);

    // Last.fm
    m_lastFmEnableScrobble = m_settings->value("LastFm/EnableScrobble", false).toBool();
    m_delayBeforeNotification = m_settings->value("LastFm/DelayBeforeNotification", 5000).toInt();
    m_percentageBeforeScrobbling = m_settings->value("LastFm/PercentageBeforeScrobbling", 60).toInt();
    m_lastFmKey = m_settings->value("LastFm/SessionKey", "").toByteArray();

    // Barre d'état
    QTime duration(0, 0);
    m_listInfos = new QLabel(tr("%n song(s), %1", "", 0).arg(duration.toString()));
    statusBar()->addPermanentWidget(m_listInfos);

    // Menus
    connect(m_uiWidget->actionNewPlayList, SIGNAL(triggered()), this, SLOT(openDialogCreateStaticList()));
    connect(m_uiWidget->actionNewDynamicPlayList, SIGNAL(triggered()), this, SLOT(openDialogCreateDynamicList()));
    connect(m_uiWidget->actionNewFolder, SIGNAL(triggered()), this, SLOT(openDialogCreateFolder()));
    connect(m_uiWidget->actionAddFiles, SIGNAL(triggered()), this, SLOT(openDialogAddSongs()));
    connect(m_uiWidget->actionAddFolder, SIGNAL(triggered()), this, SLOT(openDialogAddFolder()));
    connect(m_uiWidget->actionInformations, SIGNAL(triggered()), this, SLOT(openDialogSongInfos()));
    connect(m_uiWidget->actionOpenInExplorer, SIGNAL(triggered()), this, SLOT(openSongInExplorer()));
    connect(m_uiWidget->actionImportITunes, SIGNAL(triggered()), this, SLOT(importFromITunes()));
    connect(m_uiWidget->actionImportSongbird, SIGNAL(triggered()), this, SLOT(importFromSongbird()));

    connect(m_uiWidget->actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(m_uiWidget->actionSelectNone, SIGNAL(triggered()), this, SLOT(selectNone()));
    connect(m_uiWidget->actionPreferences, SIGNAL(triggered()), this, SLOT(openDialogPreferences()));

    connect(m_uiWidget->actionPlay, SIGNAL(triggered()), this, SLOT(togglePlay()));
    connect(m_uiWidget->actionStop, SIGNAL(triggered()), this, SLOT(stop()));
    connect(m_uiWidget->actionPrevious, SIGNAL(triggered()), this, SLOT(previousSong()));
    connect(m_uiWidget->actionNext, SIGNAL(triggered()), this, SLOT(nextSong()));
    connect(m_uiWidget->actionRepeat, SIGNAL(triggered(bool)), this, SLOT(setRepeat(bool)));
    connect(m_uiWidget->actionShuffle, SIGNAL(triggered(bool)), this, SLOT(setShuffle(bool)));
    connect(m_uiWidget->actionEqualizer, SIGNAL(triggered()), this, SLOT(openDialogEqualizer()));

    connect(m_uiWidget->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));


    connect(this, SIGNAL(songPlayStart(CSong *)), this, SLOT(updateSongDescription(CSong *)));
}


/**
 * Libère les ressources utilisées par l'application.
 */

CApplication::~CApplication()
{
    //qDebug() << "CApplication::~CApplication()";

    if (m_timer)
    {
        m_timer->stop();
        delete m_timer;
    }

    // Enregistrement des paramètres
    m_settings->setValue("Preferences/Volume", m_volume);
    m_settings->setValue("Preferences/Shuffle", m_isShuffle);
    m_settings->setValue("Preferences/Repeat", m_isRepeat);

    //dumpObjectTree();

    delete m_listModel;

    if (m_library)
    {
        m_library->updateDatabase();
        m_library->deleteSongs();
        delete m_library;
    }

    m_dataBase.close();

    m_soundSystem->release();

    delete m_uiWidget;
}


/**
 * Initialise l'interface graphique et charge les données.
 */

bool CApplication::initWindow(void)
{
    static bool init = false;

    if (init)
    {
        logError("l'application a déjà été initialisée", __FUNCTION__, __FILE__, __LINE__);
        return true;
    }


    // Barre de contrôle
    //QWidget * toolWidget = new QWidget(this);
    //m_uiWidget->horizontalLayout->setParent(NULL);
    //toolWidget->setLayout(m_uiWidget->horizontalLayout);
    QWidget * widgetControl = new QWidget(this);
    m_uiControl->setupUi(widgetControl);
    m_uiWidget->toolBar->addWidget(widgetControl);

    m_uiControl->btnStop->setVisible(m_settings->value("Preferences/ShowButtonStop", true).toBool());

    // Connexions des signaux et des slots
    connect(m_uiControl->songInfos, SIGNAL(clicked()), this, SLOT(selectCurrentSong()));

    connect(m_uiControl->btnPlay, SIGNAL(clicked()), this, SLOT(togglePlay()));
    connect(m_uiControl->btnStop, SIGNAL(clicked()), this, SLOT(stop()));

    connect(m_uiControl->btnPrevious, SIGNAL(clicked()), this, SLOT(previousSong()));
    connect(m_uiControl->btnNext, SIGNAL(clicked()), this, SLOT(nextSong()));

    connect(m_uiControl->btnRepeat, SIGNAL(clicked()), this, SLOT(setRepeat()));
    connect(m_uiControl->btnShuffle, SIGNAL(clicked()), this, SLOT(setShuffle()));
    connect(m_uiControl->btnMute, SIGNAL(clicked()), this, SLOT(toggleMute()));

    // Sliders
    connect(m_uiControl->sliderVolume, SIGNAL(sliderMoved(int)), this, SLOT(setVolume(int)));
    connect(m_uiControl->sliderPosition, SIGNAL(sliderReleased()), this, SLOT(updatePosition()));


    // Dock "Playlists"
    m_playListView = new CPlayListView(this);

    QDockWidget * dockWidget = new QDockWidget(tr("Playlists"), this);
    dockWidget->setObjectName("dock_playlists");
    dockWidget->setWidget(m_playListView);
    dockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    connect(m_playListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectPlayListFromTreeView(const QModelIndex&)));


    restoreGeometry(m_settings->value("Window/WindowGeometry").toByteArray());
    restoreState(m_settings->value("Window/WindowState").toByteArray());


    // Initialisation de FMOD
    if (!initSoundSystem())
    {
        QMessageBox::critical(this, QString(), tr("Failed to init sound system with FMOD."));
        QCoreApplication::exit();
        return false;
    }


    // Égaliseur
    const float eqFrequencies[10] = {32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};

    for (int i = 0; i < 10; ++i)
    {
        m_equalizerGains[i] = qBound(0.05f, m_settings->value(QString("Equalizer/Gain_%1").arg(i), 1.0f).toFloat(), 3.0f);
        FMOD_RESULT res;

        res = m_soundSystem->createDSPByType(FMOD_DSP_TYPE_PARAMEQ, &m_dsp[i]);
        res = m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_CENTER, eqFrequencies[i]);
        res = m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_BANDWIDTH, 1.0);
        res = m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_GAIN, m_equalizerGains[i]);
        res = m_soundSystem->addDSP(m_dsp[i], NULL);
    }

    setEqualizerEnabled(m_settings->value("Equalizer/Enabled", false).toBool());


    // Paramètres de lecture
    setVolume(m_settings->value("Preferences/Volume", 50).toInt());
    setShuffle(m_settings->value("Preferences/Shuffle", false).toBool());
    setRepeat(m_settings->value("Preferences/Repeat", false).toBool());


    // Chargement de la base de données
    QString dbType = m_settings->value("Database/Type", QString("QSQLITE")).toString();
    m_settings->setValue("Database/Type", dbType);
    m_dataBase = QSqlDatabase::addDatabase(dbType);

    QString dbHostName = m_settings->value("Database/Host", QString("localhost")).toString();
    int dbPort = m_settings->value("Database/Port", 0).toInt();
    QString dbBaseName = m_settings->value("Database/Base", QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QDir::separator() + "library.sqlite").toString();
    QString dbUserName = m_settings->value("Database/UserName", QString("root")).toString();
    QString dbPassword = m_settings->value("Database/Password", QString("")).toString();

    m_dataBase.setHostName(dbHostName);
    m_dataBase.setPort(dbPort);
    m_dataBase.setDatabaseName(dbBaseName);
    m_dataBase.setUserName(dbUserName);
    m_dataBase.setPassword(dbPassword);

    m_settings->setValue("Database/Host", dbHostName);
    m_settings->setValue("Database/Port", dbPort);
    m_settings->setValue("Database/Base", dbBaseName);
    m_settings->setValue("Database/UserName", dbUserName);
    m_settings->setValue("Database/Password", dbPassword);

    if (!m_dataBase.open())
    {
        QMessageBox::critical(this, QString(), tr("Failed to load database: %1.").arg(m_dataBase.lastError().text()));
        QCoreApplication::exit();
        return false;
    }

    loadDatabase();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(400);

    updateSongDescription(NULL);
    setState(Stopped);

    init = true;
    return true;
}


/**
 * Affiche une erreur de base de données.
 *
 * \param msg      Message d'erreur.
 * \param query    Requête exécutée.
 * \param fileName Fichier source à l'origine de l'erreur.
 * \param line     Ligne du fichier à l'origine de l'erreur.
 */

void CApplication::showDatabaseError(const QString& msg, const QString& query, const QString& fileName, int line)
{

#ifdef _DEBUG
    QMessageBox::warning(this, tr("Database error"), tr("File: %1 (%2)\n\nQuery: %3\n\nError: %4").arg(fileName).arg(line).arg(query).arg(msg));
#endif

    qWarning() << "Database error (in file" << fileName << ", line" << line << "):" << msg;
    qWarning() << "  Query:" << query;
}


/**
 * Modifie la hauteur des lignes des tableaux.
 *
 * \param height Hauteur des lignes en pixels, entre 15 et 50.
 */

void CApplication::setRowHeight(int height)
{
    height = qBound(15, height, 50);
    m_settings->setValue("Preferences/RowHeight", height);

    // Mise à jour des vues
    m_library->verticalHeader()->setDefaultSectionSize(height);

    const QList<IPlayList *> playLists = getAllPlayLists();

    for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        (*it)->verticalHeader()->setDefaultSectionSize(height);
    }
}


/**
 * Retourne la hauteur des lignes des tableaux.
 *
 * \return Hauteur des lignes en pixels (par défaut, 19).
 */

int CApplication::getRowHeight(void) const
{
    return m_settings->value("Preferences/RowHeight", 19).toInt();
}


/**
 * Affiche ou masque le bouton "Stop".
 *
 * \param show Booléen.
 */

void CApplication::showButtonStop(bool show)
{
    m_settings->setValue("Preferences/ShowButtonStop", show);
    m_uiControl->btnStop->setVisible(show);
}


/**
 * Active ou désactive le scrobbling avec Last.fm.
 *
 * \param enable Booléen.
 */

void CApplication::enableScrobbling(bool enable)
{
    m_lastFmEnableScrobble = enable;
    m_settings->setValue("LastFm/EnableScrobble", enable);
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
 * Modifie le gain de l'égaliseur pour une bande de fréquence.
 *
 * \param frequency Bande de fréquence.
 * \param gain      Valeur du gain (entre 0.05 et 3).
 */

void CApplication::setEqualizerGain(TEqualizerFrequency frequency, double gain)
{
    m_equalizerGains[frequency] = qBound(0.05, gain, 3.0);
    m_settings->setValue(QString("Equalizer/Gain_%1").arg(frequency), m_equalizerGains[frequency]);
    FMOD_RESULT res = m_dsp[frequency]->setParameter(FMOD_DSP_PARAMEQ_GAIN, m_equalizerGains[frequency]);
}


/**
 * Récupère le gain de l'égaliseur pour une bande de fréquence.
 *
 * \param frequency Bande de fréquence.
 * \return Valeur du gain (entre 0.05 et 3).
 */

double CApplication::getEqualizerGain(TEqualizerFrequency frequency)
{
    return m_equalizerGains[frequency];
}


/**
 * Réinitialise les gains de l'égaliseur.
 * Tous les gains sont définis à 1.
 */

void CApplication::resetEqualizer(void)
{
    setEqualizerGain(CApplication::EqFreq32 , 1.0);
    setEqualizerGain(CApplication::EqFreq64 , 1.0);
    setEqualizerGain(CApplication::EqFreq125, 1.0);
    setEqualizerGain(CApplication::EqFreq250, 1.0);
    setEqualizerGain(CApplication::EqFreq500, 1.0);
    setEqualizerGain(CApplication::EqFreq1K , 1.0);
    setEqualizerGain(CApplication::EqFreq2K , 1.0);
    setEqualizerGain(CApplication::EqFreq4K , 1.0);
    setEqualizerGain(CApplication::EqFreq8K , 1.0);
    setEqualizerGain(CApplication::EqFreq16K, 1.0);
}


/**
 * Active ou désactive l'égaliseur.
 *
 * \param enabled Booléen.
 */

void CApplication::setEqualizerEnabled(bool enabled)
{
    m_settings->setValue(QString("Equalizer/Enabled"), enabled);

    if (enabled)
    {
        for (int i = 0; i < 10; ++i)
        {
            FMOD_RESULT res = m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_GAIN, m_equalizerGains[i]);
        }
    }
    else
    {
        for (int i = 0; i < 10; ++i)
        {
            FMOD_RESULT res = m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_GAIN, 1.0);
        }
    }
}


/**
 * Indique si l'égaliseur est activé.
 *
 * \return Booléen.
 */

bool CApplication::isEqualizerEnabled(void) const
{
    return m_settings->value(QString("Equalizer/Enabled"), false).toBool();
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
        return NULL;

    const QMap<int, CSong *> songList = m_library->getSongsMap();
    return songList.value(id);
/*
    for (QList<CSong *>::const_iterator it = songList.begin(); it != songList.end(); ++it)
    {
        if ((*it)->getId() == id)
        {
            return (*it);
        }
    }

    return NULL;
*/
}


/**
 * Retourne le dossier correspondant à un identifiant.
 *
 * \param id Identifiant du dossier.
 * \return Pointeur sur le dossier, ou NULL si \a id n'est pas valide.
 */

CFolder * CApplication::getFolderFromId(int id) const
{
    return m_listModel->getFolderFromId(id);
/*
    if (id < 0)
        return NULL;

    for (QList<CFolder *>::const_iterator it = m_folders.begin(); it != m_folders.end(); ++it)
    {
        if ((*it)->getId() == id)
            return (*it);
    }

    return NULL;
*/
}


/**
 * Retourne la liste de lecture correspondant à un identifiant.
 *
 * \param id Identifiant de la liste.
 * \return Pointeur sur la liste de lecture, ou NULL si \a id n'est pas valide.
 */

IPlayList * CApplication::getPlayListFromId(int id) const
{
    return m_listModel->getPlayListFromId(id);
/*
    if (id <= 0)
        return NULL;

    for (QList<IPlayList *>::const_iterator it = m_playLists.begin(); it != m_playLists.end(); ++it)
    {
        if ((*it)->getIdPlayList() == id)
            return (*it);
    }

    return NULL;
*/
}


/**
 * Retourne la liste des listes de lecture contenant un morceau.
 *
 * \todo Chercher dans les dossiers.
 *
 * \param song Morceau à rechercher.
 * \return Liste des listes de lecture.
 */

QList<IPlayList *> CApplication::getPlayListsWithSong(CSong * song) const
{
    Q_CHECK_PTR(song);

    const QList<IPlayList *> playLists = m_listModel->getPlayLists();
    QList<IPlayList *> playListsRet;

    for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        if ((*it)->hasSong(song))
            playListsRet.append(*it);
    }

    return playListsRet;
}


/**
 * Retourne la liste des listes de lecture de la médiathèque.
 *
 * \return Listes de lecture.
 */

QList<IPlayList *> CApplication::getAllPlayLists(void) const
{
    return m_listModel->getPlayLists();
}


/**
 * Enlève une liste de morceaux de la médiathèque.
 * La liste ne doit pas contenir de doublons.
 *
 * \todo Implémentation.
 *
 * \param songs Liste des morceaux à enlever.
 */

void CApplication::removeSongs(const QList<CSong *> songs)
{
    //qDebug() << "CApplication::removeSongs()";

    m_library->removeSongsFromTable(songs);

    for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
    {
        Q_CHECK_PTR(*it);
        emit songRemoved(*it);
    }
/*
    m_library->m_automaticSort = false;
    for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
    {
        Q_CHECK_PTR(*it);
        m_library->removeSongFromTable(*it);
        emit songRemoved(*it);
    }
    m_library->m_automaticSort = true;
*/

    // Suppression des morceaux de chaque liste statique et mise à jour des listes dynamiques
    const QList<IPlayList *> playLists = getAllPlayLists();

    for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        CStaticPlayList * playList = qobject_cast<CStaticPlayList *>(*it);
        CDynamicList * dynamicList = qobject_cast<CDynamicList *>(*it);

        if (playList)
        {
            playList->removeSongs(songs, false);
        }
        else if (dynamicList)
        {
            dynamicList->update();
        }
    }

    // Mise à jour de la base
    QSqlQuery query(m_dataBase);
    query.prepare("DELETE FROM song WHERE song_id = ?");

    for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
    {
        int songId = (*it)->getId();

        if (songId > 0)
        {
            query.bindValue(0, songId);

            if (!query.exec())
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                continue;
            }
        }

        delete *it; // C'est pour ça que la liste ne doit pas contenir de doublons
    }

    updateListInformations();
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

    if (m_dataBase.driverName() == "QPSQL")
    {
        query.prepare("SELECT currval('artist_artist_id_seq')");

        if (!query.exec())
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }

        if (query.next())
        {
            return query.value(0).toInt();
        }
        else
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }
    }
    else
    {
        return query.lastInsertId().toInt();
    }
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

    if (m_dataBase.driverName() == "QPSQL")
    {
        query.prepare("SELECT currval('album_album_id_seq')");

        if (!query.exec())
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }

        if (query.next())
        {
            return query.value(0).toInt();
        }
        else
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }
    }
    else
    {
        return query.lastInsertId().toInt();
    }
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

    if (m_dataBase.driverName() == "QPSQL")
    {
        query.prepare("SELECT currval('genre_genre_id_seq')");

        if (!query.exec())
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }

        if (query.next())
        {
            return query.value(0).toInt();
        }
        else
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }
    }
    else
    {
        return query.lastInsertId().toInt();
    }
}


/**
 * Retourne la liste des genres classée par nom.
 *
 * \todo Compléter la liste des genres prédéfinis.
 *
 * \return Liste des genres, qui contient l'ensemble des genres utilisés
 *         par les morceaux, en plus de certains genres prédéfinis.
 */

QStringList CApplication::getGenreList(void)
{
    QStringList genres;

    // Genres prédéfinis
    genres.append("Classical");
    genres.append("Country");
    genres.append("Funk");
    genres.append("Hard Rock");
    genres.append("Heavy Metal");
    genres.append("Jazz");
    genres.append("Punk");
    genres.append("Rap");
    genres.append("Reggae");
    genres.append("Rock");

    // Liste des genres utilisés
    QSqlQuery query(m_dataBase);

    if (query.exec("SELECT genre_name FROM genres"))
    {
        while (query.next())
        {
            genres.append(query.value(0).toString());
        }
    }
    else
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    genres.removeDuplicates();
    genres.sort();
    return genres;
}


QFile * CApplication::getLogFile(const QString& logName)
{
    QString fileName = logName + QDateTime::currentDateTime().toString("-yyyy-MM-dd");

    if (!m_logList.contains(fileName))
    {
        QFile * logFile = new QFile(m_applicationPath + fileName + ".log", this);

        if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append))
        {
            logError("erreur lors de l'ouverture du fichier de log", __FUNCTION__, __FILE__, __LINE__);
            return NULL;
        }

        m_logList[fileName] = logFile;
    }

    return m_logList.value(fileName);
}


/**
 * Gestion des messages d'erreur.
 *
 * \param message Message d'erreur.
 * \param file    Nom du fichier source.
 * \param line    Ligne dans le fichier source.
 */

void CApplication::logError(const QString& message, const QString& function, const char * file, int line)
{
    static QFile * logFile = NULL;

    if (!logFile)
    {
        logFile = getLogFile("errors");
    }

    QTextStream stream(logFile);
    stream << function << ": " << message << "\n";

#ifdef QT_DEBUG
    qWarning() << tr("%2 (%3 line %4): %1").arg(message).arg(function).arg(file).arg(line);
#endif
}


/**
 * Affiche un message dans la barre d'état.
 * Le message est affiché pendant 3 secondes.
 *
 * \param message Message à afficher.
 */

void CApplication::notifyInformation(const QString& message)
{
    statusBar()->showMessage(message, 3000);
}


/**
 * Lance le processus d'authentication avec Last.fm.
 * Le navigateur doit s'ouvrir pour que l'utilisateur puisse se connecter.
 */

void CApplication::connectToLastFm(void)
{
    //qDebug() << "CApplication::connectToLastFm()";
    new CAuthentication(this);
}


/**
 * Sélectionne tous les morceaux de la liste affichée.
 */

void CApplication::selectAll(void)
{
    Q_CHECK_PTR(m_displayedSongTable);
    m_displayedSongTable->selectAll();
}


/**
 * Désélectionne tous les morceaux de la liste affichée.
 */

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
    //qDebug() << "CApplication::play()";

    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        if (m_state == Paused)
        {
            //qDebug() << "CApplication::play() : chanson en pause";

            setState(Playing);
            //m_uiControl->btnPlay->setIcon(QPixmap(":/icons/pause"));
            //m_uiWidget->actionPlay->setText(tr("Pause"));

            emit songResumed(m_currentSongItem->getSong());
            m_currentSongItem->getSong()->play();

            m_currentSongTable->m_model->setCurrentSong(m_currentSongItem);
        }

        m_state = Playing;
    }
    else
    {
        m_state = Stopped;

        //qDebug() << "CApplication::play() : lancement du premier morceau";
        m_currentSongTable = m_displayedSongTable;

        // Recherche du morceau sélectionné
        m_currentSongItem = m_currentSongTable->getSelectedSongItem();

        if (m_currentSongItem)
        {
            if (m_isShuffle)
                m_currentSongTable->initShuffle(m_currentSongItem);
        }
        else
        {
            if (m_isShuffle)
                m_currentSongTable->initShuffle();

            // Lecture du premier morceau de la liste
            m_currentSongItem = m_currentSongTable->getNextSong(NULL, m_isShuffle);

            if (m_currentSongItem && !m_currentSongItem->getSong()->isEnabled())
            {
                nextSong();
                return;
            }
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
        Q_CHECK_PTR(m_currentSongTable);

        m_currentSongItem->getSong()->stop();
        emit songStopped(m_currentSongItem->getSong());
        updateSongDescription(NULL);
        m_currentSongItem = NULL;

        m_currentSongTable->m_model->setCurrentSong(NULL);
    }
    else
    {
        Q_ASSERT(m_currentSongTable == NULL);
    }

    m_currentSongTable = NULL;
    m_state = Stopped;

    setState(Stopped);
    //m_uiControl->btnPlay->setIcon(QPixmap(":/icons/play"));
    //m_uiWidget->actionPlay->setText(tr("Play"));

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
        Q_CHECK_PTR(m_currentSongTable);

        //m_uiControl->btnPlay->setIcon(QPixmap(":/icons/play"));
        //m_uiWidget->actionPlay->setText(tr("Play"));
        setState(Paused);

        m_currentSongItem->getSong()->pause();
        emit songPaused(m_currentSongItem->getSong());
        m_state = Paused;
        m_currentSongTable->m_model->setCurrentSong(m_currentSongItem);
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
    //qDebug() << "CApplication::previousSong()";

    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        int position = m_currentSongItem->getSong()->getPosition();

        m_currentSongItem->getSong()->stop();
        updateSongDescription(NULL);
        m_currentSongTable->m_model->setCurrentSong(NULL);

        //m_uiControl->btnPlay->setIcon(QPixmap(":/icons/play"));
        setState(Stopped);

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
/*
            m_currentSongItem = NULL;
            m_currentSongTable = NULL;
            m_state = Stopped;
*/
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

        if (m_currentSongItem && !m_currentSongItem->getSong()->isEnabled())
        {
            previousSong();
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
    //qDebug() << "CApplication::nextSong()";

    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        m_currentSongItem->getSong()->stop();
        updateSongDescription(NULL);
        m_currentSongTable->m_model->setCurrentSong(NULL);

        //m_uiControl->btnPlay->setIcon(QPixmap(":/icons/play"));
        setState(Stopped);

        m_currentSongItem = m_currentSongTable->getNextSong(m_currentSongItem, m_isShuffle);

        if (m_currentSongItem && !m_currentSongItem->getSong()->isEnabled())
        {
            nextSong();
            return;
        }

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
        m_currentSongTable->m_model->setCurrentSong(NULL);
    }

    m_currentSongItem = songItem;

    if (m_isShuffle && (m_state == Stopped || m_currentSongTable != m_displayedSongTable))
    {
        m_displayedSongTable->initShuffle(m_currentSongItem);
    }

    m_currentSongTable = m_displayedSongTable;

    if (m_currentSongItem->getSong()->loadSound())
    {
        //m_uiControl->btnPlay->setIcon(QPixmap(":/icons/pause"));
        setState(Playing);
        startPlay();
    }
    else
    {
        //m_uiControl->btnPlay->setIcon(QPixmap(":/icons/play"));
        setState(Stopped);
        m_currentSongItem = NULL;
        m_currentSongTable = NULL;
    }
}


void CApplication::setRepeat(void)
{
    setRepeat(!m_isRepeat);
}


void CApplication::setRepeat(bool repeat)
{
    if (repeat != m_isRepeat)
    {
        m_isRepeat = repeat;
        //m_uiControl->btnRepeat->setChecked(repeat);
        m_uiControl->btnRepeat->setIcon(QPixmap(m_isRepeat ? ":/icons/repeat_on" : ":/icons/repeat_off"));
        m_uiWidget->actionRepeat->setChecked(repeat);
    }
}


void CApplication::setShuffle(void)
{
    setShuffle(!m_isShuffle);
}


void CApplication::setShuffle(bool shuffle)
{
    if (shuffle != m_isShuffle)
    {
        m_isShuffle = shuffle;
        //m_uiControl->btnShuffle->setChecked(shuffle);
        m_uiControl->btnShuffle->setIcon(QPixmap(m_isShuffle ? ":/icons/shuffle_on" : ":/icons/shuffle_off"));
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

        m_uiControl->btnMute->setIcon(QPixmap(mute ? ":/icons/muet" : ":/icons/volume"));
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

        m_uiControl->sliderVolume->setValue(volume);
    }
}


void CApplication::setPosition(int position)
{
    Q_ASSERT(position >= 0);

    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        const int songPositionBefore = m_currentSongItem->getSong()->getPosition();
        m_currentSongItem->getSong()->setPosition(position);
        const int songPosition = m_currentSongItem->getSong()->getPosition();

        if (songPosition >= 0)
        {
            // Last.fm
            if (m_lastFmEnableScrobble)
            {
                if (m_state == Playing)
                {
                    m_lastFmTimeListened += (songPositionBefore - m_lastFmLastPosition);
                }

                m_lastFmLastPosition = songPosition;
            }

            m_uiControl->sliderPosition->setValue(songPosition);

            QTime positionTime(0, 0);
            positionTime = positionTime.addMSecs(songPosition);
            m_uiControl->lblPosition->setText(positionTime.toString("m:ss"));
        }
    }
}

/*
/// \todo Supprimer
void CApplication::openPlayList(IPlayList * playList)
{
    Q_CHECK_PTR(playList);

    m_displayedSongTable = playList;
    //TODO: maj vue
}
*/
/*
void CApplication::renamePlayList(IPlayList * playList)
{
    Q_CHECK_PTR(playList);

    //TODO: open dialog rename Playlist
    //TODO: maj DB
}
*/
/*
void CApplication::editDynamicPlayList(CDynamicList * playList)
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
void CApplication::deletePlayList(IPlayList * playList)
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

    for (QList<IPlayList *>::const_iterator it = m_playLists.begin(); it != m_playLists.end(); ++it)
    {
        if (*it == playList)
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
void CApplication::renameListFolder(CFolder * folder)
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
void CApplication::deleteListFolder(CFolder * folder)
{
    Q_CHECK_PTR(folder);

    //TODO: confirmation
    //TODO: maj vue

    QList<IPlayList *> playLists = folder->getPlayLists();
    for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        (*it)->setFolder(nullptr);
        m_playLists.append(*it);
    }

    m_folders.removeOne(folder);
    delete folder;

    //TODO: maj DB
}
*/


/**
 * Affiche la boite de dialogue pour modifier les préférences.
 */

void CApplication::openDialogPreferences(void)
{
    CDialogPreferences * dialog = new CDialogPreferences(this, m_settings);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour modifier les paramètres de l'égaliseur.
 */

void CApplication::openDialogEqualizer(void)
{
    CDialogEqualizer * dialog = new CDialogEqualizer(this);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour visualiser et modifier les métadonnées d'un morceau.
 */

void CApplication::openDialogEditMetadata(void)
{
    Q_CHECK_PTR(m_displayedSongTable);

    // Liste des morceaux sélectionnés
    QList<CSongTableItem *> songItemList = m_displayedSongTable->getSelectedSongItems();

    if (songItemList.size() > 1)
    {
        return;
    }

    // Recherche du morceau sélectionné
    CSongTableItem * songItem = m_displayedSongTable->getSelectedSongItem();

    if (songItem)
    {
        CDialogEditMetadata * dialog = new CDialogEditMetadata(this, songItem->getSong());
        dialog->show();
    }
}


/**
 * Affiche une boite de dialogue pour sélectionner des fichiers à ajouter à la médiathèque.
 */

void CApplication::openDialogAddSongs(void)
{
    QStringList fileList = QFileDialog::getOpenFileNames(this, QString(), QString(), tr("Media files (*.flac *.ogg *.mp3);;MP3 (*.mp3);;FLAC (*.flac);;OGG (*.ogg);;All files (*.*)"));
    importSongs(fileList);
}


/**
 * Affiche une boite de dialogue pour ajouter un dossier à la médiathèque.
 */

void CApplication::openDialogAddFolder(void)
{
    QString folder = QFileDialog::getExistingDirectory(this);

    if (folder.isEmpty())
        return;

    importSongs(importFolder(folder));
}


/**
 * Ajoute une liste de fichiers à la médiathèque.
 * Une boite de progression est affichée si l'opération prend plusieurs secondes.
 *
 * \param fileList Liste des fichiers à ajouter.
 */

void CApplication::importSongs(const QStringList& fileList)
{
    if (fileList.isEmpty())
        return;

    QList<CSong *> songs;

    QProgressDialog progress(tr("Loading files..."), tr("Abort"), 0, fileList.size(), this);
    int i = 0;

    for (QStringList::const_iterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        progress.setValue(i++);
        CSong * song = CSong::loadFromFile(this, *it);
        if (song) songs.append(song);
        //addSong(*it);
        qApp->processEvents();

        if (progress.wasCanceled())
            break;
    }
/*
    // Ajout des morceaux à la médiathèque
    for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
    {
        m_library->addSong(*it);
        emit songAdded(*it);
    }
*/
    m_library->addSongs(songs);
    emit songsAdded();

    notifyInformation(tr("%n song(s) added to the library.", "", songs.size()));
    updateListInformations();
}


/**
 * Affiche une boite de dialogue pour visualiser et éditer les informations du morceau sélectionné.
 */

void CApplication::openDialogSongInfos(void)
{
    Q_CHECK_PTR(m_displayedSongTable);

    // Liste des morceaux sélectionnés
    QList<CSongTableItem *> songItemList = m_displayedSongTable->getSelectedSongItems();

    if (songItemList.size() > 1)
    {
        CDialogEditSongs * dialog = new CDialogEditSongs(songItemList, this);
        dialog->show();

        return;
    }

    // Recherche du morceau sélectionné
    CSongTableItem * songItem = m_displayedSongTable->getSelectedSongItem();

    if (songItem)
    {
        CDialogEditSong * dialog = new CDialogEditSong(songItem, m_displayedSongTable, this);
        dialog->show();
    }
}


/**
 * Affiche la boite de dialogue pour crée une nouvelle liste de lecture statique.
 */

void CApplication::openDialogCreateStaticList(void)
{
    openDialogCreateStaticList(NULL);
}


/**
 * Affiche la boite de dialogue pour crée une nouvelle liste de lecture statique.
 *
 * \param folder Pointeur sur le dossier où créer la liste.
 * \param songs  Liste de morceaux à ajouter à la liste.
 */

void CApplication::openDialogCreateStaticList(CFolder * folder, const QList<CSong *>& songs)
{
    if (!folder)
        folder = m_listModel->getRootFolder();

    CDialogEditStaticPlayList * dialog = new CDialogEditStaticPlayList(NULL, this, folder, songs);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour créer une nouvelle liste de lecture dynamique.
 *
 * \param folder Pointeur sur le dossier où créer la liste.
 */

void CApplication::openDialogCreateDynamicList(CFolder * folder)
{
    if (!folder)
        folder = m_listModel->getRootFolder();

    CDialogEditDynamicList * dialog = new CDialogEditDynamicList(NULL, this, folder);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour créer un nouveau dossier.
 *
 * \param folder Pointeur sur le dossier où créer le dossier.
 */

void CApplication::openDialogCreateFolder(CFolder * folder)
{
    if (!folder)
        folder = m_listModel->getRootFolder();

    CDialogEditFolder * dialog = new CDialogEditFolder(NULL, this, folder);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour modifier une liste de lecture statique.
 *
 * \param playList Liste à modifier.
 */

void CApplication::openDialogEditStaticPlayList(CStaticPlayList * playList)
{
    CDialogEditStaticPlayList * dialog = new CDialogEditStaticPlayList(playList, this, playList->getFolder());
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour modifier une liste de lecture dynamique.
 *
 * \param playList Liste à modifier.
 */

void CApplication::openDialogEditDynamicList(CDynamicList * playList)
{
    CDialogEditDynamicList * dialog = new CDialogEditDynamicList(playList, this, playList->getFolder());
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour modifier un dossier.
 *
 * \param folder Dossier à modifier.
 */

void CApplication::openDialogEditFolder(CFolder * folder)
{
    CDialogEditFolder * dialog = new CDialogEditFolder(folder, this, folder->getFolder());
    dialog->show();
}


void CApplication::relocateSong(void)
{
    Q_CHECK_PTR(m_displayedSongTable);

    // Liste des morceaux sélectionnés
    QList<CSongTableItem *> songItemList = m_displayedSongTable->getSelectedSongItems();

    if (songItemList.size() > 1)
    {
        logError("plusieurs morceaux sélectionnés", __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    // Recherche du morceau sélectionné
    CSongTableItem * songItem = m_displayedSongTable->getSelectedSongItem();

    if (!songItem)
    {
        return;
    }

    CSong * song = songItem->getSong();

    if (song)
    {
        QString fileName = QFileDialog::getOpenFileName(this, QString(), QString(), tr("Media files (*.flac *.ogg *.mp3);;MP3 (*.mp3);;FLAC (*.flac);;OGG (*.ogg);;All files (*.*)"));

        if (fileName.isEmpty())
        {
            return;
        }

        // Le fichier est déjà dans la médiathèque
        if (CSong::getId(this, fileName) >= 0)
        {
            logError(QString("le fichier %1 est déjà dans la médiathèque").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
            return;
        }

        song->m_properties.fileName = fileName;
        //song->m_isModified = true;

        // Recherche de la durée du morceau
        FMOD_SOUND_TYPE type;
        FMOD_RESULT res;
        FMOD::Sound * sound;

        // Chargement du son
        res = m_soundSystem->createStream(qPrintable(fileName), FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, NULL, &sound);

        if (res != FMOD_OK || !sound)
        {
            logError(QString("erreur lors du chargement du fichier %1 avec FMOD").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
            return;
        }

        res = sound->getLength(reinterpret_cast<unsigned int *>(&(song->m_properties.duration)), FMOD_TIMEUNIT_MS);
        if (res != FMOD_OK)
        {
            logError(QString("impossible de calculer la durée du morceau %1").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
            song->m_properties.duration = 0;
        }

        // Recherche du format du morceau
        res = sound->getFormat(&type, NULL, NULL, NULL);

        if (res != FMOD_OK)
        {
            logError(QString("impossible de déterminer le format du morceau %1").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
        }
        else
        {
            switch (type)
            {
                default:
                    logError("format inconnu", __FUNCTION__, __FILE__, __LINE__);
                    return;

                case FMOD_SOUND_TYPE_MPEG:
                    song->m_properties.format = CSong::FormatMP3;
                    break;

                case FMOD_SOUND_TYPE_OGGVORBIS:
                    song->m_properties.format = CSong::FormatOGG;
                    break;

                case FMOD_SOUND_TYPE_FLAC:
                    song->m_properties.format = CSong::FormatFLAC;
                    break;
            }
        }

        sound->release();

        // Chargement des métadonnées
        if (!song->loadTags(true))
        {
            return;
        }

        song->m_fileStatus = true;
        song->updateDatabase();
    }
}


/**
 * Ouvre la fenêtre pour importer la médiathèque depuis iTunes.
 */

void CApplication::importFromITunes(void)
{
    CImporterITunes * dialog = new CImporterITunes(this);
    dialog->show();
}


/**
 * Ouvre la fenêtre pour importer la médiathèque depuis Songbird.
 *
 * \todo Implémentation.
 */

void CApplication::importFromSongbird(void)
{
    //CImporterSongbird * dialog = new CImporterSongbird(this);
    //dialog->show();
}


/**
 * Ajoute une liste de lecture à la vue.
 * Si la liste a déjà été ajoutée, rien n'est fait.
 *
 * \param playList Pointeur sur la liste de lecture à ajouter.
 */

void CApplication::addPlayList(IPlayList * playList)
{
    Q_CHECK_PTR(playList);

    m_listModel->addPlayList(playList);

/*
    if (m_playLists.contains(playList))
    {
        initPlayList(playList);
        //playList->m_index = m_playListView->addSongTable(playList);
    }
    else
    {
        initPlayList(playList);
        playList->m_index = m_playListView->addSongTable(playList, m_playListView->getFolderModelIndex(playList->getFolder()));
    }
*/
}

/*
void CApplication::initPlayList(IPlayList * playList)
{
    Q_CHECK_PTR(playList);

    if (m_playLists.contains(playList))
    {
        // Liste de lecture dynamique
        CDynamicList * dynamicList = qobject_cast<CDynamicList *>(playList);

        if (dynamicList)
        {
            dynamicList->update();
        }

        return;
    }

    m_playLists.append(playList);

    // Ajout dans le panneau gauche
    //m_uiWidget->splitter->addWidget(playList);
    playList->hide();

    connect(playList, SIGNAL(songStarted(CSongTableItem *)), this, SLOT(playSong(CSongTableItem *)));
    connect(playList, SIGNAL(nameChanged(const QString&, const QString&)), m_playListView, SLOT(onPlayListRenamed(const QString&, const QString&)));
    connect(playList, SIGNAL(rowCountChanged()), this, SLOT(updateListInformations()));

    // Liste de lecture statique
    CStaticPlayList * staticList = qobject_cast<CStaticPlayList *>(playList);

    if (staticList)
    {
        connect(staticList, SIGNAL(songsAdded()), this, SLOT(updateListInformations()));
        connect(staticList, SIGNAL(songRemoved(CSong *)), this, SLOT(updateListInformations()));
    }

    // Liste de lecture dynamique
    CDynamicList * dynamicList = qobject_cast<CDynamicList *>(playList);

    if (dynamicList)
    {
        connect(dynamicList, SIGNAL(listUpdated()), this, SLOT(updateListInformations()));
        dynamicList->update();
    }
}
*/

void CApplication::addFolder(CFolder * folder)
{
    Q_CHECK_PTR(folder);

    m_listModel->addFolder(folder);
/*
    if (m_folders.contains(folder))
    {
        initFolder(folder);
    }
    else
    {
        initFolder(folder);
        folder->m_index = m_playListView->addFolder(folder, m_playListView->getFolderModelIndex(folder->getFolder()));
    }
*/
}

/*
void CApplication::initFolder(CFolder * folder)
{
    Q_CHECK_PTR(folder);

    if (m_folders.contains(folder))
    {
        return;
    }

    m_folders.append(folder);
    connect(folder, SIGNAL(nameChanged(const QString&, const QString&)), m_playListView, SLOT(onFolderRenamed(const QString&, const QString&)));

    //...
}
*/

/**
 * Ajoute un morceau à la médiathèque.
 * Le fichier doit être un son valide, et ne doit pas être déjà présent dans la médiathèque.
 *
 * \param fileName Fichier à charger.
 * \return Pointeur sur le morceau.
 */

CSong * CApplication::addSong(const QString& fileName)
{
    //qDebug() << "Chargement du fichier " << fileName;

    CSong * song = CSong::loadFromFile(this, fileName);
    if (song)
    {
        m_library->addSong(song);
        updateListInformations();
        emit songAdded(song);
        emit songsAdded();
    }

    return song;
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
 * Méthode appelée lorsqu'un morceau est modifié.
 * Le signal songModified est émis.
 */

void CApplication::onSongModified(void)
{
    CSong * song = qobject_cast<CSong *>(sender());

    if (song)
    {
        emit songModified(song);
    }
}


/**
 * Liste les morceaux contenus dans un répertoire.
 *
 * \param pathName Nom du répertoire à parcourir récursivement.
 * \return Liste des fichiers du répertoire.
 */

QStringList CApplication::importFolder(const QString& pathName)
{
    QStringList fileList;
    QDir dir(pathName);

    QStringList dirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (QStringList::const_iterator it = dirList.begin(); it != dirList.end(); ++it)
    {
        fileList.append(importFolder(dir.absoluteFilePath(*it)));
    }

    QStringList fileDirList = dir.entryList(QDir::Files | QDir::Readable, QDir::Name);

    for (QStringList::const_iterator it = fileDirList.begin(); it != fileDirList.end(); ++it)
    {
        fileList.append(QDir::toNativeSeparators(dir.absoluteFilePath(*it)));
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

    for (QList<CFolder *>::const_iterator it = m_folders.begin(); it != m_folders.end(); ++it)
    {
        const QList<IPlayList *> playLists = folder->getPlayLists();

        for (QList<IPlayList *>::const_iterator it2 = playLists.begin(); it2 != playLists.end(); ++it2)
        {
            CStaticPlayList * staticPlayList = dynamic_cast<CStaticPlayList *>(*it2);

            if (staticPlayList)
            {
                staticPlayList->removeSong(songItem->getSong());
            }
        }
    }

    for (QList<IPlayList *>::const_iterator it = m_playLists.begin(); it != m_playLists.end(); ++it)
    {
        CStaticPlayList * staticPlayList = dynamic_cast<CStaticPlayList *>(*it);

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
 * Ouvre la boite de dialogue pour modifier la liste de lecture ou le dossier selectionné dans la vue.
 */

void CApplication::editSelectedItem(void)
{
    IPlayList * playList = qobject_cast<IPlayList *>(m_playListView->getSelectedSongTable());

    if (playList)
    {
        CStaticPlayList * staticList = qobject_cast<CStaticPlayList *>(playList);

        if (staticList)
        {
            openDialogEditStaticPlayList(staticList);
        }
        else
        {
            CDynamicList * dynamicList = qobject_cast<CDynamicList *>(playList);

            if (dynamicList)
            {
                openDialogEditDynamicList(dynamicList);
            }
        }

        return;
    }

    CFolder * folder = qobject_cast<CFolder *>(m_playListView->getSelectedFolder());

    if (folder)
    {
        openDialogEditFolder(folder);
        return;
    }
}


/**
 * Supprime la liste de lecture ou le dossier sélectionné dans la vue CPlayListView.
 * Affiche une boite de dialogue de confirmation.
 *
 * \todo Gérer les dossiers.
 * \todo Gérer le cas où la liste est utilisée dans un critère d'une liste dynamique.
 */

void CApplication::removeSelectedItem(void)
{
    IPlayList * playList = qobject_cast<IPlayList *>(m_playListView->getSelectedSongTable());

    if (playList)
    {
        // Confirmation
        if (QMessageBox::question(this, QString(), tr("Are you sure you want to delete this playlist?"), tr("Yes"), tr("No"), 0, 1) == 1)
        {
            return;
        }

        if (playList == m_displayedSongTable)
        {
            displaySongTable(m_library);
        }

        if (playList == m_currentSongTable)
        {
            stop();
        }

        m_listModel->removePlayList(playList);
        return;
    }

    CFolder * folder = qobject_cast<CFolder *>(m_playListView->getSelectedFolder());

    if (folder)
    {
        // Confirmation
        CDialogRemoveFolder * dialog = new CDialogRemoveFolder(this, folder);

        if (dialog->exec() == QDialog::Rejected)
        {
            qDebug() << "Suppression du dossier annulée...";
            return;
        }

        m_listModel->removeFolder(folder, dialog->isResursive());
        delete dialog;
    }
}


/**
 * Méthode appelée quand la lecture d'un morceau se termine.
 */

void CApplication::onPlayEnd(void)
{
    if (m_currentSongItem)
    {
        //qDebug() << "CApplication::onPlayEnd()";
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
        m_uiControl->songInfos->setText(song->getTitle() + " - " + song->getArtistName() + " - " + song->getAlbumTitle());
        m_uiControl->sliderPosition->setEnabled(true);

        const int duration = song->getDuration();
        if (duration >= 0)
        {
            m_uiControl->sliderPosition->setRange(0, duration);

            QTime durationTime(0, 0);
            durationTime = durationTime.addMSecs(duration);
            m_uiControl->lblTime->setText(durationTime.toString("m:ss")); /// \todo Stocker dans les settings
        }
    }
    else
    {
        m_uiControl->songInfos->setText(""); // "Pas de morceau en cours de lecture..."
        m_uiControl->sliderPosition->setEnabled(false);
        m_uiControl->sliderPosition->setRange(0, 1000);
        m_uiControl->lblPosition->setText("0:00");
        m_uiControl->lblTime->setText("0:00");
    }

    m_uiControl->sliderPosition->setValue(0);
}


/**
 * Met à jour les informations sur la liste de morceaux affichée.
 */

void CApplication::updateListInformations(void)
{
    QTime duration(0, 0);
    int numSongs = 0;

    if (m_displayedSongTable)
    {
        duration = duration.addMSecs(m_displayedSongTable->getTotalDuration());
        numSongs = m_displayedSongTable->getNumSongs();
    }

    // Barre d'état
    m_listInfos->setText(tr("%n song(s), %1", "", numSongs).arg(duration.toString()));
}


/**
 * Met à jour la position de lecture depuis la position du curseur.
 */

void CApplication::updatePosition(void)
{
    setPosition(m_uiControl->sliderPosition->value());
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
                    //qDebug() << "Last.fm : update";
                    CUpdateNowPlaying * query = new CUpdateNowPlaying(this, m_lastFmKey, m_currentSongItem->getSong());
                    //updateLastFmNowPlaying(m_currentSongItem->getSong());
                    m_lastFmState = Notified;
                }
            }
            else if (m_lastFmState == Notified)
            {
                if (m_lastFmTimeListened > 4 * 60000 || m_lastFmTimeListened > m_currentSongItem->getSong()->getDuration() * m_percentageBeforeScrobbling / 100)
                {
                    //qDebug() << "Last.fm : scrobble";
                    CScrobble * query = new CScrobble(this, m_lastFmKey, m_currentSongItem->getSong());
                    m_lastFmState = Scrobbled;
                }
            }
        }

        if (m_currentSongItem->getSong()->isEnded())
        {
            //qDebug() << "m_currentSong->isEnded()";
            m_currentSongItem->getSong()->emitPlayEnd();
            return;
        }

        if (position >= 0)
        {
            if (!m_uiControl->sliderPosition->isSliderDown())
            {
                m_uiControl->sliderPosition->setValue(position);
            }

            QTime positionTime(0, 0);
            positionTime = positionTime.addMSecs(position);
            m_uiControl->lblPosition->setText(positionTime.toString("m:ss"));
        }
    }
}


void CApplication::selectPlayListFromTreeView(const QModelIndex& index)
{
    //qDebug() << "Changement de liste...";

    CSongTable * songTable = m_playListView->getSongTable(index);

    if (songTable)
    {
        displaySongTable(songTable);
    }
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
            //m_displayedSongTable->hide();
            m_displayedSongTable->setParent(NULL);
        }

        m_playListView->selectionModel()->clearSelection();
        m_playListView->selectionModel()->setCurrentIndex(m_playListView->getSongTableModelIndex(songTable), QItemSelectionModel::Select | QItemSelectionModel::Rows);

        m_displayedSongTable = songTable;
        setCentralWidget(m_displayedSongTable);
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
 * \todo Utiliser des méthodes dédiées dans les classes CSongTable et CFolder.
 */

void CApplication::loadDatabase(void)
{
    QSqlQuery query(m_dataBase);


    // Création des relations
    QStringList tables = m_dataBase.tables(QSql::Tables);

    if (m_dataBase.driverName() == "QSQLITE")
    {
        if (!tables.contains("folder"))
        {
            if (!query.exec("CREATE TABLE folder ("
                                "folder_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                                "folder_name VARCHAR NOT NULL,"
                                "folder_parent INTEGER NOT NULL,"
                                "folder_position INTEGER NOT NULL,"
                                "folder_expanded INTEGER NOT NULL"
                                //",UNIQUE (folder_parent, folder_position)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            if (!query.exec("INSERT INTO folder VALUES (0, '', 0, 1, 1)"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("playlist"))
        {
            if (!query.exec("CREATE TABLE playlist ("
                                "playlist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                                "playlist_name VARCHAR NOT NULL,"
                                "folder_id INTEGER NOT NULL,"
                                "list_position INTEGER NOT NULL,"
                                "list_columns VARCHAR NOT NULL"
                                //",UNIQUE (folder_id, list_position)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            if (!query.exec("INSERT INTO playlist (playlist_id, playlist_name, folder_id, list_position, list_columns) "
                            "VALUES (0, 'Library', 0, 0, '')"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("dynamic_list"))
        {
            if (!query.exec("CREATE TABLE dynamic_list ("
                                "dynamic_list_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                                "criteria_id INTEGER NOT NULL,"
                                "playlist_id INTEGER NOT NULL,"
                                "UNIQUE (playlist_id)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("criteria"))
        {
            if (!query.exec("CREATE TABLE criteria ("
                                "criteria_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                                "dynamic_list_id INTEGER NOT NULL,"
                                "criteria_parent INTEGER NOT NULL,"
                                "criteria_position INTEGER NOT NULL,"
                                "criteria_type INTEGER NOT NULL,"
                                "criteria_condition INTEGER NOT NULL,"
                                "criteria_value1 VARCHAR,"
                                "criteria_value2 VARCHAR,"
                                "UNIQUE (dynamic_list_id, criteria_parent, criteria_position)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("static_list"))
        {
            if (!query.exec("CREATE TABLE static_list ("
                                "static_list_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                                "playlist_id INTEGER NOT NULL,"
                                "UNIQUE (playlist_id)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("static_list_song"))
        {
            if (!query.exec("CREATE TABLE static_list_song ("
                                "static_list_id INTEGER NOT NULL,"
                                "song_id INTEGER NOT NULL,"
                                "song_position INTEGER NOT NULL,"
                                "UNIQUE (static_list_id, song_position)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("song"))
        {
            if (!query.exec("CREATE TABLE song ("
                                "song_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                                "song_filename VARCHAR NOT NULL UNIQUE,"
                                "song_filesize INTEGER NOT NULL,"
                                "song_bitrate INTEGER NOT NULL,"
                                "song_sample_rate INTEGER NOT NULL,"
                                "song_format INTEGER NOT NULL,"
                                "song_channels INTEGER NOT NULL,"
                                "song_duration INTEGER NOT NULL,"
                                "song_creation DATETIME NOT NULL,"
                                "song_modification DATETIME NOT NULL,"
                                "song_enabled INTEGER NOT NULL,"
                                "song_title VARCHAR NOT NULL,"
                                "song_title_sort VARCHAR NOT NULL,"
                                "song_subtitle VARCHAR NOT NULL,"
                                "song_grouping VARCHAR NOT NULL,"
                                "artist_id INTEGER NOT NULL,"
                                "album_id INTEGER NOT NULL,"
                                "album_artist_id INTEGER NOT NULL,"
                                "song_composer VARCHAR NOT NULL,"
                                "song_composer_sort VARCHAR NOT NULL,"
                                "song_year INTEGER NOT NULL,"
                                "song_track_number INTEGER NOT NULL,"
                                "song_track_count INTEGER NOT NULL,"
                                "song_disc_number INTEGER NOT NULL,"
                                "song_disc_count INTEGER NOT NULL,"
                                "genre_id INTEGER NOT NULL,"
                                "song_rating INTEGER NOT NULL,"
                                "song_comments VARCHAR NOT NULL,"
                                "song_bpm INTEGER NOT NULL,"
                                "song_lyrics TEXT NOT NULL,"
                                "song_language VARCHAR(2) NOT NULL,"
                                "song_lyricist VARCHAR NOT NULL,"
                                "song_compilation INTEGER NOT NULL,"
                                "song_skip_shuffle INTEGER NOT NULL,"
                                "song_play_count INTEGER NOT NULL,"
                                "song_play_time TIMESTAMP,"
                                "song_play_time_utc TIMESTAMP,"
                                "song_track_gain FLOAT,"
                                "song_track_peak FLOAT,"
                                "song_album_gain FLOAT,"
                                "song_album_peak FLOAT"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("album"))
        {
            if (!query.exec("CREATE TABLE album ("
                                "album_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                                "album_title VARCHAR NOT NULL,"
                                "album_title_sort VARCHAR,"
                                "UNIQUE (album_title, album_title_sort)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            if (!query.exec("INSERT INTO album (album_id, album_title, album_title_sort) VALUES (0, '', '')"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("artist"))
        {
            if (!query.exec("CREATE TABLE artist ("
                                "artist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                                "artist_name VARCHAR NOT NULL,"
                                "artist_name_sort VARCHAR,"
                                "UNIQUE (artist_name, artist_name_sort)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            if (!query.exec("INSERT INTO artist (artist_id, artist_name, artist_name_sort) VALUES (0, '', '')"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("genre"))
        {
            if (!query.exec("CREATE TABLE genre ("
                                "genre_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                                "genre_name VARCHAR NOT NULL UNIQUE"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            if (!query.exec("INSERT INTO genre (genre_id, genre_name) VALUES (0, '')"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("play"))
        {
            if (!query.exec("CREATE TABLE play ("
                                "song_id INTEGER NOT NULL,"
                                "play_time TIMESTAMP,"
                                "play_time_utc TIMESTAMP"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }
    }
    else if (m_dataBase.driverName() == "QMYSQL")
    {
        if (!tables.contains("folder"))
        {
            if (!query.exec("CREATE TABLE folder ("
                                "folder_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                                "folder_name VARCHAR NOT NULL,"
                                "folder_parent INTEGER NOT NULL,"
                                "folder_position INTEGER NOT NULL,"
                                "folder_expanded INTEGER NOT NULL"
                                //",UNIQUE (folder_parent, folder_position)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            if (!query.exec("INSERT INTO folder VALUES (0, '', 0, 1, 1)"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("playlist"))
        {
            if (!query.exec("CREATE TABLE playlist ("
                                "playlist_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                                "playlist_name VARCHAR NOT NULL,"
                                "folder_id INTEGER NOT NULL,"
                                "list_position INTEGER NOT NULL,"
                                "list_columns VARCHAR NOT NULL"
                                //",UNIQUE (folder_id, list_position)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            if (!query.exec("INSERT INTO playlist (playlist_id, playlist_name, folder_id, list_position, list_columns) "
                            "VALUES (0, 'Library', 0, 0, '')"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("dynamic_list"))
        {
            if (!query.exec("CREATE TABLE dynamic_list ("
                                "dynamic_list_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                                "criteria_id INTEGER NOT NULL,"
                                "playlist_id INTEGER NOT NULL,"
                                "UNIQUE (playlist_id)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("criteria"))
        {
            if (!query.exec("CREATE TABLE criteria ("
                                "criteria_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                                "dynamic_list_id INTEGER NOT NULL,"
                                "criteria_parent INTEGER NOT NULL,"
                                "criteria_position INTEGER NOT NULL,"
                                "criteria_type INTEGER NOT NULL,"
                                "criteria_condition INTEGER NOT NULL,"
                                "criteria_value1 VARCHAR,"
                                "criteria_value2 VARCHAR,"
                                "UNIQUE (dynamic_list_id, criteria_parent, criteria_position)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("static_list"))
        {
            if (!query.exec("CREATE TABLE static_list ("
                                "static_list_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                                "playlist_id INTEGER NOT NULL,"
                                "UNIQUE (playlist_id)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("static_list_song"))
        {
            if (!query.exec("CREATE TABLE static_list_song ("
                                "static_list_id INTEGER NOT NULL,"
                                "song_id INTEGER NOT NULL,"
                                "song_position INTEGER NOT NULL,"
                                "UNIQUE (static_list_id, song_position)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("song"))
        {
            if (!query.exec("CREATE TABLE song ("
                                "song_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                                "song_filename VARCHAR NOT NULL UNIQUE,"
                                "song_filesize INTEGER NOT NULL,"
                                "song_bitrate INTEGER NOT NULL,"
                                "song_sample_rate INTEGER NOT NULL,"
                                "song_format INTEGER NOT NULL,"
                                "song_channels INTEGER NOT NULL,"
                                "song_duration INTEGER NOT NULL,"
                                "song_creation DATETIME NOT NULL,"
                                "song_modification DATETIME NOT NULL,"
                                "song_enabled INTEGER NOT NULL,"
                                "song_title VARCHAR NOT NULL,"
                                "song_title_sort VARCHAR NOT NULL,"
                                "song_subtitle VARCHAR NOT NULL,"
                                "song_grouping VARCHAR NOT NULL,"
                                "artist_id INTEGER NOT NULL,"
                                "album_id INTEGER NOT NULL,"
                                "album_artist_id INTEGER NOT NULL,"
                                "song_composer VARCHAR NOT NULL,"
                                "song_composer_sort VARCHAR NOT NULL,"
                                "song_year INTEGER NOT NULL,"
                                "song_track_number INTEGER NOT NULL,"
                                "song_track_count INTEGER NOT NULL,"
                                "song_disc_number INTEGER NOT NULL,"
                                "song_disc_count INTEGER NOT NULL,"
                                "genre_id INTEGER NOT NULL,"
                                "song_rating INTEGER NOT NULL,"
                                "song_comments VARCHAR NOT NULL,"
                                "song_bpm INTEGER NOT NULL,"
                                "song_lyrics TEXT NOT NULL,"
                                "song_language VARCHAR(2) NOT NULL,"
                                "song_lyricist VARCHAR NOT NULL,"
                                "song_compilation INTEGER NOT NULL,"
                                "song_skip_shuffle INTEGER NOT NULL,"
                                "song_play_count INTEGER NOT NULL,"
                                "song_play_time TIMESTAMP,"
                                "song_play_time_utc TIMESTAMP,"
                                "song_track_gain FLOAT,"
                                "song_track_peak FLOAT,"
                                "song_album_gain FLOAT,"
                                "song_album_peak FLOAT"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("album"))
        {
            if (!query.exec("CREATE TABLE album ("
                                "album_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                                "album_title VARCHAR NOT NULL,"
                                "album_title_sort VARCHAR,"
                                "UNIQUE (album_title, album_title_sort)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            if (!query.exec("INSERT INTO album (album_id, album_title, album_title_sort) VALUES (0, '', '')"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("artist"))
        {
            if (!query.exec("CREATE TABLE artist ("
                                "artist_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                                "artist_name VARCHAR NOT NULL,"
                                "artist_name_sort VARCHAR,"
                                "UNIQUE (artist_name, artist_name_sort)"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            if (!query.exec("INSERT INTO artist (artist_id, artist_name, artist_name_sort) VALUES (0, '', '')"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("genre"))
        {
            if (!query.exec("CREATE TABLE genre ("
                                "genre_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                                "genre_name VARCHAR NOT NULL UNIQUE"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            if (!query.exec("INSERT INTO genre (genre_id, genre_name) VALUES (0, '')"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }

        if (!tables.contains("play"))
        {
            if (!query.exec("CREATE TABLE play ("
                                "song_id INTEGER NOT NULL,"
                                "play_time TIMESTAMP,"
                                "play_time_utc TIMESTAMP"
                            ")"))
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
        }
    }


    // Création des vues
    tables = m_dataBase.tables(QSql::Views);

    if (!tables.contains("albums"))
    {
        if (!query.exec("CREATE VIEW albums AS SELECT DISTINCT(album_title) FROM album NATURAL JOIN song WHERE song_id IS NOT NULL"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("artists"))
    {
        if (!query.exec("CREATE VIEW artists AS SELECT DISTINCT(artist_name) FROM artist NATURAL JOIN song WHERE song_id IS NOT NULL"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("genres"))
    {
        if (!query.exec("CREATE VIEW genres AS SELECT DISTINCT(genre_name) FROM genre NATURAL JOIN song WHERE song_id IS NOT NULL"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }


    // Création de la médiathèque
    m_library = new CLibrary(this);
    m_library->m_idPlayList = 0;
    //m_uiWidget->splitter->addWidget(m_library);
    setCentralWidget(m_library);
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

    //m_playListView->setCurrentIndex(m_playListView->addSongTable(m_library));


    // Liste des morceaux
    QList<CSong *> songList = CSong::loadAllSongsFromDatabase(this);
    m_library->addSongs(songList);


    // Chargement des listes de lecture et des dossiers
    m_listModel = new CListModel(this);
    m_listModel->loadFromDatabase();

    m_playListView->setModel(m_listModel);


    displaySongTable(m_library);


#if 0

    // Création des dossiers
    if (!query.exec("SELECT folder_id, folder_name, folder_parent, folder_position, folder_expanded FROM folder /*WHERE folder_id != 0*/ ORDER BY folder_position"))
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        while (query.next())
        {
            CFolder * folder = new CFolder(this, query.value(1).toString());
            folder->m_id       = query.value(0).toInt();
            folder->m_folder   = reinterpret_cast<CFolder *>(query.value(2).toInt());
            folder->m_position = query.value(3).toInt();
            folder->m_open     = query.value(4).toBool();

            if (folder->m_folder < 0)
            {
                logError("le dossier parent a un identifiant invalide", __FUNCTION__, __FILE__, __LINE__);
            }

            initFolder(folder);
        }
    }

    // On déplace les dossiers dans l'arborescence
    for (QList<CFolder *>::const_iterator it = m_folders.begin(); it != m_folders.end(); ++it)
    {
        long folderId = reinterpret_cast<long>((*it)->m_folder);
        if (folderId >= 0)
        {
            if ((*it)->getId() != 0)
            {
                (*it)->m_folder = getFolderFromId(folderId);
                (*it)->m_folder->m_folders.append(*it);
            }
        }
        else
        {
            logError("le dossier parent a un identifiant invalide", __FUNCTION__, __FILE__, __LINE__);
        }
    }


    // Création des listes de lecture statiques
    if (!query.exec("SELECT static_list_id, playlist_name, list_columns, playlist_id, folder_id "
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

        // Dossier contenant la liste
        int folderId = query.value(4).toInt();
        if (folderId >= 0)
        {
            playList->m_folder = getFolderFromId(folderId);
            playList->m_folder->m_playLists.append(playList);
        }
        else
        {
            logError("le dossier parent a un identifiant invalide", __FUNCTION__, __FILE__, __LINE__);
        }

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

        initPlayList(playList);
    }


    // Création des listes de lecture dynamiques
    if (!query.exec("SELECT dynamic_list_id, playlist_name, list_columns, playlist_id, folder_id, list_position "
                    "FROM dynamic_list NATURAL JOIN playlist ORDER BY list_position"))
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    while (query.next())
    {
        CDynamicList * playList = new CDynamicList(this, query.value(1).toString());
        playList->m_id = query.value(0).toInt();
        playList->m_idPlayList = query.value(3).toInt();
        playList->initColumns(query.value(2).toString());

        // Dossier contenant la liste
        int folderId = query.value(4).toInt();
        if (folderId >= 0)
        {
            playList->m_folder = getFolderFromId(folderId);
            playList->m_folder->m_playLists.append(playList);
        }
        else
        {
            logError("le dossier parent a un identifiant invalide", __FUNCTION__, __FILE__, __LINE__);
        }

        playList->loadFromDatabase();

        initPlayList(playList);
    }


    // Remplissage du modèle
    for (QList<CFolder *>::const_iterator it = m_folders.begin(); it != m_folders.end(); ++it)
    {
        if (!(*it)->m_folder)
        {
            /*(*it)->m_index = */m_playListView->addFolder(*it);
        }
    }

    for (QList<IPlayList *>::const_iterator it = m_playLists.begin(); it != m_playLists.end(); ++it)
    {
        if (!(*it)->m_folder)
        {
            /*(*it)->m_index = */m_playListView->addSongTable(*it);
        }
    }

#endif
}


/**
 * Démarre la lecture du morceau.
 */

void CApplication::startPlay(void)
{
    //qDebug() << "CApplication::startPlay()";

    Q_CHECK_PTR(m_currentSongItem);
    Q_CHECK_PTR(m_currentSongTable);

    setState(Playing);
    //m_uiControl->btnPlay->setIcon(QPixmap(":/icons/pause"));

    m_currentSongItem->getSong()->play();
    emit songPlayStart(m_currentSongItem->getSong());
    connect(m_currentSongItem->getSong(), SIGNAL(playEnd()), this, SLOT(onPlayEnd()), Qt::UniqueConnection);

    updateSongDescription(m_currentSongItem->getSong());
    m_currentSongTable->m_model->setCurrentSong(m_currentSongItem);

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


void CApplication::setState(State state)
{
    switch (state)
    {
        case Playing:
            m_uiControl->btnPlay->setIcon(QPixmap(":/icons/pause"));
            m_uiControl->btnStop->setEnabled(true);
            m_uiControl->btnPrevious->setEnabled(true);
            m_uiControl->btnNext->setEnabled(true);
            m_uiWidget->actionPlay->setText(tr("Pause"));
            break;

        case Paused:
            m_uiControl->btnPlay->setIcon(QPixmap(":/icons/play"));
            m_uiControl->btnStop->setEnabled(true);
            m_uiControl->btnPrevious->setEnabled(true);
            m_uiControl->btnNext->setEnabled(true);
            m_uiWidget->actionPlay->setText(tr("Play"));
            break;

        case Stopped:
            m_uiControl->btnPlay->setIcon(QPixmap(":/icons/play"));
            m_uiControl->btnStop->setEnabled(false);
            m_uiControl->btnPrevious->setEnabled(false);
            m_uiControl->btnNext->setEnabled(false);
            m_uiWidget->actionPlay->setText(tr("Play"));
            break;
    }
}


void CApplication::keyPressEvent(QKeyEvent * event)
{
    //qDebug() << "CApplication::keyPressEvent() : " << event->key();
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

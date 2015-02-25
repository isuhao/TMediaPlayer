/*
Copyright (C) 2012-2015 Teddy Michel

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

#include "CMainWindow.hpp"
#include "CMediaManager.hpp"
#include "CSong.hpp"
#include "CStaticList.hpp"
#include "CDynamicList.hpp"
#include "CFolder.hpp"
#include "CLibraryView.hpp"
#include "CLibraryModel.hpp"
#include "CLyricWiki.hpp"
#include "Dialog/CDialogEditDynamicList.hpp"
#include "Dialog/CDialogEditFolder.hpp"
#include "Dialog/CDialogEditMetadata.hpp"
#include "Dialog/CDialogEditSong.hpp"
#include "Dialog/CDialogEditSongs.hpp"
#include "Dialog/CDialogEditStaticPlayList.hpp"
#include "Dialog/CDialogPreferences.hpp"
#include "Dialog/CDialogEqualizer.hpp"
#include "Dialog/CDialogEffects.hpp"
#include "Dialog/CDialogNotifications.hpp"
#include "Dialog/CDialogLastPlays.hpp"
#include "Dialog/CDialogRemoveFolder.hpp"
#include "Dialog/CDialogAbout.hpp"
#include "Importer/CImporterITunes.hpp"
#include "CLibrary.hpp"
#include "CSliderStyle.hpp"
#include "CWidgetLyrics.hpp"
#include "CCDRomDrive.hpp"
#include "CQueuePlayList.hpp"
#include "Utils.hpp"

// Last.fm
#include "Last.fm/CAuthentication.hpp"
#include "Last.fm/CUpdateNowPlaying.hpp"
#include "Last.fm/CScrobble.hpp"
#include "Last.fm/CGetRecentTracks.hpp"

// Qt
#include <QStandardItemModel>
#include <QSettings>
#include <QMessageBox>
#include <QActionGroup>
#include <QKeyEvent>
#include <QSqlError>
#include <QSqlQuery>
#include <QFileDialog>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QDockWidget>

#ifndef T_NO_SINGLE_APP
#  include <QLocalServer>
#  include <QLocalSocket>
#endif

// FMOD
#include <fmod/fmod.hpp>

// DEBUG
#include <QtDebug>
#include <QSqlDriver>


const int timerPeriod = 250; ///< Intervalle entre chaque mise-à-jour des informations.
const int timerCDRomDrivesPeriod = 5000; ///< Intervalle entre chaque mise-à-jour des lecteurs de CD-Rom.


/**
 * Constructeur de la classe principale de l'application.
 *
 * \param mediaManager Pointeur sur le gestionnaire de médias.
 */

CMainWindow::CMainWindow(CMediaManager * mediaManager) :
QMainWindow            (nullptr),
m_mediaManager         (mediaManager),

#ifndef T_NO_SINGLE_APP
m_localServer          (nullptr),
#endif

m_uiWidget             (new Ui::TMediaPlayer()),
m_uiControl            (new Ui::WidgetControl()),
m_queue                (nullptr),
m_playListView         (nullptr),
m_listModel            (nullptr),
m_dialogEditSong       (nullptr),
m_timer                (this),
m_timerCDRomDrives     (this),
m_listInfos            (nullptr),
m_currentSongItem      (nullptr),
m_currentSongTable     (nullptr),
m_library              (nullptr),
m_displayedSongTable   (nullptr),
m_widgetLyrics         (nullptr),
m_state                (Stopped),
m_showRemainingTime    (false),
m_repeatMode           (NoRepeat),

m_dialogNotifications  (nullptr),
m_dialogLastPlays      (nullptr),

// Last.fm
m_lastFmEnableScrobble       (false),
m_delayBeforeNotification    (5000),
m_percentageBeforeScrobbling (60),
m_lastFmTimeListened         (0),
m_lastFmLastPosition         (0),
m_lastFmState                (NoScrobble)
{
    Q_CHECK_PTR(m_mediaManager);

#ifndef T_NO_SINGLE_APP
    m_localServer = new QLocalServer(this);
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(activateThisWindow()));
    m_localServer->listen("tmediaplayer-" + CMediaManager::getAppVersion());
#endif // T_NO_SINGLE_APP

    connect(m_mediaManager, SIGNAL(informationNotified(const QString&)), this, SLOT(notifyInformation2(const QString&)));

    // Last.fm
    m_lastFmEnableScrobble = m_mediaManager->getSettings()->value("LastFm/EnableScrobble", false).toBool();
    m_delayBeforeNotification = m_mediaManager->getSettings()->value("LastFm/DelayBeforeNotification", 5000).toInt();
    m_percentageBeforeScrobbling = m_mediaManager->getSettings()->value("LastFm/PercentageBeforeScrobbling", 60).toInt();
    m_lastFmKey = m_mediaManager->getSettings()->value("LastFm/SessionKey", "").toByteArray();

    // Timers
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateTimer()));
    connect(&m_timerCDRomDrives, SIGNAL(timeout()), this, SLOT(updateCDRomDrives()));
}


/**
 * Libère les ressources utilisées par l'application.
 */

CMainWindow::~CMainWindow()
{
    m_timer.stop();
    m_timerCDRomDrives.stop();

    // Enregistrement des paramètres
    m_mediaManager->getSettings()->setValue("Preferences/Repeat", m_repeatMode);

    //dumpObjectTree();

    delete m_listModel;
    delete m_queue;

    // Destruction des lecteurs de CD-ROM
    for (QList<CCDRomDrive *>::ConstIterator drive = m_cdRomDrives.begin(); drive != m_cdRomDrives.end(); ++drive)
    {
        delete *drive;
    }

    m_cdRomDrives.clear();

    // Destruction de la médiathèque
    if (m_library)
    {
        m_library->updateDatabase();
        m_library->deleteSongs();
        delete m_library;
    }

    delete m_uiWidget;
}


/**
 * Initialise l'interface graphique et charge les données.
 */

bool CMainWindow::initWindow()
{
    static bool init = false;

    if (init)
    {
        m_mediaManager->logError(tr("the application has already been initialized"), __FUNCTION__, __FILE__, __LINE__);
        return true;
    }


    // Pour organiser les docks sur plusieurs lignes ou colonnes
    setDockNestingEnabled(true);

    // Initialisation de l'interface graphique
    m_uiWidget->setupUi(this);
    m_uiWidget->actionTogglePlay->setShortcut(Qt::Key_Space);

    // Barre d'état
    QTime duration(0, 0);
    m_listInfos = new QLabel(tr("%n song(s), %1", "", 0).arg(duration.toString()));
    statusBar()->addPermanentWidget(m_listInfos);

    // Menus
#if QT_VERSION >= 0x050000
    connect(m_uiWidget->actionNewPlayList       , &QAction::triggered, this, &CMainWindow::openDialogCreateStaticList_Slot );
    connect(m_uiWidget->actionNewDynamicPlayList, &QAction::triggered, this, &CMainWindow::openDialogCreateDynamicList_Slot);
    connect(m_uiWidget->actionNewFolder         , &QAction::triggered, this, &CMainWindow::openDialogCreateFolder_Slot     );
    connect(m_uiWidget->actionAddFiles          , &QAction::triggered, this, &CMainWindow::openDialogAddSongs         );
    connect(m_uiWidget->actionAddFolder         , &QAction::triggered, this, &CMainWindow::openDialogAddFolder        );
    connect(m_uiWidget->actionInformations      , &QAction::triggered, this, &CMainWindow::openDialogSongInfos        );
    connect(m_uiWidget->actionOpenInExplorer    , &QAction::triggered, this, &CMainWindow::openSongInExplorer         );
    connect(m_uiWidget->actionImportITunes      , &QAction::triggered, this, &CMainWindow::importFromITunes           );
    connect(m_uiWidget->actionImportSongbird    , &QAction::triggered, this, &CMainWindow::importFromSongbird         );
    connect(m_uiWidget->actionNotifications     , &QAction::triggered, this, &CMainWindow::openDialogNotifications    );
    connect(m_uiWidget->actionLastPlays         , &QAction::triggered, this, &CMainWindow::openDialogLastPlays        );

    connect(m_uiWidget->actionSelectAll         , &QAction::triggered, this, &CMainWindow::selectAll                  );
    connect(m_uiWidget->actionSelectNone        , &QAction::triggered, this, &CMainWindow::selectNone                 );
    connect(m_uiWidget->actionPreferences       , &QAction::triggered, this, &CMainWindow::openDialogPreferences      );

    connect(m_uiWidget->actionTogglePlay        , &QAction::triggered, this, &CMainWindow::togglePlay                 );
    connect(m_uiWidget->actionStop              , &QAction::triggered, this, &CMainWindow::stop                       );
    connect(m_uiWidget->actionPrevious          , &QAction::triggered, this, &CMainWindow::previousSong               );
    connect(m_uiWidget->actionNext              , &QAction::triggered, this, &CMainWindow::nextSong                   );

#if __cplusplus < 201103L
    connect(m_uiWidget->actionNoRepeat          , &QAction::triggered, this, &CMainWindow::setRepeatModeNoRepeat      );
    connect(m_uiWidget->actionRepeatList        , &QAction::triggered, this, &CMainWindow::setRepeatModeRepeatList    );
    connect(m_uiWidget->actionRepeatSong        , &QAction::triggered, this, &CMainWindow::setRepeatModeRepeatSong    );
#else
    connect(m_uiWidget->actionNoRepeat          , &QAction::triggered, [=](){ this->setRepeatMode(NoRepeat  ); });
    connect(m_uiWidget->actionRepeatList        , &QAction::triggered, [=](){ this->setRepeatMode(RepeatList); });
    connect(m_uiWidget->actionRepeatSong        , &QAction::triggered, [=](){ this->setRepeatMode(RepeatSong); });
#endif

    connect(m_uiWidget->actionShuffle           , &QAction::triggered, this, &CMainWindow::setShuffle                 );
    connect(m_uiWidget->actionMute              , &QAction::triggered, this, &CMainWindow::setMute                    );
    connect(m_uiWidget->actionEqualizer         , &QAction::triggered, this, &CMainWindow::openDialogEqualizer        );
    connect(m_uiWidget->actionEffects           , &QAction::triggered, this, &CMainWindow::openDialogEffects          );

    //connect(m_uiWidget->actionAboutQt           , &QAction::triggered, qApp, &QApplication::aboutQt                    );
    connect(m_uiWidget->actionAboutQt           , SIGNAL(triggered(    )), qApp, SLOT(aboutQt                    ()));
    connect(m_uiWidget->actionAbout             , &QAction::triggered, this, &CMainWindow::openDialogAbout            );


    connect(this, &CMainWindow::songPlayStart, this, &CMainWindow::updateSongDescription);
#else
    connect(m_uiWidget->actionNewPlayList       , SIGNAL(triggered(    )), this, SLOT(openDialogCreateStaticList ()));
    connect(m_uiWidget->actionNewDynamicPlayList, SIGNAL(triggered(    )), this, SLOT(openDialogCreateDynamicList()));
    connect(m_uiWidget->actionNewFolder         , SIGNAL(triggered(    )), this, SLOT(openDialogCreateFolder     ()));
    connect(m_uiWidget->actionAddFiles          , SIGNAL(triggered(    )), this, SLOT(openDialogAddSongs         ()));
    connect(m_uiWidget->actionAddFolder         , SIGNAL(triggered(    )), this, SLOT(openDialogAddFolder        ()));
    connect(m_uiWidget->actionInformations      , SIGNAL(triggered(    )), this, SLOT(openDialogSongInfos        ()));
    connect(m_uiWidget->actionOpenInExplorer    , SIGNAL(triggered(    )), this, SLOT(openSongInExplorer         ()));
    connect(m_uiWidget->actionImportITunes      , SIGNAL(triggered(    )), this, SLOT(importFromITunes           ()));
    connect(m_uiWidget->actionImportSongbird    , SIGNAL(triggered(    )), this, SLOT(importFromSongbird         ()));
    connect(m_uiWidget->actionNotifications     , SIGNAL(triggered(    )), this, SLOT(openDialogNotifications    ()));
    connect(m_uiWidget->actionLastPlays         , SIGNAL(triggered(    )), this, SLOT(openDialogLastPlays        ()));

    connect(m_uiWidget->actionSelectAll         , SIGNAL(triggered(    )), this, SLOT(selectAll                  ()));
    connect(m_uiWidget->actionSelectNone        , SIGNAL(triggered(    )), this, SLOT(selectNone                 ()));
    connect(m_uiWidget->actionPreferences       , SIGNAL(triggered(    )), this, SLOT(openDialogPreferences      ()));

    connect(m_uiWidget->actionTogglePlay        , SIGNAL(triggered(    )), this, SLOT(togglePlay                 ()));
    connect(m_uiWidget->actionStop              , SIGNAL(triggered(    )), this, SLOT(stop                       ()));
    connect(m_uiWidget->actionPrevious          , SIGNAL(triggered(    )), this, SLOT(previousSong               ()));
    connect(m_uiWidget->actionNext              , SIGNAL(triggered(    )), this, SLOT(nextSong                   ()));
    connect(m_uiWidget->actionNoRepeat          , SIGNAL(triggered(    )), this, SLOT(setRepeatModeNoRepeat      ()));
    connect(m_uiWidget->actionRepeatList        , SIGNAL(triggered(    )), this, SLOT(setRepeatModeRepeatList    ()));
    connect(m_uiWidget->actionRepeatSong        , SIGNAL(triggered(    )), this, SLOT(setRepeatModeRepeatSong    ()));
    connect(m_uiWidget->actionShuffle           , SIGNAL(triggered(bool)), this, SLOT(setShuffle             (bool)));
    connect(m_uiWidget->actionMute              , SIGNAL(triggered(bool)), this, SLOT(setMute                (bool)));
    connect(m_uiWidget->actionEqualizer         , SIGNAL(triggered(    )), this, SLOT(openDialogEqualizer        ()));
    connect(m_uiWidget->actionEffects           , SIGNAL(triggered(    )), this, SLOT(openDialogEffects          ()));

    connect(m_uiWidget->actionAboutQt           , SIGNAL(triggered(    )), qApp, SLOT(aboutQt                    ()));
    connect(m_uiWidget->actionAbout             , SIGNAL(triggered(    )), this, SLOT(openDialogAbout            ()));


    connect(this, SIGNAL(songPlayStart(CSong *)), this, SLOT(updateSongDescription(CSong *)));
#endif

    QActionGroup * repeatActionGroup = new QActionGroup(this);
    repeatActionGroup->addAction(m_uiWidget->actionNoRepeat);
    repeatActionGroup->addAction(m_uiWidget->actionRepeatList);
    repeatActionGroup->addAction(m_uiWidget->actionRepeatSong);


    // Barre de contrôle
    QWidget * widgetControl = new QWidget(this);
    m_uiControl->setupUi(widgetControl);
    m_uiWidget->toolBar->addWidget(widgetControl);
    m_uiControl->sliderPosition->setStyle(new CSliderStyle);

    m_uiControl->btnStop->setVisible(m_mediaManager->getSettings()->value("Preferences/ShowButtonStop", true).toBool());

    m_showRemainingTime = m_mediaManager->getSettings()->value("Preferences/ShowRemainingTime", false).toBool();

    // Connexions des signaux et des slots
    connect(m_uiControl->songInfos, SIGNAL(clicked()), this, SLOT(selectCurrentSong()));

    connect(m_uiControl->btnTogglePlay, SIGNAL(clicked()), this, SLOT(togglePlay()));
    connect(m_uiControl->btnStop, SIGNAL(clicked()), this, SLOT(stop()));

#if QT_VERSION < 0x050000
    connect(m_uiControl->btnPrevious, SIGNAL(clicked()), this, SLOT(previousSong())     );
    connect(m_uiControl->btnNext    , SIGNAL(clicked()), this, SLOT(nextSong())         );

    connect(m_uiControl->btnRepeat  , SIGNAL(clicked()), this, SLOT(setNextRepeatMode()));
    connect(m_uiControl->btnShuffle , SIGNAL(clicked()), this, SLOT(setShuffle())       );
    connect(m_uiControl->btnMute    , SIGNAL(clicked()), this, SLOT(toggleMute())       );
#else
    connect(m_uiControl->btnPrevious, &QToolButton::clicked, this, &CMainWindow::previousSong     );
    connect(m_uiControl->btnNext    , &QToolButton::clicked, this, &CMainWindow::nextSong         );

    connect(m_uiControl->btnRepeat  , &QToolButton::clicked, this, &CMainWindow::setNextRepeatMode);
    connect(m_uiControl->btnShuffle , &QToolButton::clicked, this, &CMainWindow::setShuffle       );
    connect(m_uiControl->btnMute    , &QToolButton::clicked, this, &CMainWindow::toggleMute       );
#endif

    connect(m_uiControl->btnClearFilter, SIGNAL(clicked()), this, SLOT(clearFilter()));

    // Sliders
    connect(m_uiControl->sliderVolume, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
    connect(m_uiControl->sliderPosition, SIGNAL(sliderReleased()), this, SLOT(updatePosition()));

    // Filtre
    connect(m_uiControl->editFilter, SIGNAL(textEdited(const QString&)), this, SLOT(onFilterChange(const QString&)));


    // Dock "Playlists"
    m_playListView = new CLibraryView(this);

    QDockWidget * dockPlayLists = new QDockWidget(tr("Playlists"), this);
    dockPlayLists->setObjectName("dock_playlists");
    dockPlayLists->setWidget(m_playListView);
    dockPlayLists->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    connect(m_playListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectPlayListFromTreeView(const QModelIndex&)));

    addDockWidget(Qt::LeftDockWidgetArea, dockPlayLists);


    // Dock "Lyrics"
    m_widgetLyrics = new CWidgetLyrics(this);

    QDockWidget * dockLyrics = new QDockWidget(tr("Lyrics"), this);
    dockLyrics->setObjectName("dock_lyrics");
    dockLyrics->setWidget(m_widgetLyrics);
    dockLyrics->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    addDockWidget(Qt::RightDockWidgetArea, dockLyrics);


    restoreGeometry(m_mediaManager->getSettings()->value("Window/WindowGeometry").toByteArray());
    restoreState(m_mediaManager->getSettings()->value("Window/WindowState").toByteArray());


    // Initialisation de FMOD
    initSoundSystem();

    // Paramètres de lecture
    setVolume(m_mediaManager->getVolume());
    setShuffle(m_mediaManager->getSettings()->value("Preferences/Shuffle", false).toBool());

    int repeatModeNum = m_mediaManager->getSettings()->value("Preferences/Repeat", 0).toInt();

    switch (repeatModeNum)
    {
        default:
        case NoRepeat  : setRepeatMode(NoRepeat  ); break;
        case RepeatList: setRepeatMode(RepeatList); break;
        case RepeatSong: setRepeatMode(RepeatSong); break;
    }


    if (!m_mediaManager->loadDatabase())
    {
        QMessageBox::critical(this, QString(), tr("Failed to load database."));
        QCoreApplication::exit();
        return false;
    }

    loadDatabase();

    // Timers
    m_timer.start(timerPeriod);
    m_timerCDRomDrives.start(timerCDRomDrivesPeriod);

    updateSongDescription(nullptr);
    setState(Stopped);

    //new CGetRecentTracks(this, m_lastFmKey);

    init = true;
    return true;
}


/*/*
 * Affiche une erreur de base de données.
 *
 * \param msg      Message d'erreur.
 * \param query    Requête exécutée.
 * \param fileName Fichier source à l'origine de l'erreur.
 * \param line     Ligne du fichier à l'origine de l'erreur.
 */
/*
void CMainWindow::showDatabaseError(const QString& msg, const QString& query, const QString& fileName, int line)
{

#ifdef QT_DEBUG
    QMessageBox::warning(this, tr("Database error"), tr("File: %1 (%2)\n\nQuery: %3\n\nError: %4").arg(fileName).arg(line).arg(query).arg(msg));
#endif

    m_mediaManager->logError(msg + "\n" + tr("Query: ") + query, "", fileName.toUtf8().data(), line);
}
*/

/**
 * Modifie la hauteur des lignes des tableaux.
 *
 * \param height Hauteur des lignes en pixels, entre 15 et 50.
 */

void CMainWindow::setRowHeight(int height)
{
    height = qBound(15, height, 50);
    m_mediaManager->getSettings()->setValue("Preferences/RowHeight", height);

    // Mise à jour des vues
    m_library->verticalHeader()->setDefaultSectionSize(height);

    const QList<IPlayList *> playLists = getAllPlayLists();

    for (QList<IPlayList *>::ConstIterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        (*it)->verticalHeader()->setDefaultSectionSize(height);
    }
}


/**
 * Retourne la hauteur des lignes des tableaux.
 *
 * \return Hauteur des lignes en pixels (par défaut, 19).
 */

int CMainWindow::getRowHeight() const
{
    return m_mediaManager->getSettings()->value("Preferences/RowHeight", 19).toInt();
}


/**
 * Affiche ou masque le bouton "Stop".
 *
 * \param show Booléen.
 */

void CMainWindow::showButtonStop(bool show)
{
    m_mediaManager->getSettings()->setValue("Preferences/ShowButtonStop", show);
    m_uiControl->btnStop->setVisible(show);
}


void CMainWindow::showRemainingTime(bool show)
{
    m_mediaManager->getSettings()->setValue("Preferences/ShowRemainingTime", show);
    m_showRemainingTime = show;

    if (!m_showRemainingTime && m_currentSongItem != nullptr)
    {
        int duration = m_currentSongItem->getSong()->getDuration();
        QTime durationTime(0, 0);
        durationTime = durationTime.addMSecs(duration);
        m_uiControl->lblTime->setText(durationTime.toString("m:ss"));
    }
}


/**
 * Active ou désactive le scrobbling avec Last.fm.
 *
 * \param enable Booléen.
 */

void CMainWindow::enableScrobbling(bool enable)
{
    m_lastFmEnableScrobble = enable;
    m_mediaManager->getSettings()->setValue("LastFm/EnableScrobble", enable);
}


/**
 * Modifie le délai avant d'envoyer une notification à Last.fm.
 *
 * \param delay Délai en millisecondes.
 */

void CMainWindow::setDelayBeforeNotification(int delay)
{
    delay = qBound(2000, delay, 20000);
    m_delayBeforeNotification = delay;
    m_mediaManager->getSettings()->setValue("LastFm/DelayBeforeNotification", delay);
}


void CMainWindow::setPercentageBeforeScrobbling(int percentage)
{
    percentage = qBound(50, percentage, 100);
    m_percentageBeforeScrobbling = percentage;
    m_mediaManager->getSettings()->setValue("LastFm/PercentageBeforeScrobbling", percentage);
}


/**
 * Retourne le filtre de recherche actuel.
 *
 * \return Filtre de recherche.
 */

QString CMainWindow::getFilter() const
{
    return m_uiControl->editFilter->text().trimmed();
}


/**
 * Change la liste de morceaux à afficher.
 * Si \a songTable est invalide, la médiathèque est affichée.
 *
 * \param songTable Liste de morceaux à afficher.
 */

void CMainWindow::setDisplayedSongTable(CMediaTableView * songTable)
{
    m_displayedSongTable = (songTable ? songTable : m_library);
}


/**
 * Modifie le morceau et la liste de lecture courants.
 * Attention : cette méthode doit être appellée uniquement pour mettre à jour
 * le morceau courant lorsque les données d'une liste sont modifiées.
 *
 * \param songItem  Morceau courant.
 * \param songTable Liste de lecture courante.
 */

void CMainWindow::setCurrentSongItem(CMediaTableItem * songItem, CMediaTableView * songTable)
{
    m_currentSongItem = songItem;
    m_currentSongTable = songTable;
}


/**
 * Retourne le pointeur sur un morceau à partir de son identifiant en base de données.
 *
 * \param id Identifiant du morceau.
 * \return Pointeur sur le morceau, ou nullptr si l'identifiant est invalide.
 */

CSong * CMainWindow::getSongFromId(int id) const
{
    if (id <= 0)
        return nullptr;

    const QMap<int, CSong *> songList = m_library->getSongsMap();
    return songList.value(id);
}


/**
 * Retourne le dossier correspondant à un identifiant.
 *
 * \param id Identifiant du dossier.
 * \return Pointeur sur le dossier, ou nullptr si \a id n'est pas valide.
 */

CFolder * CMainWindow::getFolderFromId(int id) const
{
    return m_listModel->getFolderFromId(id);
}


/**
 * Retourne la liste de lecture correspondant à un identifiant.
 *
 * \param id Identifiant de la liste.
 * \return Pointeur sur la liste de lecture, ou nullptr si \a id n'est pas valide.
 */

IPlayList * CMainWindow::getPlayListFromId(int id) const
{
    return m_listModel->getPlayListFromId(id);
}


/**
 * Retourne la liste des listes de lecture contenant un morceau.
 *
 * \param song Morceau à rechercher.
 * \return Liste des listes de lecture.
 */

QList<IPlayList *> CMainWindow::getPlayListsWithSong(CSong * song) const
{
    Q_CHECK_PTR(song);

    const QList<IPlayList *> playLists = m_listModel->getPlayLists();
    QList<IPlayList *> playListsRet;

    for (QList<IPlayList *>::ConstIterator it = playLists.begin(); it != playLists.end(); ++it)
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

QList<IPlayList *> CMainWindow::getAllPlayLists() const
{
    return m_listModel->getPlayLists();
}


/**
 * Enlève une liste de morceaux de la médiathèque.
 * La liste ne doit pas contenir de doublons.
 *
 * \param songs Liste des morceaux à enlever.
 */

void CMainWindow::removeSongs(const QList<CSong *> songs)
{
    m_library->removeSongsFromTable(songs);

/*
    for (QList<CSong *>::ConstIterator it = songs.begin(); it != songs.end(); ++it)
    {
        Q_CHECK_PTR(*it);
        emit songRemoved(*it);
    }
*/
    // Suppression des morceaux de chaque liste statique et mise à jour des listes dynamiques
    const QList<IPlayList *> playLists = getAllPlayLists();

    for (QList<IPlayList *>::ConstIterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        CStaticList * playList = qobject_cast<CStaticList *>(*it);
        CDynamicList * dynamicList = qobject_cast<CDynamicList *>(*it);

        if (playList)
        {
            playList->removeSongs(songs, false);
        }
        else if (dynamicList)
        {
            dynamicList->tryUpdateList();
        }
    }

    // Mise à jour de la base
    QSqlQuery query1(m_mediaManager->getDataBase());
    query1.prepare("DELETE FROM song WHERE song_id = ?");

    QSqlQuery query2(m_mediaManager->getDataBase());
    query2.prepare("DELETE FROM play WHERE song_id = ?");

    for (QList<CSong *>::ConstIterator it = songs.begin(); it != songs.end(); ++it)
    {
        int songId = (*it)->getId();

        if (songId > 0)
        {
            query1.bindValue(0, songId);
            query2.bindValue(0, songId);

            if (!query1.exec())
            {
                m_mediaManager->logDatabaseError(query1.lastError().text(), query1.lastQuery(), __FILE__, __LINE__);
                continue;
            }

            if (!query2.exec())
            {
                m_mediaManager->logDatabaseError(query2.lastError().text(), query2.lastQuery(), __FILE__, __LINE__);
                continue;
            }
        }

        delete *it; // C'est pour ça que la liste ne doit pas contenir de doublons
    }

    updateListInformations();
}


void CMainWindow::setSelectionInformations(int numSongs, qlonglong durationMS)
{
    if (numSongs > 1)
    {
        QTime duration(0, 0);
        duration = duration.addMSecs(static_cast<int>(durationMS % 86400000));

        // Barre d'état
        if (durationMS > 86400000)
        {
            int numDays = static_cast<int>(durationMS / 86400000);
            m_listInfos->setText(tr("%n selected song(s), %1", "", numSongs).arg(tr("%n day(s) %1", "", numDays).arg(duration.toString())));
        }
        else
        {
            m_listInfos->setText(tr("%n selected song(s), %1", "", numSongs).arg(duration.toString()));
        }
    }
    else
    {
        updateListInformations();
    }
}


void CMainWindow::onPlayListChange(IPlayList * playList)
{
    if (playList)
        emit listModified(playList);
}


/**
 * Donne la position de lecture.
 *
 * \return Position de lecture, ou 0 si aucun morceau n'est en cours de lecture.
 */

int CMainWindow::getPosition() const
{
    return (m_currentSongItem ? m_currentSongItem->getSong()->getPosition() : 0);
}


/**
 * Affiche un message dans la barre d'état.
 * Le message est affiché pendant 5 secondes.
 *
 * \param message Message à afficher.
 */

void CMainWindow::notifyInformation2(const QString& message)
{
    statusBar()->showMessage(message, 5000);
}


/**
 * Lance le processus d'authentication avec Last.fm.
 * Le navigateur doit s'ouvrir pour que l'utilisateur puisse se connecter.
 */

void CMainWindow::connectToLastFm()
{
    new CAuthentication(m_mediaManager);
}


/**
 * Méthode appellée lorsqu'on ferme la boite de dialogue de modification d'un morceau.
 */

void CMainWindow::onDialogEditSongClosed()
{
    m_dialogEditSong = nullptr;
}


/**
 * Modifie le filtre de recherche à appliquer aux listes de lecture.
 *
 * \param filter Filtre de recherche.
 */

void CMainWindow::onFilterChange(const QString& filter)
{
    if (!m_displayedSongTable)
    {
        m_mediaManager->logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    m_displayedSongTable->applyFilter(filter);
    updateListInformations();

    if (filter.isEmpty())
    {
        m_uiControl->btnClearFilter->setEnabled(false);
        m_uiControl->editFilter->setPalette(QPalette());
    }
    else
    {
        m_uiControl->btnClearFilter->setEnabled(true);
        QPalette palette = m_uiControl->editFilter->palette();
        palette.setColor(QPalette::Base, Qt::green);
        m_uiControl->editFilter->setPalette(palette);
    }
}


/**
 * Sélectionne tous les morceaux de la liste affichée.
 */

void CMainWindow::selectAll()
{
    if (!m_displayedSongTable)
    {
        m_mediaManager->logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    m_displayedSongTable->selectAll();
}


/**
 * Désélectionne tous les morceaux de la liste affichée.
 */

void CMainWindow::selectNone()
{
    if (!m_displayedSongTable)
    {
        m_mediaManager->logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    m_displayedSongTable->clearSelection();
}


/**
 * Démarre la lecture du morceau sélectionné.
 */

void CMainWindow::play()
{
    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        if (m_state == Paused)
        {
            setState(Playing);

            emit songResumed(m_currentSongItem->getSong());
            m_currentSongItem->getSong()->play();

            m_currentSongTable->m_model->setCurrentSong(m_currentSongItem);
        }

        m_state = Playing;
    }
    else
    {
        m_state = Stopped;

        m_currentSongTable = m_displayedSongTable;

        // Recherche du morceau sélectionné
        m_currentSongItem = m_currentSongTable->getSelectedSongItem();
        
        if (m_mediaManager->isShuffle())
        {
            m_currentSongTable->initShuffle(m_currentSongItem);
        }

        if (m_currentSongItem == nullptr)
        {
            // Lecture du premier morceau de la liste
            m_currentSongItem = m_currentSongTable->getNextSong(nullptr, m_mediaManager->isShuffle());

            if (m_currentSongItem && !m_currentSongItem->getSong()->isEnabled())
            {
                nextSong();
                return;
            }
        }

        if (!m_currentSongItem)
        {
            m_currentSongTable = nullptr;
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

void CMainWindow::stop()
{
    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        m_currentSongItem->getSong()->stop();
        emit songStopped(m_currentSongItem->getSong());
        updateSongDescription(nullptr);
        m_currentSongItem = nullptr;

        m_currentSongTable->m_model->setCurrentSong(nullptr);
    }

    m_currentSongTable = nullptr;
    m_state = Stopped;

    setState(Stopped);

    m_lastFmTimeListened = 0;
    m_lastFmState = NoScrobble;
}


/**
 * Met la lecture en pause.
 */

void CMainWindow::pause()
{
    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        setState(Paused);

        m_currentSongItem->getSong()->pause();
        emit songPaused(m_currentSongItem->getSong());
        m_state = Paused;
        m_currentSongTable->m_model->setCurrentSong(m_currentSongItem);
    }
}


/**
 * Inverse l'état de la lecture.
 */

void CMainWindow::togglePlay()
{
    if (m_state != Playing)
        play();
    else
        pause();
}


/**
 * Active le morceau précédent dans la liste en cours de lecture.
 * Si le morceau actuel est le premier de la liste, ou que la position de lecture est
 * supérieure à 4 secondes, on revient au début du morceau.
 */

void CMainWindow::previousSong()
{
    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        int position = m_currentSongItem->getSong()->getPosition();

        // Retour au début du morceau
        if (position > 4000)
        {
            setPosition(0);
            return;
        }

        m_currentSongItem->getSong()->stop();
        updateSongDescription(nullptr);
        m_currentSongTable->m_model->setCurrentSong(nullptr);

        setState(Stopped);

        CMediaTableItem * songItem = m_currentSongTable->getPreviousSong(m_currentSongItem, m_mediaManager->isShuffle());

        // Premier morceau de la liste
        if (!songItem)
        {
            if (m_repeatMode == RepeatList)
            {
                songItem = m_currentSongTable->getLastSong(m_mediaManager->isShuffle());

                if (!songItem)
                {
                    m_currentSongTable = nullptr;
                    m_currentSongItem = nullptr;
                    m_state = Stopped;
                    return;
                }
            }
            else
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
        }

        m_currentSongItem = songItem;

        if (!m_currentSongItem->getSong()->isEnabled())
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
            m_currentSongItem = nullptr;
            m_currentSongTable = nullptr;
            m_state = Stopped;
            return;
        }
    }
    else
    {
        m_state = Stopped;
    }
}


/**
 * Passe au morceau suivant dans la liste en cours de lecture.
 */

void CMainWindow::nextSong()
{
    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        CMediaTableItem * firstSongToPlay = m_currentSongItem;

        forever
        {
            m_currentSongItem->getSong()->stop();
            updateSongDescription(nullptr);
            m_currentSongTable->m_model->setCurrentSong(nullptr);

            setState(Stopped);

            if (m_repeatMode != RepeatSong)
            {
                m_currentSongItem = m_currentSongTable->getNextSong(m_currentSongItem, m_mediaManager->isShuffle());
            }

            if (m_currentSongItem && !m_currentSongItem->getSong()->isEnabled())
            {
                continue;
            }

            // Fin de la liste et répétition de la liste activée
            if (!m_currentSongItem && m_repeatMode == RepeatList)
            {
                m_currentSongItem = m_currentSongTable->getNextSong(nullptr, m_mediaManager->isShuffle());
            }

            if (!m_currentSongItem)
            {
                m_currentSongTable = nullptr;
                m_state = Stopped;
                return;
            }

            if (m_currentSongTable == m_displayedSongTable)
            {
                selectCurrentSong();
            }

            // Chargement du morceau
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

                break;
            }

            // On a fait le tour de la liste et aucun morceau ne peut être joué
            if (firstSongToPlay == m_currentSongItem)
            {
                stop();
                break;
            }
        }
    }
    else
    {
        m_state = Stopped;
    }
}


void CMainWindow::changeCurrentSongList(CMediaTableItem * songItem, CMediaTableView * playList)
{
    if (!songItem || !playList)
    {
        return;
    }

    if (!m_currentSongItem || !m_currentSongTable)
    {
        return;
    }

    if (m_currentSongItem->getSong() != songItem->getSong())
    {
        return;
    }

    if (playList != m_currentSongTable)
    {
        m_currentSongTable->m_model->setCurrentSong(nullptr);
        m_currentSongTable = playList;
        m_currentSongItem = songItem;
        m_currentSongTable->m_model->setCurrentSong(songItem);
    }
}


/**
 * Lance la lecture d'un morceau de la liste actuellement affichée.
 *
 * \param songItem Morceau à lire.
 */

void CMainWindow::playSong(CMediaTableItem * songItem)
{
    if (!songItem)
    {
        m_mediaManager->logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        m_currentSongItem->getSong()->stop();
        updateSongDescription(nullptr);
        m_currentSongTable->m_model->setCurrentSong(nullptr);
    }

    m_currentSongItem = songItem;

    if (m_mediaManager->isShuffle() && (m_state == Stopped || m_currentSongTable != m_displayedSongTable))
    {
        m_displayedSongTable->initShuffle(m_currentSongItem);
    }

    m_currentSongTable = m_displayedSongTable;

    if (m_currentSongItem->getSong()->loadSound())
    {
        setState(Playing);
        startPlay();
    }
    else
    {
        setState(Stopped);
        m_currentSongItem = nullptr;
        m_currentSongTable = nullptr;
    }
}


/**
 * Passe au mode de répétition suivant.
 */

void CMainWindow::setNextRepeatMode()
{
    switch (m_repeatMode)
    {
        case NoRepeat  : setRepeatMode(RepeatList); break;
        case RepeatList: setRepeatMode(RepeatSong); break;
        case RepeatSong: setRepeatMode(NoRepeat  ); break;
    }
}


/**
 * Modifie le mode de répétition.
 *
 * \param repeatMode Mode de répétition.
 */

void CMainWindow::setRepeatMode(TRepeatMode repeatMode)
{
    m_repeatMode = repeatMode;

    switch (m_repeatMode)
    {
        default:
        case NoRepeat:
            m_uiWidget->actionNoRepeat->setChecked(true);
            m_uiWidget->menuRepeat->setIcon(QPixmap(":/icons/repeatOff"));
            m_uiControl->btnRepeat->setIcon(QPixmap(":/icons/repeatOff"));
            break;

        case RepeatList:
            m_uiWidget->actionRepeatList->setChecked(true);
            m_uiWidget->menuRepeat->setIcon(QPixmap(":/icons/repeatList"));
            m_uiControl->btnRepeat->setIcon(QPixmap(":/icons/repeatList"));
            break;

        case RepeatSong:
            m_uiWidget->actionRepeatSong->setChecked(true);
            m_uiWidget->menuRepeat->setIcon(QPixmap(":/icons/repeatSong"));
            m_uiControl->btnRepeat->setIcon(QPixmap(":/icons/repeatSong"));
            break;
    }
}

#if QT_VERSION < 0x050000

/**
 * Inverse l'état de la lecture aléatoire.
 */

void CMainWindow::setShuffle()
{
    setShuffle(!m_mediaManager->isShuffle());
}

#endif

/**
 * Active ou désactive la lecture aléatoire.
 *
 * \param shuffle Indique si la lecture aléatoire doit être activée.
 */

void CMainWindow::setShuffle(bool shuffle)
{
    m_mediaManager->setShuffle(shuffle);
    bool isShuffle = m_mediaManager->isShuffle();

    m_uiControl->btnShuffle->setIcon(QPixmap(isShuffle ? ":/icons/shuffle_on" : ":/icons/shuffle_off"));
    m_uiWidget->actionShuffle->setChecked(isShuffle);

    // Mélange de la liste en cours de lecture
    if (isShuffle && m_currentSongItem != nullptr)
    {
        m_currentSongTable->initShuffle(m_currentSongItem);
    }
}


/**
 * Active ou désactive le son.
 *
 * \param mute True pour couper le son, false pour le remettre.
 */

void CMainWindow::setMute(bool mute)
{
    m_mediaManager->setMute(mute);
    bool isMute = m_mediaManager->isMute();

    if (m_currentSongItem)
    {
        m_currentSongItem->getSong()->setMute(isMute);
    }

    m_uiWidget->actionMute->setChecked(isMute);

    m_uiWidget->actionMute->setIcon(QPixmap(isMute ? ":/icons/muet" : ":/icons/volume"));
    m_uiControl->btnMute->setIcon(QPixmap(isMute ? ":/icons/muet" : ":/icons/volume"));
}


/**
 * Active ou désactive le son.
 */

void CMainWindow::toggleMute()
{
    setMute(!m_mediaManager->isMute());
}


/**
 * Modifie le volume.
 *
 * \param volume Volume du son (entre 0 et 100).
 */

void CMainWindow::setVolume(int volume)
{
    m_mediaManager->setVolume(volume);
    volume = m_mediaManager->getVolume();

    if (m_currentSongItem)
    {
        m_currentSongItem->getSong()->setVolume(volume);
    }

    m_uiControl->sliderVolume->setValue(volume);
}


/**
 * Modifie la position de lecture.
 *
 * \param position Position de lecture, en millisecondes.
 */

void CMainWindow::setPosition(int position)
{
    if (position < 0)
    {
        m_mediaManager->logError(tr("invalid argument (%1)").arg(position), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

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


/**
 * Affiche la boite de dialogue pour modifier les préférences.
 */

void CMainWindow::openDialogPreferences()
{
    CDialogPreferences * dialog = new CDialogPreferences(this, m_mediaManager->getSettings());
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour voir la liste des notifications.
 */

void CMainWindow::openDialogNotifications()
{
    if (m_dialogNotifications != nullptr)
    {
        m_dialogNotifications->setFocus();
        return;
    }

    m_dialogNotifications = new CDialogNotifications(this);
    connect(m_dialogNotifications, SIGNAL(closed()), this, SLOT(onDialogNotificationsClosed()));
    m_dialogNotifications->show();
}


void CMainWindow::onDialogNotificationsClosed()
{
    m_dialogNotifications = nullptr;
}


/**
 * Affiche la boite de dialogue pour voir les dernières écoutes.
 */

void CMainWindow::openDialogLastPlays()
{
    if (m_dialogLastPlays != nullptr)
    {
        m_dialogLastPlays->setFocus();
        return;
    }

    m_dialogLastPlays = new CDialogLastPlays(this);
    connect(m_dialogLastPlays, SIGNAL(closed()), this, SLOT(onDialogLastPlaysClosed()));
    m_dialogLastPlays->show();
}


void CMainWindow::onDialogLastPlaysClosed()
{
    m_dialogLastPlays = nullptr;
}


/**
 * Affiche la boite de dialogue pour modifier les paramètres de l'égaliseur.
 */

void CMainWindow::openDialogEqualizer()
{
    CDialogEqualizer * dialog = new CDialogEqualizer(this);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour modifier les effects sonores.
 */

void CMainWindow::openDialogEffects()
{
    CDialogEffects * dialog = new CDialogEffects(this);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour visualiser et modifier les métadonnées d'un morceau.
 */

void CMainWindow::openDialogEditMetadata()
{
    Q_CHECK_PTR(m_displayedSongTable);

    // Liste des morceaux sélectionnés
    QList<CMediaTableItem *> songItemList = m_displayedSongTable->getSelectedSongItems();

    if (songItemList.size() > 1)
    {
        return;
    }

    // Recherche du morceau sélectionné
    CMediaTableItem * songItem = m_displayedSongTable->getSelectedSongItem();

    if (songItem)
    {
        CDialogEditMetadata * dialog = new CDialogEditMetadata(this, songItem->getSong());
        dialog->show();
    }
}


/**
 * Affiche une boite de dialogue pour sélectionner des fichiers à ajouter à la médiathèque.
 */

void CMainWindow::openDialogAddSongs()
{
    QStringList fileList = QFileDialog::getOpenFileNames(this, QString(), m_mediaManager->getSettings()->value("Preferences/LastDirectory", QString()).toString(), tr("Media files (*.flac *.ogg *.mp3 *wav);;MP3 (*.mp3);;FLAC (*.flac);;OGG (*.ogg);;WAV (*.wav);;All files (*.*)"));

    if (fileList.isEmpty())
        return;

    QFileInfo fileInfo(fileList.at(0));
    m_mediaManager->getSettings()->setValue("Preferences/LastDirectory", fileInfo.path());

    importSongs(fileList);
}


/**
 * Affiche une boite de dialogue pour ajouter un dossier à la médiathèque.
 */

void CMainWindow::openDialogAddFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this, QString(), m_mediaManager->getSettings()->value("Preferences/LastDirectory", QString()).toString());

    if (folder.isEmpty())
        return;

    m_mediaManager->getSettings()->setValue("Preferences/LastDirectory", folder);

    importSongs(listFilesInFolder(folder));
}


/**
 * Ajoute une liste de fichiers à la médiathèque.
 * Une boite de progression est affichée si l'opération prend plusieurs secondes.
 *
 * \param fileList Liste des fichiers à ajouter.
 */

QList<CSong *> CMainWindow::importSongs(const QStringList& fileList)
{
    if (fileList.isEmpty())
    {
        return QList<CSong *>();
    }

    QList<CSong *> songs;

    QProgressDialog progress(tr("Loading files..."), tr("Abort"), 0, fileList.size(), this);
    int i = 0;

    for (QStringList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        progress.setValue(i++);

        QString fileName = *it;
        fileName.replace('\\', '/');
        CSong * song = CSong::loadFromFile(m_mediaManager, fileName);
        if (song != nullptr)
        {
            songs.append(song);
        }

        qApp->processEvents();

        if (progress.wasCanceled())
        {
            break;
        }
    }
/*
    // Ajout des morceaux à la médiathèque
    for (QList<CSong *>::ConstIterator it = songs.begin(); it != songs.end(); ++it)
    {
        m_library->addSong(*it);
        emit songAdded(*it);
    }
*/
    m_library->addSongs(songs);
    emit songsAdded();

    m_mediaManager->notifyInformation(tr("%n song(s) added to the library.", "", songs.size()));
    updateListInformations();

    return songs;
}


/**
 * Affiche une boite de dialogue pour visualiser et éditer les informations du morceau sélectionné.
 */

void CMainWindow::openDialogSongInfos()
{
    Q_CHECK_PTR(m_displayedSongTable);

    // Liste des morceaux sélectionnés
    QList<CMediaTableItem *> songItemList = m_displayedSongTable->getSelectedSongItems();

    if (songItemList.size() > 1)
    {
        CDialogEditSongs * dialog = new CDialogEditSongs(songItemList, this);
        dialog->show();

        return;
    }

    // Recherche du morceau sélectionné
    CMediaTableItem * songItem = m_displayedSongTable->getSelectedSongItem();

    if (songItem)
    {
        m_dialogEditSong = new CDialogEditSong(songItem, m_displayedSongTable, this);
        connect(m_dialogEditSong, SIGNAL(closed()), this, SLOT(onDialogEditSongClosed()));
        m_dialogEditSong->show();
    }
}


/**
 * Affiche la boite de dialogue pour crée une nouvelle liste de lecture statique.
 */

void CMainWindow::openDialogCreateStaticList()
{
    openDialogCreateStaticList(nullptr);
}


/**
 * Affiche la boite de dialogue pour crée une nouvelle liste de lecture statique.
 *
 * \param folder Pointeur sur le dossier où créer la liste.
 * \param songs  Liste de morceaux à ajouter à la liste.
 */

void CMainWindow::openDialogCreateStaticList(CFolder * folder, const QList<CSong *>& songs)
{
    if (!folder)
        folder = m_listModel->getRootFolder();

    CDialogEditStaticPlayList * dialog = new CDialogEditStaticPlayList(nullptr, this, folder, songs);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour créer une nouvelle liste de lecture dynamique.
 *
 * \param folder Pointeur sur le dossier où créer la liste.
 */

void CMainWindow::openDialogCreateDynamicList(CFolder * folder)
{
    if (!folder)
        folder = m_listModel->getRootFolder();

    CDialogEditDynamicList * dialog = new CDialogEditDynamicList(nullptr, this, folder);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour créer un nouveau dossier.
 *
 * \param folder Pointeur sur le dossier où créer le dossier.
 */

void CMainWindow::openDialogCreateFolder(CFolder * folder)
{
    if (!folder)
        folder = m_listModel->getRootFolder();

    CDialogEditFolder * dialog = new CDialogEditFolder(nullptr, this, folder);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour modifier une liste de lecture statique.
 *
 * \param playList Liste à modifier.
 */

void CMainWindow::openDialogEditStaticPlayList(CStaticList * playList)
{
    CDialogEditStaticPlayList * dialog = new CDialogEditStaticPlayList(playList, this, playList->getFolder());
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour modifier une liste de lecture dynamique.
 *
 * \param playList Liste à modifier.
 */

void CMainWindow::openDialogEditDynamicList(CDynamicList * playList)
{
    CDialogEditDynamicList * dialog = new CDialogEditDynamicList(playList, this, playList->getFolder());
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour modifier un dossier.
 *
 * \param folder Dossier à modifier.
 */

void CMainWindow::openDialogEditFolder(CFolder * folder)
{
    CDialogEditFolder * dialog = new CDialogEditFolder(folder, this, folder->getFolder());
    dialog->show();
}


/**
 * Affiche la boite de dialogue "À propos".
 */

void CMainWindow::openDialogAbout()
{
    CDialogAbout * dialog = new CDialogAbout(this);
    dialog->show();
}


/**
 * Affiche une boite de dialogue pour relocaliser un morceau.
 */

void CMainWindow::relocateSong()
{
    Q_CHECK_PTR(m_displayedSongTable);

    // Liste des morceaux sélectionnés
    QList<CMediaTableItem *> songItemList = m_displayedSongTable->getSelectedSongItems();

    if (songItemList.size() > 1)
    {
        m_mediaManager->logError(tr("several songs selected"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    // Recherche du morceau sélectionné
    CMediaTableItem * songItem = m_displayedSongTable->getSelectedSongItem();

    if (!songItem)

        return;

    CSong * song = songItem->getSong();

    if (song)
    {
        QString fileName = QFileDialog::getOpenFileName(this, QString(), QString(), tr("Media files (*.flac *.ogg *.mp3 *.wav);;MP3 (*.mp3);;FLAC (*.flac);;OGG (*.ogg);;WAV (*.wav);;All files (*.*)"));

        if (fileName.isEmpty())
            return;

        const int newSongId = CSong::getId(m_mediaManager, fileName);
        CSong * newSong = getSongFromId(newSongId);
        const int oldSongId = song->getId();

        // Le fichier est déjà dans la médiathèque
        if (newSongId >= 0)
        {
            QMessageBox dialog(QMessageBox::Question, QString(), tr("This file is already in the library. Do you want to merge the two songs?"), QMessageBox::NoButton, this);
            /*QPushButton * buttonYes =*/ dialog.addButton(tr("Yes"), QMessageBox::YesRole);
            QPushButton * buttonNo = dialog.addButton(tr("No"), QMessageBox::NoRole);

            dialog.exec();

            if (dialog.clickedButton() == buttonNo)
                return;

            // Vérifier que le morceau n'est pas en cours de lecture
            //TODO...
            //A priori ça ne risque pas, mais on ne sait jamais

            QSqlQuery query(m_mediaManager->getDataBase());

            // Remplacement du morceau dans les listes statiques
            query.prepare("UPDATE static_list_song SET song_id = ? WHERE song_id = ?");
            query.bindValue(0, newSongId);
            query.bindValue(1, oldSongId);

            if (!query.exec())
            {
                m_mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            QList<IPlayList *> playLists = getAllPlayLists();

            for (QList<IPlayList *>::ConstIterator it = playLists.begin(); it != playLists.end(); ++it)
            {
                if (qobject_cast<CStaticList *>(*it))
                {
                    (*it)->replaceSong(song, newSong);
                }
            }

            // Fusion des lectures
            query.prepare("UPDATE play SET song_id = ? WHERE song_id = ?");
            query.bindValue(0, newSongId);
            query.bindValue(1, oldSongId);

            if (!query.exec())
            {
                m_mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            // Rechargement des lectures (pour ne pas avoir à trier manuellement les dates)
            query.prepare("SELECT play_time, play_time_utc FROM play WHERE song_id = ? ORDER BY play_time_utc ASC");
            query.bindValue(0, newSongId);

            if (!query.exec())
            {
                m_mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }
            else
            {
                newSong->m_plays.clear();

                while (query.next())
                {
                    CSong::TSongPlay playTime;
                    playTime.time = query.value(0).toDateTime();
                    playTime.timeUTC = query.value(1).toDateTime();
                    playTime.timeUTC.setTimeSpec(Qt::UTC);

                    if (playTime.time.isNull())
                        newSong->m_plays.append(playTime);
                    else
                        newSong->m_plays.prepend(playTime);
                }
            }

            // Mise-à-jour des informations du morceau
            int numPlays = newSong->m_plays.size();

            if (numPlays > 0)
            {
                CSong::TSongPlay lastPlay = newSong->m_plays.first();

                query.prepare("UPDATE song SET "
                                  "song_play_count    = ?,"
                                  "song_play_time     = ?,"
                                  "song_play_time_utc = ? "
                              "WHERE song_id = ?");

                query.bindValue(0, numPlays);
                query.bindValue(1, lastPlay.time);
                query.bindValue(2, lastPlay.timeUTC);
                query.bindValue(3, newSongId);

                if (!query.exec())
                {
                    m_mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                }
            }

            // Suppression de l'ancien morceau
            removeSongs(QList<CSong *>() << song);

            return;
        }

        song->m_properties.fileName = fileName;
        //song->m_isModified = true; // Pourquoi pas ?

        // Recherche de la durée du morceau
        FMOD_SOUND_TYPE type;
        FMOD_RESULT res;
        FMOD::Sound * sound;

        // Chargement du son
        //res = m_soundSystem->createStream(qPrintable(fileName), FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, nullptr, &sound);
        res = m_mediaManager->getSoundSystem()->createStream(reinterpret_cast<const char *>(fileName.utf16()), FMOD_UNICODE | FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, nullptr, &sound);

        if (res != FMOD_OK || !sound)
        {
            m_mediaManager->logError(tr("error while loading the file \"%1\" with FMOD").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
            return;
        }

        // Durée du morceau
        res = sound->getLength(reinterpret_cast<unsigned int *>(&(song->m_properties.duration)), FMOD_TIMEUNIT_MS);

        if (res != FMOD_OK)
        {
            m_mediaManager->logError(tr("can't compute song duration for file \"%1\"").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
            song->m_properties.duration = 0;
        }

        // Recherche du format du morceau
        res = sound->getFormat(&type, nullptr, nullptr, nullptr);

        if (res != FMOD_OK)
        {
            m_mediaManager->logError(tr("can't find song format for file \"%1\"").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
        }
        else
        {
            switch (type)
            {
                default:
                    m_mediaManager->logError(tr("unknown format"), __FUNCTION__, __FILE__, __LINE__);
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

                case FMOD_SOUND_TYPE_WAV:
                    song->m_properties.format = CSong::FormatWAV;
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

void CMainWindow::importFromITunes()
{
    CImporterITunes * dialog = new CImporterITunes(this);
    dialog->show();
}


/**
 * Ouvre la fenêtre pour importer la médiathèque depuis Songbird.
 *
 * \todo Implémentation.
 */

void CMainWindow::importFromSongbird()
{
    //CImporterSongbird * dialog = new CImporterSongbird(this);
    //dialog->show();
}


/**
 * Ajoute une liste de lecture à la vue.
 *
 * \param playList Pointeur sur la liste de lecture à ajouter.
 */

void CMainWindow::addPlayList(IPlayList * playList)
{
    if (playList)
        m_listModel->addPlayList(playList);
    else
        m_mediaManager->logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
}


/**
 * Ajoute un dossier de listes de lecture à la vue.
 *
 * \param folder Pointeur sur le dossier à ajouter.
 */

void CMainWindow::addFolder(CFolder * folder)
{
    if (folder)
        m_listModel->addFolder(folder);
    else
        m_mediaManager->logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
}


/**
 * Ajoute un morceau à la médiathèque.
 * Le fichier doit être un son valide, et ne doit pas être déjà présent dans la médiathèque.
 *
 * \param fileName Fichier à charger.
 * \return Pointeur sur le morceau.
 */

CSong * CMainWindow::addSong(const QString& fileName)
{
    CSong * song = CSong::loadFromFile(m_mediaManager, fileName);

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

void CMainWindow::selectCurrentSong()
{
    selectSong(m_currentSongTable, m_currentSongItem);
}


/**
 * Sélectionne un morceau dans une liste.
 *
 * \param songTable Liste de morceaux à afficher.
 * \param songItem  Morceau à sélectionner.
 */

void CMainWindow::selectSong(CMediaTableView * songTable, CMediaTableItem * songItem)
{
    if (!songTable || !songItem)
    {
        m_mediaManager->logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    displaySongTable(songTable);
    songTable->selectSongItem(songItem);
    songTable->setFocus(Qt::OtherFocusReason);
}


/*/*
 * Méthode appelée lorsqu'un morceau est modifié.
 * Le signal songModified est émis.
 */
/*
void CMainWindow::onSongModified()
{
    CSong * song = qobject_cast<CSong *>(sender());

    if (song)
    {
        emit songModified(song);
    }
}
*/

/**
 * Affiche le morceau sélectionné dans l'explorateur de fichiers.
 */

void CMainWindow::openSongInExplorer()
{
    // Recherche du morceau sélectionné
    CMediaTableItem * songItem = m_displayedSongTable->getSelectedSongItem();

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
 *
 * \todo Déplacer vers la classe CLibraryView ?
 */

void CMainWindow::editSelectedItem()
{
    IPlayList * playList = qobject_cast<IPlayList *>(m_playListView->getSelectedSongTable());

    if (playList)
    {
        CStaticList * staticList = qobject_cast<CStaticList *>(playList);

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
    }
    else
    {
        CFolder * folder = qobject_cast<CFolder *>(m_playListView->getSelectedFolder());

        if (folder)
        {
            openDialogEditFolder(folder);
        }
    }
}


/**
 * Supprime la liste de lecture ou le dossier sélectionné dans la vue CLibraryView.
 * Affiche une boite de dialogue de confirmation.
 *
 * \todo Déplacer vers la classe CLibraryView ?
 *
 * \todo Gérer les dossiers.
 * \todo Gérer le cas où la liste est utilisée dans un critère d'une liste dynamique.
 */

void CMainWindow::removeSelectedItem()
{
    IPlayList * playList = qobject_cast<IPlayList *>(m_playListView->getSelectedSongTable());

    if (playList)
    {
        // Confirmation
        QMessageBox dialog(QMessageBox::Question, QString(), tr("Are you sure you want to delete this playlist?"), QMessageBox::NoButton, this);
        /*QPushButton * buttonYes =*/ dialog.addButton(tr("Yes"), QMessageBox::YesRole);
        QPushButton * buttonNo = dialog.addButton(tr("No"), QMessageBox::NoRole);

        dialog.exec();

        if (dialog.clickedButton() == buttonNo)
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
            return;
        }

        m_listModel->removeFolder(folder, dialog->isResursive());
        delete dialog;
    }
}


#ifndef T_NO_SINGLE_APP

/**
 * Cette méthode est appellée si on essaye de lancer une autre instance de l'application.
 */

void CMainWindow::activateThisWindow()
{
    QLocalSocket * socket = m_localServer->nextPendingConnection();

    if (socket != nullptr)
    {
        connect(socket, SIGNAL(readyRead()), this, SLOT(receiveDataFromOtherInstance()));
    }

    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();
    activateWindow();
}


void CMainWindow::receiveDataFromOtherInstance()
{
    QLocalSocket * socket = qobject_cast<QLocalSocket *>(sender());

    if (socket == nullptr)
    {
        return;
    }

    QStringList fileList;
    QByteArray data = socket->readAll();
    QDataStream in(&data, QIODevice::ReadOnly);
    in >> fileList;

    loadFiles(fileList);

    socket->deleteLater();
}

#endif // T_NO_SINGLE_APP


void CMainWindow::loadFiles(const QStringList& fileList)
{
    QList<CSong *> songs;
    QList<CSong *> newSongs;

    // Ajout des morceaux
    for (QStringList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        QString fileName = *it;
        fileName.replace('\\', '/');

        CSong * song = CSong::loadFromFile(m_mediaManager, fileName);

        if (song == nullptr)
        {
            int songId = CSong::getId(m_mediaManager, fileName);

            if (songId >= 0)
            {
                song = getSongFromId(songId);
            }
        }
        else
        {
            newSongs.append(song);
        }

        if (song != nullptr)
        {
            songs.append(song);
        }
    }

    if (!newSongs.isEmpty())
    {
        m_library->addSongs(newSongs);
        emit songsAdded();

        m_mediaManager->notifyInformation(tr("%n song(s) added to the library.", "", newSongs.size()));
        updateListInformations();
    }

    // Lecture du dernier morceau
    if (!songs.isEmpty())
    {
        CSong * song = songs.last();
        CMediaTableItem * songItem = m_library->getFirstSongItem(song);

        if (songItem != nullptr)
        {
            playSong(songItem);
        }
    }
}


/**
 * Méthode appelée quand la lecture d'un morceau se termine.
 *
 * \todo Pouvoir arrêter la lecture ou fermer l'application à la fin d'un morceau.
 */

void CMainWindow::onPlayEnd()
{
    if (m_currentSongItem == nullptr)
    {
        return;
    }

    if (m_currentSongTable == m_queue)
    {
        m_queue->removeSongFromTable(m_currentSongItem->getPosition() - 1);
    }

    CSong * currentSong = m_currentSongItem->getSong();

    updateSongDescription(nullptr);

    // Morceau dans la file d'attente
    CMediaTableItem * queueSong = m_queue->getSongItemForRow(0);

    if (queueSong)
    {
        displaySongTable(m_queue);
        playSong(queueSong);
    }
    else
    {
        nextSong();
    }

    emit songPlayEnd(currentSong);

    // On retrie les listes de lecture statiques
    QList<IPlayList *> playLists = getAllPlayLists();

    for (QList<IPlayList *>::ConstIterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        int col = (*it)->getColumnSorted();
        if (col == CMediaTableView::ColPlayCount || col == CMediaTableView::ColLastPlayTime)
        {
            (*it)->sort();
        }
    }

    int col = m_library->getColumnSorted();
    if (col == CMediaTableView::ColPlayCount || col == CMediaTableView::ColLastPlayTime)
    {
        m_library->sort();
    }
}


/**
 * Met à jour les informations sur le morceau en cours de lecture.
 *
 * \param song Morceau à utiliser, ou nullptr pour n'afficher aucune information.
 */

void CMainWindow::updateSongDescription(CSong * song)
{
    if (song != nullptr)
    {
        // Description du morceau
        QString artistName = song->getArtistName();
        QString albumTitle = song->getAlbumTitle();

        QString description = song->getTitle();

        if (!artistName.isEmpty())
            description += " - " + artistName;

        if (!albumTitle.isEmpty())
            description += " - " + albumTitle;

        m_uiControl->songInfos->setText(description.replace('&', "&&"));
        m_uiControl->sliderPosition->setEnabled(true);

        const int duration = song->getDuration();
        if (duration >= 0)
        {
            m_uiControl->sliderPosition->setRange(0, duration);

            QTime durationTime(0, 0);
            durationTime = durationTime.addMSecs(duration);
            m_uiControl->lblTime->setText(durationTime.toString("m:ss")); /// \todo Stocker dans les settings
        }

        m_widgetLyrics->setSong(song);
    }
    else
    {
        m_uiControl->songInfos->setText(QString()); // "Pas de morceau en cours de lecture..."
        m_uiControl->sliderPosition->setEnabled(false);
        m_uiControl->sliderPosition->setRange(0, 1000);
        m_uiControl->lblPosition->setText("0:00");
        m_uiControl->lblTime->setText("0:00");

        m_widgetLyrics->setSong(song);
    }

    m_uiControl->sliderPosition->setValue(0);
}


/**
 * Met à jour les informations sur la liste de morceaux affichée.
 */

void CMainWindow::updateListInformations()
{
    QTime duration(0, 0);
    int numSongs = 0;
    qlonglong durationMS = 0;

    if (m_displayedSongTable == nullptr)
    {
        m_mediaManager->logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
    }
    else
    {
        durationMS = m_displayedSongTable->getTotalDuration(true);
        duration = duration.addMSecs(static_cast<int>(durationMS % 86400000));
        numSongs = m_displayedSongTable->getNumSongs(true);
    }

    m_listInfos->setText(tr("%n song(s), %1", "", numSongs).arg(durationToString(durationMS)));
}


/**
 * Met à jour la position de lecture depuis la position du curseur.
 */

void CMainWindow::updatePosition()
{
    setPosition(m_uiControl->sliderPosition->value());
}


/**
 * Méthode appelée régulièrement pour mettre à jour les informations sur la lecture,
 * et pour passer au morceau suivant si nécessaire.
 */

void CMainWindow::updateTimer()
{
    // Morceau actuel
    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        const int position = m_currentSongItem->getSong()->getPosition();

        if (m_lastFmEnableScrobble && (m_lastFmState == Started || m_lastFmState == Notified) && m_state == Playing)
        {
            int elapsedTime = position - m_lastFmLastPosition;
            m_lastFmTimeListened += elapsedTime;
            m_lastFmLastPosition = position;

            if (m_lastFmState == Started)
            {
                if (m_lastFmTimeListened > m_delayBeforeNotification)
                {
                    /*CUpdateNowPlaying * query =*/ new CUpdateNowPlaying(m_mediaManager, m_lastFmKey, m_currentSongItem->getSong());
                    m_lastFmState = Notified;
                }
            }
            else if (m_lastFmState == Notified)
            {
                if (m_lastFmTimeListened > 4 * 60000 || m_lastFmTimeListened > m_currentSongItem->getSong()->getDuration() * m_percentageBeforeScrobbling / 100)
                {
                    /*CScrobble * query =*/ new CScrobble(m_mediaManager, m_lastFmKey, m_currentSongItem->getSong());
                    m_lastFmState = Scrobbled;
                }
            }
        }

        if (m_currentSongItem->getSong()->isEnded())
        {
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

            // Mise à jour du temps restant
            if (m_showRemainingTime)
            {
                int duration = m_currentSongItem->getSong()->getDuration() - position;

                if (duration < 0)
                {
                    duration = 0;
                }

                QTime remainingTime(0, 0);
                remainingTime = remainingTime.addMSecs(duration);
                m_uiControl->lblTime->setText(remainingTime.toString("m:ss"));
            }
        }
    }
}


void CMainWindow::updateCDRomDrives()
{
    // Vérification des lecteurs de CD-ROM
    m_playListView->updateCDRomDrives();
}


void CMainWindow::selectPlayListFromTreeView(const QModelIndex& index)
{
    CMediaTableView * songTable = m_playListView->getSongTable(index);

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

void CMainWindow::displaySongTable(CMediaTableView * songTable)
{
    if (songTable == nullptr)
    {
        return;
    }

    if (songTable != m_displayedSongTable)
    {
        if (m_displayedSongTable)
        {
            m_displayedSongTable->setParent(nullptr);
        }

        m_playListView->selectionModel()->clearSelection();
        m_playListView->selectionModel()->setCurrentIndex(m_playListView->getSongTableModelIndex(songTable), QItemSelectionModel::Select | QItemSelectionModel::Rows);

        m_displayedSongTable = songTable;

        CDynamicList * dynamicList = qobject_cast<CDynamicList *>(m_displayedSongTable);
        if (dynamicList != nullptr && dynamicList->isAutoUpdate())
        {
            dynamicList->updateList();
        }

        m_displayedSongTable->applyFilter(m_uiControl->editFilter->text());
        setCentralWidget(m_displayedSongTable);
        m_displayedSongTable->show();

        updateListInformations();
        m_displayedSongTable->onSelectionChange();
    }
}


/**
 * Initialise FMOD.
 *
 * \todo Déplacer le contenu vers CMediaManager::initSoundSystem, puis supprimer cette méthode.
 */

void CMainWindow::initSoundSystem()
{
    if (!m_mediaManager->initSoundSystem())
    {
        QMessageBox::critical(this, QString(), tr("Failed to init sound system with FMOD."));
        QCoreApplication::exit();
        return;
    }

    // Lecteurs de CD-ROM
    int numDrives;
    FMOD_RESULT res = m_mediaManager->getSoundSystem()->getNumCDROMDrives(&numDrives);

    if (res != FMOD_OK)
    {
        m_mediaManager->logError(tr("can't get number of CD-ROM drives"), __FUNCTION__, __FILE__, __LINE__);
    }
    else
    {
        for (int drive = 0; drive < numDrives; ++drive)
        {
            char driveName[128]  = "";
            char SCSIName[128]   = "";
            char deviceName[128] = "";

            res = m_mediaManager->getSoundSystem()->getCDROMDriveName(drive, driveName, 128, SCSIName, 128, deviceName, 128);

            if (res != FMOD_OK)
            {
                m_mediaManager->logError(tr("can't get name of drive #%1").arg(drive), __FUNCTION__, __FILE__, __LINE__);
            }
            else
            {
                CCDRomDrive * cdRomDrive = new CCDRomDrive(driveName, this, SCSIName, deviceName);
                cdRomDrive->hide();

                m_cdRomDrives.append(cdRomDrive);

                connect(cdRomDrive, SIGNAL(songStarted(CMediaTableItem *)), this, SLOT(playSong(CMediaTableItem *)));
            }
        }
    }

    // File d'attente
    m_queue = new CQueuePlayList(this);
    m_queue->hide();
    connect(m_queue, SIGNAL(songStarted(CMediaTableItem *)), this, SLOT(playSong(CMediaTableItem *)));
}


/**
 * Charge la base de données.
 *
 * \todo Utiliser des méthodes dédiées dans les classes CMediaTableView et CFolder.
 */

void CMainWindow::loadDatabase()
{
    QSqlQuery query(m_mediaManager->getDataBase());

    // Création de la médiathèque
    m_library = new CLibrary(this);
    m_library->m_idPlayList = 0;
    setCentralWidget(m_library);
    connect(m_library, SIGNAL(songStarted(CMediaTableItem *)), this, SLOT(playSong(CMediaTableItem *)));

    if (!query.exec("SELECT list_columns FROM playlist WHERE playlist_id = 0"))
    {
        m_mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else if (query.next())
    {
        m_library->m_idPlayList = 0;
        m_library->initColumns(query.value(0).toString());
    }


    // Liste des morceaux
    QList<CSong *> songList = CSong::loadAllSongsFromDatabase(m_mediaManager);
    m_library->addSongs(songList);


    // Chargement des listes de lecture et des dossiers
    m_listModel = new CLibraryModel(this);
    m_listModel->loadFromDatabase();

    m_playListView->setModel(m_listModel);


    displaySongTable(m_library);
}


/**
 * Démarre la lecture du morceau.
 */

void CMainWindow::startPlay()
{
    Q_CHECK_PTR(m_currentSongItem);
    Q_CHECK_PTR(m_currentSongTable);

    // Met à jour la liste dynamique
    CDynamicList * dynamicList = qobject_cast<CDynamicList *>(m_currentSongTable);
    if (dynamicList != nullptr && dynamicList->isAutoUpdate())
    {
        dynamicList->updateList();
    }

    setState(Playing);

    CSong * song = m_currentSongItem->getSong();
/*
    if (song->isInCDRomDrive())
    {
        FMOD_RESULT res;
        res = song->m_channel->stop();

        if (res != FMOD_OK)
        {
            logError(tr("can't play track #%1 in CD-ROM drive \"%2\"").arg(song->m_cdRomTrackNumber + 1).arg(song->m_cdRomDrive->getDriveName()), __FUNCTION__, __FILE__, __LINE__);
        }

        res = song->m_channel->setPosition(song->m_cdRomTrackNumber, FMOD_TIMEUNIT_SENTENCE_SUBSOUND);

        if (res != FMOD_OK)
        {
            logError(tr("can't play track #%1 in CD-ROM drive \"%2\"").arg(song->m_cdRomTrackNumber + 1).arg(song->m_cdRomDrive->getDriveName()), __FUNCTION__, __FILE__, __LINE__);
        }
    }
*/
    song->play();
    emit songPlayStart(song);
    connect(song, SIGNAL(playEnd()), this, SLOT(onPlayEnd()), Qt::UniqueConnection);

    updateSongDescription(song);
    m_currentSongTable->m_model->setCurrentSong(m_currentSongItem);

    m_state = Playing;

    // Last.fm
    if (m_lastFmEnableScrobble)
    {
        m_lastFmTimeListened = 0;
        m_lastFmLastPosition = 0;

        if (song->getDuration() >= 30000)
            m_lastFmState = Started;
        else
            m_lastFmState = NoScrobble;
    }
}


/**
 * Modifie l'état de l'application.
 *
 * \param state État de l'application.
 */

void CMainWindow::setState(TState state)
{
    switch (state)
    {
        case Playing:
            m_uiControl->btnTogglePlay->setIcon(QPixmap(":/icons/pause"));
            m_uiControl->btnStop->setEnabled(true);
            m_uiControl->btnPrevious->setEnabled(true);
            m_uiControl->btnNext->setEnabled(true);
            m_uiWidget->actionTogglePlay->setText(tr("Pause"));
            m_uiWidget->actionTogglePlay->setIcon(QPixmap(":/icons/pause"));
            break;

        case Paused:
            m_uiControl->btnTogglePlay->setIcon(QPixmap(":/icons/play"));
            m_uiControl->btnStop->setEnabled(true);
            m_uiControl->btnPrevious->setEnabled(true);
            m_uiControl->btnNext->setEnabled(true);
            m_uiWidget->actionTogglePlay->setText(tr("Play"));
            m_uiWidget->actionTogglePlay->setIcon(QPixmap(":/icons/play"));
            break;

        case Stopped:
            m_uiControl->btnTogglePlay->setIcon(QPixmap(":/icons/play"));
            m_uiControl->btnStop->setEnabled(false);
            m_uiControl->btnPrevious->setEnabled(false);
            m_uiControl->btnNext->setEnabled(false);
            m_uiWidget->actionTogglePlay->setText(tr("Play"));
            m_uiWidget->actionTogglePlay->setIcon(QPixmap(":/icons/play"));
            break;
    }
}


/**
 * Méthode appelée lors de la fermeture de la fenêtre.
 * Sauvegarde l'état de la fenêtre.
 *
 * \param event Évènement de fermeture.
 */

void CMainWindow::closeEvent(QCloseEvent * event)
{
    Q_CHECK_PTR(event);

    if (m_currentSongItem)
    {
        QMessageBox dialog(QMessageBox::Question, QString(), tr("A song is being played. Are you sure you want to quit the application?"), QMessageBox::NoButton, this);
        /*QPushButton * buttonYes =*/ dialog.addButton(tr("Yes"), QMessageBox::YesRole);
        QPushButton * buttonNo = dialog.addButton(tr("No"), QMessageBox::NoRole);

        dialog.exec();

        if (dialog.clickedButton() == buttonNo)
        {
            event->ignore();
            return;
        }
    }

    if (m_dialogNotifications != nullptr)
    {
        m_dialogNotifications->close();
    }

    if (m_dialogLastPlays != nullptr)
    {
        m_dialogLastPlays->close();
    }

    m_mediaManager->getSettings()->setValue("Window/WindowGeometry", saveGeometry());
    m_mediaManager->getSettings()->setValue("Window/WindowState", saveState());

    event->accept();
    QMainWindow::closeEvent(event);
}

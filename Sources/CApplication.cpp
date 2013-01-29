/*
Copyright (C) 2012-2013 Teddy Michel

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
#include "CLyricWiki.hpp"
#include "CLibraryFolder.hpp"
#include "Dialog/CDialogEditDynamicList.hpp"
#include "Dialog/CDialogEditFolder.hpp"
#include "Dialog/CDialogEditMetadata.hpp"
#include "Dialog/CDialogEditSong.hpp"
#include "Dialog/CDialogEditSongs.hpp"
#include "Dialog/CDialogEditStaticPlayList.hpp"
#include "Dialog/CDialogPreferences.hpp"
#include "Dialog/CDialogEqualizer.hpp"
#include "Dialog/CDialogNotifications.hpp"
#include "Dialog/CDialogLastPlays.hpp"
#include "Dialog/CDialogRemoveFolder.hpp"
#include "Dialog/CDialogAbout.hpp"
#include "Importer/CImporterITunes.hpp"
#include "CLibrary.hpp"
#include "CSliderStyle.hpp"
#include "CWidgetLyrics.hpp"
#include "CCDRomDrive.hpp"

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


const int timerPeriod = 250; ///< Intervalle entre chaque mise-à-jour des informations.

const QString appVersion = "1.0.41";     ///< Numéro de version de l'application.
const QString appDate    = "24/01/2013"; ///< Date de sortie de cette version.


QString CApplication::getAppVersion() const
{
    return appVersion;
}


QString CApplication::getAppDate() const
{
    return appDate;
}


/**
 * Constructeur de la classe principale de l'application.
 */

CApplication::CApplication() :
    QMainWindow            (NULL),
    m_uiWidget             (new Ui::TMediaPlayer()),
    m_uiControl            (new Ui::WidgetControl()),
    m_soundSystem          (NULL),
    m_playListView         (NULL),
    m_listModel            (NULL),
    m_dialogEditSong       (NULL),
    m_settings             (NULL),
    m_timer                (NULL),
    m_listInfos            (NULL),
    m_currentSongItem      (NULL),
    m_currentSongTable     (NULL),
    m_library              (NULL),
    m_displayedSongTable   (NULL),
    m_widgetLyrics         (NULL),
    m_state                (Stopped),
    m_showRemainingTime    (false),
    m_repeatMode           (NoRepeat),
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

    // Internationnalisation
    QString lang = m_settings->value("Preferences/Language", QLocale::system().name()).toString();

    if (lang.isEmpty() || !m_translator.load(QString("Lang/TMediaPlayer_") + lang))
        m_translator.load(QString("Lang/TMediaPlayer_") + QLocale::system().name());

    qApp->installTranslator(&m_translator);
    
#if QT_VERSION >= 0x050000
    m_applicationPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator();
#else
    m_applicationPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QDir::separator();
#endif

    // Création du répertoire si nécessaire
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
#if QT_VERSION >= 0x050000
    connect(m_uiWidget->actionNewPlayList       , &QAction::triggered, this, &CApplication::openDialogCreateStaticList );
    connect(m_uiWidget->actionNewDynamicPlayList, &QAction::triggered, this, &CApplication::openDialogCreateDynamicList);
    connect(m_uiWidget->actionNewFolder         , &QAction::triggered, this, &CApplication::openDialogCreateFolder     );
    connect(m_uiWidget->actionAddFiles          , &QAction::triggered, this, &CApplication::openDialogAddSongs         );
    connect(m_uiWidget->actionAddFolder         , &QAction::triggered, this, &CApplication::openDialogAddFolder        );
    connect(m_uiWidget->actionInformations      , &QAction::triggered, this, &CApplication::openDialogSongInfos        );
    connect(m_uiWidget->actionOpenInExplorer    , &QAction::triggered, this, &CApplication::openSongInExplorer         );
    connect(m_uiWidget->actionImportITunes      , &QAction::triggered, this, &CApplication::importFromITunes           );
    connect(m_uiWidget->actionImportSongbird    , &QAction::triggered, this, &CApplication::importFromSongbird         );
    connect(m_uiWidget->actionNotifications     , &QAction::triggered, this, &CApplication::openDialogNotifications    );
    connect(m_uiWidget->actionLastPlays         , &QAction::triggered, this, &CApplication::openDialogLastPlays        );

    connect(m_uiWidget->actionSelectAll         , &QAction::triggered, this, &CApplication::selectAll                  );
    connect(m_uiWidget->actionSelectNone        , &QAction::triggered, this, &CApplication::selectNone                 );
    connect(m_uiWidget->actionPreferences       , &QAction::triggered, this, &CApplication::openDialogPreferences      );

    connect(m_uiWidget->actionPlay              , &QAction::triggered, this, &CApplication::togglePlay                 );
    connect(m_uiWidget->actionPause             , &QAction::triggered, this, &CApplication::pause                      );
    connect(m_uiWidget->actionStop              , &QAction::triggered, this, &CApplication::stop                       );
    connect(m_uiWidget->actionPrevious          , &QAction::triggered, this, &CApplication::previousSong               );
    connect(m_uiWidget->actionNext              , &QAction::triggered, this, &CApplication::nextSong                   );

#if __cplusplus < 201103L
    connect(m_uiWidget->actionNoRepeat          , &QAction::triggered, this, &CApplication::setRepeatModeNoRepeat      );
    connect(m_uiWidget->actionRepeatList        , &QAction::triggered, this, &CApplication::setRepeatModeRepeatList    );
    connect(m_uiWidget->actionRepeatSong        , &QAction::triggered, this, &CApplication::setRepeatModeRepeatSong    );
#else
    connect(m_uiWidget->actionNoRepeat          , &QAction::triggered, [=](){ this->setRepeatMode(NoRepeat  ); });
    connect(m_uiWidget->actionRepeatList        , &QAction::triggered, [=](){ this->setRepeatMode(RepeatList); });
    connect(m_uiWidget->actionRepeatSong        , &QAction::triggered, [=](){ this->setRepeatMode(RepeatSong); });
#endif

    connect(m_uiWidget->actionShuffle           , &QAction::triggered, this, &CApplication::setShuffle                 );
    connect(m_uiWidget->actionMute              , &QAction::triggered, this, &CApplication::setMute                    );
    connect(m_uiWidget->actionEqualizer         , &QAction::triggered, this, &CApplication::openDialogEqualizer        );

    connect(m_uiWidget->actionAboutQt           , &QAction::triggered, qApp, &QApplication::aboutQt                    );
    connect(m_uiWidget->actionAbout             , &QAction::triggered, this, &CApplication::openDialogAbout            );


    connect(this, &CApplication::songPlayStart, this, &CApplication::updateSongDescription);
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

    connect(m_uiWidget->actionPlay              , SIGNAL(triggered(    )), this, SLOT(togglePlay                 ()));
    connect(m_uiWidget->actionPause             , SIGNAL(triggered(    )), this, SLOT(pause                      ()));
    connect(m_uiWidget->actionStop              , SIGNAL(triggered(    )), this, SLOT(stop                       ()));
    connect(m_uiWidget->actionPrevious          , SIGNAL(triggered(    )), this, SLOT(previousSong               ()));
    connect(m_uiWidget->actionNext              , SIGNAL(triggered(    )), this, SLOT(nextSong                   ()));
    connect(m_uiWidget->actionNoRepeat          , SIGNAL(triggered(    )), this, SLOT(setRepeatModeNoRepeat      ()));
    connect(m_uiWidget->actionRepeatList        , SIGNAL(triggered(    )), this, SLOT(setRepeatModeRepeatList    ()));
    connect(m_uiWidget->actionRepeatSong        , SIGNAL(triggered(    )), this, SLOT(setRepeatModeRepeatSong    ()));
    connect(m_uiWidget->actionShuffle           , SIGNAL(triggered(bool)), this, SLOT(setShuffle             (bool)));
    connect(m_uiWidget->actionMute              , SIGNAL(triggered(bool)), this, SLOT(setMute                (bool)));
    connect(m_uiWidget->actionEqualizer         , SIGNAL(triggered(    )), this, SLOT(openDialogEqualizer        ()));

    connect(m_uiWidget->actionAboutQt           , SIGNAL(triggered(    )), qApp, SLOT(aboutQt                    ()));
    connect(m_uiWidget->actionAbout             , SIGNAL(triggered(    )), this, SLOT(openDialogAbout            ()));


    connect(this, SIGNAL(songPlayStart(CSong *)), this, SLOT(updateSongDescription(CSong *)));
#endif
}


/**
 * Libère les ressources utilisées par l'application.
 */

CApplication::~CApplication()
{
    if (m_timer)
    {
        m_timer->stop();
        delete m_timer;
    }

    // Enregistrement des paramètres
    m_settings->setValue("Preferences/Volume", m_volume);
    m_settings->setValue("Preferences/Shuffle", m_isShuffle);
    m_settings->setValue("Preferences/Repeat", m_repeatMode);

    //dumpObjectTree();

    delete m_listModel;

    // Destruction des lecteurs de CD-ROM
    for (QList<CCDRomDrive *>::const_iterator drive = m_cdRomDrives.begin(); drive != m_cdRomDrives.end(); ++drive)
    {
        delete *drive;
    }

    m_cdRomDrives.clear();

    for (QList<CLibraryFolder *>::const_iterator folder = m_libraryFolders.begin(); folder != m_libraryFolders.end(); ++folder)
    {
        delete *folder;
    }

    m_libraryFolders.clear();

    // Destruction de la médiathèque
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

bool CApplication::initWindow()
{
    static bool init = false;

    if (init)
    {
        logError(tr("the application has already been initialized"), __FUNCTION__, __FILE__, __LINE__);
        return true;
    }


    // Barre de contrôle
    QWidget * widgetControl = new QWidget(this);
    m_uiControl->setupUi(widgetControl);
    m_uiWidget->toolBar->addWidget(widgetControl);
    m_uiControl->sliderPosition->setStyle(new CSliderStyle);

    m_uiControl->btnStop->setVisible(m_settings->value("Preferences/ShowButtonStop", true).toBool());

    m_showRemainingTime = m_settings->value("Preferences/ShowRemainingTime", false).toBool();

    // Connexions des signaux et des slots
    connect(m_uiControl->songInfos, SIGNAL(clicked()), this, SLOT(selectCurrentSong()));

    connect(m_uiControl->btnPlay, SIGNAL(clicked()), this, SLOT(togglePlay()));
    connect(m_uiControl->btnStop, SIGNAL(clicked()), this, SLOT(stop()));

    connect(m_uiControl->btnPrevious, SIGNAL(clicked()), this, SLOT(previousSong()));
    connect(m_uiControl->btnNext, SIGNAL(clicked()), this, SLOT(nextSong()));

    connect(m_uiControl->btnRepeat, SIGNAL(clicked()), this, SLOT(setNextRepeatMode()));
    connect(m_uiControl->btnShuffle, SIGNAL(clicked()), this, SLOT(setShuffle()));
    connect(m_uiControl->btnMute, SIGNAL(clicked()), this, SLOT(toggleMute()));

    connect(m_uiControl->btnClearFilter, SIGNAL(clicked()), this, SLOT(clearFilter()));

    // Sliders
    connect(m_uiControl->sliderVolume, SIGNAL(sliderMoved(int)), this, SLOT(setVolume(int)));
    connect(m_uiControl->sliderPosition, SIGNAL(sliderReleased()), this, SLOT(updatePosition()));

    // Filtre
    connect(m_uiControl->editFilter, SIGNAL(textEdited(const QString&)), this, SLOT(onFilterChange(const QString&)));


    // Dock "Playlists"
    m_playListView = new CPlayListView(this);

    QDockWidget * dockPlayLists = new QDockWidget(tr("Playlists"), this);
    dockPlayLists->setObjectName("dock_playlists");
    dockPlayLists->setWidget(m_playListView);
    dockPlayLists->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    connect(m_playListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectPlayListFromTreeView(const QModelIndex&)));


    // Dock "Lyrics"
    m_widgetLyrics = new CWidgetLyrics(this);

    QDockWidget * dockLyrics = new QDockWidget(tr("Lyrics"), this);
    dockLyrics->setObjectName("dock_lyrics");
    dockLyrics->setWidget(m_widgetLyrics);
    dockLyrics->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);


    restoreGeometry(m_settings->value("Window/WindowGeometry").toByteArray());
    restoreState(m_settings->value("Window/WindowState").toByteArray());


    // Initialisation de FMOD
    if (!initSoundSystem())
    {
        QMessageBox::critical(this, QString(), tr("Failed to init sound system with FMOD."));
        QCoreApplication::exit();
        return false;
    }


    // Paramètres de lecture
    setVolume(m_settings->value("Preferences/Volume", 50).toInt());
    setShuffle(m_settings->value("Preferences/Shuffle", false).toBool());

    int repeatModeNum = m_settings->value("Preferences/Repeat", 0).toInt();

    switch (repeatModeNum)
    {
        default:
        case NoRepeat  : setRepeatMode(NoRepeat  ); break;
        case RepeatList: setRepeatMode(RepeatList); break;
        case RepeatSong: setRepeatMode(RepeatSong); break;
    }


    // Chargement de la base de données
    QString dbType = m_settings->value("Database/Type", QString("QSQLITE")).toString();
    m_settings->setValue("Database/Type", dbType);
    m_dataBase = QSqlDatabase::addDatabase(dbType);

    QString dbHostName = m_settings->value("Database/Host", QString("localhost")).toString();
    int dbPort = m_settings->value("Database/Port", 0).toInt();

#if QT_VERSION >= 0x050000
    QString dbBaseName = m_settings->value("Database/Base", QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "library.sqlite").toString();
#else
    QString dbBaseName = m_settings->value("Database/Base", QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QDir::separator() + "library.sqlite").toString();
#endif

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

    QString presetName = m_settings->value(QString("Equalizer/PresetName"), QString()).toString();
    int presetId = getEqualizerPresetIdFromName(presetName);

    if (presetId > 0)
    {
        TEqualizerPreset * currentEqualizerPreset = getEqualizerPresetFromId(presetId);

        if (currentEqualizerPreset)
        {
            bool currentEqualizerPresetDefined = true;

            for (int f = 0; f < 10; ++f)
            {
                if (currentEqualizerPreset->value[f] != m_equalizerGains[f])
                    currentEqualizerPresetDefined = false;
            }

            if (currentEqualizerPresetDefined)
            {
                m_currentEqualizerPreset = currentEqualizerPreset;
            }
        }
    }


    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTimer()));
    m_timer->start(timerPeriod);

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

#ifdef QT_DEBUG
    QMessageBox::warning(this, tr("Database error"), tr("File: %1 (%2)\n\nQuery: %3\n\nError: %4").arg(fileName).arg(line).arg(query).arg(msg));
#endif

    logError(msg + "\n" + tr("Query: ") + query, "", fileName.toUtf8().data(), line);
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

int CApplication::getRowHeight() const
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


void CApplication::showRemainingTime(bool show)
{
    m_settings->setValue("Preferences/ShowRemainingTime", show);
    m_showRemainingTime = show;
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


/**
 * Modifie le délai avant d'envoyer une notification à Last.fm.
 *
 * \param delay Délai en millisecondes.
 */

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
    /*FMOD_RESULT res =*/ m_dsp[frequency]->setParameter(FMOD_DSP_PARAMEQ_GAIN, m_equalizerGains[frequency]);
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

void CApplication::resetEqualizer()
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
            /*FMOD_RESULT res =*/ m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_GAIN, m_equalizerGains[i]);
        }
    }
    else
    {
        for (int i = 0; i < 10; ++i)
        {
            /*FMOD_RESULT res =*/ m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_GAIN, 1.0);
        }
    }
}


/**
 * Indique si l'égaliseur est activé.
 *
 * \return Booléen.
 */

bool CApplication::isEqualizerEnabled() const
{
    return m_settings->value(QString("Equalizer/Enabled"), false).toBool();
}


QString CApplication::getEqualizerPresetName(int id) const
{
    for (QList<TEqualizerPreset *>::const_iterator it = m_equalizerPresets.begin(); it != m_equalizerPresets.end(); ++it)
    {
        if ((*it)->id == id)
            return (*it)->name;
    }

    return QString();
}


int CApplication::getEqualizerPresetIdFromName(const QString& name) const
{
    for (QList<TEqualizerPreset *>::const_iterator it = m_equalizerPresets.begin(); it != m_equalizerPresets.end(); ++it)
    {
        if ((*it)->name == name)
            return (*it)->id;
    }

    return 0;
}


CApplication::TEqualizerPreset * CApplication::getEqualizerPresetFromId(int id) const
{
    for (QList<TEqualizerPreset *>::const_iterator it = m_equalizerPresets.begin(); it != m_equalizerPresets.end(); ++it)
    {
        if ((*it)->id == id)
            return *it;
    }

    return NULL;
}


void CApplication::saveEqualizerPreset(TEqualizerPreset * equalizer)
{
    Q_CHECK_PTR(equalizer);

    QSqlQuery query(m_dataBase);

    // Nouveau préréglage
    if (equalizer->id <= 0)
    {
        query.prepare("INSERT INTO equalizer("
                          "equalizer_name, "
                          "equalizer_val0, "
                          "equalizer_val1, "
                          "equalizer_val2, "
                          "equalizer_val3, "
                          "equalizer_val4, "
                          "equalizer_val5, "
                          "equalizer_val6, "
                          "equalizer_val7, "
                          "equalizer_val8, "
                          "equalizer_val9"
                      ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

        query.bindValue( 0, equalizer->name);
        query.bindValue( 1, equalizer->value[0]);
        query.bindValue( 2, equalizer->value[1]);
        query.bindValue( 3, equalizer->value[2]);
        query.bindValue( 4, equalizer->value[3]);
        query.bindValue( 5, equalizer->value[4]);
        query.bindValue( 6, equalizer->value[5]);
        query.bindValue( 7, equalizer->value[6]);
        query.bindValue( 8, equalizer->value[7]);
        query.bindValue( 9, equalizer->value[8]);
        query.bindValue(10, equalizer->value[9]);

        if (!query.exec())
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return;
        }

        if (m_dataBase.driverName() == "QPSQL")
        {
            query.prepare("SELECT currval('equalizer_seq')");

            if (!query.exec())
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return;
            }

            if (query.next())
            {
                equalizer->id = query.value(0).toInt();
            }
            else
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return;
            }
        }
        else
        {
            equalizer->id = query.lastInsertId().toInt();
        }

        m_equalizerPresets.append(equalizer);
    }
    else
    {
        query.prepare("UPDATE equalizer SET "
                        "equalizer_name = ?,"
                        "equalizer_val0 = ?,"
                        "equalizer_val1 = ?,"
                        "equalizer_val2 = ?,"
                        "equalizer_val3 = ?,"
                        "equalizer_val4 = ?,"
                        "equalizer_val5 = ?,"
                        "equalizer_val6 = ?,"
                        "equalizer_val7 = ?,"
                        "equalizer_val8 = ?,"
                        "equalizer_val9 = ? "
                      "WHERE equalizer_id = ?");

        query.bindValue( 0, equalizer->name);
        query.bindValue( 1, equalizer->value[0]);
        query.bindValue( 2, equalizer->value[1]);
        query.bindValue( 3, equalizer->value[2]);
        query.bindValue( 4, equalizer->value[3]);
        query.bindValue( 5, equalizer->value[4]);
        query.bindValue( 6, equalizer->value[5]);
        query.bindValue( 7, equalizer->value[6]);
        query.bindValue( 8, equalizer->value[7]);
        query.bindValue( 9, equalizer->value[8]);
        query.bindValue(10, equalizer->value[9]);
        query.bindValue(11, equalizer->id);

        if (!query.exec())
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    setCurrentEqualizerPreset(equalizer);
}


void CApplication::deleteEqualizerPreset(TEqualizerPreset * equalizer)
{
    Q_CHECK_PTR(equalizer);

    // Suppression en base de données
    if (equalizer->id > 0)
    {
        QSqlQuery query(m_dataBase);
        
        query.prepare("DELETE FROM equalizer WHERE equalizer_id = ?");
        query.bindValue(0, equalizer->id);

        if (!query.exec())
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        m_equalizerPresets.removeOne(equalizer);
/*
        for (QList<TEqualizer>::iterator it = m_equalizers.begin(); it != m_equalizers.end(); ++it)
        {
            if (it->id == equalizer.id)
            {
                
                *it = equalizer;
                break;
            }
        }
*/
        equalizer->id = 0;
    }

    m_currentEqualizerPreset = equalizer;
    m_settings->setValue("Equalizer/PresetName", QString());
}


void CApplication::setCurrentEqualizerPreset(TEqualizerPreset * equalizer)
{
    Q_CHECK_PTR(equalizer);

    m_currentEqualizerPreset = equalizer;

    setEqualizerGain(EqFreq32 , equalizer->value[0]);
    setEqualizerGain(EqFreq64 , equalizer->value[1]);
    setEqualizerGain(EqFreq125, equalizer->value[2]);
    setEqualizerGain(EqFreq250, equalizer->value[3]);
    setEqualizerGain(EqFreq500, equalizer->value[4]);
    setEqualizerGain(EqFreq1K , equalizer->value[5]);
    setEqualizerGain(EqFreq2K , equalizer->value[6]);
    setEqualizerGain(EqFreq4K , equalizer->value[7]);
    setEqualizerGain(EqFreq8K , equalizer->value[8]);
    setEqualizerGain(EqFreq16K, equalizer->value[9]);

    m_settings->setValue("Equalizer/PresetName", equalizer->name);
}


/**
 * Retourne le filtre de recherche actuel.
 *
 * \return Filtre de recherche.
 */

QString CApplication::getFilter() const
{
    return m_uiControl->editFilter->text().trimmed();
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
 * Modifie le morceau et la liste de lecture courants.
 * Attention : cette méthode doit être appellée uniquement pour mettre à jour
 * le morceau courant lorsque les données d'une liste sont modifiées.
 *
 * \param songItem  Morceau courant.
 * \param songTable Liste de lecture courante.
 */

void CApplication::setCurrentSongItem(CSongTableItem * songItem, CSongTable * songTable)
{
    m_currentSongItem = songItem;
    m_currentSongTable = songTable;
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
}


CLibraryFolder * CApplication::getLibraryFolder(int folderId) const
{
    for (QList<CLibraryFolder *>::const_iterator it = m_libraryFolders.begin(); it != m_libraryFolders.end(); ++it)
    {
        if ((*it)->id == folderId)
            return *it;
    }

    return NULL;
}


int CApplication::getLibraryFolderId(const QString& fileName) const
{
    for (QList<CLibraryFolder *>::const_iterator it = m_libraryFolders.begin(); it != m_libraryFolders.end(); ++it)
    {
        if (fileName.startsWith((*it)->pathName))
            return (*it)->id;
    }

    return -1;
}


void CApplication::addLibraryFolder(CLibraryFolder * folder)
{
    if (!folder || m_libraryFolders.contains(folder))
        return;

    m_libraryFolders.append(folder);
}


/**
 * Supprime un répertoire de la médiathèque.
 *
 * \param folder Pointeur sur le répertoire à supprimer.
 */

void CApplication::removeLibraryFolder(CLibraryFolder * folder)
{
    if (!folder)
        return;

    m_libraryFolders.removeAll(folder);

    if (folder->id > 0)
    {
        QSqlQuery query(getDataBase());
        query.prepare("DELETE FROM libpath WHERE path_id = ?");
        query.bindValue(0, folder->id);

        if (!query.exec())
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    delete folder;
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
}


/**
 * Retourne la liste des listes de lecture contenant un morceau.
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

QList<IPlayList *> CApplication::getAllPlayLists() const
{
    return m_listModel->getPlayLists();
}


/**
 * Enlève une liste de morceaux de la médiathèque.
 * La liste ne doit pas contenir de doublons.
 *
 * \param songs Liste des morceaux à enlever.
 */

void CApplication::removeSongs(const QList<CSong *> songs)
{
    m_library->removeSongsFromTable(songs);

/*
    for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
    {
        Q_CHECK_PTR(*it);
        emit songRemoved(*it);
    }
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
            dynamicList->updateList();
        }
    }

    // Mise à jour de la base
    QSqlQuery query1(m_dataBase);
    query1.prepare("DELETE FROM song WHERE song_id = ?");

    QSqlQuery query2(m_dataBase);
    query2.prepare("DELETE FROM play WHERE song_id = ?");

    for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
    {
        int songId = (*it)->getId();

        if (songId > 0)
        {
            query1.bindValue(0, songId);
            query2.bindValue(0, songId);

            if (!query1.exec())
            {
                showDatabaseError(query1.lastError().text(), query1.lastQuery(), __FILE__, __LINE__);
                continue;
            }

            if (!query2.exec())
            {
                showDatabaseError(query2.lastError().text(), query2.lastQuery(), __FILE__, __LINE__);
                continue;
            }
        }

        delete *it; // C'est pour ça que la liste ne doit pas contenir de doublons
    }

    updateListInformations();
}


void CApplication::setSelectionInformations(int numSongs, qlonglong durationMS)
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


void CApplication::onPlayListChange(IPlayList * playList)
{
    if (playList)
        emit listModified(playList);
}


/**
 * Donne la position de lecture.
 *
 * \return Position de lecture, ou 0 si aucun morceau n'est en cours de lecture.
 */

int CApplication::getPosition() const
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
        query.prepare("SELECT currval('artist_seq')");

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
        query.prepare("SELECT currval('album_seq')");

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
        query.prepare("SELECT currval('genre_seq')");

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

QStringList CApplication::getGenreList()
{
    QStringList genres;

    // Genres prédéfinis
    genres.append("Blues");
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


/**
 * Retourne le pointeur sur un fichier de log.
 *
 * \param logName Nom du fichier de log.
 * \return Pointeur sur le fichier ouvert en écriture.
 */

QFile * CApplication::getLogFile(const QString& logName)
{
    QString fileName = logName + QDateTime::currentDateTime().toString("-yyyy-MM-dd");

    if (!m_logList.contains(fileName))
    {
        QString logFileName = m_applicationPath + fileName + ".log";
        QFile * logFile = new QFile(logFileName, this);

        if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append))
        {
            logError(tr("can't open the log file \"%1\"").arg(logFileName), __FUNCTION__, __FILE__, __LINE__);
            return NULL;
        }

        m_logList[fileName] = logFile;
    }

    return m_logList.value(fileName);
}


/**
 * Gestion des messages d'erreur.
 *
 * \param message  Message d'erreur.
 * \param function Nom de la fonction où l'erreur est survenue.
 * \param file     Nom du fichier source contenant la fonction.
 * \param line     Ligne dans le fichier source.
 */

void CApplication::logError(const QString& message, const QString& function, const char * file, int line)
{
    static QFile * logFile = NULL;
    static bool fileOpened = false;
    
    // L'ouverture du fichier n'est tentée qu'une seule fois pour éviter des appels récursifs infinis entre getLogFile et logError.
    if (!fileOpened || !logFile)
    {
        logFile = getLogFile("errors");
        fileOpened = true;
    }

    QString txt = tr("%2 (%3 line %4): %1").arg(message).arg(function).arg(file).arg(line);

    QTextStream stream(logFile);
    stream << txt << "\n";

#ifdef QT_DEBUG
    qWarning() << txt;
#endif
}


/**
 * Affiche un message dans la barre d'état.
 * Le message est affiché pendant 5 secondes.
 *
 * \param message Message à afficher.
 */

void CApplication::notifyInformation(const QString& message)
{
    statusBar()->showMessage(message, 5000);
    m_infosNotified << TNotification(message, QDateTime::currentDateTime());
}


/**
 * Lance le processus d'authentication avec Last.fm.
 * Le navigateur doit s'ouvrir pour que l'utilisateur puisse se connecter.
 */

void CApplication::connectToLastFm()
{
    new CAuthentication(this);
}


/**
 * Méthode appellée lorsqu'on ferme la boite de dialogue de modification d'un morceau.
 */

void CApplication::onDialogEditSongClosed()
{
    m_dialogEditSong = NULL;
}


/**
 * Modifie le filtre de recherche à appliquer aux listes de lecture.
 *
 * \param filter Filtre de recherche.
 */

void CApplication::onFilterChange(const QString& filter)
{
    if (!m_displayedSongTable)
    {
        logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    m_displayedSongTable->applyFilter(filter);

    if (filter.isEmpty())
        m_uiControl->btnClearFilter->setEnabled(false);
    else
        m_uiControl->btnClearFilter->setEnabled(true);
}


/**
 * Sélectionne tous les morceaux de la liste affichée.
 */

void CApplication::selectAll()
{
    if (!m_displayedSongTable)
    {
        logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    m_displayedSongTable->selectAll();
}


/**
 * Désélectionne tous les morceaux de la liste affichée.
 */

void CApplication::selectNone()
{
    if (!m_displayedSongTable)
    {
        logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    m_displayedSongTable->clearSelection();
}


/**
 * Démarre la lecture du morceau sélectionné.
 */

void CApplication::play()
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

void CApplication::stop()
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

    m_lastFmTimeListened = 0;
    m_lastFmState = NoScrobble;
}


/**
 * Met la lecture en pause.
 */

void CApplication::pause()
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
 * Lance ou interrompt la lecture.
 */

void CApplication::togglePlay()
{
    if (m_state != Playing)
        play();
    else
        pause();
}


/**
 * Active le morceau précédent du morceau actuel.
 * Si le morceau actuel est le premier de la liste, ou que la position de lecture est
 * supérieure à 4 secondes, on revient au début du morceau.
 */

void CApplication::previousSong()
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
        updateSongDescription(NULL);
        m_currentSongTable->m_model->setCurrentSong(NULL);

        setState(Stopped);

        CSongTableItem * songItem = m_currentSongTable->getPreviousSong(m_currentSongItem, m_isShuffle);

        // Premier morceau de la liste
        if (!songItem)
        {
            if (m_repeatMode == RepeatList)
            {
                songItem = m_currentSongTable->getLastSong(m_isShuffle);

                if (!songItem)
                {
                    m_currentSongTable = NULL;
                    m_currentSongItem = NULL;
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
            m_currentSongItem = NULL;
            m_currentSongTable = NULL;
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
 * Passe au morceau suivant.
 */

void CApplication::nextSong()
{
    if (m_currentSongItem)
    {
        Q_CHECK_PTR(m_currentSongTable);

        m_currentSongItem->getSong()->stop();
        updateSongDescription(NULL);
        m_currentSongTable->m_model->setCurrentSong(NULL);

        setState(Stopped);

        if (m_repeatMode != RepeatSong)
        {
            m_currentSongItem = m_currentSongTable->getNextSong(m_currentSongItem, m_isShuffle);
        }

        if (m_currentSongItem && !m_currentSongItem->getSong()->isEnabled())
        {
            nextSong();
            return;
        }

        // Fin de la liste et répétition de la liste activée
        if (!m_currentSongItem && m_repeatMode == RepeatList)
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
    if (!songItem)
    {
        logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

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
        setState(Playing);
        startPlay();
    }
    else
    {
        setState(Stopped);
        m_currentSongItem = NULL;
        m_currentSongTable = NULL;
    }
}


/**
 * Passe au mode de répétition suivant.
 */

void CApplication::setNextRepeatMode()
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

void CApplication::setRepeatMode(TRepeatMode repeatMode)
{
    m_repeatMode = repeatMode;

    switch (m_repeatMode)
    {
        case NoRepeat:

            m_uiWidget->actionNoRepeat->setChecked(true);
            m_uiWidget->actionRepeatList->setChecked(false);
            m_uiWidget->actionRepeatSong->setChecked(false);
                
            m_uiControl->btnRepeat->setIcon(QPixmap(":/icons/repeatOff"));
            break;

        case RepeatList:

            m_uiWidget->actionNoRepeat->setChecked(false);
            m_uiWidget->actionRepeatList->setChecked(true);
            m_uiWidget->actionRepeatSong->setChecked(false);
                
            m_uiControl->btnRepeat->setIcon(QPixmap(":/icons/repeatList"));
            break;

        case RepeatSong:

            m_uiWidget->actionNoRepeat->setChecked(false);
            m_uiWidget->actionRepeatList->setChecked(false);
            m_uiWidget->actionRepeatSong->setChecked(true);
                
            m_uiControl->btnRepeat->setIcon(QPixmap(":/icons/repeatSong"));
            break;
    }
}


void CApplication::setShuffle()
{
    setShuffle(!m_isShuffle);
}


void CApplication::setShuffle(bool shuffle)
{
    if (shuffle != m_isShuffle)
    {
        m_isShuffle = shuffle;
        m_uiControl->btnShuffle->setIcon(QPixmap(m_isShuffle ? ":/icons/shuffle_on" : ":/icons/shuffle_off"));
        m_uiWidget->actionShuffle->setChecked(shuffle);
    }
}


/**
 * Active ou désactive le son.
 *
 * \param mute True pour couper le son, false pour le remettre.
 */

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


/**
 * Active ou désactive le son.
 */

void CApplication::toggleMute()
{
    setMute(!m_isMute);
}


/**
 * Modifie le volume.
 *
 * \param volume Volume du son (entre 0 et 100).
 */

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


/**
 * Modifie la position de lecture.
 *
 * \param position Position de lecture, en millisecondes.
 */

void CApplication::setPosition(int position)
{
    if (position < 0)
    {
        logError(tr("invalid argument (%1)").arg(position), __FUNCTION__, __FILE__, __LINE__);
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

void CApplication::openDialogPreferences()
{
    CDialogPreferences * dialog = new CDialogPreferences(this, m_settings);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour voir la liste des notifications.
 */

void CApplication::openDialogNotifications()
{
    CDialogNotifications * dialog = new CDialogNotifications(this);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour voir les dernières écoutes.
 */

void CApplication::openDialogLastPlays()
{
    CDialogLastPlays * dialog = new CDialogLastPlays(this);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour modifier les paramètres de l'égaliseur.
 */

void CApplication::openDialogEqualizer()
{
    CDialogEqualizer * dialog = new CDialogEqualizer(this);
    dialog->show();
}


/**
 * Affiche la boite de dialogue pour visualiser et modifier les métadonnées d'un morceau.
 */

void CApplication::openDialogEditMetadata()
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

void CApplication::openDialogAddSongs()
{
    QStringList fileList = QFileDialog::getOpenFileNames(this, QString(), m_settings->value("Preferences/LastDirectory", QString()).toString(), tr("Media files (*.flac *.ogg *.mp3);;MP3 (*.mp3);;FLAC (*.flac);;OGG (*.ogg);;All files (*.*)"));

    if (fileList.isEmpty())
        return;

    QFileInfo fileInfo(fileList.at(0));
    m_settings->setValue("Preferences/LastDirectory", fileInfo.path());

    importSongs(fileList);
}


/**
 * Affiche une boite de dialogue pour ajouter un dossier à la médiathèque.
 */

void CApplication::openDialogAddFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this, QString(), m_settings->value("Preferences/LastDirectory", QString()).toString());

    if (folder.isEmpty())
        return;

    m_settings->setValue("Preferences/LastDirectory", folder);

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

        QString fileName = *it;
        fileName.replace('\\', '/');
        CSong * song = CSong::loadFromFile(this, fileName);
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

void CApplication::openDialogSongInfos()
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
        m_dialogEditSong = new CDialogEditSong(songItem, m_displayedSongTable, this);
        connect(m_dialogEditSong, SIGNAL(closed()), this, SLOT(onDialogEditSongClosed()));
        m_dialogEditSong->show();
    }
}


/**
 * Affiche la boite de dialogue pour crée une nouvelle liste de lecture statique.
 */

void CApplication::openDialogCreateStaticList()
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


/**
 * Affiche la boite de dialogue "À propos".
 */

void CApplication::openDialogAbout()
{
    CDialogAbout * dialog = new CDialogAbout(this);
    dialog->show();
}


/**
 * Affiche une boite de dialogue pour relocaliser un morceau.
 */

void CApplication::relocateSong()
{
    Q_CHECK_PTR(m_displayedSongTable);

    // Liste des morceaux sélectionnés
    QList<CSongTableItem *> songItemList = m_displayedSongTable->getSelectedSongItems();

    if (songItemList.size() > 1)
    {
        logError(tr("several songs selected"), __FUNCTION__, __FILE__, __LINE__);
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

        const int newSongId = CSong::getId(this, fileName);
        CSong * newSong = getSongFromId(newSongId);
        const int oldSongId = song->getId();

        // Le fichier est déjà dans la médiathèque
        if (newSongId >= 0)
        {
            QMessageBox::StandardButton res = QMessageBox::question(this, QString(), tr("This file is already in the library. Do you want to merge the two songs?"), QMessageBox::Yes | QMessageBox::No);

            if (res == QMessageBox::No)
            {
                return;
            }

            // Vérifier que le morceau n'est pas en cours de lecture
            //TODO...
            //A priori ça ne risque pas, mais on ne sait jamais

            QSqlQuery query(m_dataBase);

            // Remplacement du morceau dans les listes statiques
            query.prepare("UPDATE static_list_song SET song_id = ? WHERE song_id = ?");
            query.bindValue(0, newSongId);
            query.bindValue(1, oldSongId);

            if (!query.exec())
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            QList<IPlayList *> playLists = getAllPlayLists();

            for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
            {
                if (qobject_cast<CStaticPlayList *>(*it))
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
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            }

            // Rechargement des lectures (pour ne pas avoir à trier manuellement les dates)
            query.prepare("SELECT play_time, play_time_utc FROM play WHERE song_id = ? ORDER BY play_time_utc ASC");
            query.bindValue(0, newSongId);

            if (!query.exec())
            {
                showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
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
                    showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
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
        //res = m_soundSystem->createStream(qPrintable(fileName), FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, NULL, &sound);
        res = m_soundSystem->createStream(reinterpret_cast<const char *>(fileName.utf16()), FMOD_UNICODE | FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, NULL, &sound);

        if (res != FMOD_OK || !sound)
        {
            logError(tr("error while loading the file \"%1\" with FMOD").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
            return;
        }

        // Durée du morceau
        res = sound->getLength(reinterpret_cast<unsigned int *>(&(song->m_properties.duration)), FMOD_TIMEUNIT_MS);

        if (res != FMOD_OK)
        {
            logError(tr("can't compute song duration for file \"%1\"").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
            song->m_properties.duration = 0;
        }

        // Recherche du format du morceau
        res = sound->getFormat(&type, NULL, NULL, NULL);

        if (res != FMOD_OK)
        {
            logError(tr("can't find song format for file \"%1\"").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
        }
        else
        {
            switch (type)
            {
                default:
                    logError(tr("unknown format"), __FUNCTION__, __FILE__, __LINE__);
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

void CApplication::importFromITunes()
{
    CImporterITunes * dialog = new CImporterITunes(this);
    dialog->show();
}


/**
 * Ouvre la fenêtre pour importer la médiathèque depuis Songbird.
 *
 * \todo Implémentation.
 */

void CApplication::importFromSongbird()
{
    //CImporterSongbird * dialog = new CImporterSongbird(this);
    //dialog->show();
}


/**
 * Ajoute une liste de lecture à la vue.
 *
 * \param playList Pointeur sur la liste de lecture à ajouter.
 */

void CApplication::addPlayList(IPlayList * playList)
{
    if (playList)
        m_listModel->addPlayList(playList);
    else
        logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
}


/**
 * Ajoute un dossier de listes de lecture à la vue.
 *
 * \param folder Pointeur sur le dossier à ajouter.
 */

void CApplication::addFolder(CFolder * folder)
{
    if (folder)
        m_listModel->addFolder(folder);
    else
        logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
}


/**
 * Ajoute un morceau à la médiathèque.
 * Le fichier doit être un son valide, et ne doit pas être déjà présent dans la médiathèque.
 *
 * \param fileName Fichier à charger.
 * \return Pointeur sur le morceau.
 */

CSong * CApplication::addSong(const QString& fileName)
{
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

void CApplication::selectCurrentSong()
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
        logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
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

void CApplication::onSongModified()
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
        fileList.append(dir.absoluteFilePath(*it).replace('\\', '/'));
    }

    return fileList;
}


/**
 * Affiche le morceau sélectionné dans l'explorateur de fichiers.
 */

void CApplication::openSongInExplorer()
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

void CApplication::editSelectedItem()
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

void CApplication::removeSelectedItem()
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
            return;

        m_listModel->removeFolder(folder, dialog->isResursive());
        delete dialog;
    }
}


/**
 * Méthode appelée quand la lecture d'un morceau se termine.
 *
 * \todo Pouvoir arrêter la lecture ou fermer l'application à la fin d'un morceau.
 */

void CApplication::onPlayEnd()
{
    if (m_currentSongItem)
    {
        CSong * currentSong = m_currentSongItem->getSong();

        updateSongDescription(NULL);
        nextSong();

        emit songPlayEnd(currentSong);

        // On retrie les listes de lecture
        QList<IPlayList *> playLists = getAllPlayLists();

        for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
        {
            int col = (*it)->getColumnSorted();
            if (col == CSongTable::ColPlayCount || col == CSongTable::ColLastPlayTime)
                (*it)->sort();
        }
        
        int col = m_library->getColumnSorted();
        if (col == CSongTable::ColPlayCount || col == CSongTable::ColLastPlayTime)
            m_library->sort();

        //m_library->update(); // Why ???
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

void CApplication::updateListInformations()
{
    QTime duration(0, 0);
    int numSongs = 0;
    qlonglong durationMS = 0;

    if (m_displayedSongTable)
    {
        durationMS = m_displayedSongTable->getTotalDuration();
        duration = duration.addMSecs(static_cast<int>(durationMS % 86400000));
        numSongs = m_displayedSongTable->getNumSongs();
    }

    m_listInfos->setText(tr("%n song(s), %1", "", numSongs).arg(durationToString(durationMS)));
}


/**
 * Met à jour la position de lecture depuis la position du curseur.
 */

void CApplication::updatePosition()
{
    setPosition(m_uiControl->sliderPosition->value());
}


/**
 * Méthode appelée régulièrement pour mettre à jour les informations sur la lecture,
 * et pour passer au morceau suivant si nécessaire.
 */

void CApplication::updateTimer()
{
    // Vérification des lecteurs de CD-ROM
    static int timerCDRomDrive = 0;

    if (++timerCDRomDrive > 10)
    {
        m_playListView->updateCDRomDrives();
        timerCDRomDrive = 0;
    }

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
                    /*CUpdateNowPlaying * query =*/ new CUpdateNowPlaying(this, m_lastFmKey, m_currentSongItem->getSong());
                    m_lastFmState = Notified;
                }
            }
            else if (m_lastFmState == Notified)
            {
                if (m_lastFmTimeListened > 4 * 60000 || m_lastFmTimeListened > m_currentSongItem->getSong()->getDuration() * m_percentageBeforeScrobbling / 100)
                {
                    /*CScrobble * query =*/ new CScrobble(this, m_lastFmKey, m_currentSongItem->getSong());
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
                    duration = 0;

                QTime remainingTime(0, 0);
                remainingTime = remainingTime.addMSecs(duration);
                m_uiControl->lblTime->setText(remainingTime.toString("m:ss"));
            }
        }
    }
}


void CApplication::selectPlayListFromTreeView(const QModelIndex& index)
{
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
            m_displayedSongTable->setParent(NULL);

        m_playListView->selectionModel()->clearSelection();
        m_playListView->selectionModel()->setCurrentIndex(m_playListView->getSongTableModelIndex(songTable), QItemSelectionModel::Select | QItemSelectionModel::Rows);

        m_displayedSongTable = songTable;
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
 * \return Booléen indiquant le succès ou l'échec du chargement.
 */

bool CApplication::initSoundSystem()
{
    bool ret = true;
    FMOD_RESULT res;

    res = FMOD::System_Create(&m_soundSystem);
    if (res != FMOD_OK)
        return false;

    unsigned int version;
    res = m_soundSystem->getVersion(&version);
    if (res != FMOD_OK)
        return false;

    if (version < FMOD_VERSION)
    {
        QMessageBox::critical(this, QString(), tr("This program requires FMOD %1 or superior.").arg(FMOD_VERSION));
        return false;
    }

    int numDrivers;
    res = m_soundSystem->getNumDrivers(&numDrivers);
    if (res != FMOD_OK)
        return false;

    if (numDrivers == 0)
    {
        res = m_soundSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        if (res != FMOD_OK)
            return false;
    }
    else
    {
        FMOD_CAPS caps;
        FMOD_SPEAKERMODE speakermode;
        res = m_soundSystem->getDriverCaps(0, &caps, NULL, &speakermode);
        if (res != FMOD_OK)
            return false;

        // Set the user selected speaker mode
        res = m_soundSystem->setSpeakerMode(speakermode);
        if (res != FMOD_OK)
            return false;

        if (caps & FMOD_CAPS_HARDWARE_EMULATED)
        {
            res = m_soundSystem->setDSPBufferSize(1024, 10);
            if (res != FMOD_OK)
                return false;
        }

        char name[256] = "";
        res = m_soundSystem->getDriverInfo(0, name, 256, 0);
        if (res != FMOD_OK)
            return false;

        if (strstr(name, "SigmaTel"))
        {
            res = m_soundSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
            if (res != FMOD_OK)
                return false;
        }
    }

    res = m_soundSystem->init(100, FMOD_INIT_NORMAL, 0);
    if (res == FMOD_ERR_OUTPUT_CREATEBUFFER)
    {
        res = m_soundSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        if (res != FMOD_OK)
            return false;
        res = m_soundSystem->init(2, FMOD_INIT_NORMAL, 0);
    }

    ret = (res == FMOD_OK);

    // Lecteurs de CD-ROM
    int numDrives;
    res = m_soundSystem->getNumCDROMDrives(&numDrives);

    if (res != FMOD_OK)
    {
        logError(tr("can't get number of CD-ROM drives"), __FUNCTION__, __FILE__, __LINE__);
    }
    else
    {
        for (int drive = 0; drive < numDrives; ++drive)
        {
            char driveName[128]  = "";
            char SCSIName[128]   = "";
            char deviceName[128] = "";

            res = m_soundSystem->getCDROMDriveName(drive, driveName, 128, SCSIName, 128, deviceName, 128);

            if (res != FMOD_OK)
            {
                logError(tr("can't get name of drive #%1").arg(drive), __FUNCTION__, __FILE__, __LINE__);
            }
            else
            {
                CCDRomDrive * cdRomDrive = new CCDRomDrive(driveName, this, SCSIName, deviceName);
                cdRomDrive->hide();

                m_cdRomDrives.append(cdRomDrive);

                connect(cdRomDrive, SIGNAL(songStarted(CSongTableItem *)), this, SLOT(playSong(CSongTableItem *)));
            }
        }
    }

    return ret;
}


/**
 * Charge la base de données.
 *
 * \todo Utiliser des méthodes dédiées dans les classes CSongTable et CFolder.
 */

void CApplication::loadDatabase()
{
    QSqlQuery query(m_dataBase);

    // Création des relations
    if (m_dataBase.driverName() == "QSQLITE")
    {
        createDatabaseSQLite();
    }
    else if (m_dataBase.driverName() == "QMYSQL")
    {
        createDatabaseMySQL();
    }
    else if (m_dataBase.driverName() == "QPSQL")
    {
        createDatabasePostgreSQL();
    }

    // Création des vues
    QStringList tables = m_dataBase.tables(QSql::Views);

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


    // Liste des répertoires
    if (!query.exec("SELECT path_id, path_location, path_keep_organized, path_format, path_format_items FROM libpath"))
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        while (query.next())
        {
            CLibraryFolder * libraryFolder = new CLibraryFolder(this);
            
            libraryFolder->id            = query.value(0).toInt();
            libraryFolder->pathName      = query.value(1).toString();
            libraryFolder->keepOrganized = query.value(2).toBool();
            libraryFolder->format        = query.value(3).toString();

            libraryFolder->convertStringToFormatItems(query.value(4).toString());

            m_libraryFolders.append(libraryFolder);
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
    else if (query.next())
    {
        m_library->m_idPlayList = 0;
        m_library->initColumns(query.value(0).toString());
    }

    //m_playListView->setCurrentIndex(m_playListView->addSongTable(m_library));


    // Préréglages d'égaliseur
    if (!query.exec("SELECT "
                        "equalizer_id,"
                        "equalizer_name,"
                        "equalizer_val0,"
                        "equalizer_val1,"
                        "equalizer_val2,"
                        "equalizer_val3,"
                        "equalizer_val4,"
                        "equalizer_val5,"
                        "equalizer_val6,"
                        "equalizer_val7,"
                        "equalizer_val8,"
                        "equalizer_val9 "
                    "FROM equalizer ORDER BY equalizer_name"))
    {
        showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        while (query.next())
        {
            TEqualizerPreset * eq = new TEqualizerPreset();
            eq->id   = query.value(0).toInt();
            eq->name = query.value(1).toString();

            for (int f = 0; f < 10; ++f)
            {
                eq->value[f] = query.value(2 + f).toFloat();
            }

            m_equalizerPresets.append(eq);
        }
    }


    // Liste des morceaux
    QList<CSong *> songList = CSong::loadAllSongsFromDatabase(this);
    m_library->addSongs(songList);


    // Chargement des listes de lecture et des dossiers
    m_listModel = new CListModel(this);
    m_listModel->loadFromDatabase();

    m_playListView->setModel(m_listModel);


    displaySongTable(m_library);
}


/**
 * Démarre la lecture du morceau.
 */

void CApplication::startPlay()
{
    Q_CHECK_PTR(m_currentSongItem);
    Q_CHECK_PTR(m_currentSongTable);

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

void CApplication::setState(TState state)
{
    switch (state)
    {
        case Playing:
            m_uiControl->btnPlay->setIcon(QPixmap(":/icons/pause"));
            m_uiControl->btnStop->setEnabled(true);
            m_uiControl->btnPrevious->setEnabled(true);
            m_uiControl->btnNext->setEnabled(true);
            m_uiWidget->actionPlay->setText(tr("Pause"));
            m_uiWidget->actionPlay->setIcon(QPixmap(":/icons/pause"));
            break;

        case Paused:
            m_uiControl->btnPlay->setIcon(QPixmap(":/icons/play"));
            m_uiControl->btnStop->setEnabled(true);
            m_uiControl->btnPrevious->setEnabled(true);
            m_uiControl->btnNext->setEnabled(true);
            m_uiWidget->actionPlay->setText(tr("Play"));
            m_uiWidget->actionPlay->setIcon(QPixmap(":/icons/play"));
            break;

        case Stopped:
            m_uiControl->btnPlay->setIcon(QPixmap(":/icons/play"));
            m_uiControl->btnStop->setEnabled(false);
            m_uiControl->btnPrevious->setEnabled(false);
            m_uiControl->btnNext->setEnabled(false);
            m_uiWidget->actionPlay->setText(tr("Play"));
            m_uiWidget->actionPlay->setIcon(QPixmap(":/icons/play"));
            break;
    }
}


/**
 * Crée la structure de la base de données pour SQLite.
 */

void CApplication::createDatabaseSQLite()
{
    QSqlQuery query(m_dataBase);
    QStringList tables = m_dataBase.tables(QSql::Tables);

    if (!tables.contains("folder"))
    {
        if (!query.exec("CREATE TABLE folder ("
                            "folder_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "folder_name VARCHAR(512) NOT NULL,"
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
                            "playlist_name VARCHAR(512) NOT NULL,"
                            "folder_id INTEGER NOT NULL,"
                            "list_position INTEGER NOT NULL,"
                            "list_columns VARCHAR(512) NOT NULL"
                            //",UNIQUE (folder_id, list_position)"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO playlist (playlist_id, playlist_name, folder_id, list_position, list_columns) "
                        "VALUES (0, 'Library', 0, -1, '0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120')"))
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
                            "auto_update INTEGER NOT NULL,"
                            "only_checked INTEGER NOT NULL,"
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
                            "criteria_value1 VARCHAR(512),"
                            "criteria_value2 VARCHAR(512),"
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
                            "song_filename VARCHAR(512) NOT NULL UNIQUE,"
                            "song_filesize INTEGER NOT NULL,"
                            "song_bitrate INTEGER NOT NULL,"
                            "song_sample_rate INTEGER NOT NULL,"
                            "song_format INTEGER NOT NULL,"
                            "song_channels INTEGER NOT NULL,"
                            "song_duration INTEGER NOT NULL,"
                            "song_creation DATETIME NOT NULL,"
                            "song_modification DATETIME NOT NULL,"
                            "song_enabled INTEGER NOT NULL,"
                            "song_title VARCHAR(512) NOT NULL,"
                            "song_title_sort VARCHAR(512) NOT NULL,"
                            "song_subtitle VARCHAR(512) NOT NULL,"
                            "song_grouping VARCHAR(512) NOT NULL,"
                            "artist_id INTEGER NOT NULL,"
                            "album_id INTEGER NOT NULL,"
                            "album_artist_id INTEGER NOT NULL,"
                            "song_composer VARCHAR(512) NOT NULL,"
                            "song_composer_sort VARCHAR(512) NOT NULL,"
                            "song_year INTEGER NOT NULL,"
                            "song_track_number INTEGER NOT NULL,"
                            "song_track_count INTEGER NOT NULL,"
                            "song_disc_number INTEGER NOT NULL,"
                            "song_disc_count INTEGER NOT NULL,"
                            "genre_id INTEGER NOT NULL,"
                            "song_rating INTEGER NOT NULL,"
                            "song_comments TEXT NOT NULL,"
                            "song_bpm INTEGER NOT NULL,"
                            "song_lyrics TEXT NOT NULL,"
                            "song_language VARCHAR(2) NOT NULL,"
                            "song_lyricist VARCHAR(512) NOT NULL,"
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
                            "album_title VARCHAR(512) NOT NULL,"
                            "album_title_sort VARCHAR(512),"
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
                            "artist_name VARCHAR(512) NOT NULL,"
                            "artist_name_sort VARCHAR(512),"
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
                            "genre_name VARCHAR(512) NOT NULL UNIQUE"
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
                            "play_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "song_id INTEGER NOT NULL,"
                            "play_time TIMESTAMP,"
                            "play_time_utc TIMESTAMP"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("libpath"))
    {
        if (!query.exec("CREATE TABLE libpath ("
                            "path_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "path_location VARCHAR(512) NOT NULL UNIQUE"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("equalizer"))
    {
        if (!query.exec("CREATE TABLE equalizer ("
                            "equalizer_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "equalizer_name VARCHAR(512) NOT NULL UNIQUE,"
                            "equalizer_val0 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val1 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val2 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val3 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val4 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val5 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val6 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val7 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val8 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val9 FLOAT NOT NULL DEFAULT 1.0"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }
}


/**
 * Crée la structure de la base de données pour MySQL.
 */

void CApplication::createDatabaseMySQL()
{
    QSqlQuery query(m_dataBase);
    QStringList tables = m_dataBase.tables(QSql::Tables);

    if (!tables.contains("folder"))
    {
        if (!query.exec("CREATE TABLE folder ("
                            "folder_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "folder_name VARCHAR(512) NOT NULL,"
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
                            "playlist_name VARCHAR(512) NOT NULL,"
                            "folder_id INTEGER NOT NULL,"
                            "list_position INTEGER NOT NULL,"
                            "list_columns VARCHAR(512) NOT NULL"
                            //",UNIQUE (folder_id, list_position)"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO playlist (playlist_id, playlist_name, folder_id, list_position, list_columns) "
                        "VALUES (0, 'Library', 0, -1, '0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120')"))
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
                            "auto_update INTEGER NOT NULL,"
                            "only_checked INTEGER NOT NULL,"
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
                            "criteria_value1 VARCHAR(512),"
                            "criteria_value2 VARCHAR(512),"
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
                            "song_filename VARCHAR(512) NOT NULL UNIQUE,"
                            "song_filesize INTEGER NOT NULL,"
                            "song_bitrate INTEGER NOT NULL,"
                            "song_sample_rate INTEGER NOT NULL,"
                            "song_format INTEGER NOT NULL,"
                            "song_channels INTEGER NOT NULL,"
                            "song_duration INTEGER NOT NULL,"
                            "song_creation DATETIME NOT NULL,"
                            "song_modification DATETIME NOT NULL,"
                            "song_enabled INTEGER NOT NULL,"
                            "song_title VARCHAR(512) NOT NULL,"
                            "song_title_sort VARCHAR(512) NOT NULL,"
                            "song_subtitle VARCHAR(512) NOT NULL,"
                            "song_grouping VARCHAR(512) NOT NULL,"
                            "artist_id INTEGER NOT NULL,"
                            "album_id INTEGER NOT NULL,"
                            "album_artist_id INTEGER NOT NULL,"
                            "song_composer VARCHAR(512) NOT NULL,"
                            "song_composer_sort VARCHAR(512) NOT NULL,"
                            "song_year INTEGER NOT NULL,"
                            "song_track_number INTEGER NOT NULL,"
                            "song_track_count INTEGER NOT NULL,"
                            "song_disc_number INTEGER NOT NULL,"
                            "song_disc_count INTEGER NOT NULL,"
                            "genre_id INTEGER NOT NULL,"
                            "song_rating INTEGER NOT NULL,"
                            "song_comments TEXT NOT NULL,"
                            "song_bpm INTEGER NOT NULL,"
                            "song_lyrics TEXT NOT NULL,"
                            "song_language VARCHAR(2) NOT NULL,"
                            "song_lyricist VARCHAR(512) NOT NULL,"
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
                            "album_title VARCHAR(512) NOT NULL,"
                            "album_title_sort VARCHAR(512),"
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
                            "artist_name VARCHAR(512) NOT NULL,"
                            "artist_name_sort VARCHAR(512),"
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
                            "genre_name VARCHAR(512) NOT NULL UNIQUE"
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
                            "play_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "song_id INTEGER NOT NULL,"
                            "play_time TIMESTAMP,"
                            "play_time_utc TIMESTAMP"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("libpath"))
    {
        if (!query.exec("CREATE TABLE libpath ("
                            "path_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "path_location VARCHAR(512) NOT NULL UNIQUE"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("equalizer"))
    {
        if (!query.exec("CREATE TABLE equalizer ("
                            "equalizer_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "equalizer_name VARCHAR(512) NOT NULL UNIQUE,"
                            "equalizer_val0 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val1 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val2 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val3 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val4 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val5 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val6 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val7 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val8 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val9 FLOAT NOT NULL DEFAULT 1.0"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }
}


/**
 * Crée la structure de la base de données pour PostgreSQL.
 */

void CApplication::createDatabasePostgreSQL()
{
    QSqlQuery query(m_dataBase);
    QStringList tables = m_dataBase.tables(QSql::Tables);

    if (!tables.contains("folder"))
    {
        if (!query.exec("CREATE TABLE folder ("
                            "folder_id SERIAL PRIMARY KEY,"
                            "folder_name VARCHAR(512) NOT NULL,"
                            "folder_parent INTEGER NOT NULL,"
                            "folder_position INTEGER NOT NULL,"
                            "folder_expanded INTEGER NOT NULL"
                            //",UNIQUE (folder_parent, folder_position)"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE folder_folder_id_seq RENAME TO folder_seq"))
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
                            "playlist_id SERIAL PRIMARY KEY,"
                            "playlist_name VARCHAR(512) NOT NULL,"
                            "folder_id INTEGER NOT NULL,"
                            "list_position INTEGER NOT NULL,"
                            "list_columns VARCHAR(512) NOT NULL"
                            //",UNIQUE (folder_id, list_position)"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO playlist (playlist_id, playlist_name, folder_id, list_position, list_columns)"
                        "VALUES (0, 'Library', 0, -1, '0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120')"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("dynamic_list"))
    {
        if (!query.exec("CREATE TABLE dynamic_list ("
                            "dynamic_list_id SERIAL PRIMARY KEY,"
                            "criteria_id INTEGER NOT NULL,"
                            "playlist_id INTEGER NOT NULL,"
                            "auto_update INTEGER NOT NULL,"
                            "only_checked INTEGER NOT NULL,"
                            "UNIQUE (playlist_id)"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE dynamic_list_dynamic_list_id_seq RENAME TO dynamic_list_seq"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("criteria"))
    {
        if (!query.exec("CREATE TABLE criteria ("
                            "criteria_id SERIAL PRIMARY KEY,"
                            "dynamic_list_id INTEGER NOT NULL,"
                            "criteria_parent INTEGER NOT NULL,"
                            "criteria_position INTEGER NOT NULL,"
                            "criteria_type INTEGER NOT NULL,"
                            "criteria_condition INTEGER NOT NULL,"
                            "criteria_value1 VARCHAR(512),"
                            "criteria_value2 VARCHAR(512),"
                            "UNIQUE (dynamic_list_id, criteria_parent, criteria_position)"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE criteria_criteria_id_seq RENAME TO criteria_seq"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("static_list"))
    {
        if (!query.exec("CREATE TABLE static_list ("
                            "static_list_id SERIAL PRIMARY KEY,"
                            "playlist_id INTEGER NOT NULL,"
                            "UNIQUE (playlist_id)"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE static_list_static_list_id_seq RENAME TO static_list_seq"))
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
                            "song_id SERIAL PRIMARY KEY,"
                            "song_filename VARCHAR(512) NOT NULL UNIQUE,"
                            "song_filesize INTEGER NOT NULL,"
                            "song_bitrate INTEGER NOT NULL,"
                            "song_sample_rate INTEGER NOT NULL,"
                            "song_format INTEGER NOT NULL,"
                            "song_channels INTEGER NOT NULL,"
                            "song_duration INTEGER NOT NULL,"
                            "song_creation TIMESTAMP NOT NULL,"
                            "song_modification TIMESTAMP NOT NULL,"
                            "song_enabled INTEGER NOT NULL,"
                            "song_title VARCHAR(512) NOT NULL,"
                            "song_title_sort VARCHAR(512) NOT NULL,"
                            "song_subtitle VARCHAR(512) NOT NULL,"
                            "song_grouping VARCHAR(512) NOT NULL,"
                            "artist_id INTEGER NOT NULL,"
                            "album_id INTEGER NOT NULL,"
                            "album_artist_id INTEGER NOT NULL,"
                            "song_composer VARCHAR(512) NOT NULL,"
                            "song_composer_sort VARCHAR(512) NOT NULL,"
                            "song_year INTEGER NOT NULL,"
                            "song_track_number INTEGER NOT NULL,"
                            "song_track_count INTEGER NOT NULL,"
                            "song_disc_number INTEGER NOT NULL,"
                            "song_disc_count INTEGER NOT NULL,"
                            "genre_id INTEGER NOT NULL,"
                            "song_rating INTEGER NOT NULL,"
                            "song_comments TEXT NOT NULL,"
                            "song_bpm INTEGER NOT NULL,"
                            "song_lyrics TEXT NOT NULL,"
                            "song_language CHAR(2) NOT NULL,"
                            "song_lyricist VARCHAR(512) NOT NULL,"
                            "song_compilation INTEGER NOT NULL,"
                            "song_skip_shuffle INTEGER NOT NULL,"
                            "song_play_count INTEGER NOT NULL,"
                            "song_play_time TIMESTAMP,"
                            "song_play_time_utc TIMESTAMP,"
                            "song_track_gain FLOAT NOT NULL,"
                            "song_track_peak FLOAT NOT NULL,"
                            "song_album_gain FLOAT NOT NULL,"
                            "song_album_peak FLOAT NOT NULL"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("album"))
    {
        if (!query.exec("CREATE TABLE album ("
                            "album_id SERIAL PRIMARY KEY,"
                            "album_title VARCHAR(512) NOT NULL,"
                            "album_title_sort VARCHAR(512),"
                            "UNIQUE (album_title, album_title_sort)"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE album_album_id_seq RENAME TO album_seq"))
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
                            "artist_id SERIAL PRIMARY KEY,"
                            "artist_name character varying(512) NOT NULL,"
                            "artist_name_sort character varying(512),"
                            "UNIQUE (artist_name, artist_name_sort)"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE artist_artist_id_seq RENAME TO artist_seq"))
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
                            "genre_id SERIAL PRIMARY KEY,"
                            "genre_name VARCHAR(512) NOT NULL UNIQUE"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE genre_genre_id_seq RENAME TO genre_seq"))
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
                            "play_id SERIAL PRIMARY KEY,"
                            "song_id INTEGER NOT NULL,"
                            "play_time TIMESTAMP,"
                            "play_time_utc TIMESTAMP"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("libpath"))
    {
        if (!query.exec("CREATE TABLE libpath ("
                            "path_id SERIAL PRIMARY KEY,"
                            "path_location VARCHAR(512) NOT NULL UNIQUE"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE libpath_path_id_seq RENAME TO libpath_seq"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("equalizer"))
    {
        if (!query.exec("CREATE TABLE equalizer ("
                            "equalizer_id SERIAL PRIMARY KEY,"
                            "equalizer_name character varying(512) NOT NULL,"
                            "equalizer_val0 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val1 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val2 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val3 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val4 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val5 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val6 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val7 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val8 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val9 double precision NOT NULL DEFAULT 1.0"
                        ")"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE equalizer_equalizer_id_seq RENAME TO equalizer_seq"))
        {
            showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }
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


QString CApplication::durationToString(qlonglong durationMS)
{
    QTime duration(0, 0);
    duration = duration.addMSecs(static_cast<int>(durationMS % 86400000));

    if (durationMS > 86400000)
    {
        int numDays = static_cast<int>(durationMS / 86400000);
        return tr("%n day(s) %1", "", numDays).arg(duration.toString());
    }
    else
    {
        return duration.toString();
    }
}

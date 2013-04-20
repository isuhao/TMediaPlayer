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

#ifndef FILE_C_APPLICATION
#define FILE_C_APPLICATION

#include "CSongTableModel.hpp"
#include "CEqualizerPreset.hpp"
#include <QMainWindow>
#include <QList>
#include <QSqlDatabase>
#include <QMutexLocker>
#include <QFile>
#include <QTranslator>
#include "ui_TMediaPlayer.h"
#include "ui_WidgetControl.h"


class CSong;
class CSongTable;
class CFolder;
class CCDRomDrive;
class CQueuePlayList;
class IPlayList;
class CPlayListView;
class CDynamicList;
class CStaticPlayList;
class CLibrary;
class CListModel;
class CLibraryFolder;
class CWidgetLyrics;
class CEqualizerPreset;
class QStandardItemModel;
class QSettings;
class QTextEdit;
class QNetworkReply;

namespace FMOD
{
    class System;
}


/**
 * Fenêtre principale de l'application.
 * Contient la librairie et toutes les listes de lecture, et les paramètres de lecture.
 */

class CApplication : public QMainWindow
{
    Q_OBJECT
        
    friend class CDialogEditStaticPlayList;
    friend class CDialogEditDynamicList;
    friend class CDialogEditFolder;

public:

    static QString getAppVersion();
    static QString getAppDate();


    /// État de lecture.
    enum TState
    {
        Stopped,
        Playing,
        Paused
    };


    /**
     * Mode de répétition.
     * \todo Implémenter la répétition de l'album ou de l'artiste.
     */
    enum TRepeatMode
    {
        NoRepeat     = 0, ///< Pas de répétition.
        RepeatList   = 1, ///< Répétition de la liste.
        RepeatSong   = 2  ///< Répétition du morceau.
      //RepeatAlbum  = 3, ///< Répétition de l'album.
      //RepeatArtist = 4  ///< Répétition de tous les morceaux de l'artiste.
    };


    CApplication();
    virtual ~CApplication();

    bool initWindow();
    void showDatabaseError(const QString& msg, const QString& query, const QString& fileName, int line);


    // Préférences
    void setRowHeight(int height);
    int getRowHeight() const;
    void showButtonStop(bool show = true);
    void showRemainingTime(bool show = true);


    // Last.fm
    void enableScrobbling(bool enable = true);
    void setDelayBeforeNotification(int delay);
    void setPercentageBeforeScrobbling(int percentage);


    // Notifications
    struct TNotification
    {
        QString message; ///< Texte de la notification.
        QDateTime date;  ///< Date de l'envoi.

        TNotification(const QString& m, const QDateTime& d) : message(m), date(d) { }
    };

    inline QList<TNotification> getNotifications() const;


    // Égaliseur
    void setEqualizerGain(CEqualizerPreset::TFrequency frequency, double gain);
    double getEqualizerGain(CEqualizerPreset::TFrequency frequency) const;
    void resetEqualizer();
    bool isEqualizerEnabled() const;
    inline QList<CEqualizerPreset *> getEqualizerPresets() const;
    void addEqualizerPreset(CEqualizerPreset * preset);
    void deleteEqualizerPreset(CEqualizerPreset * preset);
    CEqualizerPreset * getEqualizerPresetFromId(int id) const;
    CEqualizerPreset * getEqualizerPresetFromName(const QString& name) const;
    inline CEqualizerPreset * getCurrentEqualizerPreset() const;
    void setCurrentEqualizerPreset(CEqualizerPreset * equalizer);


    // Dossiers de la médiathèque
    inline QList<CLibraryFolder *> getLibraryFolders() const;
    CLibraryFolder * getLibraryFolder(int folderId) const;
    int getLibraryFolderId(const QString& fileName) const;
    void addLibraryFolder(CLibraryFolder * folder);
    void removeLibraryFolder(CLibraryFolder * folder);


    // Filtre de recherche
    QString getFilter() const;

    inline CSongTableItem * getCurrentSongItem() const;
    inline CSongTable * getCurrentSongTable() const;
    inline CLibrary * getLibrary() const;
    inline CSongTable * getDisplayedSongTable() const;
    void setDisplayedSongTable(CSongTable * songTable);
    void setCurrentSongItem(CSongTableItem * songItem, CSongTable * songTable);
    CSong * getSongFromId(int id) const;
    CFolder * getFolderFromId(int id) const;
    IPlayList * getPlayListFromId(int id) const;
    QList<IPlayList *> getPlayListsWithSong(CSong * song) const;
    QList<IPlayList *> getAllPlayLists() const;
    CSong * addSong(const QString& fileName);
    void removeSongs(const QList<CSong *> songs);
    inline CDialogEditSong * getDialogEditSong() const;
    void setSelectionInformations(int numSongs, qlonglong durationMS);
    void onPlayListChange(IPlayList * playList);

    inline bool isPlaying() const;
    inline bool isPaused() const;
    inline bool isStopped() const;
    inline TRepeatMode getRepeatMode() const;
    inline bool isShuffle() const;
    inline bool isMute() const;
    inline int getVolume() const;
    int getPosition() const;

    int getArtistId(const QString& name, const QString& nameSort);
    int getAlbumId(const QString& title, const QString& titleSort);
    int getGenreId(const QString& name);
    QStringList getGenreList();

    inline FMOD::System * getSoundSystem() const
    {
        return m_soundSystem;
    }

    inline QSqlDatabase getDataBase() const
    {
        return m_dataBase;
    }

    inline QSettings * getSettings() const
    {
        return m_settings;
    }

    inline CQueuePlayList * getQueue() const
    {
        return m_queue;
    }

    inline QList<CCDRomDrive *> getCDRomDrives() const
    {
        return m_cdRomDrives;
    }

    inline QString getApplicationPath() const
    {
        return m_applicationPath;
    }

    QFile * getLogFile(const QString& logName);
    void logError(const QString& message, const QString& function, const char * file, int line);
    void notifyInformation(const QString& message);

    void openDialogCreateStaticList(CFolder * folder, const QList<CSong *>& songs = QList<CSong *>());
    static QString durationToString(qlonglong durationMS);

public slots:

    void selectAll();
    void selectNone();
    void play();
    void stop();
    void pause();
    void togglePlay();
    void previousSong();
    void nextSong();
    void playSong(CSongTableItem * songItem);

#if (QT_VERSION < 0x050000) || __cplusplus < 201103L
    void setRepeatModeNoRepeat()   { setRepeatMode(NoRepeat  ); }
    void setRepeatModeRepeatList() { setRepeatMode(RepeatList); }
    void setRepeatModeRepeatSong() { setRepeatMode(RepeatSong); }
#endif
    void setNextRepeatMode();
    void setRepeatMode(TRepeatMode repeatMode);
    
#if QT_VERSION < 0x050000
    void setShuffle();
#endif

    void setShuffle(bool shuffle);
    void setMute(bool mute);
    void toggleMute();
    void setVolume(int volume);
    void setPosition(int position);
    void setEqualizerEnabled(bool enabled = true);

    void openDialogPreferences();
    void openDialogNotifications();
    void openDialogLastPlays();
    void openDialogEqualizer();
    void openDialogEditMetadata();
    void openDialogAddSongs();
    void openDialogAddFolder();
    void openDialogSongInfos();
    
    void openDialogCreateStaticList();
    void openDialogCreateDynamicList(CFolder * folder = NULL);
    void openDialogCreateFolder(CFolder * folder = NULL);
    void openDialogEditStaticPlayList(CStaticPlayList * playList);
    void openDialogEditDynamicList(CDynamicList * playList);
    void openDialogEditFolder(CFolder * folder);
    void openDialogAbout();
    void relocateSong();
    void importFromITunes();
    void importFromSongbird();
    void selectCurrentSong();
    void selectSong(CSongTable * songTable, CSongTableItem * songItem);
    void openSongInExplorer();
    void editSelectedItem();
    void removeSelectedItem();
    void onSongModified();
    
#if QT_VERSION >= 0x050000
    void openDialogCreateStaticList_Slot(bool checked) { openDialogCreateStaticList(); }
    void openDialogCreateDynamicList_Slot(bool checked) { openDialogCreateDynamicList(); }
    void openDialogCreateFolder_Slot(bool checked) { openDialogCreateFolder(); }
#endif

signals:

    // Signaux sur les morceaux
    void songAdded(CSong * song);             ///< Signal émis lorsqu'un morceau est ajouté à la médiathèque.
    void songsAdded();                        ///< Signal émis lorsqu'un ou plusieurs morceaux sont ajoutés à la médiathèque.
    void songModified(CSong * song);          ///< Signal émis lorsque les informations d'un morceau sont modifiées.
    void songRemoved(CSong * song);           ///< Signal émis lorsqu'un morceau est retiré de la médiathèque.
  //void songMoved(CSong * song);             ///< Signal émis lorsque le fichier d'un morceau est déplacé.
    void songPlayStart(CSong * song);         ///< Signal émis lorsque la lecture d'un morceau démarre.
    void songPlayEnd(CSong * song);           ///< Signal émis lorsque la lecture d'un morceau se termine.
    void songPaused(CSong * song);            ///< Signal émis lorsque la lecture est mise en pause.
    void songResumed(CSong * song);           ///< Signal émis lorsque la lecture est relancée.
    void songStopped(CSong * song);           ///< Signal émis lorsque la lecture est stoppée.

    // Signaux sur les listes de lecture
  //void listAdded(IPlayList * playList);     ///< Signal émis lorsqu'une liste est créée.
    void listModified(IPlayList * playList);  ///< Signal émis lorsqu'une liste est modifiée.
  //void listRemoved(IPlayList * playList);   ///< Signal émis lorsqu'une liste est supprimée.

    // Signaux sur les dossiers
  //void folderAdded(CFolder * folder);      ///< Signal émis lorsqu'un dossier est crée.
  //void folderRemoved(CFolder * folder);    ///< Signal émis lorsqu'un dossier est supprimé.

protected slots:
    
#ifndef T_NO_SINGLE_APP
	void activateThisWindow();
#endif // T_NO_SINGLE_APP

    void onPlayEnd();
    void updateSongDescription(CSong * song);
    void updateListInformations();
    void updatePosition();
    void updateTimer();
    void selectPlayListFromTreeView(const QModelIndex& index);
    void connectToLastFm();
    void onDialogEditSongClosed();
    void onFilterChange(const QString& filter);
    void clearFilter() { onFilterChange(QString()); }

protected:

    void addPlayList(IPlayList * playList);
    void addFolder(CFolder * folder);
    QStringList importFolder(const QString& pathName);
    void importSongs(const QStringList& fileList);
    void displaySongTable(CSongTable * songTable);
    bool initSoundSystem();
    void loadDatabase();
    void startPlay();
    void setState(TState state);

    void createDatabaseSQLite();
    void createDatabaseMySQL();
    void createDatabasePostgreSQL();

    virtual void closeEvent(QCloseEvent * event);

private:

    Ui::TMediaPlayer * m_uiWidget;        ///< Widget représentant la fenêtre principale.
    Ui::WidgetControl * m_uiControl;      ///< Widget représentant la barre de contrôle.
    QTranslator m_translator;
    QList<CCDRomDrive *> m_cdRomDrives;   ///< Liste des lecteurs de CD-ROM.
    CQueuePlayList * m_queue;             ///< File d'attente.
    FMOD::System * m_soundSystem;         ///< Système de son de FMOD.
    CPlayListView * m_playListView;       ///< Vue pour afficher les listes de lecture.
    CListModel * m_listModel;             ///< Modèle contenant les listes de lecture.
    CDialogEditSong * m_dialogEditSong;   ///< Pointeur sur la boite de dialogue pour modifier les informations d'un morceau.
    QSqlDatabase m_dataBase;              ///< Base de données.
    QSettings * m_settings;               ///< Paramètres de l'application.
    QTimer * m_timer;                     ///< Timer pour mettre à jour l'affichage.
    QLabel * m_listInfos;                 ///< Label pour afficher les informations sur la liste affichée.
    CSongTableItem * m_currentSongItem;   ///< Pointeur sur l'item en cours de lecture.
    CSongTable * m_currentSongTable;      ///< Liste de morceaux contenant le morceau en cours de lecture.
    CLibrary * m_library;                 ///< Librairie (liste de tous les morceaux).
    CSongTable * m_displayedSongTable;    ///< Liste de morceaux affichée.
    CWidgetLyrics * m_widgetLyrics;       ///< Widget pour visualiser et modifier les paroles des morceaux.
    TState m_state;                       ///< État de lecture.
    bool m_showRemainingTime;             ///< Indique si on doit afficher le temps restant ou la durée du morceau en cours de lecture.
    TRepeatMode m_repeatMode;             ///< Mode de répétition.
    bool m_isShuffle;                     ///< Indique si la lecture aléatoire est activée.
    bool m_isMute;                        ///< Indique si le son est coupé.
    int m_volume;                         ///< Volume sonore (entre 0 et 100).

    // Égaliseur
    double m_equalizerGains[10];                  ///< Gains de l'égaliseur.
    FMOD::DSP * m_dsp[10];                        ///< Gains de l'égaliseur pour FMOD.
    QList<CEqualizerPreset *> m_equalizerPresets; ///< Liste des préréglages d'égaliseur.
    CEqualizerPreset * m_currentEqualizerPreset;  ///< Préréglage de l'égaliseur actuel.

    QMap<QString, QFile *> m_logList;         ///< Liste des fichiers de log ouverts.
    QString m_applicationPath;                ///< Répertoire contenant l'application.
    QList<CLibraryFolder *> m_libraryFolders; ///< Liste des répertoires de la médiathèque.

    QList<TNotification> m_infosNotified;     ///< Liste des notifications.

    // État de Last.fm
    enum TLastFmState
    {
        NoScrobble, ///< Pas de scrobble.
        Started,    ///< Lecture commencé.
        Notified,   ///< Notification de lecture.
        Scrobbled   ///< Scrobble effectué.
    };

    bool m_lastFmEnableScrobble;          ///< Indique si le scrobbling est activé.
    int m_delayBeforeNotification;        ///< Délai avant d'envoyer une notification.
    int m_percentageBeforeScrobbling;     ///< Pourcentage d'écouter avant de scrobbler.
    QByteArray m_lastFmKey;               ///< Clé utilisée pour l'authentification à Last.fm.
    int m_lastFmTimeListened;
    int m_lastFmLastPosition;
    TLastFmState m_lastFmState;           ///< État actuel de Last.fm.
};


QList<CApplication::TNotification> CApplication::getNotifications() const
{
    return m_infosNotified;
}


inline QList<CEqualizerPreset *> CApplication::getEqualizerPresets() const
{
    return m_equalizerPresets;
}


inline CEqualizerPreset * CApplication::getCurrentEqualizerPreset() const
{
    return m_currentEqualizerPreset;
}


/**
 * Retourne la chanson actuellement lue.
 *
 * \return Chanson en cours de lecture, ou NULL.
 */

inline CSongTableItem * CApplication::getCurrentSongItem() const
{
    return m_currentSongItem;
}


/**
 * Retourne la liste de morceaux actuellement lue.
 *
 * \return Liste de morceaux en cours de lecture, ou NULL.
 */

inline CSongTable * CApplication::getCurrentSongTable() const
{
    return m_currentSongTable;
}


/**
 * Retourne le médiathèque.
 *
 * \return Médiathèque, qui contient l'ensemble des morceaux.
 */

inline CLibrary * CApplication::getLibrary() const
{
    return m_library;
}


/**
 * Retourne la liste de morceaux actuellement affichée.
 *
 * \return Liste de morceaux affichée.
 */

inline CSongTable * CApplication::getDisplayedSongTable() const
{
    return m_displayedSongTable;
}


inline CDialogEditSong * CApplication::getDialogEditSong() const
{
    return m_dialogEditSong;
}


/**
 * Indique si la lecture est en cours.
 *
 * \return Booléen.
 */

inline bool CApplication::isPlaying() const
{
    return (m_state == Playing);
}


/**
 * Indique si la lecture est en pause.
 *
 * \return Booléen.
 */

inline bool CApplication::isPaused() const
{
    return (m_state == Paused);
}


/**
 * Indique si la lecture est stoppée.
 *
 * \return Booléen.
 */

inline bool CApplication::isStopped() const
{
    return (m_state == Stopped);
}


/**
 * Retourne le mode de répétition.
 *
 * \return Mode de répétition.
 */

inline CApplication::TRepeatMode CApplication::getRepeatMode() const
{
    return m_repeatMode;
}


/**
 * Indique si la lecture aléatoire est active.
 *
 * \return Booléen.
 */

bool CApplication::isShuffle() const
{
    return m_isShuffle;
}


/**
 * Indique si le son est coupé.
 *
 * \return Booléen.
 */

inline bool CApplication::isMute() const
{
    return m_isMute;
}


/**
 * Donne le volume sonore.
 *
 * \return Volume sonore, entre 0 et 100.
 */

inline int CApplication::getVolume() const
{
    return m_volume;
}


inline QList<CLibraryFolder *> CApplication::getLibraryFolders() const
{
    return m_libraryFolders;
}

#endif // FILE_C_APPLICATION

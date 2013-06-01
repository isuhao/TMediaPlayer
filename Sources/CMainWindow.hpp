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

#ifndef FILE_C_MAIN_WINDOW_HPP_
#define FILE_C_MAIN_WINDOW_HPP_

#include <QMainWindow>
#include <QList>
#include <QMutexLocker>
#include <QFile>
#include <QTimer>

#include "ui_TMediaPlayer.h"
#include "ui_WidgetControl.h"


class CSong;
class CMediaTableView;
class CMediaTableItem;
class CFolder;
class CCDRomDrive;
class CQueuePlayList;
class CLibraryView;
class CDynamicList;
class CStaticList;
class CLibrary;
class CLibraryModel;
class CDialogEditSong;
class CWidgetLyrics;
class CMediaManager;
class CEqualizerPreset;
class IPlayList;

class QStandardItemModel;
class QTextEdit;
class QNetworkReply;


/**
 * Fenêtre principale de l'application.
 * Contient la librairie et toutes les listes de lecture, et les paramètres de lecture.
 */

class CMainWindow : public QMainWindow
{
    Q_OBJECT

    friend class CDialogEditStaticPlayList;
    friend class CDialogEditDynamicList;
    friend class CDialogEditFolder;

public:

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


    explicit CMainWindow(CMediaManager * mediaManager);
    virtual ~CMainWindow();

    bool initWindow();
    //void showDatabaseError(const QString& msg, const QString& query, const QString& fileName, int line);


    // Préférences
    void setRowHeight(int height);
    int getRowHeight() const;
    void showButtonStop(bool show = true);
    void showRemainingTime(bool show = true);


    // Last.fm
    void enableScrobbling(bool enable = true);
    void setDelayBeforeNotification(int delay);
    void setPercentageBeforeScrobbling(int percentage);


    // Filtre de recherche
    QString getFilter() const;

    inline CMediaTableItem * getCurrentSongItem() const;
    inline CMediaTableView * getCurrentSongTable() const;
    inline CLibrary * getLibrary() const;
    inline CMediaTableView * getDisplayedSongTable() const;
    void setDisplayedSongTable(CMediaTableView * songTable);
    void setCurrentSongItem(CMediaTableItem * songItem, CMediaTableView * songTable);
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
    int getPosition() const;

    inline CQueuePlayList * getQueue() const
    {
        return m_queue;
    }

    inline QList<CCDRomDrive *> getCDRomDrives() const
    {
        return m_cdRomDrives;
    }

    inline CMediaManager * getMediaManager() const
    {
        return m_mediaManager;
    }

    void openDialogCreateStaticList(CFolder * folder, const QList<CSong *>& songs = QList<CSong *>());

public slots:

    void notifyInformation2(const QString& message);

    void selectAll();
    void selectNone();
    void play();
    void stop();
    void pause();
    void togglePlay();
    void previousSong();
    void nextSong();
    void changeCurrentSongList(CMediaTableItem * songItem, CMediaTableView * playList);
    void playSong(CMediaTableItem * songItem);

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

    void openDialogPreferences();
    void openDialogNotifications();
    void openDialogLastPlays();
    void openDialogEqualizer();
    void openDialogEditMetadata();
    void openDialogAddSongs();
    void openDialogAddFolder();
    void openDialogSongInfos();

    void openDialogCreateStaticList();
    void openDialogCreateDynamicList(CFolder * folder = nullptr);
    void openDialogCreateFolder(CFolder * folder = nullptr);
    void openDialogEditStaticPlayList(CStaticList * playList);
    void openDialogEditDynamicList(CDynamicList * playList);
    void openDialogEditFolder(CFolder * folder);
    void openDialogAbout();
    void relocateSong();
    void importFromITunes();
    void importFromSongbird();
    void selectCurrentSong();
    void selectSong(CMediaTableView * songTable, CMediaTableItem * songItem);
    void openSongInExplorer();
    void editSelectedItem();
    void removeSelectedItem();
    //void onSongModified();

#if QT_VERSION >= 0x050000
    void openDialogCreateStaticList_Slot(bool checked) { openDialogCreateStaticList(); }
    void openDialogCreateDynamicList_Slot(bool checked) { openDialogCreateDynamicList(); }
    void openDialogCreateFolder_Slot(bool checked) { openDialogCreateFolder(); }
#endif

signals:

    // Signaux sur les morceaux
    void songAdded(CSong * song);             ///< Signal émis lorsqu'un morceau est ajouté à la médiathèque.
    void songsAdded();                        ///< Signal émis lorsqu'un ou plusieurs morceaux sont ajoutés à la médiathèque.
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
    void updateCDRomDrives();
    void selectPlayListFromTreeView(const QModelIndex& index);
    void connectToLastFm();
    void onDialogEditSongClosed();
    void onFilterChange(const QString& filter);
    void clearFilter() { onFilterChange(QString()); }

protected:

    void addPlayList(IPlayList * playList);
    void addFolder(CFolder * folder);
    void importSongs(const QStringList& fileList);
    void initSoundSystem();
    void displaySongTable(CMediaTableView * songTable);
    void loadDatabase();
    void startPlay();
    void setState(TState state);

    virtual void closeEvent(QCloseEvent * event);

private:

    CMediaManager * m_mediaManager;         ///< Pointeur sur le gestionnaire de médias.
    Ui::TMediaPlayer * m_uiWidget;          ///< Widget représentant la fenêtre principale.
    Ui::WidgetControl * m_uiControl;        ///< Widget représentant la barre de contrôle.
    QList<CCDRomDrive *> m_cdRomDrives;     ///< Liste des lecteurs de CD-ROM.
    CQueuePlayList * m_queue;               ///< File d'attente.
    CLibraryView * m_playListView;          ///< Vue pour afficher les listes de lecture.
    CLibraryModel * m_listModel;            ///< Modèle contenant les listes de lecture.
    CDialogEditSong * m_dialogEditSong;     ///< Pointeur sur la boite de dialogue pour modifier les informations d'un morceau.
    QTimer m_timer;                         ///< Timer pour mettre à jour l'affichage.
    QTimer m_timerCDRomDrives;              ///< Timer pour mettre à jour les lecteurs de CD-Rom.
    QLabel * m_listInfos;                   ///< Label pour afficher les informations sur la liste affichée.
    CMediaTableItem * m_currentSongItem;    ///< Pointeur sur l'item en cours de lecture.
    CMediaTableView * m_currentSongTable;   ///< Liste de morceaux contenant le morceau en cours de lecture.
    CLibrary * m_library;                   ///< Librairie (liste de tous les morceaux).
    CMediaTableView * m_displayedSongTable; ///< Liste de morceaux affichée.
    CWidgetLyrics * m_widgetLyrics;         ///< Widget pour visualiser et modifier les paroles des morceaux.
    TState m_state;                         ///< État de lecture.
    bool m_showRemainingTime;               ///< Indique si on doit afficher le temps restant ou la durée du morceau en cours de lecture.
    TRepeatMode m_repeatMode;               ///< Mode de répétition.
    bool m_isShuffle;                       ///< Indique si la lecture aléatoire est activée.

    // États de Last.fm
    enum TLastFmState
    {
        NoScrobble, ///< Pas de scrobble.
        Started,    ///< Lecture commencée.
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


/**
 * Retourne la chanson actuellement lue.
 *
 * \return Chanson en cours de lecture, ou nullptr.
 */

inline CMediaTableItem * CMainWindow::getCurrentSongItem() const
{
    return m_currentSongItem;
}


/**
 * Retourne la liste de morceaux actuellement lue.
 *
 * \return Liste de morceaux en cours de lecture, ou nullptr.
 */

inline CMediaTableView * CMainWindow::getCurrentSongTable() const
{
    return m_currentSongTable;
}


/**
 * Retourne le médiathèque.
 *
 * \return Médiathèque, qui contient l'ensemble des morceaux.
 */

inline CLibrary * CMainWindow::getLibrary() const
{
    return m_library;
}


/**
 * Retourne la liste de morceaux actuellement affichée.
 *
 * \return Liste de morceaux affichée.
 */

inline CMediaTableView * CMainWindow::getDisplayedSongTable() const
{
    return m_displayedSongTable;
}


inline CDialogEditSong * CMainWindow::getDialogEditSong() const
{
    return m_dialogEditSong;
}


/**
 * Indique si la lecture est en cours.
 *
 * \return Booléen.
 */

inline bool CMainWindow::isPlaying() const
{
    return (m_state == Playing);
}


/**
 * Indique si la lecture est en pause.
 *
 * \return Booléen.
 */

inline bool CMainWindow::isPaused() const
{
    return (m_state == Paused);
}


/**
 * Indique si la lecture est stoppée.
 *
 * \return Booléen.
 */

inline bool CMainWindow::isStopped() const
{
    return (m_state == Stopped);
}


/**
 * Retourne le mode de répétition.
 *
 * \return Mode de répétition.
 */

inline CMainWindow::TRepeatMode CMainWindow::getRepeatMode() const
{
    return m_repeatMode;
}


/**
 * Indique si la lecture aléatoire est active.
 *
 * \return Booléen.
 */

bool CMainWindow::isShuffle() const
{
    return m_isShuffle;
}

#endif // FILE_C_MAIN_WINDOW_HPP_

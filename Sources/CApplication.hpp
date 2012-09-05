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

#ifndef FILE_C_APPLICATION
#define FILE_C_APPLICATION

#include <QMainWindow>
#include <QList>
#include <QSqlDatabase>
#include <QMutexLocker>
#include <QFile>
#include "CSongTableModel.hpp"
#include "ui_TMediaPlayer.h"
#include "ui_WidgetControl.h"


class CSong;
class CSongTable;
class CFolder;
class IPlayList;
class CPlayListView;
class CDynamicList;
class CStaticPlayList;
class CLibrary;
class CListModel;
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
    friend class CDynamicList;

public:

    enum State
    {
        Stopped,
        Playing,
        Paused
    };

    CApplication(void);
    virtual ~CApplication();

    bool initWindow(void);
    void showDatabaseError(const QString& msg, const QString& query, const QString& fileName, int line);

    // Préférences
    void setRowHeight(int height);
    int getRowHeight(void) const;
    void showButtonStop(bool show = true);
    void showRemainingTime(bool show = true);

    // Last.fm
    void enableScrobbling(bool enable = true);
    void setDelayBeforeNotification(int delay);
    void setPercentageBeforeScrobbling(int percentage);

    // Égaliseur
    enum TEqualizerFrequency
    {
        EqFreq32  = 0, ///< 32 Hz.
        EqFreq64  = 1, ///< 64 Hz.
        EqFreq125 = 2, ///< 125 Hz.
        EqFreq250 = 3, ///< 250 Hz.
        EqFreq500 = 4, ///< 500 Hz.
        EqFreq1K  = 5, ///< 1 KHz.
        EqFreq2K  = 6, ///< 2 kHz.
        EqFreq4K  = 7, ///< 4 kHz.
        EqFreq8K  = 8, ///< 8 kHz.
        EqFreq16K = 9  ///< 16 kHz.
    };

    void setEqualizerGain(TEqualizerFrequency frequency, double gain);
    double getEqualizerGain(TEqualizerFrequency frequency);
    void resetEqualizer(void);
    bool isEqualizerEnabled(void) const;

    inline CSongTableItem * getCurrentSongItem(void) const;
    inline CSongTable * getCurrentSongTable(void) const;
    inline CLibrary * getLibrary(void) const;
    inline CSongTable * getDisplayedSongTable(void) const;
    void setDisplayedSongTable(CSongTable * songTable);
    CSong * getSongFromId(int id) const;
    CFolder * getFolderFromId(int id) const;
    IPlayList * getPlayListFromId(int id) const;
    QList<IPlayList *> getPlayListsWithSong(CSong * song) const;
    QList<IPlayList *> getAllPlayLists(void) const;
    CSong * addSong(const QString& fileName);
    void removeSongs(const QList<CSong *> songs);
    inline CDialogEditSong * getDialogEditSong(void) const;

    inline bool isPlaying(void) const;
    inline bool isPaused(void) const;
    inline bool isStopped(void) const;
    bool isRepeat(void) const;
    bool isShuffle(void) const;
    bool isMute(void) const;
    int getVolume(void) const;
    int getPosition(void) const;

    int getArtistId(const QString& name, const QString& nameSort);
    int getAlbumId(const QString& title, const QString& titleSort);
    int getGenreId(const QString& name);
    QStringList getGenreList(void);

    inline FMOD::System * getSoundSystem(void) const
    {
        return m_soundSystem;
    }

    inline QSqlDatabase getDataBase(void) const
    {
        return m_dataBase;
    }

    inline QSettings * getSettings(void) const
    {
        return m_settings;
    }

    QFile * getLogFile(const QString& logName);
    void logError(const QString& message, const QString& function, const char * file, int line);
    void notifyInformation(const QString& message);

    void openDialogCreateStaticList(CFolder * folder, const QList<CSong *>& songs = QList<CSong *>());

public slots:

    void selectAll(void);
    void selectNone(void);
    void play(void);
    void stop(void);
    void pause(void);
    void togglePlay(void);
    void previousSong(void);
    void nextSong(void);
    void playSong(CSongTableItem * songItem);
    void setRepeat(void);
    void setRepeat(bool repeat);
    void setShuffle(void);
    void setShuffle(bool shuffle);
    void setMute(bool mute);
    void toggleMute(void);
    void setVolume(int volume);
    void setPosition(int position);
    void setEqualizerEnabled(bool enabled = true);

    //void openPlayList(IPlayList * playList);
    //void renamePlayList(IPlayList * playList);
    //void editDynamicPlayList(CDynamicList * playList);
    //void deletePlayList(IPlayList * playList);
    //void addListFolder(void);
    //void renameListFolder(CFolder * folder);
    //void deleteListFolder(CFolder * folder);

    void openDialogPreferences(void);
    void openDialogEqualizer(void);
    void openDialogEditMetadata(void);
    void openDialogAddSongs(void);
    void openDialogAddFolder(void);
    void openDialogSongInfos(void);
    void openDialogCreateStaticList(void);
    void openDialogCreateDynamicList(CFolder * folder = NULL);
    void openDialogCreateFolder(CFolder * folder = NULL);
    void openDialogEditStaticPlayList(CStaticPlayList * playList);
    void openDialogEditDynamicList(CDynamicList * playList);
    void openDialogEditFolder(CFolder * folder);
    void relocateSong(void);
    void importFromITunes(void);
    void importFromSongbird(void);
    //void editSong(CSongTableItem * songItem);
    //void removeSong(CSongTableItem * songItem);
    void selectCurrentSong(void);
    void selectSong(CSongTable * songTable, CSongTableItem * songItem);
    void openSongInExplorer(void);
    void editSelectedItem(void);
    void removeSelectedItem(void);
    void onSongModified(void);
    void findLyrics(void);

signals:

    // Signaux sur les morceaux
    void songAdded(CSong * song);             ///< Signal émis lorsqu'un morceau est ajouté à la médiathèque.
    void songsAdded(void);                    ///< Signal émis lorsqu'un ou plusieurs morceaux sont ajoutés à la médiathèque.
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
    //void listRemoved(IPlayList * playList);   ///< Signal émis lorsqu'une liste est supprimée.

    // Signaux sur les dossiers
    //void folderAdded(CFolder * folder);   ///< Signal émis lorsqu'un dossier est crée.
    //void folderRemoved(CFolder * folder); ///< Signal émis lorsqu'un dossier est supprimé.

protected slots:

    void onPlayEnd(void);
    void updateSongDescription(CSong * song);
    void updateListInformations(void);
    void updatePosition(void);
    void updateTimer(void);
    void selectPlayListFromTreeView(const QModelIndex& index);
    void connectToLastFm(void);
    void onDialogEditSongClosed(void);
    void onFilterChange(const QString& filter);

protected:

    void addPlayList(IPlayList * playList);
    //void initPlayList(IPlayList * playList);
    void addFolder(CFolder * folder);
    //void initFolder(CFolder * folder);
    QStringList importFolder(const QString& pathName);
    void importSongs(const QStringList& fileList);
    void displaySongTable(CSongTable * songTable);
    bool initSoundSystem(void);
    void loadDatabase(void);
    void startPlay(void);
    void setState(State state);

    virtual void keyPressEvent(QKeyEvent * event);
    virtual void closeEvent(QCloseEvent * event);

private:

    Ui::TMediaPlayer * m_uiWidget;      ///< Widget représentant la fenêtre principale.
    Ui::WidgetControl * m_uiControl;    ///< Widget représentant la barre de contrôle.
    FMOD::System * m_soundSystem;       ///< Système de son de FMOD.
    CPlayListView * m_playListView;     ///< Vue pour afficher les listes de lecture.
    CListModel * m_listModel;           ///< Modèle contenant les listes de lecture.
    CDialogEditSong * m_dialogEditSong; ///< Pointeur sur la boite de dialogue pour modifier les informations d'un morceau.
    QSqlDatabase m_dataBase;            ///< Base de données.
    QSettings * m_settings;             ///< Paramètres de l'application.
    QTimer * m_timer;                   ///< Timer pour mettre à jour l'affichage.
    QLabel * m_listInfos;               ///< Label pour afficher les informations sur la liste affichée.
    CSongTableItem * m_currentSongItem; ///< Pointeur sur l'item en cours de lecture.
    CSongTable * m_currentSongTable;    ///< Liste de morceaux contenant le morceau en cours de lecture.
    CLibrary * m_library;               ///< Librairie (liste de tous les morceaux).
    CSongTable * m_displayedSongTable;  ///< Liste de morceaux affichée.
    QTextEdit * m_lyricsEdit;           ///< Zone de texte pour les paroles.
    State m_state;                      ///< État de lecture.
    bool m_showRemainingTime;           ///< Indique si on doit afficher le temps restant ou la durée du morceau en cours de lecture.
    bool m_isRepeat;                    ///< Indique si la répétition est activée.
    bool m_isShuffle;                   ///< Indique si la lecture aléatoire est activée.
    bool m_isMute;                      ///< Indique si le son est coupé.
    int m_volume;                       ///< Volume sonore (entre 0 et 100).
    double m_equalizerGains[10];        ///< Gains de l'égaliseur.
    FMOD::DSP * m_dsp[10];
    //QList<CFolder *> m_folders;         ///< Liste de l'ensemble des dossiers de listes de lecture.
    //QList<IPlayList *> m_playLists;     ///< Liste de l'ensemble des listes de lectures.
    QMap<QString, QFile *> m_logList;   ///< Liste des fichiers de log ouverts.
    QString m_applicationPath;

    // Last.fm
    enum TLastFmState
    {
        NoScrobble,
        Started,
        Notified,
        Scrobbled
    };

    bool m_lastFmEnableScrobble;      ///< Indique si le scrobbling est activé.
    int m_delayBeforeNotification;    ///< Délai avant d'envoyer une notification.
    int m_percentageBeforeScrobbling; ///< Pourcentage d'écouter avant de scrobbler.
    QByteArray m_lastFmKey;           ///< Clé utilisée pour l'authentification à Last.fm.
    int m_lastFmTimeListened;
    int m_lastFmLastPosition;
    TLastFmState m_lastFmState;
};


/**
 * Retourne la chanson actuellement lue.
 *
 * \return Chanson en cours de lecture, ou NULL.
 */

inline CSongTableItem * CApplication::getCurrentSongItem(void) const
{
    return m_currentSongItem;
}


/**
 * Retourne la liste de morceaux actuellement lue.
 *
 * \return Liste de morceaux en cours de lecture, ou NULL.
 */

inline CSongTable * CApplication::getCurrentSongTable(void) const
{
    return m_currentSongTable;
}


/**
 * Retourne le médiathèque.
 *
 * \return Médiathèque, qui contient l'ensemble des morceaux.
 */

inline CLibrary * CApplication::getLibrary(void) const
{
    return m_library;
}


/**
 * Retourne la liste de morceaux actuellement affichée.
 *
 * \return Liste de morceaux affichée.
 */

inline CSongTable * CApplication::getDisplayedSongTable(void) const
{
    return m_displayedSongTable;
}


inline CDialogEditSong * CApplication::getDialogEditSong(void) const
{
    return m_dialogEditSong;
}


/**
 * Indique si la lecture est en cours.
 *
 * \return Booléen.
 */

inline bool CApplication::isPlaying(void) const
{
    return (m_state == Playing);
}


/**
 * Indique si la lecture est en pause.
 *
 * \return Booléen.
 */

inline bool CApplication::isPaused(void) const
{
    return (m_state == Paused);
}


/**
 * Indique si la lecture est stoppée.
 *
 * \return Booléen.
 */

inline bool CApplication::isStopped(void) const
{
    return (m_state == Stopped);
}

#endif // FILE_C_APPLICATION

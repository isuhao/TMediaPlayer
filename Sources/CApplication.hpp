
#ifndef FILE_C_APPLICATION
#define FILE_C_APPLICATION

#include <QMainWindow>
#include <QList>
#include <QSqlDatabase>
#include <QMutexLocker>
#include "CSongTableModel.hpp"
#include "ui_TMediaPlayer.h"


class CSong;
class CSongTable;
class CListFolder;
class CPlayList;
class CPlayListView;
class CDynamicPlayList;
class QStandardItemModel;
class QSettings;
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

public:

    enum State
    {
        Stopped,
        Playing,
        Paused
    };

    CApplication(void);
    ~CApplication();

    void initWindow(void);
    void showDatabaseError(const QString& msg, const QString& query, const QString& fileName, int line);

    // Préférences
    void setRowHeight(int height);
    int getRowHeight(void) const;
    void showButtonStop(bool show = true);

    // Last.fm
    void enableScrobbling(bool enable = true);
    void setDelayBeforeNotification(int delay);
    void setPercentageBeforeScrobbling(int percentage);

    inline CSongTableItem * getCurrentSongItem(void) const;
    inline CSongTable * getCurrentSongTable(void) const;
    inline CSongTable * getLibrary(void) const;
    inline CSongTable * getDisplayedSongTable(void) const;
    void setDisplayedSongTable(CSongTable * songTable);
    CSong * getSongFromId(int id) const;
    QList<CPlayList *> getPlayListsWithSong(CSong * song) const;
    QList<CPlayList *> getAllPlayLists(void) const;

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

    void setRepeat(bool repeat);
    void setShuffle(bool shuffle);
    void setMute(bool mute);
    void toggleMute(void);
    void setVolume(int volume);

    void setPosition(int position);

    //void openPlayList(CPlayList * playList);
    //void renamePlayList(CPlayList * playList);
    //void editDynamicPlayList(CDynamicPlayList * playList);
    //void deletePlayList(CPlayList * playList);

    //void addListFolder(void);
    //void renameListFolder(CListFolder * folder);
    //void deleteListFolder(CListFolder * folder);

    void openDialogPreferences(void);
    void openDialogAddSongs(void);
    void openDialogAddFolder(void);
    void openDialogSongInfos(void);
    void openDialogAddStaticPlayList(CListFolder * folder = NULL);
    void openDialogAddDynamicList(CListFolder * folder = NULL);

    //void editSong(CSongTableItem * songItem);
    //void removeSong(CSongTableItem * songItem);

    void selectCurrentSong(void);
    void selectSong(CSongTable * songTable, CSongTableItem * songItem);

    void openSongInExplorer(void);

signals:

    // Signaux sur les morceaux
    //void songAdded(CSong * song);             ///< Signal émis lorsqu'un morceau est ajouté à la médiathèque.
    //void songModified(CSong * song);          ///< Signal émis lorsque les informations d'un morceau sont modifiées.
    //void songRemoved(CSong * song);           ///< Signal émis lorsqu'un morceau est retiré de la médiathèque.
    void songPlayStart(CSong * song);         ///< Signal émis lorsque la lecture d'un morceau démarre.
    void songPlayEnd(CSong * song);           ///< Signal émis lorsque la lecture d'un morceau se termine.
    void songPaused(CSong * song);            ///< Signal émis lorsque la lecture est mise en pause.
    void songResumed(CSong * song);           ///< Signal émis lorsque la lecture est relancée.
    void songStopped(CSong * song);           ///< Signal émis lorsque la lecture est stoppée.

    // Signaux sur les listes de lecture
    //void listAdded(CPlayList * playList);     ///< Signal émis lorsqu'une liste est créée.
    //void listRemoved(CPlayList * playList);   ///< Signal émis lorsqu'une liste est supprimée.

    // Signaux sur les dossiers
    //void folderAdded(CListFolder * folder);   ///< Signal émis lorsqu'un dossier est crée.
    //void folderRemoved(CListFolder * folder); ///< Signal émis lorsqu'un dossier est supprimé.

protected slots:

    void onPlayEnd(void);
    void updateSongDescription(CSong * song);
    void updateListInformations(void);
    void updatePosition(void);
    void update(void);
    void selectPlayListFromTreeView(const QModelIndex& index);

    // Last.fm
    void connectToLastFm(void);
    //void replyLastFmGetToken(QNetworkReply * reply);
    //void getLastFmSession(void);
    //void replyLastFmFinished(QNetworkReply * reply);
    //void replyLastFmUpdateNowPlaying(QNetworkReply * reply);

protected:

    void addPlayList(CPlayList * playList);
    void addSong(const QString& fileName);
    QStringList addFolder(const QString& pathName);

    void displaySongTable(CSongTable * songTable);
    bool initSoundSystem(void);
    void loadDatabase(void);
    void startPlay(void);

    virtual void keyPressEvent(QKeyEvent * event);
    virtual void closeEvent(QCloseEvent * event);

    // Last.fm
    //void scrobbleLastFm(CSong * song);
    //void updateLastFmNowPlaying(CSong * song);

private:

    Ui::TMediaPlayer * m_uiWidget;      ///< Widget représentant la fenêtre principale.
    FMOD::System * m_soundSystem;       ///< Système de son de FMOD.
    CPlayListView * m_playListView;     ///< Vue pour afficher les listes de lecture.
    QSqlDatabase m_dataBase;            ///< Base de données.
    QSettings * m_settings;             ///< Paramètres de l'application.
    QTimer * m_timer;                   ///< Timer pour mettre à jour l'affichage.
    QLabel * m_listInfos;               ///< Label pour afficher les informations sur la liste affichée.
    CSongTableItem * m_currentSongItem; ///< Pointeur sur l'item en cours de lecture.
    CSongTable * m_currentSongTable;    ///< Liste de morceaux contenant le morceau en cours de lecture. \todo Fusion avec le précédent param.
    CSongTable * m_library;             ///< Librairie (liste de tous les morceaux).
    CSongTable * m_displayedSongTable;  ///< Liste de morceaux affichée.
    State m_state;                      ///< État de lecture.
    bool m_isRepeat;                    ///< Indique si la répétition est activée.
    bool m_isShuffle;                   ///< Indique si la lecture aléatoire est activée.
    bool m_isMute;                      ///< Indique si le son est coupé.
    int m_volume;                       ///< Volume sonore (entre 0 et 100).
    QList<CListFolder *> m_folders;     ///< Liste des dossiers de listes de lecture.
    QList<CPlayList *> m_playLists;     ///< Liste des listes de lectures sans dossier.

    // Last.fm
    enum TLastFmState
    {
        NoScrobble,
        Started,
        Notified,
        Scrobbled
    };

    bool m_lastFmEnableScrobble;        ///< Indique si le scrobbling est activé.
    int m_delayBeforeNotification;      ///< Délai avant d'envoyer une notification.
    int m_percentageBeforeScrobbling;   ///< Pourcentage d'écouter avant de scrobbler.
    //QTimer * m_timerLastFm;             ///< Timer utilisé pour récupérer la clé.
    //int m_lastFmSessionRequest;         ///< Nombre de requête envoyées pour récupérer la clé.
    //QByteArray m_lastFmToken;           ///< Token utilisé pour la connexion à Last.fm.
    QByteArray m_lastFmKey;             ///< Clé utilisée pour l'authentification à Last.fm.
    //const QByteArray m_lastFmAPIKey;
    //const QByteArray m_lastFMSecret;
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

inline CSongTable * CApplication::getLibrary(void) const
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

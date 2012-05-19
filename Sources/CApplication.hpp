
#ifndef FILE_CAPPLICATION
#define FILE_CAPPLICATION

#include <QMainWindow>
#include <QList>
#include <QSqlDatabase>
#include "ui_TMediaPlayer.h"


class CSong;
class CSongTable;
class CListFolder;
class CPlayList;
class CDynamicPlayList;
class QStandardItemModel;
class QSettings;

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

public:

    enum State
    {
        Stopped,
        Playing,
        Paused
    };

    CApplication(void);
    ~CApplication();

    inline CSong * getCurrentSong(void) const;
    inline CSongTable * getCurrentSongTable(void) const;
    inline CSongTable * getLibrary(void) const;
    inline CSongTable * getDisplayedSongTable(void) const;
    void setDisplayedSongTable(CSongTable * songTable);
    CSong * getSongFromId(int id) const;

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

public slots:

    void play(void);
    void stop(void);
    void pause(void);
    void togglePlay(void);

    void previousSong(void);
    void nextSong(void);
    void playSong(int pos);

    void setRepeat(bool repeat);
    void setShuffle(bool shuffle);
    void setMute(bool mute);
    void toggleMute(void);
    void setVolume(int volume);

    void setPosition(int position);

    void openPlayList(CPlayList * playList);
    void renamePlayList(CPlayList * playList);
    void editDynamicPlayList(CDynamicPlayList * playList);
    void addPlayList(CListFolder * folder = NULL);
    void addDynamicList(CListFolder * folder = NULL);
    void deletePlayList(CPlayList * playList);

    void addListFolder(void);
    void renameListFolder(CListFolder * folder);
    void deleteListFolder(CListFolder * folder);

    void openDialogAddSongs(void);
    void openDialogAddFolder(void);
    void openDialogSongInfos(void);

    void editSong(CSong * song);
    void removeSong(CSong * song);

signals:

    void songAdded(CSong * song);
    void songModified(CSong * song);
    void songRemoved(CSong * song);
    void songPlayStart(CSong * song);
    void songPlayEnd(CSong * song);
    void songPaused(CSong * song);
    void songResumed(CSong * song);
    void songStopped(CSong * song);
    void listAdded(CPlayList * playList);
    void listRemoved(CPlayList * playList);

protected slots:

    void onPlayEnd(void);
    void updateSongDescription(CSong * song);
    void updatePosition(void);
    void update(void);
    void selectPlayListFromTreeView(const QModelIndex& index);

protected:

    void addSong(const QString& fileName);
    QStringList addFolder(const QString& pathName);

    void displaySongTable(CSongTable * songTable);
    bool initSoundSystem(void);
    void loadDatabase(void);
    void startPlay(void);

    virtual void closeEvent(QCloseEvent * event);

private:

    Ui::TMediaPlayer * m_uiWidget;
    FMOD::System * m_soundSystem;
    QStandardItemModel * m_listModel;
    QSqlDatabase m_dataBase;            ///< Base de données.
    QSettings * m_settings;             ///< Paramètres de l'application.
    QTimer * m_timer;                   ///< Timer pour mettre à jour l'affichage.
    CSong * m_currentSong;              ///< Pointeur sur le morceau en cours de lecture.
    int m_currentSongIndex;             ///< Indice du morceau en cours de lecture dans la liste.
    CSongTable * m_currentSongTable;    ///< Liste de morceaux contenant le morceau en cours de lecture.
    CSongTable * m_library;             ///< Librairie (liste de tous les morceaux).
    CSongTable * m_displayedSongTable;  ///< Liste de morceaux affichée.
    State m_state;                      ///< État de lecture.
    bool m_isRepeat;                    ///< Indique si la répétition est activée.
    bool m_isShuffle;                   ///< Indique si la lecture aléatoire est activée.
    bool m_isMute;                      ///< Indique si le son est coupé.
    int m_volume;                       ///< Volume sonore (entre 0 et 100).
    QList<CListFolder *> m_folders;     ///< Liste des dossiers de listes de lecture.
    QList<CPlayList *> m_playLists;     ///< Liste des listes de lectures sans dossier.
};


/**
 * Retourne la chanson actuellement lue.
 *
 * \return Chanson en cours de lecture, ou NULL.
 */

inline CSong * CApplication::getCurrentSong(void) const
{
    return m_currentSong;
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

#endif

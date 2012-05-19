
#ifndef FILE_CSONGTABLE
#define FILE_CSONGTABLE

#include <QObject>
#include <QVariant>
#include <QList>
#include <QTableView>


class CSong;
class CSongTableModel;


class CSongTable : public QTableView
{
    Q_OBJECT

    friend class CApplication;

public:

    enum TColumnType
    {
        Title,
        Artist,
        Album,
        AlbumArtist,
        Composer,
        Year,
        TrackNumber,
        DiscNumber,
        Genre,
        Rating,
        Comments,
        PlayCount,
        LastPlayTime,
        FileName,
        BitRate,
        Format,
        Duration
    };

    CSongTable(QWidget * parent = NULL);
    ~CSongTable();

    inline QList<CSong *> getSongs(void) const;
    inline int getNumSongs(void) const;
    inline CSong * getSongForIndex(int pos) const;
    int getPreviousSong(int pos, bool shuffle) const;
    int getNextSong(int pos, bool shuffle) const;
    int getTotalDuration(void) const;
    void deleteSongs(void);
    inline bool hasSong(CSong * song) const;
    
signals:

    void songSelected(int pos); ///< Signal émis quand un morceau est sélectionné.
    void songStarted(int pos);  ///< Signal émis quand un morceau est lancé (double-clic).

protected:

    void addSong(CSong * song, int pos = -1);
    void removeSong(CSong * song);
    void removeSong(int pos);

    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseDoubleClickEvent(QMouseEvent * event);
    virtual void openCustomMenuProject(const QPoint& point);

private:

    struct TColumn
    {
        TColumnType type;
        int width;
    };
    
    CSongTableModel * m_model; ///< Modèle utilisé pour afficher les morceaux.
    QList<CSong *> m_songs;    ///< Liste des chansons.
    QList<TColumn> m_columns;  ///< Liste des colonnes affichées.
    QList<TColumnType> m_sort; ///< Liste des critères de tri des colonnes.
};

Q_DECLARE_METATYPE(CSongTable *)


/**
 * Retourne la liste des morceaux de la liste.
 *
 * \return Liste des morceaux.
 */

inline QList<CSong *> CSongTable::getSongs(void) const
{
    return m_songs;
}


/**
 * Donne le nombre de morceaux dans la liste.
 *
 * \return Nombre de morceaux.
 */

inline int CSongTable::getNumSongs(void) const
{
    return m_songs.size();
}


inline CSong * CSongTable::getSongForIndex(int pos) const
{
    return (pos < 0 ? NULL : m_songs.at(pos));
}


/**
 * Indique si un morceau est contenu dans la liste.
 *
 * \param song Pointeur sur le morceau à recherché.
 * \return Booléen.
 */

inline bool CSongTable::hasSong(CSong * song) const
{
    if (!song) return false;
    return m_songs.contains(song);
}

#endif

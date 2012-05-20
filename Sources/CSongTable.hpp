
#ifndef FILE_CSONGTABLE
#define FILE_CSONGTABLE

#include <QObject>
#include <QVariant>
#include <QList>
#include <QTableView>
#include "CSongTableModel.hpp"


class CSong;
class CApplication;
class QMenu;


class CSongTable : public QTableView
{
    Q_OBJECT

    friend class CApplication;
    friend class CListFolder;

public:

    enum TColumnType
    {
        ColInvalid      = -1,

        ColPosition     =  0,
        ColTitle        =  1,
        ColArtist       =  2,
        ColAlbum        =  3,
        ColAlbumArtist  =  4,
        ColComposer     =  5,
        ColYear         =  6,
        ColTrackNumber  =  7,
        ColDiscNumber   =  8,
        ColGenre        =  9,
        ColRating       = 10,
        ColComments     = 11,
        ColPlayCount    = 12,
        ColLastPlayTime = 13,
        ColFileName     = 14,
        ColBitRate      = 15,
        ColFormat       = 16,
        ColDuration     = 17,

        ColNumber       = 18  ///< Nombre de types de colonnes.
    };

    struct TColumn
    {
        int pos;      ///< Position de la colonne.
        int width;    ///< Largeur de la colonne en pixels.
        bool visible; ///< Indique si la colonne est visible ou pas.

        inline TColumn(int ppos = -1, int pwidth = -1, bool pvisible = true) :
            pos(ppos), width(pwidth), visible(pvisible) { }
    };

    CSongTable(CApplication * application);
    ~CSongTable();

    inline QList<CSong *> getSongs(void) const;
    inline int getNumSongs(void) const;
    CSongTableModel::TSongItem * getSongItemForIndex(int pos) const;
    int getPreviousSong(int pos, bool shuffle) const;
    int getNextSong(int pos, bool shuffle) const;
    int getTotalDuration(void) const;
    void deleteSongs(void);
    inline bool hasSong(CSong * song) const;
    virtual bool isModified(void) const;
    
signals:

    void songSelected(int pos); ///< Signal émis quand un morceau est sélectionné.
    void songStarted(int pos);  ///< Signal émis quand un morceau est lancé (double-clic).
    void columnChanged(void);   ///< Signal émis lorsque les colonnes sont modifiées.
    
protected slots:

    void columnMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
    void sectionResized(int logicalIndex, int oldSize, int newSize);
    virtual void openCustomMenuProject(const QPoint& point);

protected:

    void addSong(CSong * song, int pos = -1);
    void removeSong(CSong * song);
    void removeSong(int pos);
    void initColumns(const QString& str);
    void showColumn(int col, bool show = true);
    QString getColumnsInfos(void) const;
    virtual bool updateDatabase(void);

    //void loadFromDatabase(int id);

    //virtual void mousePressEvent(QMouseEvent * event);
    //virtual void mouseDoubleClickEvent(QMouseEvent * event);

    CSongTableModel * m_model;    ///< Modèle utilisé pour afficher les morceaux.
    QMenu * m_menu;               ///< Menu contextuel.
    CApplication * m_application; ///< Pointeur sur l'application.
    TColumn m_columns[ColNumber]; ///< Liste des colonnes.
    int m_idPlayList;             ///< Identifiant de la liste de lecture en base de données.

private:
    
    bool m_isModified;            ///< Indique si les informations de la liste ont été modifiées.
    QList<CSong *> m_songs;       ///< Liste des chansons.
    int m_columnSort;             ///< Numéro de la colonne triée.
    Qt::SortOrder m_sortOrder;    ///< Ordre de tri.
    bool m_isColumnMoving;        ///< Indique si les colonnes sont en cours de positionnement.
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

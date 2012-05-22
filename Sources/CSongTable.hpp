
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
        ColSampleRate   = 18,
        /// \todo Ajouter "Canaux"
        /// \todo Ajouter "Paroles"

        ColNumber       = 19  ///< Nombre de types de colonnes.
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

    QList<CSong *> getSongs(void) const;
    inline int getNumSongs(void) const;
    //CSongTableItem * getSongItemForIndex(int pos) const;
    CSongTableItem * getSongItemForRow(int row) const;
    int getRowForSongItem(CSongTableItem * songItem) const;
    CSongTableItem * getSelectedSongItem(void) const;
    QList<CSongTableItem *> getSelectedSongItems(void) const;
    CSongTableItem * getPreviousSong(CSongTableItem * songItem, bool shuffle) const;
    CSongTableItem * getNextSong(CSongTableItem * songItem, bool shuffle) const;
    int getTotalDuration(void) const;
    inline bool hasSong(CSong * song) const;
    virtual bool isModified(void) const;
    
public slots:

    virtual void addSong(CSong * song, int pos = -1);
    virtual void addSongs(const QList<CSong *>& songs);
    virtual void removeSong(CSong * song);
    virtual void removeSong(int pos);

signals:

    void songSelected(CSongTableItem *); ///< Signal émis quand un morceau est sélectionné.
    void songStarted(CSongTableItem *);  ///< Signal émis quand un morceau est lancé (double-clic).
    void columnChanged(void);            ///< Signal émis lorsque les colonnes sont modifiées.
    
protected slots:

    virtual void columnMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
    virtual void columnResized(int logicalIndex, int oldSize, int newSize);
    virtual void openCustomMenuProject(const QPoint& point);
    void showColumn(int column, bool show = true);
    void sortColumn(int column, Qt::SortOrder order);

protected:

    void addSongToTable(CSong * song, int pos = -1);
    void addSongsToTable(const QList<CSong *>& songs);
    void removeSongFromTable(CSong * song);
    void removeSongFromTable(int row);
    void removeAllSongsFromTable(void);
    void deleteSongs(void);

    virtual void initColumns(const QString& str);
    QString getColumnsInfos(void) const;
    virtual bool updateDatabase(void);
    virtual void startDrag(Qt::DropActions supportedActions);

    //void loadFromDatabase(int id);

    virtual void keyPressEvent(QKeyEvent * event);
    //virtual void mouseDoubleClickEvent(QMouseEvent * event);

    CSongTableModel * m_model;    ///< Modèle utilisé pour afficher les morceaux.
    CApplication * m_application; ///< Pointeur sur l'application.
    TColumn m_columns[ColNumber]; ///< Liste des colonnes.
    int m_idPlayList;             ///< Identifiant de la liste de lecture en base de données.
    int m_columnSort;             ///< Numéro de la colonne triée.
    Qt::SortOrder m_sortOrder;    ///< Ordre de tri.
    bool m_automaticSort;         ///< Indique si le tri est automatique lorsqu'on modifie la table.

private:
    
    bool m_isModified;            ///< Indique si les informations de la liste ont été modifiées.
    //QList<CSong *> m_songs;       ///< Liste des chansons. \todo Supprimer (déjà dans le modèle) ?
    bool m_isColumnMoving;        ///< Indique si les colonnes sont en cours de positionnement.
    QPoint m_pressedPosition;
};

Q_DECLARE_METATYPE(CSongTable *)


/**
 * Donne le nombre de morceaux dans la liste.
 *
 * \return Nombre de morceaux.
 */

inline int CSongTable::getNumSongs(void) const
{
    return m_model->getNumSongs();
}


/**
 * Indique si un morceau est contenu dans la liste.
 *
 * \param song Pointeur sur le morceau à recherché.
 * \return Booléen.
 */

inline bool CSongTable::hasSong(CSong * song) const
{
    return m_model->hasSong(song);
}

#endif

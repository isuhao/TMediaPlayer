/*
Copyright (C) 2012-2014 Teddy Michel

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

#ifndef FILE_C_MEDIA_TABLE_VIEW_HPP_
#define FILE_C_MEDIA_TABLE_VIEW_HPP_

#include <QObject>
#include <QVariant>
#include <QList>
#include <QTableView>
#include <QMap>
#include <QMenu>

#include "CMediaTableModel.hpp"


class CSong;
class CMainWindow;
class CStaticList;
class CMediaTableModel;
class CMediaTableItem;
class QMenu;
class QAction;


/**
 * Vue sous forme de tableau utilisée pour afficher une liste de morceaux.
 */

class CMediaTableView : public QTableView
{
    Q_OBJECT

    friend class CMainWindow;
    friend class CFolder;

public:

    /**
     * Liste des colonnes affichables.
     *
     * Procédure pour ajouter un type de colonne (par exemple "ColBidule") :
     * - Ajouter la valeur "ColBidule" dans l'énumération TColumnType.
     * - Incrémenter la valeur de ColNumber dans l'énumération TColumnType.
     * - Modifier la méthode CMediaTableView::getColumnTypeFromInteger.
     * - Modifier la méthode CMediaTableView::getColumnTypeName.
     * - Dans la liste des slots de CMediaTableHeader, ajouter le slot "showColBidule".
     * - Dans le constructeur de CMediaTableHeader, ajouter la ligne "T_CREATE_ACTION(ColBidule)".
     * - Modifier la méthode CMediaTableModel::data pour afficher les données.
     * - Modifier la méthode CMediaTableModel::sort pour trier les données.
     */

    enum TColumnType
    {
        ColInvalid          = -1,

        ColPosition         =  0, ///< #.
        ColTitle            =  1, ///< Titre.
        ColArtist           =  2, ///< Artiste.
        ColAlbum            =  3, ///< Album.
        ColAlbumArtist      =  4, ///< Artiste de l'album.
        ColComposer         =  5, ///< Compositeur.
        ColYear             =  6, ///< Année.
        ColTrackNumber      =  7, ///< Piste.
        ColDiscNumber       =  8, ///< Disque.
        ColGenre            =  9, ///< Genre.
        ColRating           = 10, ///< Note.
        ColComments         = 11, ///< Commentaires.
        ColPlayCount        = 12, ///< Lectures.
        ColLastPlayTime     = 13, ///< Dernière lecture.
        ColPathName         = 14, ///< Nom du fichier (chemin complet).
        ColBitRate          = 15, ///< Débit.
        ColFormat           = 16, ///< Format.
        ColDuration         = 17, ///< Durée.
        ColSampleRate       = 18, ///< Fréquence d'échantillonnage.
        ColCreationDate     = 19, ///< Date de création.
        ColModificationDate = 20, ///< Date de modification.
        ColChannels         = 21, ///< Canaux.
        ColFileSize         = 22, ///< Taille du fichier.
        ColLyrics           = 23, ///< Paroles.
        ColLanguage         = 24, ///< Langue.
        ColLyricist         = 25, ///< Parolier.
        ColGrouping         = 26, ///< Regroupement.
        ColSubTitle         = 27, ///< Sous-titre.
        ColTrackGain        = 28, ///< Gain de la piste.
        ColTrackPeak        = 29, ///< Pic de la piste.
        ColAlbumGain        = 30, ///< Gain de l'album.
        ColAlbumPeak        = 31, ///< Pic de l'album.
        ColBPM              = 32, ///< Battements par minute.
        ColTitleSort        = 33, ///< Titre pour le tri.
        ColArtistSort       = 34, ///< Artiste pour le tri.
        ColAlbumSort        = 35, ///< Album pour le tri.
        ColAlbumArtistSort  = 36, ///< Artiste de l'album pour le tri.
        ColComposerSort     = 37, ///< Compositeur pour le tri.
        ColFileName         = 38, ///< Nom du fichier (sans le chemin).

        ColNumber           = 39  ///< Nombre de types de colonnes.
    };

    static inline TColumnType getColumnTypeFromInteger(int column);
    static inline QString getColumnTypeName(TColumnType column);

    struct TColumn
    {
        int pos;      ///< Position de la colonne.
        int width;    ///< Largeur de la colonne en pixels.
        bool visible; ///< Indique si la colonne est visible ou pas.

        inline TColumn(int ppos = -1, int pwidth = -1, bool pvisible = true) :
            pos(ppos), width(pwidth), visible(pvisible) { }
    };


    explicit CMediaTableView(CMainWindow * mediaWindow);
    virtual ~CMediaTableView();

    QList<CSong *> getSongs() const;
    inline int getNumSongs() const;
    CMediaTableItem * getFirstSongItem(CSong * song) const;
    CMediaTableItem * getSongItemForRow(int row) const;
    int getRowForSongItem(CMediaTableItem * songItem) const;
    CMediaTableItem * getSelectedSongItem() const;
    QList<CMediaTableItem *> getSelectedSongItems() const;
    QList<CSong *> getSelectedSongs() const;
    CMediaTableItem * getPreviousSong(CMediaTableItem * songItem, bool shuffle) const;
    CMediaTableItem * getNextSong(CMediaTableItem * songItem, bool shuffle) const;
    CMediaTableItem * getLastSong(bool shuffle) const;
    qlonglong getTotalDuration() const;
    void applyFilter(const QString& filter);
    inline bool hasSong(CSong * song) const;
    inline int getColumnSorted() const;
    virtual bool isModified() const;
    void replaceSong(CSong * oldSong, CSong * newSong);

    inline int getIdPlayList() const;

public slots:

    void selectSongItem(CMediaTableItem * songItem);
    void changeCurrentSongList();
    void playSelectedSong();

    virtual void removeDuplicateSongs()
    {
        return;
    }

signals:

    void songSelected(CMediaTableItem *); ///< Signal émis quand un morceau est sélectionné.
    void songStarted(CMediaTableItem *);  ///< Signal émis quand un morceau est lancé (double-clic).
    void columnChanged();                 ///< Signal émis lorsque les colonnes sont modifiées.
    void rowCountChanged();               ///< Signal émis lorsque le nombre de morceaux de la liste change.

protected slots:

    virtual void columnMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
    virtual void columnResized(int logicalIndex, int oldSize, int newSize);
    void showColumn(int column, bool show = true);
    void onSortAboutToChange();
    void sortColumn(int column, Qt::SortOrder order);
    void sort();
    void goToSongTable();
    void addToPlayList();
    void removeSongsFromLibrary();
    void moveSongs();
    void analyzeSongs();
    void checkSelection();
    void uncheckSelection();
    void onRowCountChange(const QModelIndex& parent, int start, int end);
    void onSelectionChange();
    void addSongToQueueBegining();
    void addSongToQueueEnd();

protected:

    void addSongToTable(CSong * song, int pos = -1);
    void addSongsToTable(const QList<CSong *>& songs);
    void removeSongFromTable(CSong * song);
    void removeSongFromTable(int row);
    void removeSongsFromTable(const QList<CSong *>& songs);
    void removeAllSongsFromTable();
    void initShuffle(CMediaTableItem * firstSong = nullptr);

    virtual void initColumns(const QString& str);
    QString getColumnsInfos() const;
    virtual bool updateDatabase();
    virtual void startDrag(Qt::DropActions supportedActions);

    //void loadFromDatabase(int id);

    virtual void keyPressEvent(QKeyEvent * event);
    virtual void mouseDoubleClickEvent(QMouseEvent * event);
    virtual void contextMenuEvent(QContextMenuEvent * event);

    CMainWindow * m_mainWindow;
    CMediaTableModel * m_model;   ///< Modèle utilisé pour afficher les morceaux.
    TColumn m_columns[ColNumber]; ///< Liste des colonnes.
    int m_idPlayList;             ///< Identifiant de la liste de lecture en base de données.
    int m_columnSort;             ///< Numéro de la colonne triée.
    Qt::SortOrder m_sortOrder;    ///< Ordre de tri.
    bool m_automaticSort;         ///< Indique si le tri est automatique lorsqu'on modifie la table.
    CMediaTableItem * m_selectedItem;
    QMap<CMediaTableView *, QAction *> m_actionGoToSongTable;
    QMap<CStaticList *, QAction *> m_actionAddToPlayList;

private:

    virtual bool canImportSongs() const
    {
        return false;
    }

    virtual bool canEditSongs() const
    {
        return true;
    }

    virtual bool canEditPlayList() const
    {
        return false;
    }

    virtual bool canMoveToPlayList() const
    {
        return true;
    }

    bool m_isModified;                        ///< Indique si les informations de la liste ont été modifiées.
    bool m_isColumnMoving;                    ///< Indique si les colonnes sont en cours de positionnement.
    QList<CMediaTableItem *> m_selectedItems; ///< Morceaux sélectionnés avant le tri.
    CMediaTableItem * m_currentItem;          ///< Morceau actif avant le tri.
    QMenu m_menu;                             ///< Menu contextuel.
};

Q_DECLARE_METATYPE(CMediaTableView *)


inline CMediaTableView::TColumnType CMediaTableView::getColumnTypeFromInteger(int column)
{
    switch (column)
    {
        default: return ColInvalid         ;
        case  0: return ColPosition        ;
        case  1: return ColTitle           ;
        case  2: return ColArtist          ;
        case  3: return ColAlbum           ;
        case  4: return ColAlbumArtist     ;
        case  5: return ColComposer        ;
        case  6: return ColYear            ;
        case  7: return ColTrackNumber     ;
        case  8: return ColDiscNumber      ;
        case  9: return ColGenre           ;
        case 10: return ColRating          ;
        case 11: return ColComments        ;
        case 12: return ColPlayCount       ;
        case 13: return ColLastPlayTime    ;
        case 14: return ColPathName        ;
        case 15: return ColBitRate         ;
        case 16: return ColFormat          ;
        case 17: return ColDuration        ;
        case 18: return ColSampleRate      ;
        case 19: return ColCreationDate    ;
        case 20: return ColModificationDate;
        case 21: return ColChannels        ;
        case 22: return ColFileSize        ;
        case 23: return ColLyrics          ;
        case 24: return ColLanguage        ;
        case 25: return ColLyricist        ;
        case 26: return ColGrouping        ;
        case 27: return ColSubTitle        ;
        case 28: return ColTrackGain       ;
        case 29: return ColTrackPeak       ;
        case 30: return ColAlbumGain       ;
        case 31: return ColAlbumPeak       ;
        case 32: return ColBPM             ;
        case 33: return ColTitleSort       ;
        case 34: return ColArtistSort      ;
        case 35: return ColAlbumSort       ;
        case 36: return ColAlbumArtistSort ;
        case 37: return ColComposerSort    ;
        case 38: return ColFileName        ;
    }
}


inline QString CMediaTableView::getColumnTypeName(CMediaTableView::TColumnType column)
{
    switch (column)
    {
        default:
        case ColPosition        : return tr(""                 );
        case ColTitle           : return tr("Title"            );
        case ColArtist          : return tr("Artist"           );
        case ColAlbum           : return tr("Album"            );
        case ColAlbumArtist     : return tr("Album artist"     );
        case ColComposer        : return tr("Composer"         );
        case ColYear            : return tr("Year"             );
        case ColTrackNumber     : return tr("Track"            );
        case ColDiscNumber      : return tr("Disc"             );
        case ColGenre           : return tr("Genre"            );
        case ColRating          : return tr("Rating"           );
        case ColComments        : return tr("Comments"         );
        case ColPlayCount       : return tr("Plays"            );
        case ColLastPlayTime    : return tr("Last played"      );
        case ColPathName        : return tr("Pathname"         );
        case ColBitRate         : return tr("Bit rate"         );
        case ColFormat          : return tr("Format"           );
        case ColDuration        : return tr("Duration"         );
        case ColSampleRate      : return tr("Sample rate"      );
        case ColCreationDate    : return tr("Creation date"    );
        case ColModificationDate: return tr("Modification date");
        case ColChannels        : return tr("Channels"         );
        case ColFileSize        : return tr("File size"        );
        case ColLyrics          : return tr("Lyrics"           );
        case ColLanguage        : return tr("Language"         );
        case ColLyricist        : return tr("Lyricist"         );
        case ColGrouping        : return tr("Grouping"         );
        case ColSubTitle        : return tr("Subtitle"         );
        case ColTrackGain       : return tr("Track gain"       );
        case ColTrackPeak       : return tr("Track peak"       );
        case ColAlbumGain       : return tr("Album gain"       );
        case ColAlbumPeak       : return tr("Album peak"       );
        case ColBPM             : return tr("BPM"              );
        case ColTitleSort       : return tr("Sort Title"       );
        case ColArtistSort      : return tr("Sort Artist"      );
        case ColAlbumSort       : return tr("Sort Album"       );
        case ColAlbumArtistSort : return tr("Sort Album artist");
        case ColComposerSort    : return tr("Sort Composer"    );
        case ColFileName        : return tr("Filename"         );
    }
}


/**
 * Donne le nombre de morceaux dans la liste.
 *
 * \return Nombre de morceaux.
 */

inline int CMediaTableView::getNumSongs() const
{
    return m_model->getNumSongs();
}


/**
 * Indique si un morceau est contenu dans la liste.
 *
 * \param song Pointeur sur le morceau à recherché.
 * \return Booléen.
 */

inline bool CMediaTableView::hasSong(CSong * song) const
{
    return m_model->hasSong(song);
}


inline int CMediaTableView::getColumnSorted() const
{
    return m_columnSort;
}


inline int CMediaTableView::getIdPlayList() const
{
    return m_idPlayList;
}

#endif // FILE_C_MEDIA_TABLE_VIEW_HPP_

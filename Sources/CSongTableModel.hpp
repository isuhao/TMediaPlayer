
#ifndef FILE_CSONGTABLEMODEL
#define FILE_CSONGTABLEMODEL

#include <QAbstractTableModel>
#include <QList>
#include <QStringList>
#include "CSong.hpp"


class QMouseEvent;


class CSongTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    struct TSongItem
    {
        int pos;
        CSong * song;

        inline TSongItem(int ppos, CSong * psong) : pos(ppos), song(psong) { }
    };

    CSongTableModel(const QList<CSong *>& data = QList<CSong *>(), QObject * parent = NULL);

    void setCanDrop(bool canDrop);

    void setData(const QList<CSong *>& data);
    QList<CSong *> getData() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void sort(int column, Qt::SortOrder order);

    // Glisser-déposer
    Qt::ItemFlags flags(const QModelIndex& index) const;
    QStringList mimeTypes(void) const;
    QMimeData * mimeData(const QModelIndexList& indexes) const;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

    void insertRow(CSong * song, int pos = -1);
    void removeRow(int pos);
    void clear(void);

    TSongItem * getSongItem(const QModelIndex& index) const;
    TSongItem * getSongItem(int row) const;

private:

    static inline bool cmpSongPositionAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->pos < song2->pos);
    }

    static inline bool cmpSongPositionDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongPositionAsc(song2, song1);
    }

    static inline bool cmpSongTitleAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getTitleSort(false) < song2->song->getTitleSort(false));
    }

    static inline bool cmpSongTitleDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongTitleAsc(song2, song1);
    }

    static inline bool cmpSongArtistAsc(TSongItem * song1, TSongItem * song2)
    {
        const QString artist1 = song1->song->getArtistNameSort(false);
        const QString artist2 = song2->song->getArtistNameSort(false);

        if (artist1 < artist2) return true;
        if (artist1 > artist2) return false;

        return cmpSongAlbumAsc(song1, song2);
    }

    static inline bool cmpSongArtistDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongArtistAsc(song2, song1);
    }

    static inline bool cmpSongAlbumAsc(TSongItem * song1, TSongItem * song2)
    {
        const QString album1 = song1->song->getAlbumTitleSort(false);
        const QString album2 = song2->song->getAlbumTitleSort(false);

        if (album1 < album2) return true;
        if (album1 > album2) return false;

        return cmpSongDiscAsc(song1, song2);
    }

    static inline bool cmpSongAlbumDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongAlbumAsc(song2, song1);
    }

    static inline bool cmpSongAlbumArtistAsc(TSongItem * song1, TSongItem * song2)
    {
        const QString artist1 = song1->song->getAlbumArtistSort(false);
        const QString artist2 = song2->song->getAlbumArtistSort(false);

        if (artist1 < artist2) return true;
        if (artist1 > artist2) return false;

        return cmpSongAlbumAsc(song1, song2);
    }

    static inline bool cmpSongAlbumArtistDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongAlbumArtistAsc(song2, song1);
    }

    static inline bool cmpSongComposerAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getComposerSort(false) < song2->song->getComposerSort(false));
    }

    static inline bool cmpSongComposerDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongComposerAsc(song2, song1);
    }

    static inline bool cmpSongYearAsc(TSongItem * song1, TSongItem * song2)
    {
        const int year1 = song1->song->getYear();
        const int year2 = song2->song->getYear();

        if (year1 < year2) return true;
        if (year1 > year2) return false;

        return cmpSongArtistAsc(song1, song2);
    }

    static inline bool cmpSongYearDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongYearAsc(song2, song1);
    }

    static inline bool cmpSongTrackAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getTrackNumber() < song2->song->getTrackNumber());
    }

    static inline bool cmpSongTrackDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongTrackAsc(song2, song1);
    }

    static inline bool cmpSongDiscAsc(TSongItem * song1, TSongItem * song2)
    {
        const int disc1 = song1->song->getDiscNumber();
        const int disc2 = song2->song->getDiscNumber();

        if (disc1 < disc2) return true;
        if (disc1 > disc2) return false;

        return cmpSongTrackAsc(song1, song2);
    }

    static inline bool cmpSongDiscDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongDiscAsc(song2, song1);
    }

    static inline bool cmpSongGenreAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getGenre() < song2->song->getGenre());
    }

    static inline bool cmpSongGenreDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongGenreAsc(song2, song1);
    }

    static inline bool cmpSongRatingAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getRating() < song2->song->getRating());
    }

    static inline bool cmpSongRatingDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongRatingAsc(song2, song1);
    }

    static inline bool cmpSongCommentsAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getComments() < song2->song->getComments());
    }

    static inline bool cmpSongCommentsDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongCommentsAsc(song2, song1);
    }

    static inline bool cmpSongPlayCountAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getNumPlays() < song2->song->getNumPlays());
    }

    static inline bool cmpSongPlayCountDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongPlayCountAsc(song2, song1);
    }

    static inline bool cmpSongLastPlayedAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getLastPlay() < song2->song->getLastPlay());
    }

    static inline bool cmpSongLastPlayedDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongLastPlayedAsc(song2, song1);
    }

    static inline bool cmpSongFileNameAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getFileName() < song2->song->getFileName());
    }

    static inline bool cmpSongFileNameDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongFileNameAsc(song2, song1);
    }

    static inline bool cmpSongBitRateAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getBitRate() < song2->song->getBitRate());
    }

    static inline bool cmpSongBitRateDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongBitRateAsc(song2, song1);
    }

    static inline bool cmpSongFormatAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getFileType() < song2->song->getFileType());
    }

    static inline bool cmpSongFormatDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongFormatAsc(song2, song1);
    }

    static inline bool cmpSongDurationAsc(TSongItem * song1, TSongItem * song2)
    {
        return (song1->song->getDuration() < song2->song->getDuration());
    }

    static inline bool cmpSongDurationDesc(TSongItem * song1, TSongItem * song2)
    {
        return cmpSongDurationAsc(song2, song1);
    }

    bool m_canDrop;
    QList<TSongItem *> m_data;
};

#endif // FILE_CSONGTABLEMODEL

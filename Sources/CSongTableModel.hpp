
#ifndef FILE_CSONGTABLEMODEL
#define FILE_CSONGTABLEMODEL

#include <QAbstractTableModel>
#include <QList>
#include "CSong.hpp"


class QMouseEvent;


class CSongTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    CSongTableModel(const QList<CSong *>& data = QList<CSong *>(), QObject * parent = NULL);

    void setData(const QList<CSong *>& data);
    QList<CSong *> getData() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void sort(int column, Qt::SortOrder order);

    void insertRow(CSong * song, int pos = -1);
    void removeRow(int pos);
    void clear(void);

    CSong * getSong(const QModelIndex& index) const;
    CSong * getSong(int row) const;

private:

    struct TSongItem
    {
        int pos;
        CSong * song;

        inline TSongItem(int ppos, CSong * psong) : pos(ppos), song(psong) { }
    };

    static inline bool cmpSongPositionAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.pos < song2.pos);
    }

    static inline bool cmpSongTitleAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getTitleSort(false) < song2.song->getTitleSort(false));
    }

    static inline bool cmpSongArtistAsc(const TSongItem& song1, const TSongItem& song2)
    {
        const QString artist1 = song1.song->getAlbumTitleSort(false);
        const QString artist2 = song2.song->getAlbumTitleSort(false);

        if (artist1 < artist2) return true;
        if (artist1 > artist2) return false;

        return cmpSongAlbumAsc(song1, song2);
    }

    static inline bool cmpSongAlbumAsc(const TSongItem& song1, const TSongItem& song2)
    {
        const QString album1 = song1.song->getAlbumTitleSort(false);
        const QString album2 = song2.song->getAlbumTitleSort(false);

        if (album1 < album2) return true;
        if (album1 > album2) return false;

        return cmpSongDiscAsc(song1, song2);
    }

    static inline bool cmpSongAlbumArtistAsc(const TSongItem& song1, const TSongItem& song2)
    {
        const QString artist1 = song1.song->getAlbumArtistSort(false);
        const QString artist2 = song2.song->getAlbumArtistSort(false);

        if (artist1 < artist2) return true;
        if (artist1 > artist2) return false;

        return cmpSongAlbumAsc(song1, song2);
    }

    static inline bool cmpSongComposerAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getComposerSort(false) < song2.song->getComposerSort(false));
    }

    static inline bool cmpSongYearAsc(const TSongItem& song1, const TSongItem& song2)
    {
        const int year1 = song1.song->getYear();
        const int year2 = song2.song->getYear();

        if (year1 < year2) return true;
        if (year1 > year2) return false;

        return cmpSongArtistAsc(song1, song2);
    }

    static inline bool cmpSongTrackAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getTrackNumber() < song2.song->getTrackNumber());
    }

    static inline bool cmpSongDiscAsc(const TSongItem& song1, const TSongItem& song2)
    {
        const int disc1 = song1.song->getDiscNumber();
        const int disc2 = song2.song->getDiscNumber();

        if (disc1 < disc2) return true;
        if (disc1 > disc2) return false;

        return cmpSongTrackAsc(song1, song2);
    }

    static inline bool cmpSongGenreAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getGenre() < song2.song->getGenre());
    }

    static inline bool cmpSongRatingAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getRating() < song2.song->getRating());
    }

    static inline bool cmpSongCommentsAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getComments() < song2.song->getComments());
    }

    static inline bool cmpSongPlayCountAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getNumPlays() < song2.song->getNumPlays());
    }

    static inline bool cmpSongLastPlayedAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getLastPlay() < song2.song->getLastPlay());
    }

    static inline bool cmpSongFileNameAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getFileName() < song2.song->getFileName());
    }

    static inline bool cmpSongBitRateAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getBitRate() < song2.song->getBitRate());
    }

    static inline bool cmpSongFormatAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getFileType() < song2.song->getFileType());
    }

    static inline bool cmpSongDurationAsc(const TSongItem& song1, const TSongItem& song2)
    {
        return (song1.song->getDuration() < song2.song->getDuration());
    }

    QList<TSongItem> m_data;
};

#endif // FILE_CSONGTABLEMODEL

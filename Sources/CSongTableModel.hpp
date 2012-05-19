
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
    inline const QList<CSong *>& getData() const { return m_data; };

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

    static inline bool cmpSongTitleAsc(CSong * song1, CSong * song2)
    {
        return (song1->getTitleSort(false) < song2->getTitleSort(false));
    }

    static inline bool cmpSongArtistAsc(CSong * song1, CSong * song2)
    {
        const QString artist1 = song1->getAlbumTitleSort(false);
        const QString artist2 = song2->getAlbumTitleSort(false);

        if (artist1 < artist2) return true;
        if (artist1 > artist2) return false;

        return cmpSongAlbumAsc(song1, song2);
    }

    static inline bool cmpSongAlbumAsc(CSong * song1, CSong * song2)
    {
        const QString album1 = song1->getAlbumTitleSort(false);
        const QString album2 = song2->getAlbumTitleSort(false);

        if (album1 < album2) return true;
        if (album1 > album2) return false;

        return cmpSongDiscAsc(song1, song2);
    }

    static inline bool cmpSongAlbumArtistAsc(CSong * song1, CSong * song2)
    {
        const QString artist1 = song1->getAlbumArtistSort(false);
        const QString artist2 = song2->getAlbumArtistSort(false);

        if (artist1 < artist2) return true;
        if (artist1 > artist2) return false;

        return cmpSongAlbumAsc(song1, song2);
    }

    static inline bool cmpSongComposerAsc(CSong * song1, CSong * song2)
    {
        return (song1->getComposerSort(false) < song2->getComposerSort(false));
    }

    static inline bool cmpSongYearAsc(CSong * song1, CSong * song2)
    {
        const int year1 = song1->getYear();
        const int year2 = song2->getYear();

        if (year1 < year2) return true;
        if (year1 > year2) return false;

        return cmpSongArtistAsc(song1, song2);
    }

    static inline bool cmpSongTrackAsc(CSong * song1, CSong * song2)
    {
        return (song1->getTrackNumber() < song2->getTrackNumber());
    }

    static inline bool cmpSongDiscAsc(CSong * song1, CSong * song2)
    {
        const int disc1 = song1->getDiscNumber();
        const int disc2 = song2->getDiscNumber();

        if (disc1 < disc2) return true;
        if (disc1 > disc2) return false;

        return cmpSongTrackAsc(song1, song2);
    }

    QList<CSong *> m_data;
};

#endif // FILE_CSONGTABLEMODEL

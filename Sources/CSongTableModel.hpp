
#ifndef FILE_C_SONG_TABLE_MODEL
#define FILE_C_SONG_TABLE_MODEL

#include <QAbstractTableModel>
#include <QList>
#include <QStringList>
#include "CSong.hpp"


class QMouseEvent;


class CSongTableItem
{
    friend class CSongTableModel;

public:

    CSongTableItem(void);

    inline int getPosition(void) const;
    inline CSong * getSong(void) const;
    inline bool isValid(void) const;

private:

    CSongTableItem(int position, CSong * song);

    int m_position; ///< Position dans la liste.
    CSong * m_song; ///< Pointeur sur le morceau.
};


inline int CSongTableItem::getPosition(void) const
{
    return m_position;
}


inline CSong * CSongTableItem::getSong(void) const
{
    return m_song;
}


inline bool CSongTableItem::isValid(void) const
{
    return (m_position >= 0 && m_song);
}


class CSongTableModel : public QAbstractTableModel
{
    Q_OBJECT

    friend class CSongTable;

public:

    CSongTableModel(const QList<CSong *>& data = QList<CSong *>(), QWidget * parent = NULL);
    CSongTableModel(QWidget * parent);

    void setCanDrop(bool canDrop);

    void setSongs(const QList<CSong *>& data);
    QList<CSong *> getSongs(void) const;
    inline int getNumSongs(void) const;
    bool hasSong(CSong * song) const;

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
    void removeRow(int row);
    void clear(void);

    CSongTableItem * getSongItem(const QModelIndex& index) const;
    CSongTableItem * getSongItem(int row) const;
    int getRowForSongItem(CSongTableItem * songItem) const;
    CSongTableItem * getPreviousSong(CSongTableItem * songItem, bool shuffle) const;
    CSongTableItem * getNextSong(CSongTableItem * songItem, bool shuffle) const;

signals:

    void columnSorted(int column, Qt::SortOrder order); ///< Signal émis lorsqu'une colonne est triée.

private:

    static inline bool cmpSongPositionAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        int pos1 = song1->getPosition();
        int pos2 = song2->getPosition();

        if (pos1 < pos2) return true;
        if (pos1 > pos2) return false;

        return cmpSongArtistAsc(song2, song1);
    }

    static inline bool cmpSongPositionDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongPositionAsc(song2, song1);
    }

    static inline bool cmpSongTitleAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        const QString title1 = song1->getSong()->getTitleSort(false);
        const QString title2 = song2->getSong()->getTitleSort(false);

        return (QString::compare(title1, title2, Qt::CaseInsensitive) < 0);
    }

    static inline bool cmpSongTitleDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongTitleAsc(song2, song1);
    }

    static inline bool cmpSongArtistAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        const QString artist1 = song1->getSong()->getArtistNameSort(false);
        const QString artist2 = song2->getSong()->getArtistNameSort(false);

        int cmp = QString::compare(artist1, artist2, Qt::CaseInsensitive);

        if (cmp < 0) return true;
        if (cmp > 0) return false;

        return cmpSongAlbumAsc(song1, song2);
    }

    static inline bool cmpSongArtistDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongArtistAsc(song2, song1);
    }

    static inline bool cmpSongAlbumAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        const QString album1 = song1->getSong()->getAlbumTitleSort(false);
        const QString album2 = song2->getSong()->getAlbumTitleSort(false);

        int cmp = QString::compare(album1, album2, Qt::CaseInsensitive);

        if (cmp < 0) return true;
        if (cmp > 0) return false;

        return cmpSongDiscAsc(song1, song2);
    }

    static inline bool cmpSongAlbumDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongAlbumAsc(song2, song1);
    }

    static inline bool cmpSongAlbumArtistAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        const QString artist1 = song1->getSong()->getAlbumArtistSort(false);
        const QString artist2 = song2->getSong()->getAlbumArtistSort(false);

        int cmp = QString::compare(artist1, artist2, Qt::CaseInsensitive);

        if (cmp < 0) return true;
        if (cmp > 0) return false;

        return cmpSongAlbumAsc(song1, song2);
    }

    static inline bool cmpSongAlbumArtistDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongAlbumArtistAsc(song2, song1);
    }

    static inline bool cmpSongComposerAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getComposerSort(false) < song2->getSong()->getComposerSort(false));
    }

    static inline bool cmpSongComposerDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongComposerAsc(song2, song1);
    }

    static inline bool cmpSongYearAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        const int year1 = song1->getSong()->getYear();
        const int year2 = song2->getSong()->getYear();

        if (year1 < year2) return true;
        if (year1 > year2) return false;

        return cmpSongArtistAsc(song1, song2);
    }

    static inline bool cmpSongYearDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongYearAsc(song2, song1);
    }

    static inline bool cmpSongTrackAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getTrackNumber() < song2->getSong()->getTrackNumber());
    }

    static inline bool cmpSongTrackDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongTrackAsc(song2, song1);
    }

    static inline bool cmpSongDiscAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        const int disc1 = song1->getSong()->getDiscNumber();
        const int disc2 = song2->getSong()->getDiscNumber();

        if (disc1 < disc2) return true;
        if (disc1 > disc2) return false;

        return cmpSongTrackAsc(song1, song2);
    }

    static inline bool cmpSongDiscDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongDiscAsc(song2, song1);
    }

    static inline bool cmpSongGenreAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getGenre() < song2->getSong()->getGenre());
    }

    static inline bool cmpSongGenreDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongGenreAsc(song2, song1);
    }

    static inline bool cmpSongRatingAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getRating() < song2->getSong()->getRating());
    }

    static inline bool cmpSongRatingDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongRatingAsc(song2, song1);
    }

    static inline bool cmpSongCommentsAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getComments() < song2->getSong()->getComments());
    }

    static inline bool cmpSongCommentsDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongCommentsAsc(song2, song1);
    }

    static inline bool cmpSongPlayCountAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getNumPlays() < song2->getSong()->getNumPlays());
    }

    static inline bool cmpSongPlayCountDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongPlayCountAsc(song2, song1);
    }

    static inline bool cmpSongLastPlayedAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getLastPlay() < song2->getSong()->getLastPlay());
    }

    static inline bool cmpSongLastPlayedDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongLastPlayedAsc(song2, song1);
    }

    static inline bool cmpSongFileNameAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getFileName() < song2->getSong()->getFileName());
    }

    static inline bool cmpSongFileNameDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongFileNameAsc(song2, song1);
    }

    static inline bool cmpSongBitRateAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getBitRate() < song2->getSong()->getBitRate());
    }

    static inline bool cmpSongBitRateDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongBitRateAsc(song2, song1);
    }

    static inline bool cmpSongFormatAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getFormat() < song2->getSong()->getFormat());
    }

    static inline bool cmpSongFormatDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongFormatAsc(song2, song1);
    }

    static inline bool cmpSongDurationAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getDuration() < song2->getSong()->getDuration());
    }

    static inline bool cmpSongDurationDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongDurationAsc(song2, song1);
    }

    static inline bool cmpSongSampleRateAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getSampleRate() < song2->getSong()->getSampleRate());
    }

    static inline bool cmpSongSampleRateDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongSampleRateAsc(song2, song1);
    }

    static inline bool cmpSongCreationDateAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getCreationDate() < song2->getSong()->getCreationDate());
    }

    static inline bool cmpSongCreationDateDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongCreationDateAsc(song2, song1);
    }

    static inline bool cmpSongModificationDateAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getModificationDate() < song2->getSong()->getModificationDate());
    }

    static inline bool cmpSongModificationDateDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongModificationDateAsc(song2, song1);
    }

    static inline bool cmpSongChannelsAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getNumChannels() < song2->getSong()->getNumChannels());
    }

    static inline bool cmpSongChannelsDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongChannelsAsc(song2, song1);
    }

    static inline bool cmpSongFileSizeAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getFileSize() < song2->getSong()->getFileSize());
    }

    static inline bool cmpSongFileSizeDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongFileSizeAsc(song2, song1);
    }

    static inline bool cmpSongLyricsAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (song1->getSong()->getLyrics() < song2->getSong()->getLyrics());
    }

    static inline bool cmpSongLyricsDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongLyricsAsc(song2, song1);
    }

    static inline bool cmpSongLanguageAsc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return (CSong::getLanguageName(song1->getSong()->getLanguage()) < CSong::getLanguageName(song2->getSong()->getLanguage()));
    }

    static inline bool cmpSongLanguageDesc(CSongTableItem * song1, CSongTableItem * song2)
    {
        return cmpSongLyricsAsc(song2, song1);
    }


    bool m_canDrop;
    QList<CSongTableItem *> m_data;
};


inline int CSongTableModel::getNumSongs(void) const
{
    return m_data.size();
}

#endif // FILE_C_SONG_TABLE_MODEL

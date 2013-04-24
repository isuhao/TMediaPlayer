/*
Copyright (C) 2012-2013 Teddy Michel

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

#ifndef FILE_C_SONG_TABLE_MODEL
#define FILE_C_SONG_TABLE_MODEL

#include <QAbstractTableModel>
#include <QList>
#include <QStringList>
#include "CSong.hpp"
#include "CMediaTableItem.hpp"


class QMouseEvent;


/**
 * Modèle permettant de stocker une liste de morceaux.
 * Doit être utilisé avec la classe CMediaTableView.
 */

class CMediaTableModel : public QAbstractTableModel
{
    Q_OBJECT

    friend class CMediaTableView;

public:

    explicit CMediaTableModel(CMainWindow * application, const QList<CSong *>& data = QList<CSong *>(), QWidget * parent = nullptr);
    CMediaTableModel(CMainWindow * application, QWidget * parent);

    void setCanDrop(bool canDrop);

    void setSongs(const QList<CSong *>& songs);
    QList<CSong *> getSongs() const;
    inline int getNumSongs() const;
    bool hasSong(CSong * song) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void sort(int column, Qt::SortOrder order);

    // Glisser-déposer
    Qt::ItemFlags flags(const QModelIndex& index) const;
    QStringList mimeTypes() const;
    QMimeData * mimeData(const QModelIndexList& indexes) const;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    void moveRows(const QList<int>& rows, int rowDest);

    void insertRow(CSong * song, int pos = -1);
    void removeRow(int row);
    void removeSongs(const QList<CSong *>& songs);
    void clear();

    CMediaTableItem * getSongItem(const QModelIndex& index) const;
    CMediaTableItem * getSongItem(int row) const;
    int getRowForSongItem(CMediaTableItem * songItem) const;
    CMediaTableItem * getPreviousSong(CMediaTableItem * songItem, bool shuffle) const;
    CMediaTableItem * getNextSong(CMediaTableItem * songItem, bool shuffle) const;
    CMediaTableItem * getLastSong(bool shuffle) const;
    void setCurrentSong(CMediaTableItem * songItem);
    void initShuffle(CMediaTableItem * firstSong = nullptr);
    inline CMediaTableItem * getCurrentSongItem() const;
    void replaceSong(CSong * oldSong, CSong * newSong);
    void applyFilter(const QString& filter);

signals:

    void columnSorted(int column, Qt::SortOrder order);          ///< Signal émis lorsqu'une colonne est triée.
    void columnAboutToBeSorted(int column, Qt::SortOrder order); ///< Signal émis lorsqu'une colonne va être triée.

protected:

    void applyFilterForShuffleList(const QString& filter);

private:

    static inline bool cmpSongPositionAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        int pos1 = song1->getPosition();
        int pos2 = song2->getPosition();

        if (pos1 == pos2)
            return cmpSongArtistAsc(song2, song1);

        return (pos1 < pos2);
    }

    static inline bool cmpSongPositionDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongPositionAsc(song2, song1);
    }

    static inline bool cmpSongTitleAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const QString title1 = song1->getSong()->getTitleSort(false);
        const QString title2 = song2->getSong()->getTitleSort(false);

        int cmp = QString::compare(title1, title2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongPathNameAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongTitleDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongTitleAsc(song2, song1);
    }

    static inline bool cmpSongArtistAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const QString artist1 = song1->getSong()->getArtistNameSort(false);
        const QString artist2 = song2->getSong()->getArtistNameSort(false);

        int cmp = QString::compare(artist1, artist2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongAlbumAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongArtistDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongArtistAsc(song2, song1);
    }

    static inline bool cmpSongAlbumAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const QString album1 = song1->getSong()->getAlbumTitleSort(false);
        const QString album2 = song2->getSong()->getAlbumTitleSort(false);

        int cmp = QString::compare(album1, album2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongDiscAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongAlbumDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongAlbumAsc(song2, song1);
    }

    static inline bool cmpSongAlbumArtistAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const QString artist1 = song1->getSong()->getAlbumArtistSort(false);
        const QString artist2 = song2->getSong()->getAlbumArtistSort(false);

        int cmp = QString::compare(artist1, artist2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongAlbumAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongAlbumArtistDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongAlbumArtistAsc(song2, song1);
    }

    static inline bool cmpSongComposerAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        QString composer1 = song1->getSong()->getComposerSort(false);
        QString composer2 = song2->getSong()->getComposerSort(false);

        int cmp = QString::compare(composer1, composer2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongComposerDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongComposerAsc(song2, song1);
    }

    static inline bool cmpSongYearAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const int year1 = song1->getSong()->getYear();
        const int year2 = song2->getSong()->getYear();

        if (year1 == year2)
            return cmpSongArtistAsc(song1, song2);

        return (year1 < year2);
    }

    static inline bool cmpSongYearDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongYearAsc(song2, song1);
    }

    static inline bool cmpSongTrackAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const int track1 = song1->getSong()->getTrackNumber();
        const int track2 = song2->getSong()->getTrackNumber();

        if (track1 == track2)
            return cmpSongTitleAsc(song1, song2);

        return (track1 < track2);
    }

    static inline bool cmpSongTrackDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongTrackAsc(song2, song1);
    }

    static inline bool cmpSongDiscAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const int disc1 = song1->getSong()->getDiscNumber();
        const int disc2 = song2->getSong()->getDiscNumber();

        if (disc1 == disc2)
            return cmpSongTrackAsc(song1, song2);

        return (disc1 < disc2);

        if (disc1 < disc2) return true;
        if (disc1 > disc2) return false;
    }

    static inline bool cmpSongDiscDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongDiscAsc(song2, song1);
    }

    static inline bool cmpSongGenreAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        QString genre1 = song1->getSong()->getGenre();
        QString genre2 = song2->getSong()->getGenre();

        int cmp = QString::compare(genre1, genre2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongGenreDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongGenreAsc(song2, song1);
    }

    static inline bool cmpSongRatingAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        int rating1 = song1->getSong()->getRating();
        int rating2 = song2->getSong()->getRating();

        if (rating1 == rating2)
            return cmpSongArtistAsc(song1, song2);

        return (rating1 < rating2);
    }

    static inline bool cmpSongRatingDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongRatingAsc(song2, song1);
    }

    static inline bool cmpSongCommentsAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        QString comment1 = song1->getSong()->getComments();
        QString comment2 = song2->getSong()->getComments();

        if (comment1 == comment2)
            return cmpSongArtistAsc(song1, song2);

        return (comment1 < comment2);
    }

    static inline bool cmpSongCommentsDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongCommentsAsc(song2, song1);
    }

    static inline bool cmpSongPlayCountAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        int playCount1 = song1->getSong()->getNumPlays();
        int playCount2 = song2->getSong()->getNumPlays();

        if (playCount1 == playCount2)
            return cmpSongArtistAsc(song1, song2);

        return (playCount1 < playCount2);
    }

    static inline bool cmpSongPlayCountDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongPlayCountAsc(song2, song1);
    }

    static inline bool cmpSongLastPlayedAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        QDateTime lastPlay1 = song1->getSong()->getLastPlay();
        QDateTime lastPlay2 = song2->getSong()->getLastPlay();

        if (lastPlay1 == lastPlay2)
            return cmpSongArtistAsc(song1, song2);

        return (lastPlay1 < lastPlay2);
    }

    static inline bool cmpSongLastPlayedDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongLastPlayedAsc(song2, song1);
    }

    static inline bool cmpSongPathNameAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return (song1->getSong()->getFileName() < song2->getSong()->getFileName());
    }

    static inline bool cmpSongPathNameDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongPathNameAsc(song2, song1);
    }
    
    static inline bool cmpSongFileNameAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const QString pathName1 = song1->getSong()->getFileName();
        const QString pathName2 = song2->getSong()->getFileName();

        const QString fileName1 = pathName1.mid(pathName1.lastIndexOf('/') + 1);
        const QString fileName2 = pathName2.mid(pathName2.lastIndexOf('/') + 1);

        if (fileName1 == fileName2)
            return cmpSongPathNameAsc(song1, song2);

        return (fileName1 < fileName2);
    }

    static inline bool cmpSongFileNameDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongFileNameAsc(song2, song1);
    }

    static inline bool cmpSongBitRateAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        int bitRate1 = song1->getSong()->getBitRate();
        int bitRate2 = song2->getSong()->getBitRate();

        if (bitRate1 == bitRate2)
            return cmpSongArtistAsc(song1, song2);

        return (bitRate1 < bitRate2);
    }

    static inline bool cmpSongBitRateDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongBitRateAsc(song2, song1);
    }

    static inline bool cmpSongFormatAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        CSong::TFormat format1 = song1->getSong()->getFormat();
        CSong::TFormat format2 = song2->getSong()->getFormat();

        if (format1 == format2)
            return cmpSongArtistAsc(song1, song2);

        return (format1 < format2); // TODO: classer par ordre alphabétique ?
    }

    static inline bool cmpSongFormatDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongFormatAsc(song2, song1);
    }

    static inline bool cmpSongDurationAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        int duration1 = song1->getSong()->getDuration();
        int duration2 = song2->getSong()->getDuration();

        if (duration1 == duration2)
            return cmpSongArtistAsc(song1, song2);

        return (duration1 < duration2);
    }

    static inline bool cmpSongDurationDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongDurationAsc(song2, song1);
    }

    static inline bool cmpSongSampleRateAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        int sampleRate1 = song1->getSong()->getSampleRate();
        int sampleRate2 = song2->getSong()->getSampleRate();

        if (sampleRate1 == sampleRate2)
            return cmpSongArtistAsc(song1, song2);

        return (sampleRate1 < sampleRate2);
    }

    static inline bool cmpSongSampleRateDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongSampleRateAsc(song2, song1);
    }

    static inline bool cmpSongCreationDateAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        QDateTime creation1 = song1->getSong()->getCreationDate();
        QDateTime creation2 = song2->getSong()->getCreationDate();

        if (creation1 == creation2)
            return cmpSongArtistAsc(song1, song2);

        return (creation1 < creation2);
    }

    static inline bool cmpSongCreationDateDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongCreationDateAsc(song2, song1);
    }

    static inline bool cmpSongModificationDateAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        QDateTime modif1 = song1->getSong()->getModificationDate();
        QDateTime modif2 = song2->getSong()->getModificationDate();

        if (modif1 == modif2)
            return cmpSongArtistAsc(song1, song2);

        return (modif1 < modif2);
    }

    static inline bool cmpSongModificationDateDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongModificationDateAsc(song2, song1);
    }

    static inline bool cmpSongChannelsAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        int channels1 = song1->getSong()->getNumChannels();
        int channels2 = song2->getSong()->getNumChannels();

        if (channels1 == channels2)
            return cmpSongArtistAsc(song1, song2);

        return (channels1 < channels2);
    }

    static inline bool cmpSongChannelsDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongChannelsAsc(song2, song1);
    }

    static inline bool cmpSongFileSizeAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        int fileSize1 = song1->getSong()->getFileSize();
        int fileSize2 = song2->getSong()->getFileSize();

        if (fileSize1 == fileSize2)
            return cmpSongArtistAsc(song1, song2);

        return (fileSize1 < fileSize2);
    }

    static inline bool cmpSongFileSizeDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongFileSizeAsc(song2, song1);
    }

    static inline bool cmpSongLyricsAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        QString lyrics1 = song1->getSong()->getLyrics();
        QString lyrics2 = song2->getSong()->getLyrics();

        int cmp = QString::compare(lyrics1, lyrics2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongLyricsDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongLyricsAsc(song2, song1);
    }

    static inline bool cmpSongLanguageAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        TLanguage lang1 = song1->getSong()->getLanguage();
        TLanguage lang2 = song2->getSong()->getLanguage();

        if (lang1 == lang2)
            return cmpSongArtistAsc(song1, song2);

        return (getLanguageName(lang1) < getLanguageName(lang2));
    }

    static inline bool cmpSongLanguageDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongLanguageAsc(song2, song1);
    }

    static inline bool cmpSongLyricistAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        QString author1 = song1->getSong()->getLyricist();
        QString author2 = song2->getSong()->getLyricist();

        int cmp = QString::compare(author1, author2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongLyricistDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongLyricistAsc(song2, song1);
    }

    static inline bool cmpSongGroupingAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        QString grouping1 = song1->getSong()->getGrouping();
        QString grouping2 = song2->getSong()->getGrouping();

        int cmp = QString::compare(grouping1, grouping2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongGroupingDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongGroupingAsc(song2, song1);
    }

    static inline bool cmpSongSubTitleAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        QString subTitle1 = song1->getSong()->getSubTitle();
        QString subTitle2 = song2->getSong()->getSubTitle();

        int cmp = QString::compare(subTitle1, subTitle2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongSubTitleDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongSubTitleAsc(song2, song1);
    }

    static inline bool cmpSongTrackGainAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        float gain1 = song1->getSong()->getTrackGain();
        float gain2 = song2->getSong()->getTrackGain();

        if (gain1 == gain2)
            return cmpSongArtistAsc(song1, song2);

        return (gain1 < gain2);
    }

    static inline bool cmpSongTrackGainDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongTrackGainAsc(song2, song1);
    }

    static inline bool cmpSongTrackPeakAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        float peak1 = song1->getSong()->getTrackPeak();
        float peak2 = song2->getSong()->getTrackPeak();

        if (peak1 == peak2)
            return cmpSongArtistAsc(song1, song2);

        return (peak1 < peak2);
    }

    static inline bool cmpSongTrackPeakDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongTrackGainAsc(song2, song1);
    }

    static inline bool cmpSongAlbumGainAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        float gain1 = song1->getSong()->getAlbumGain();
        float gain2 = song2->getSong()->getAlbumGain();

        if (gain1 == gain2)
            return cmpSongArtistAsc(song1, song2);

        return (gain1 < gain2);
    }

    static inline bool cmpSongAlbumGainDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongAlbumGainAsc(song2, song1);
    }

    static inline bool cmpSongAlbumPeakAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        float peak1 = song1->getSong()->getAlbumPeak();
        float peak2 = song2->getSong()->getAlbumPeak();

        if (peak1 == peak2)
            return cmpSongArtistAsc(song1, song2);

        return (peak1 < peak2);
    }

    static inline bool cmpSongAlbumPeakDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongAlbumGainAsc(song2, song1);
    }

    static inline bool cmpSongBPMAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        int bpm1 = song1->getSong()->getBPM();
        int bpm2 = song2->getSong()->getBPM();

        if (bpm1 == bpm2)
            return cmpSongArtistAsc(song1, song2);

        return (bpm1 < bpm2);
    }

    static inline bool cmpSongBPMDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongBPMAsc(song2, song1);
    }

    static inline bool cmpSongTitleSortAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const QString title1 = song1->getSong()->getTitleSort();
        const QString title2 = song2->getSong()->getTitleSort();

        int cmp = QString::compare(title1, title2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongTitleSortDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongTitleAsc(song2, song1);
    }

    static inline bool cmpSongArtistSortAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const QString artist1 = song1->getSong()->getArtistNameSort();
        const QString artist2 = song2->getSong()->getArtistNameSort();
        
        int cmp = QString::compare(artist1, artist2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongArtistSortDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongArtistAsc(song2, song1);
    }

    static inline bool cmpSongAlbumSortAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const QString album1 = song1->getSong()->getAlbumTitleSort();
        const QString album2 = song2->getSong()->getAlbumTitleSort();

        int cmp = QString::compare(album1, album2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongAlbumSortDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongAlbumAsc(song2, song1);
    }

    static inline bool cmpSongAlbumArtistSortAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const QString artist1 = song1->getSong()->getAlbumArtistSort();
        const QString artist2 = song2->getSong()->getAlbumArtistSort();

        int cmp = QString::compare(artist1, artist2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongAlbumArtistSortDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongAlbumArtistAsc(song2, song1);
    }

    static inline bool cmpSongComposerSortAsc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        const QString composer1 = song1->getSong()->getComposerSort();
        const QString composer2 = song2->getSong()->getComposerSort();

        int cmp = QString::compare(composer1, composer2, Qt::CaseInsensitive);

        if (cmp == 0)
            return cmpSongArtistAsc(song1, song2);

        return (cmp < 0);
    }

    static inline bool cmpSongComposerSortDesc(CMediaTableItem * song1, CMediaTableItem * song2)
    {
        return cmpSongComposerAsc(song2, song1);
    }


    CMainWindow * m_mainWindow;                  ///< Pointeur sur l'application.
    bool m_canDrop;                                ///< Indique si la vue peut recevoir des données (liste statique).
    int m_columnSort;                              ///< Numéro de la colonne triée.
    CMediaTableItem * m_currentSongItem;            ///< Pointeur sur le morceau courant.
    QList<CMediaTableItem *> m_data;                ///< Liste des morceaux.
    QList<CMediaTableItem *> m_dataShuffle;         ///< Liste des morceaux aléatoires.
    QList<CMediaTableItem *> m_dataFiltered;        ///< Liste des morceaux filtrées.
    QList<CMediaTableItem *> m_dataShuffleFiltered; ///< Liste des morceaux aléatoires filtrées.
};


/**
 * Retourne le nombre de morceaux du modèle.
 *
 * \return Nombre de morceaux.
 */

inline int CMediaTableModel::getNumSongs() const
{
    return m_data.size();
}


/**
 * Retourne le morceau courant.
 *
 * \return Pointeur sur le morceau courant.
 */

inline CMediaTableItem * CMediaTableModel::getCurrentSongItem() const
{
    return m_currentSongItem;
}

#endif // FILE_C_SONG_TABLE_MODEL

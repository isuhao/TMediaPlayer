/*
Copyright (C) 2012 Teddy Michel

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

#ifndef FILE_C_SONG_TABLE_HEADER
#define FILE_C_SONG_TABLE_HEADER

#include <QHeaderView>
#include "CSongTable.hpp"


class QMenu;


class CSongTableHeader : public QHeaderView
{
    Q_OBJECT

    friend class CSongTable;

public:

    explicit CSongTableHeader(QWidget * parent = NULL);
    virtual ~CSongTableHeader();

signals:

    void columnShown(int col, bool shown);

protected:

    void contextMenuEvent(QContextMenuEvent * event);

protected slots:

    inline void showColTitle           (bool show = true) { emit columnShown(CSongTable::ColTitle           , show); }
    inline void showColArtist          (bool show = true) { emit columnShown(CSongTable::ColArtist          , show); }
    inline void showColAlbum           (bool show = true) { emit columnShown(CSongTable::ColAlbum           , show); }
    inline void showColAlbumArtist     (bool show = true) { emit columnShown(CSongTable::ColAlbumArtist     , show); }
    inline void showColComposer        (bool show = true) { emit columnShown(CSongTable::ColComposer        , show); }
    inline void showColYear            (bool show = true) { emit columnShown(CSongTable::ColYear            , show); }
    inline void showColTrackNumber     (bool show = true) { emit columnShown(CSongTable::ColTrackNumber     , show); }
    inline void showColDiscNumber      (bool show = true) { emit columnShown(CSongTable::ColDiscNumber      , show); }
    inline void showColGenre           (bool show = true) { emit columnShown(CSongTable::ColGenre           , show); }
    inline void showColRating          (bool show = true) { emit columnShown(CSongTable::ColRating          , show); }
    inline void showColComments        (bool show = true) { emit columnShown(CSongTable::ColComments        , show); }
    inline void showColPlayCount       (bool show = true) { emit columnShown(CSongTable::ColPlayCount       , show); }
    inline void showColLastPlayTime    (bool show = true) { emit columnShown(CSongTable::ColLastPlayTime    , show); }
    inline void showColPathName        (bool show = true) { emit columnShown(CSongTable::ColPathName        , show); }
    inline void showColBitRate         (bool show = true) { emit columnShown(CSongTable::ColBitRate         , show); }
    inline void showColFormat          (bool show = true) { emit columnShown(CSongTable::ColFormat          , show); }
    inline void showColDuration        (bool show = true) { emit columnShown(CSongTable::ColDuration        , show); }
    inline void showColSampleRate      (bool show = true) { emit columnShown(CSongTable::ColSampleRate      , show); }
    inline void showColCreationDate    (bool show = true) { emit columnShown(CSongTable::ColCreationDate    , show); }
    inline void showColModificationDate(bool show = true) { emit columnShown(CSongTable::ColModificationDate, show); }
    inline void showColChannels        (bool show = true) { emit columnShown(CSongTable::ColChannels        , show); }
    inline void showColFileSize        (bool show = true) { emit columnShown(CSongTable::ColFileSize        , show); }
    inline void showColLyrics          (bool show = true) { emit columnShown(CSongTable::ColLyrics          , show); }
    inline void showColLanguage        (bool show = true) { emit columnShown(CSongTable::ColLanguage        , show); }
    inline void showColLyricist        (bool show = true) { emit columnShown(CSongTable::ColLyricist        , show); }
    inline void showColGrouping        (bool show = true) { emit columnShown(CSongTable::ColGrouping        , show); }
    inline void showColSubTitle        (bool show = true) { emit columnShown(CSongTable::ColSubTitle        , show); }
    inline void showColTrackGain       (bool show = true) { emit columnShown(CSongTable::ColTrackGain       , show); }
    inline void showColTrackPeak       (bool show = true) { emit columnShown(CSongTable::ColTrackPeak       , show); }
    inline void showColAlbumGain       (bool show = true) { emit columnShown(CSongTable::ColAlbumGain       , show); }
    inline void showColAlbumPeak       (bool show = true) { emit columnShown(CSongTable::ColAlbumPeak       , show); }
    inline void showColBPM             (bool show = true) { emit columnShown(CSongTable::ColBPM             , show); }
    inline void showColTitleSort       (bool show = true) { emit columnShown(CSongTable::ColTitleSort       , show); }
    inline void showColArtistSort      (bool show = true) { emit columnShown(CSongTable::ColArtistSort      , show); }
    inline void showColAlbumSort       (bool show = true) { emit columnShown(CSongTable::ColAlbumSort       , show); }
    inline void showColAlbumArtistSort (bool show = true) { emit columnShown(CSongTable::ColAlbumArtistSort , show); }
    inline void showColComposerSort    (bool show = true) { emit columnShown(CSongTable::ColComposerSort    , show); }
    inline void showColFileName        (bool show = true) { emit columnShown(CSongTable::ColFileName        , show); }

private:

    QMenu * m_contextMenu;
    QMenu * m_menuInfos;
    QMenu * m_menuProperties;
    QMenu * m_menuReplayGain;
    QAction * m_actionShowCol[CSongTable::ColNumber];
};

#endif // FILE_C_SONG_TABLE_HEADER

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

#ifndef FILE_C_MEDIA_TABLE_HEADER_HPP_
#define FILE_C_MEDIA_TABLE_HEADER_HPP_

#include <QHeaderView>
#include "CMediaTableView.hpp"


class QMenu;


class CMediaTableHeader : public QHeaderView
{
    Q_OBJECT

    friend class CMediaTableView;

public:

    explicit CMediaTableHeader(QWidget * parent = nullptr);
    virtual ~CMediaTableHeader();

signals:

    void columnShown(int col, bool shown);

protected:

    void contextMenuEvent(QContextMenuEvent * event);

protected slots:

    inline void showColTitle           (bool show = true) { emit columnShown(CMediaTableView::ColTitle           , show); }
    inline void showColArtist          (bool show = true) { emit columnShown(CMediaTableView::ColArtist          , show); }
    inline void showColAlbum           (bool show = true) { emit columnShown(CMediaTableView::ColAlbum           , show); }
    inline void showColAlbumArtist     (bool show = true) { emit columnShown(CMediaTableView::ColAlbumArtist     , show); }
    inline void showColComposer        (bool show = true) { emit columnShown(CMediaTableView::ColComposer        , show); }
    inline void showColYear            (bool show = true) { emit columnShown(CMediaTableView::ColYear            , show); }
    inline void showColTrackNumber     (bool show = true) { emit columnShown(CMediaTableView::ColTrackNumber     , show); }
    inline void showColDiscNumber      (bool show = true) { emit columnShown(CMediaTableView::ColDiscNumber      , show); }
    inline void showColGenre           (bool show = true) { emit columnShown(CMediaTableView::ColGenre           , show); }
    inline void showColRating          (bool show = true) { emit columnShown(CMediaTableView::ColRating          , show); }
    inline void showColComments        (bool show = true) { emit columnShown(CMediaTableView::ColComments        , show); }
    inline void showColPlayCount       (bool show = true) { emit columnShown(CMediaTableView::ColPlayCount       , show); }
    inline void showColLastPlayTime    (bool show = true) { emit columnShown(CMediaTableView::ColLastPlayTime    , show); }
    inline void showColPathName        (bool show = true) { emit columnShown(CMediaTableView::ColPathName        , show); }
    inline void showColBitRate         (bool show = true) { emit columnShown(CMediaTableView::ColBitRate         , show); }
    inline void showColFormat          (bool show = true) { emit columnShown(CMediaTableView::ColFormat          , show); }
    inline void showColDuration        (bool show = true) { emit columnShown(CMediaTableView::ColDuration        , show); }
    inline void showColSampleRate      (bool show = true) { emit columnShown(CMediaTableView::ColSampleRate      , show); }
    inline void showColCreationDate    (bool show = true) { emit columnShown(CMediaTableView::ColCreationDate    , show); }
    inline void showColModificationDate(bool show = true) { emit columnShown(CMediaTableView::ColModificationDate, show); }
    inline void showColChannels        (bool show = true) { emit columnShown(CMediaTableView::ColChannels        , show); }
    inline void showColFileSize        (bool show = true) { emit columnShown(CMediaTableView::ColFileSize        , show); }
    inline void showColLyrics          (bool show = true) { emit columnShown(CMediaTableView::ColLyrics          , show); }
    inline void showColLanguage        (bool show = true) { emit columnShown(CMediaTableView::ColLanguage        , show); }
    inline void showColLyricist        (bool show = true) { emit columnShown(CMediaTableView::ColLyricist        , show); }
    inline void showColGrouping        (bool show = true) { emit columnShown(CMediaTableView::ColGrouping        , show); }
    inline void showColSubTitle        (bool show = true) { emit columnShown(CMediaTableView::ColSubTitle        , show); }
    inline void showColTrackGain       (bool show = true) { emit columnShown(CMediaTableView::ColTrackGain       , show); }
    inline void showColTrackPeak       (bool show = true) { emit columnShown(CMediaTableView::ColTrackPeak       , show); }
    inline void showColAlbumGain       (bool show = true) { emit columnShown(CMediaTableView::ColAlbumGain       , show); }
    inline void showColAlbumPeak       (bool show = true) { emit columnShown(CMediaTableView::ColAlbumPeak       , show); }
    inline void showColBPM             (bool show = true) { emit columnShown(CMediaTableView::ColBPM             , show); }
    inline void showColTitleSort       (bool show = true) { emit columnShown(CMediaTableView::ColTitleSort       , show); }
    inline void showColArtistSort      (bool show = true) { emit columnShown(CMediaTableView::ColArtistSort      , show); }
    inline void showColAlbumSort       (bool show = true) { emit columnShown(CMediaTableView::ColAlbumSort       , show); }
    inline void showColAlbumArtistSort (bool show = true) { emit columnShown(CMediaTableView::ColAlbumArtistSort , show); }
    inline void showColComposerSort    (bool show = true) { emit columnShown(CMediaTableView::ColComposerSort    , show); }
    inline void showColFileName        (bool show = true) { emit columnShown(CMediaTableView::ColFileName        , show); }

private:

    QMenu * m_contextMenu;
    QMenu * m_menuInfos;
    QMenu * m_menuProperties;
    QMenu * m_menuReplayGain;
    QAction * m_actionShowCol[CMediaTableView::ColNumber];
};

#endif // FILE_C_MEDIA_TABLE_HEADER_HPP_

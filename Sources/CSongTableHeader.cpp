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

#include "CSongTableHeader.hpp"
#include "CSongTable.hpp"
#include <QMenu>
#include <QContextMenuEvent>

#include <QtDebug>


#define T_CREATE_ACTION(colName, colType) \
    m_actionShowCol[colType] = m_contextMenu->addAction(CSongTable::getColumnTypeName(CSongTable::getColumnTypeFromInteger(colType)), this, SLOT(showCol##colName(bool))); \
    m_actionShowCol[colType]->setCheckable(true);


CSongTableHeader::CSongTableHeader(QWidget * parent) :
    QHeaderView             (Qt::Horizontal, parent),
    m_contextMenu           (NULL)
{
    setClickable(true);
    setMovable(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_contextMenu = new QMenu(this);
    m_actionShowCol[0] = NULL;

    T_CREATE_ACTION(Title           , CSongTable::ColTitle           );
    T_CREATE_ACTION(Artist          , CSongTable::ColArtist          );
    T_CREATE_ACTION(Album           , CSongTable::ColAlbum           );
    T_CREATE_ACTION(AlbumArtist     , CSongTable::ColAlbumArtist     );
    T_CREATE_ACTION(Composer        , CSongTable::ColComposer        );
    T_CREATE_ACTION(Year            , CSongTable::ColYear            );
    T_CREATE_ACTION(TrackNumber     , CSongTable::ColTrackNumber     );
    T_CREATE_ACTION(DiscNumber      , CSongTable::ColDiscNumber      );
    T_CREATE_ACTION(Genre           , CSongTable::ColGenre           );
    T_CREATE_ACTION(Rating          , CSongTable::ColRating          );
    T_CREATE_ACTION(Comments        , CSongTable::ColComments        );
    T_CREATE_ACTION(PlayCount       , CSongTable::ColPlayCount       );
    T_CREATE_ACTION(LastPlayTime    , CSongTable::ColLastPlayTime    );
    T_CREATE_ACTION(FileName        , CSongTable::ColFileName        );
    T_CREATE_ACTION(BitRate         , CSongTable::ColBitRate         );
    T_CREATE_ACTION(Format          , CSongTable::ColFormat          );
    T_CREATE_ACTION(Duration        , CSongTable::ColDuration        );
    T_CREATE_ACTION(SampleRate      , CSongTable::ColSampleRate      );
    T_CREATE_ACTION(CreationDate    , CSongTable::ColCreationDate    );
    T_CREATE_ACTION(ModificationDate, CSongTable::ColModificationDate);
    T_CREATE_ACTION(Channels        , CSongTable::ColChannels        );
    T_CREATE_ACTION(FileSize        , CSongTable::ColFileSize        );
    T_CREATE_ACTION(Lyrics          , CSongTable::ColLyrics          );
    T_CREATE_ACTION(Language        , CSongTable::ColLanguage        );
    T_CREATE_ACTION(Lyricist        , CSongTable::ColLyricist        );
    T_CREATE_ACTION(Grouping        , CSongTable::ColGrouping        );
    T_CREATE_ACTION(SubTitle        , CSongTable::ColSubTitle        );
}


CSongTableHeader::~CSongTableHeader()
{
    //qDebug() << "CSongTableHeader::~CSongTableHeader()";
}


void CSongTableHeader::contextMenuEvent(QContextMenuEvent * event)
{
    //qDebug() << "CSongTableHeader::contextMenuEvent()";

    QPoint point = event->pos();
    m_contextMenu->move(mapToGlobal(point));
    m_contextMenu->show();

    QHeaderView::contextMenuEvent(event);
}

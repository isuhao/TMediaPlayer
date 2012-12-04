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


#define T_CREATE_ACTION(menu, colName, colType) \
    m_actionShowCol[colType] = menu->addAction(CSongTable::getColumnTypeName(CSongTable::getColumnTypeFromInteger(colType)), this, SLOT(showCol##colName(bool))); \
    m_actionShowCol[colType]->setCheckable(true);


CSongTableHeader::CSongTableHeader(QWidget * parent) :
    QHeaderView             (Qt::Horizontal, parent),
    m_contextMenu           (NULL),
    m_menuInfos             (NULL),
    m_menuProperties        (NULL),
    m_menuReplayGain        (NULL)
{
    setClickable(true);
    setMovable(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_contextMenu = new QMenu(this);
    m_actionShowCol[0] = NULL;

    m_menuInfos = new QMenu(tr("Informations"), this);
    m_menuProperties = new QMenu(tr("Properties"), this);
    m_menuReplayGain = new QMenu(tr("Replay Gain"), this);

    // Remarque : la hauteur d'un menu est limitée à 600px, soit 27 items

    // Informations
    T_CREATE_ACTION(m_menuInfos, Title          , CSongTable::ColTitle          );
    T_CREATE_ACTION(m_menuInfos, Artist         , CSongTable::ColArtist         );
    T_CREATE_ACTION(m_menuInfos, Album          , CSongTable::ColAlbum          );
    T_CREATE_ACTION(m_menuInfos, AlbumArtist    , CSongTable::ColAlbumArtist    );
    T_CREATE_ACTION(m_menuInfos, Composer       , CSongTable::ColComposer       );
    T_CREATE_ACTION(m_menuInfos, Year           , CSongTable::ColYear           );
    T_CREATE_ACTION(m_menuInfos, TrackNumber    , CSongTable::ColTrackNumber    );
    T_CREATE_ACTION(m_menuInfos, DiscNumber     , CSongTable::ColDiscNumber     );
    T_CREATE_ACTION(m_menuInfos, Genre          , CSongTable::ColGenre          );
    T_CREATE_ACTION(m_menuInfos, Rating         , CSongTable::ColRating         );
    T_CREATE_ACTION(m_menuInfos, Comments       , CSongTable::ColComments       );
    T_CREATE_ACTION(m_menuInfos, Lyrics         , CSongTable::ColLyrics         );
    T_CREATE_ACTION(m_menuInfos, Language       , CSongTable::ColLanguage       );
    T_CREATE_ACTION(m_menuInfos, Lyricist       , CSongTable::ColLyricist       );
    T_CREATE_ACTION(m_menuInfos, Grouping       , CSongTable::ColGrouping       );
    T_CREATE_ACTION(m_menuInfos, SubTitle       , CSongTable::ColSubTitle       );
    T_CREATE_ACTION(m_menuInfos, BPM            , CSongTable::ColBPM            );
    T_CREATE_ACTION(m_menuInfos, TitleSort      , CSongTable::ColTitleSort      );
    T_CREATE_ACTION(m_menuInfos, ArtistSort     , CSongTable::ColArtistSort     );
    T_CREATE_ACTION(m_menuInfos, AlbumSort      , CSongTable::ColAlbumSort      );
    T_CREATE_ACTION(m_menuInfos, AlbumArtistSort, CSongTable::ColAlbumArtistSort);
    T_CREATE_ACTION(m_menuInfos, ComposerSort   , CSongTable::ColComposerSort   );

    // Propriétés
    T_CREATE_ACTION(m_menuProperties, PlayCount       , CSongTable::ColPlayCount       );
    T_CREATE_ACTION(m_menuProperties, LastPlayTime    , CSongTable::ColLastPlayTime    );
    T_CREATE_ACTION(m_menuProperties, PathName        , CSongTable::ColPathName        );
    T_CREATE_ACTION(m_menuProperties, FileName        , CSongTable::ColFileName        );
    T_CREATE_ACTION(m_menuProperties, BitRate         , CSongTable::ColBitRate         );
    T_CREATE_ACTION(m_menuProperties, Format          , CSongTable::ColFormat          );
    T_CREATE_ACTION(m_menuProperties, Duration        , CSongTable::ColDuration        );
    T_CREATE_ACTION(m_menuProperties, SampleRate      , CSongTable::ColSampleRate      );
    T_CREATE_ACTION(m_menuProperties, CreationDate    , CSongTable::ColCreationDate    );
    T_CREATE_ACTION(m_menuProperties, ModificationDate, CSongTable::ColModificationDate);
    T_CREATE_ACTION(m_menuProperties, Channels        , CSongTable::ColChannels        );
    T_CREATE_ACTION(m_menuProperties, FileSize        , CSongTable::ColFileSize        );

    // Replay Gain
    T_CREATE_ACTION(m_menuReplayGain, TrackGain, CSongTable::ColTrackGain);
    T_CREATE_ACTION(m_menuReplayGain, TrackPeak, CSongTable::ColTrackPeak);
    T_CREATE_ACTION(m_menuReplayGain, AlbumGain, CSongTable::ColAlbumGain);
    T_CREATE_ACTION(m_menuReplayGain, AlbumPeak, CSongTable::ColAlbumPeak);

    m_contextMenu->addMenu(m_menuInfos);
    m_contextMenu->addMenu(m_menuProperties);
    m_contextMenu->addMenu(m_menuReplayGain);
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

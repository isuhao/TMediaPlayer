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

#include "CMediaTableHeader.hpp"
#include "CMediaTableView.hpp"
#include "Utils.hpp"
#include <QMenu>
#include <QContextMenuEvent>

#include <QtDebug>


#define T_CREATE_ACTION(menu, colName, colType) \
    m_actionShowCol[colType] = menu->addAction(CMediaTableView::getColumnTypeName(CMediaTableView::getColumnTypeFromInteger(colType)), this, SLOT(showCol##colName(bool))); \
    m_actionShowCol[colType]->setCheckable(true);


CMediaTableHeader::CMediaTableHeader(QWidget * parent) :
QHeaderView      (Qt::Horizontal, parent),
m_contextMenu    (nullptr),
m_menuInfos      (nullptr),
m_menuProperties (nullptr),
m_menuReplayGain (nullptr)
{
#if QT_VERSION >= 0x050000
    setSectionsClickable(true);
    setSectionsMovable(true);
#else
    setClickable(true);
    setMovable(true);
#endif
    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_contextMenu = new QMenu(this);
    m_actionShowCol[0] = nullptr;

    m_menuInfos = new QMenu(tr("Informations"), this);
    m_menuProperties = new QMenu(tr("Properties"), this);
    m_menuReplayGain = new QMenu(tr("Replay Gain"), this);

    // Remarque : la hauteur d'un menu est limitée à 600px, soit 27 items

    // Informations
    T_CREATE_ACTION(m_menuInfos, Title          , CMediaTableView::ColTitle          );
    T_CREATE_ACTION(m_menuInfos, Artist         , CMediaTableView::ColArtist         );
    T_CREATE_ACTION(m_menuInfos, Album          , CMediaTableView::ColAlbum          );
    T_CREATE_ACTION(m_menuInfos, AlbumArtist    , CMediaTableView::ColAlbumArtist    );
    T_CREATE_ACTION(m_menuInfos, Composer       , CMediaTableView::ColComposer       );
    T_CREATE_ACTION(m_menuInfos, Year           , CMediaTableView::ColYear           );
    T_CREATE_ACTION(m_menuInfos, TrackNumber    , CMediaTableView::ColTrackNumber    );
    T_CREATE_ACTION(m_menuInfos, DiscNumber     , CMediaTableView::ColDiscNumber     );
    T_CREATE_ACTION(m_menuInfos, Genre          , CMediaTableView::ColGenre          );
    T_CREATE_ACTION(m_menuInfos, Rating         , CMediaTableView::ColRating         );
    T_CREATE_ACTION(m_menuInfos, Comments       , CMediaTableView::ColComments       );
    T_CREATE_ACTION(m_menuInfos, Lyrics         , CMediaTableView::ColLyrics         );
    T_CREATE_ACTION(m_menuInfos, Language       , CMediaTableView::ColLanguage       );
    T_CREATE_ACTION(m_menuInfos, Lyricist       , CMediaTableView::ColLyricist       );
    T_CREATE_ACTION(m_menuInfos, Grouping       , CMediaTableView::ColGrouping       );
    T_CREATE_ACTION(m_menuInfos, SubTitle       , CMediaTableView::ColSubTitle       );
    T_CREATE_ACTION(m_menuInfos, BPM            , CMediaTableView::ColBPM            );
    m_menuInfos->addSeparator();
    T_CREATE_ACTION(m_menuInfos, TitleSort      , CMediaTableView::ColTitleSort      );
    T_CREATE_ACTION(m_menuInfos, ArtistSort     , CMediaTableView::ColArtistSort     );
    T_CREATE_ACTION(m_menuInfos, AlbumSort      , CMediaTableView::ColAlbumSort      );
    T_CREATE_ACTION(m_menuInfos, AlbumArtistSort, CMediaTableView::ColAlbumArtistSort);
    T_CREATE_ACTION(m_menuInfos, ComposerSort   , CMediaTableView::ColComposerSort   );

    // Propriétés
    T_CREATE_ACTION(m_menuProperties, PlayCount       , CMediaTableView::ColPlayCount       );
    T_CREATE_ACTION(m_menuProperties, LastPlayTime    , CMediaTableView::ColLastPlayTime    );
    T_CREATE_ACTION(m_menuProperties, PathName        , CMediaTableView::ColPathName        );
    T_CREATE_ACTION(m_menuProperties, FileName        , CMediaTableView::ColFileName        );
    T_CREATE_ACTION(m_menuProperties, BitRate         , CMediaTableView::ColBitRate         );
    T_CREATE_ACTION(m_menuProperties, Format          , CMediaTableView::ColFormat          );
    T_CREATE_ACTION(m_menuProperties, Duration        , CMediaTableView::ColDuration        );
    T_CREATE_ACTION(m_menuProperties, SampleRate      , CMediaTableView::ColSampleRate      );
    T_CREATE_ACTION(m_menuProperties, CreationDate    , CMediaTableView::ColCreationDate    );
    T_CREATE_ACTION(m_menuProperties, ModificationDate, CMediaTableView::ColModificationDate);
    T_CREATE_ACTION(m_menuProperties, Channels        , CMediaTableView::ColChannels        );
    T_CREATE_ACTION(m_menuProperties, FileSize        , CMediaTableView::ColFileSize        );

    // Replay Gain
    T_CREATE_ACTION(m_menuReplayGain, TrackGain, CMediaTableView::ColTrackGain);
    T_CREATE_ACTION(m_menuReplayGain, TrackPeak, CMediaTableView::ColTrackPeak);
    T_CREATE_ACTION(m_menuReplayGain, AlbumGain, CMediaTableView::ColAlbumGain);
    T_CREATE_ACTION(m_menuReplayGain, AlbumPeak, CMediaTableView::ColAlbumPeak);

    m_contextMenu->addMenu(m_menuInfos);
    m_contextMenu->addMenu(m_menuProperties);
    m_contextMenu->addMenu(m_menuReplayGain);
}


/**
 * Détruit l'en-tête du tableau.
 */

CMediaTableHeader::~CMediaTableHeader()
{

}


/**
 * Affiche le menu contextuel.
 *
 * \param event Évènement d'affichage du menu contextuel.
 */

void CMediaTableHeader::contextMenuEvent(QContextMenuEvent * event)
{
    Q_CHECK_PTR(event);

    QPoint point = event->pos();
    m_contextMenu->move(getCorrectMenuPosition(m_contextMenu, mapToGlobal(point)));
    m_contextMenu->show();

    QHeaderView::contextMenuEvent(event);
}

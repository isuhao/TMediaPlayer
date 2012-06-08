
#include "CSongTableHeader.hpp"
#include "CSongTable.hpp"
#include <QMenu>
#include <QContextMenuEvent>

#include <QtDebug>


#define T_CREATE_ACTION(colType) \
    m_actionShowCol[CSongTable::##colType] = m_contextMenu->addAction(CSongTable::getColumnTypeName(CSongTable::getColumnTypeFromInteger(CSongTable::##colType)), this, SLOT(show##colType(bool))); \
    m_actionShowCol[CSongTable::##colType]->setCheckable(true);


CSongTableHeader::CSongTableHeader(QWidget * parent) :
    QHeaderView             (Qt::Horizontal, parent),
    m_contextMenu           (NULL)
{
    setClickable(true);
    setMovable(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_contextMenu = new QMenu(this);
    m_actionShowCol[0] = NULL;

    T_CREATE_ACTION(ColTitle           );
    T_CREATE_ACTION(ColArtist          );
    T_CREATE_ACTION(ColAlbum           );
    T_CREATE_ACTION(ColAlbumArtist     );
    T_CREATE_ACTION(ColComposer        );
    T_CREATE_ACTION(ColYear            );
    T_CREATE_ACTION(ColTrackNumber     );
    T_CREATE_ACTION(ColDiscNumber      );
    T_CREATE_ACTION(ColGenre           );
    T_CREATE_ACTION(ColRating          );
    T_CREATE_ACTION(ColComments        );
    T_CREATE_ACTION(ColPlayCount       );
    T_CREATE_ACTION(ColLastPlayTime    );
    T_CREATE_ACTION(ColFileName        );
    T_CREATE_ACTION(ColBitRate         );
    T_CREATE_ACTION(ColFormat          );
    T_CREATE_ACTION(ColDuration        );
    T_CREATE_ACTION(ColSampleRate      );
    T_CREATE_ACTION(ColCreationDate    );
    T_CREATE_ACTION(ColModificationDate);
    T_CREATE_ACTION(ColChannels        );
    T_CREATE_ACTION(ColFileSize        );
    T_CREATE_ACTION(ColLyrics          );
    T_CREATE_ACTION(ColLanguage        );
    T_CREATE_ACTION(ColLyricist        );
    T_CREATE_ACTION(ColGrouping        );
    T_CREATE_ACTION(ColSubTitle        );
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

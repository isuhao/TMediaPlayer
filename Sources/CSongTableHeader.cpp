
#include "CSongTableHeader.hpp"
#include "CSongTable.hpp"
#include <QMenu>
#include <QContextMenuEvent>

#include <QtDebug>


CSongTableHeader::CSongTableHeader(QWidget * parent) :
    QHeaderView          (Qt::Horizontal, parent),
    m_contextMenu        (NULL),
    m_actColTitle        (NULL),
    m_actColArtist       (NULL),
    m_actColAlbum        (NULL),
    m_actColAlbumArtist  (NULL),
    m_actColComposer     (NULL),
    m_actColYear         (NULL),
    m_actColTrackNumber  (NULL),
    m_actColDiscNumber   (NULL),
    m_actColGenre        (NULL),
    m_actColRating       (NULL),
    m_actColComments     (NULL),
    m_actColPlayCount    (NULL),
    m_actColLastPlayTime (NULL),
    m_actColFileName     (NULL),
    m_actColBitRate      (NULL),
    m_actColFormat       (NULL),
    m_actColDuration     (NULL)
{
    setClickable(true);
    setMovable(true);
    setHighlightSections(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_contextMenu = new QMenu(this);

    m_actColTitle        = m_contextMenu->addAction(tr("Title"       ), this, SLOT(showColTitle       (bool))); m_actColTitle       ->setCheckable(true);
    m_actColArtist       = m_contextMenu->addAction(tr("Artist"      ), this, SLOT(showColArtist      (bool))); m_actColArtist      ->setCheckable(true);
    m_actColAlbum        = m_contextMenu->addAction(tr("Album"       ), this, SLOT(showColAlbum       (bool))); m_actColAlbum       ->setCheckable(true);
    m_actColAlbumArtist  = m_contextMenu->addAction(tr("Album artist"), this, SLOT(showColAlbumArtist (bool))); m_actColAlbumArtist ->setCheckable(true);
    m_actColComposer     = m_contextMenu->addAction(tr("Composer"    ), this, SLOT(showColComposer    (bool))); m_actColComposer    ->setCheckable(true);
    m_actColYear         = m_contextMenu->addAction(tr("Year"        ), this, SLOT(showColYear        (bool))); m_actColYear        ->setCheckable(true);
    m_actColTrackNumber  = m_contextMenu->addAction(tr("Track"       ), this, SLOT(showColTrackNumber (bool))); m_actColTrackNumber ->setCheckable(true);
    m_actColDiscNumber   = m_contextMenu->addAction(tr("Disc"        ), this, SLOT(showColDiscNumber  (bool))); m_actColDiscNumber  ->setCheckable(true);
    m_actColGenre        = m_contextMenu->addAction(tr("Genre"       ), this, SLOT(showColGenre       (bool))); m_actColGenre       ->setCheckable(true);
    m_actColRating       = m_contextMenu->addAction(tr("Rating"      ), this, SLOT(showColRating      (bool))); m_actColRating      ->setCheckable(true);
    m_actColComments     = m_contextMenu->addAction(tr("Comments"    ), this, SLOT(showColComments    (bool))); m_actColComments    ->setCheckable(true);
    m_actColPlayCount    = m_contextMenu->addAction(tr("Plays"       ), this, SLOT(showColPlayCount   (bool))); m_actColPlayCount   ->setCheckable(true);
    m_actColLastPlayTime = m_contextMenu->addAction(tr("Last played" ), this, SLOT(showColLastPlayTime(bool))); m_actColLastPlayTime->setCheckable(true);
    m_actColFileName     = m_contextMenu->addAction(tr("File name"   ), this, SLOT(showColFileName    (bool))); m_actColFileName    ->setCheckable(true);
    m_actColBitRate      = m_contextMenu->addAction(tr("Bit rate"    ), this, SLOT(showColBitRate     (bool))); m_actColBitRate     ->setCheckable(true);
    m_actColFormat       = m_contextMenu->addAction(tr("Format"      ), this, SLOT(showColFormat      (bool))); m_actColFormat      ->setCheckable(true);
    m_actColDuration     = m_contextMenu->addAction(tr("Duration"    ), this, SLOT(showColDuration    (bool))); m_actColDuration    ->setCheckable(true);
}


CSongTableHeader::~CSongTableHeader()
{

}


void CSongTableHeader::contextMenuEvent(QContextMenuEvent * event)
{
    qDebug() << "Context menu...";

    QPoint point = event->pos();
    m_contextMenu->move(mapToGlobal(point));
    m_contextMenu->show();

    QHeaderView::contextMenuEvent(event);
}


void CSongTableHeader::showColTitle(bool show)
{
    emit columnShown(CSongTable::ColTitle, show);
}

void CSongTableHeader::showColArtist(bool show)
{
    emit columnShown(CSongTable::ColArtist, show);
}

void CSongTableHeader::showColAlbum(bool show)
{
    emit columnShown(CSongTable::ColAlbum, show);
}

void CSongTableHeader::showColAlbumArtist(bool show)
{
    emit columnShown(CSongTable::ColAlbumArtist, show);
}

void CSongTableHeader::showColComposer(bool show)
{
    emit columnShown(CSongTable::ColComposer, show);
}

void CSongTableHeader::showColYear(bool show)
{
    emit columnShown(CSongTable::ColYear, show);
}

void CSongTableHeader::showColTrackNumber(bool show)
{
    emit columnShown(CSongTable::ColTrackNumber, show);
}

void CSongTableHeader::showColDiscNumber(bool show)
{
    emit columnShown(CSongTable::ColDiscNumber, show);
}

void CSongTableHeader::showColGenre(bool show)
{
    emit columnShown(CSongTable::ColGenre, show);
}

void CSongTableHeader::showColRating(bool show)
{
    emit columnShown(CSongTable::ColRating, show);
}

void CSongTableHeader::showColComments(bool show)
{
    emit columnShown(CSongTable::ColComments, show);
}

void CSongTableHeader::showColPlayCount(bool show)
{
    emit columnShown(CSongTable::ColPlayCount, show);
}

void CSongTableHeader::showColLastPlayTime(bool show)
{
    emit columnShown(CSongTable::ColLastPlayTime, show);
}

void CSongTableHeader::showColFileName(bool show)
{
    emit columnShown(CSongTable::ColFileName, show);
}

void CSongTableHeader::showColBitRate(bool show)
{
    emit columnShown(CSongTable::ColBitRate, show);
}

void CSongTableHeader::showColFormat(bool show)
{
    emit columnShown(CSongTable::ColFormat, show);
}

void CSongTableHeader::showColDuration(bool show)
{
    emit columnShown(CSongTable::ColDuration, show);
}
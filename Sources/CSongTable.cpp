
#include "CSongTable.hpp"
#include "CSongTableModel.hpp"
#include "CSong.hpp"
#include "CApplication.hpp"
#include <QStringList>
#include <QMouseEvent>
#include <QHeaderView>
#include <QMenu>

#include <QtDebug>


CSongTable::CSongTable(CApplication * application) :
    QTableView    (application),
    m_model       (NULL),
    m_menu        (NULL),
    m_application (application)
{
    Q_CHECK_PTR(application);

    m_model = new CSongTableModel();
    setModel(m_model);

    // Menu contextuel
    m_menu = new QMenu(this);
    m_menu->addAction(tr("Informations"), m_application, SLOT(openDialogSongInfos()));
    m_menu->addAction(tr("Show in explorer"));
    m_menu->addAction(tr("Remove"));
    m_menu->addAction(tr("Playlists..."));
    m_menu->addAction(tr("Add to playlist..."));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openCustomMenuProject(const QPoint&)));

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    setSizePolicy(sizePolicy);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(true);
    setShowGrid(false);

    // Glisser-déposer
    setDragEnabled(true);

    // Modification des colonnes
    horizontalHeader()->setMovable(true);
    horizontalHeader()->hideSection(0);
    connect(horizontalHeader(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(columnMoved(int, int, int)));
    connect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(sectionResized(int, int, int)));

    verticalHeader()->hide();
    verticalHeader()->setDefaultSectionSize(19);

    m_columns.resize(ColNumber);
    m_columns[ 0].type = ColPosition;
    m_columns[ 1].type = ColTitle;
    m_columns[ 2].type = ColArtist;
    m_columns[ 3].type = ColAlbum;
    m_columns[ 4].type = ColAlbumArtist;
    m_columns[ 5].type = ColComposer;
    m_columns[ 6].type = ColYear;
    m_columns[ 7].type = ColTrackNumber;
    m_columns[ 8].type = ColDiscNumber;
    m_columns[ 9].type = ColGenre;
    m_columns[10].type = ColRating;
    m_columns[11].type = ColComments;
    m_columns[12].type = ColPlayCount;
    m_columns[13].type = ColLastPlayTime;
    m_columns[14].type = ColFileName;
    m_columns[15].type = ColBitRate;
    m_columns[16].type = ColFormat;
    m_columns[17].type = ColDuration;

    for (int i = 0; i < ColNumber; ++i)
    {
        m_columns[i].width = verticalHeader()->defaultSectionSize();
        m_columns[i].position = i;
    }
}


CSongTable::~CSongTable()
{

}


CSongTableModel::TSongItem * CSongTable::getSongItemForIndex(int pos) const
{
    return (pos < 0 ? NULL : m_model->getSongItem(pos));
}


int CSongTable::getPreviousSong(int pos, bool shuffle) const
{
    Q_ASSERT(pos == -1 || (pos >= 0 && pos < m_songs.size()));

    if (pos < 0)
    {
        if (shuffle)
        {
            //TODO...
            return -1;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if (shuffle)
        {
            //TODO...
            return -1;
        }
        else
        {
            return (pos > 0 ? pos - 1 : -1);
        }
    }
}


int CSongTable::getNextSong(int pos, bool shuffle) const
{
    Q_ASSERT(pos == -1 || (pos >= 0 && pos < m_songs.size()));

    if (pos < 0)
    {
        if (shuffle)
        {
            //TODO...
            return -1;
        }
        else
        {
            return (m_songs.size() > 0 ? 0 : -1);
        }
    }
    else
    {
        if (shuffle)
        {
            //TODO...
            return -1;
        }
        else
        {
            return (pos == m_songs.size() - 1 ? -1 : pos + 1);
        }
    }
}


int CSongTable::getTotalDuration(void) const
{
    int duration = 0;

    foreach (CSong * song, m_songs)
    {
        duration += song->getDuration();
    }

    return duration;
}


void CSongTable::deleteSongs(void)
{
    foreach (CSong * song, m_songs)
    {
        delete song;
    }

    m_songs.clear();
    m_model->clear();
}


void CSongTable::addSong(CSong * song, int pos)
{
    Q_CHECK_PTR(song);

    pos = qBound(-1, pos, m_songs.size() - 1);

    if (pos < 0)
    {
        m_songs.append(song);
    }
    else
    {
        m_songs.insert(pos, song);
    }

    m_model->insertRow(song, pos);
}


/**
 * Enlève une chanson de la liste.
 * Toutes les occurences de \a song sont enlevées de la liste.
 *
 * \todo Mettre à jour le modèle.
 *
 * \param song Pointeur sur la chanson à enlever.
 */

void CSongTable::removeSong(CSong * song)
{
    Q_CHECK_PTR(song);

    m_songs.removeAll(song);
}


/**
 * Enlève une chanson de la liste.
 *
 * \param pos Position de la chanson dans la liste (à partir de 0).
 */

void CSongTable::removeSong(int pos)
{
    Q_ASSERT(pos >= 0 && pos < m_songs.size());

    m_songs.removeAt(pos);
    m_model->removeRow(pos);
}

/*
void CSongTable::mousePressEvent(QMouseEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->button() == Qt::RightButton) 
    {
        QPoint pt = event->pos();
        QModelIndex index = this->indexAt(pt);

        if (index.isValid())
        {
            qDebug() << "Clic droit sur la table";
            selectRow(index.row());
        }
        else
        {
            qDebug() << "Clic droit en dehors de la table";
        }
    }
    else if (event->button() == Qt::LeftButton) 
    {
        QPoint pt = event->pos();
        QModelIndex index = this->indexAt(pt);

        if (index.isValid())
        {
            qDebug() << "Clic droit sur la table";
            selectRow(index.row());
            emit songSelected(index.row());
        }
        else
        {
            qDebug() << "Clic droit en dehors de la table";
        }
    }

    //TODO

    QTableView::mousePressEvent(event);
}


void CSongTable::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->button() == Qt::LeftButton) 
    {
        QPoint pt = event->pos();
        QModelIndex index = this->indexAt(pt);

        if (index.isValid())
        {
            qDebug() << "Double-clic sur la table";
            selectRow(index.row());
            emit songStarted(index.row());

        }
        else
        {
            qDebug() << "Double-clic en dehors de la table";
        }
    }

    QTableView::mouseDoubleClickEvent(event);
}
*/

void CSongTable::columnMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    qDebug() << "columnMoved("<<logicalIndex<<""<<oldVisualIndex<<""<<newVisualIndex<<")";

    if (oldVisualIndex < newVisualIndex)
    {
        for (int i = 0; i < ColNumber; ++i)
        {
            if (m_columns[i].position > oldVisualIndex && m_columns[i].position <= newVisualIndex)
            {
                --(m_columns[i].position);
            }
        }
    }
    else
    {
        for (int i = 0; i < ColNumber; ++i)
        {
            if (m_columns[i].position >= oldVisualIndex && m_columns[i].position < newVisualIndex)
            {
                ++(m_columns[i].position);
            }
        }
    }

    m_columns[logicalIndex].position = newVisualIndex;
    emit columnChanged();
}


void CSongTable::sectionResized(int logicalIndex, int oldSize, int newSize)
{
    qDebug() << "sectionResized("<<logicalIndex<<""<<oldSize<<""<<newSize<<")";
    m_columns[logicalIndex].width = newSize;
    emit columnChanged();
}


/// \todo Implémentation.
void CSongTable::openCustomMenuProject(const QPoint& point)
{
    qDebug() << "CSongTable::openCustomMenuProject()";

    QModelIndex index = indexAt(point);

    if (index.isValid())
    {
        m_menu->show();
        m_menu->move(mapToGlobal(point));
    }

    //...
    //this->myCustomMenu->show();
    //this->myCustomMenu->move(this->mapToGlobal(myQPoint));
}


#include "CSongTable.hpp"
#include "CSongTableModel.hpp"
#include "CSong.hpp"
#include <QStringList>
#include <QMouseEvent>
#include <QtDebug>


CSongTable::CSongTable(QWidget * parent) :
    QTableView (parent),
    m_model    (NULL)
{
    m_model = new CSongTableModel();
    setModel(m_model);

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
}


CSongTable::~CSongTable()
{

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


/// \todo Implémentation.
void CSongTable::openCustomMenuProject(const QPoint& myQPoint)
{
    //...
    //this->myCustomMenu->show();
    //this->myCustomMenu->move(this->mapToGlobal(myQPoint));
}

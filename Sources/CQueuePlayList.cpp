/*
Copyright (C) 2012-2015 Teddy Michel

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

#include "CQueuePlayList.hpp"
#include "CMainWindow.hpp"
#include "IPlayList.hpp"
#include "CStaticList.hpp"
#include "CDynamicList.hpp"
#include "CLibrary.hpp"
#include "Dialog/CDialogEditSong.hpp"
#include "Utils.hpp"

#include <QKeyEvent>
#include <QDragMoveEvent>
#include <QPainter>
#include <QMimeData>


CQueuePlayList::CQueuePlayList(CMainWindow * mainWindow) :
CMediaTableView (mainWindow)
{
    m_model->setCanDrop(true);

    // Glisser-déposer
    setDropIndicatorShown(false);
    setAcceptDrops(true);

    m_columnSort = ColPosition;
}


CQueuePlayList::~CQueuePlayList()
{

}


/**
 * Indique si la liste de morceaux a été modifiée.
 *
 * \return Booléen (toujours false).
 */

bool CQueuePlayList::isModified() const
{
    // La file d'attente n'est pas enregistrée en base de données
    return false;
}


/**
 * Ajoute plusieurs morceaux.
 *
 * \param songs    Liste de morceaux à ajouter.
 * \param position Position dans la liste.
 */

void CQueuePlayList::addSongs(const QList<CSong *>& songs, int position)
{
    QListIterator<CSong *> it(songs);

    if (position < 0)
    {
        while (it.hasNext())
        {
            CMediaTableView::addSongToTable(it.next(), position);
        }
    }
    else
    {
        it.toBack();

        while (it.hasPrevious())
        {
            CMediaTableView::addSongToTable(it.previous(), position);
        }
    }
}


/**
 * Met à jour la base de données avec les informations de la table.
 * La file d'attente n'est pas enregistrée en base de données, donc cette méthode ne fait rien.
 *
 * \return Booléen indiquant le succès de l'opération (toujours false).
 */

bool CQueuePlayList::updateDatabase()
{
    // La file d'attente n'est pas enregistrée en base de données
    return false;
}


/**
 * Enlève une liste de morceaux de la liste de lecture.
 *
 * \param songItemList Liste des morceaux à enlever.
 */

void CQueuePlayList::removeSongs(const QList<CMediaTableItem *>& songItemList)
{
    if (songItemList.isEmpty())
    {
        return;
    }

    if (songItemList.contains(m_mainWindow->getCurrentSongItem()))
    {
        m_mainWindow->stop();
    }

    QList<CSong *> songList;

    for (QList<CMediaTableItem *>::ConstIterator it = songItemList.begin(); it != songItemList.end(); ++it)
    {
        int row = m_model->getRowForSongItem(*it);

        if (row >= 0)
        {
            m_model->removeRow(row);

            if (!songList.contains((*it)->getSong()))
            {
                songList.append((*it)->getSong());
            }
        }
    }
}


/**
 * Retire les morceaux sélectionnés de la liste.
 * Affiche une confirmation.
 *
 * \todo Si la liste est en cours de lecture, il faut mettre à jour le pointeur sur le morceau en cours.
 */

void CQueuePlayList::removeSelectedSongs()
{
    // Liste des morceaux sélectionnés
    const QModelIndexList indexList = selectionModel()->selectedRows();

    // Aucun morceau à supprimer
    if (indexList.isEmpty())
        return;

    // Si on est en train de lire un morceau de la liste, il faut mettre à jour les informations sur le morceau courant
    CMediaTableItem * currentItem = m_model->getCurrentSongItem();
    CSong * currentSong = (currentItem ? currentItem->getSong() : nullptr);

    CDialogEditSong * dialogEditSong = m_mainWindow->getDialogEditSong();
    CSong * currentSongInDialogEditSong = nullptr;

    if (dialogEditSong)
    {
        if (dialogEditSong->getSongTable() == this)
            currentSongInDialogEditSong = dialogEditSong->getSongItem()->getSong();
        else
            dialogEditSong = nullptr;
    }

    QList<CMediaTableItem *> songItemList;

    // On parcourt la liste des morceaux sélectionnés
    for (QModelIndexList::ConstIterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        CMediaTableItem * songItem = m_model->getSongItem(*it);

        // Morceau en cours de lecture
        if (m_mainWindow->getCurrentSongItem() == songItem)
        {
            currentItem = nullptr;
            currentSong = nullptr;
        }

        songItemList.append(songItem);
    }

    removeSongs(songItemList);

    selectionModel()->clearSelection();

    // On change le morceau courant affiché dans la liste
    if (currentSong)
    {
        CMediaTableItem * currentItemAfter = getFirstSongItem(currentSong);
        m_model->setCurrentSong(currentItemAfter);
        //m_mainWindow->m_currentSongItem = currentItemAfter;
        m_mainWindow->setCurrentSongItem(currentItemAfter, this);
    }

    if (dialogEditSong)
    {
        CMediaTableItem * songItem = getFirstSongItem(currentSongInDialogEditSong);

        if (songItem)
            dialogEditSong->setSongItem(songItem, this);
        else
            dialogEditSong->close();
    }
}


/**
 * Gestion des touches du clavier.
 * Les touches Entrée et Supprimer sont gérées.
 *
 * \param event Évènement du clavier.
 */

void CQueuePlayList::keyPressEvent(QKeyEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->key() == Qt::Key_Delete)
    {
        event->accept();
        removeSelectedSongs();
        return;
    }

    return CMediaTableView::keyPressEvent(event);
}


/**
 * Gestion du glisser-déposer.
 * S'occupe d'afficher la ligne indiquant la nouvelle position des items.
 *
 * \param event Évènement de déplacement.
 */

void CQueuePlayList::dragMoveEvent(QDragMoveEvent * event)
{
    CMediaTableView::dragMoveEvent(event);

    if (event->source() != this)
    {
        return;
    }

    QModelIndex index = indexAt(event->pos());

    if (index.isValid())
    {
        QRect rect = visualRect(index);
        m_dropIndicatorRect = QRect(0, rect.top(), width(), 1);
    }
    else
    {
        QRect rect = visualRect(m_model->index(m_model->rowCount() - 1, 0));
        m_dropIndicatorRect = QRect(0, rect.bottom(), width(), 1);
    }
}


void CQueuePlayList::dropEvent(QDropEvent * event)
{
    event->ignore();

    CMediaTableView::dropEvent(event);

    if (event->isAccepted())
    {
        QModelIndex index = indexAt(event->pos());
        int row = -1;

        if (index.isValid())
        {
            row = index.row();
        }
        else
        {
            row = m_model->rowCount();
        }

        const QMimeData * mimeData = event->mimeData();

        if (!mimeData)
            return;

        QByteArray encodedData = mimeData->data("application/x-ted-media-items");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);

        int numSongs;
        stream >> numSongs;

        QList<int> rowList;
        int numRowsBeforeDest = 0;

        for (int i = 0; i < numSongs; ++i)
        {
            int songRow;
            stream >> songRow;
            rowList << songRow;

            if (songRow < row)
            {
                ++numRowsBeforeDest;
            }
        }

        m_model->moveRows(rowList, row);

        QList<CSong *> songs = getSongs();
        removeAllSongsFromTable();
        addSongs(songs, false);

        // Modification de la sélection
        QItemSelection selection;
        selection.select(m_model->index(row - numRowsBeforeDest, 0), m_model->index(row - numRowsBeforeDest + numSongs - 1, 0));
        selectionModel()->select(selection, QItemSelectionModel::Current | QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}


void CQueuePlayList::paintEvent(QPaintEvent * event)
{
    CMediaTableView::paintEvent(event);

    if (state() == QAbstractItemView::DraggingState
#ifndef QT_NO_CURSOR
        && viewport()->cursor().shape() != Qt::ForbiddenCursor
#endif
        )
    {
        QPainter painter(viewport());
        QStyleOption opt;
        opt.init(this);
        opt.rect = m_dropIndicatorRect;
        style()->drawPrimitive(QStyle::PE_IndicatorItemViewItemDrop, &opt, &painter, this);
    }
}

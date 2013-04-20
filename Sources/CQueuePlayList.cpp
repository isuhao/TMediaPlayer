/*
Copyright (C) 2012-2013 Teddy Michel

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
#include "CApplication.hpp"
#include "IPlayList.hpp"
#include "CStaticPlayList.hpp"
#include "CDynamicList.hpp"
#include "CLibrary.hpp"
#include "CDialogEditSong.hpp"
#include "Utils.hpp"

#include <QKeyEvent>
#include <QDragMoveEvent>
#include <QPainter>


CQueuePlayList::CQueuePlayList(CApplication * application) :
CSongTable   (application)
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


void CQueuePlayList::addSongs(const QList<CSong *>& songs, int position)
{
    QListIterator<CSong *> it(songs);
    it.toBack();

    while (it.hasPrevious())
    {
        CSongTable::addSongToTable(it.previous(), position);
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
 * Affiche le menu contextuel.
 *
 * \param point Position du clic.
 */

void CQueuePlayList::openCustomMenuProject(const QPoint& point)
{
    QModelIndex index = indexAt(point);

    if (index.isValid())
    {
        bool severalSongs = (selectionModel()->selectedRows().size() > 1);

        if (!severalSongs)
        {
            m_selectedItem = m_model->getSongItem(index);
        }

        // Menu contextuel
        QMenu * menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        
        if (!severalSongs)
        {
            QAction * actionPlay = menu->addAction(tr("Play"), this, SLOT(playSelectedSong()));
            menu->setDefaultAction(actionPlay);
            menu->addSeparator();
        }

        menu->addAction(tr("Informations..."), m_application, SLOT(openDialogSongInfos()));

        if (!severalSongs)
        {
            menu->addAction(tr("Edit metadata..."), m_application, SLOT(openDialogEditMetadata()));
            menu->addAction(tr("Show in explorer"), m_application, SLOT(openSongInExplorer()));

            if (m_selectedItem->getSong()->getFileStatus() == false)
            {
                menu->addAction(tr("Relocate"), m_application, SLOT(relocateSong()));
            }
        }

        menu->addSeparator();
        menu->addAction(tr("Remove from queue"), this, SLOT(removeSelectedSongs()));
        menu->addAction(tr("Remove from library"), this, SLOT(removeSongsFromLibrary()));

        //if (m_application->getSettings()->value("Folders/KeepOrganized", false).toBool())
        {
            if (severalSongs)
                menu->addAction(tr("Rename files"), this, SLOT(moveSongs()));
            else
                menu->addAction(tr("Rename file"), this, SLOT(moveSongs()));
        }

        if (!severalSongs)
        {
            QAction * actionCheck = menu->addAction(tr("Check song"), this, SLOT(checkSelection()));
            QAction * actionUncheck = menu->addAction(tr("Uncheck song"), this, SLOT(uncheckSelection()));

            bool songIsChecked = m_selectedItem->getSong()->isEnabled();

            if (songIsChecked)
                actionCheck->setEnabled(false);
            else
                actionUncheck->setEnabled(false);
        }
        else
        {
            menu->addAction(tr("Check selection"), this, SLOT(checkSelection()));
            menu->addAction(tr("Uncheck selection"), this, SLOT(uncheckSelection()));
        }

        menu->addSeparator();

        if (!severalSongs)
        {
            // Listes de lecture contenant le morceau
            //TODO: gérer les dossiers
            QMenu * menuPlayList = menu->addMenu(tr("Playlists"));
            CLibrary * library = m_application->getLibrary();
            m_actionGoToSongTable[library] = menuPlayList->addAction(QPixmap(":/icons/library"), tr("Library"));
            connect(m_actionGoToSongTable[library], SIGNAL(triggered()), this, SLOT(goToSongTable()));

            QList<IPlayList *> playLists = m_application->getPlayListsWithSong(m_selectedItem->getSong());

            if (playLists.size() > 0)
            {
                menuPlayList->addSeparator();

                for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
                {
                    m_actionGoToSongTable[*it] = menuPlayList->addAction((*it)->getName());
                    connect(m_actionGoToSongTable[*it], SIGNAL(triggered()), this, SLOT(goToSongTable()));

                    if (qobject_cast<CDynamicList *>(*it))
                    {
                        m_actionGoToSongTable[*it]->setIcon(QPixmap(":/icons/dynamic_list"));
                    }
                    else if (qobject_cast<CStaticPlayList *>(*it))
                    {
                        m_actionGoToSongTable[*it]->setIcon(QPixmap(":/icons/playlist"));
                    }
                }
            }
        }

        // Ajouter à la liste de lecture
        //TODO: gérer les dossiers
        QMenu * menuAddToPlayList = menu->addMenu(tr("Add to playlist"));
        QList<IPlayList *> playLists = m_application->getAllPlayLists();

        if (playLists.isEmpty())
        {
            QAction * actionNoPlayList = menuAddToPlayList->addAction(tr("There are no playlist"));
            actionNoPlayList->setEnabled(false);
        }
        else
        {
            for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
            {
                CStaticPlayList * staticList = qobject_cast<CStaticPlayList *>(*it);
                if (staticList)
                {
                    m_actionAddToPlayList[staticList] = menuAddToPlayList->addAction(QPixmap(":/icons/playlist"), (*it)->getName());
                    connect(m_actionAddToPlayList[staticList], SIGNAL(triggered()), this, SLOT(addToPlayList()));
                }
            }
        }

        menu->addSeparator();
        menu->addAction(tr("Remove duplicates"), this, SLOT(removeDuplicateSongs()));

        menu->move(getCorrectMenuPosition(menu, mapToGlobal(point)));
        menu->show();
    }
}


/**
 * Enlève une liste de morceaux de la liste de lecture.
 *
 * \param songItemList Liste des morceaux à enlever.
 */

void CQueuePlayList::removeSongs(const QList<CSongTableItem *>& songItemList)
{
    if (songItemList.isEmpty())
    {
        return;
    }

    if (songItemList.contains(m_application->getCurrentSongItem()))
    {
        m_application->stop();
    }

    QList<CSong *> songList;

    for (QList<CSongTableItem *>::const_iterator it = songItemList.begin(); it != songItemList.end(); ++it)
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
    CSongTableItem * currentItem = m_model->getCurrentSongItem();
    CSong * currentSong = (currentItem ? currentItem->getSong() : NULL);

    CDialogEditSong * dialogEditSong = m_application->getDialogEditSong();
    CSong * currentSongInDialogEditSong = NULL;

    if (dialogEditSong)
    {
        if (dialogEditSong->getSongTable() == this)
            currentSongInDialogEditSong = dialogEditSong->getSongItem()->getSong();
        else
            dialogEditSong = NULL;
    }

    QList<CSongTableItem *> songItemList;

    // On parcourt la liste des morceaux sélectionnés
    for (QModelIndexList::const_iterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        CSongTableItem * songItem = m_model->getSongItem(*it);

        // Morceau en cours de lecture
        if (m_application->getCurrentSongItem() == songItem)
        {
            currentItem = NULL;
            currentSong = NULL;
        }

        songItemList.append(songItem);
    }

    removeSongs(songItemList);

    selectionModel()->clearSelection();

    // On change le morceau courant affiché dans la liste
    if (currentSong)
    {
        CSongTableItem * currentItemAfter = getFirstSongItem(currentSong);
        m_model->setCurrentSong(currentItemAfter);
        //m_application->m_currentSongItem = currentItemAfter;
        m_application->setCurrentSongItem(currentItemAfter, this);
    }

    if (dialogEditSong)
    {
        CSongTableItem * songItem = getFirstSongItem(currentSongInDialogEditSong);

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

    return CSongTable::keyPressEvent(event);
}


/**
 * Gestion du glisser-déposer.
 * S'occupe d'afficher la ligne indiquant la nouvelle position des items.
 *
 * \param event Évènement de déplacement.
 */

void CQueuePlayList::dragMoveEvent(QDragMoveEvent * event)
{
    CSongTable::dragMoveEvent(event);

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

    CSongTable::dropEvent(event);

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

        // Modification de la sélection
        QItemSelection selection;
        selection.select(m_model->index(row - numRowsBeforeDest, 0), m_model->index(row - numRowsBeforeDest + numSongs - 1, 0));
        selectionModel()->select(selection, QItemSelectionModel::Current | QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}


void CQueuePlayList::paintEvent(QPaintEvent * event)
{
    CSongTable::paintEvent(event);

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

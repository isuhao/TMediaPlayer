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

#include "CStaticList.hpp"
#include "CMainWindow.hpp"
#include "CMediaManager.hpp"
#include "CMediaTableItem.hpp"
#include "CDynamicList.hpp"
#include "CFolder.hpp"
#include "CLibrary.hpp"
#include "Utils.hpp"
#include "Dialog/CDialogEditSong.hpp"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QKeyEvent>
#include <QPainter>
#include <QSettings>
#include <QMimeData>

#include <QtDebug>


/**
 * Construit une liste de lecture statique.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 * \param name       Nom de la liste de lecture.
 */

CStaticList::CStaticList(CMainWindow * mainWindow, const QString& name) :
IPlayList              (mainWindow, name),
m_id                   (-1),
m_isStaticListModified (false)
{
    m_model->setCanDrop(true);

    // Glisser-déposer
    setDropIndicatorShown(false);
    setAcceptDrops(true);
}


/**
 * Détruit la liste de lecture statique.
 */

CStaticList::~CStaticList()
{

}


/**
 * Indique si la liste de lecture a été modifiée et doit être mise à jour.
 *
 * \return Booléen.
 */

bool CStaticList::isModified() const
{
    return (m_isStaticListModified || IPlayList::isModified());
}


/**
 * Ajoute une chanson à la liste.
 *
 * \todo Implémentation
 *
 * \param song Pointeur sur la chanson à ajouter.
 * \param pos  Position où ajouter la chanson. Si négatif, la chanson est ajoutée à la fin de la liste.
 */

void CStaticList::addSong(CSong * song, int pos)
{
    if (!song)
    {
        m_mainWindow->getMediaManager()->logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    //...

    CMediaTableView::addSongToTable(song, pos);
    emit songAdded(song);
}


/**
 * Ajoute plusieurs morceaux à la liste de lecture.
 * Si certains morceaux sont déjà présents dans la liste, une confirmation est demandée.
 *
 * \param songs Liste des morceaux à ajouter.
 * \param confirm Indique si on doit demander une confirmation
 */

void CStaticList::addSongs(const QList<CSong *>& songs, bool confirm)
{
    if (m_id <= 0)
    {
        m_mainWindow->getMediaManager()->logError(tr("invalid identifier (%1)").arg(m_id), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    if (songs.isEmpty())
        return;

    bool skipDuplicate = false;

    if (confirm)
    {
        // Recherche des doublons
        bool hasDuplicate = false;

        for (QList<CSong *>::ConstIterator it = songs.begin(); it != songs.end(); ++it)
        {
            if (hasSong(*it))
            {
                hasDuplicate = true;
                break;
            }
        }

        if (hasDuplicate)
        {
            /// \todo Créer la boite de dialogue à la main pour gérer les boutons en français
            QMessageBox::StandardButton ret = QMessageBox::question(this, QString(), tr("There had duplicates being added to the playlist.\nWould you like to add them?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

            if (ret == QMessageBox::No)
            {
                skipDuplicate = true;
            }
            else if (ret == QMessageBox::Cancel)
            {
                return;
            }
        }
    }

    QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());

    // Position des morceaux dans la liste
    query.prepare("SELECT MAX(song_position) FROM static_list_song WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec() || !query.next())
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    int songPosition = query.value(0).toInt() + 1;

    // Préparation de la requête SQL
    QVariantList field1;
    QVariantList field2;
    QVariantList field3;

    int numSongsAdded = 0;

    for (QList<CSong *>::ConstIterator it = songs.begin(); it != songs.end(); ++it)
    {
        Q_CHECK_PTR(*it);

        if (hasSong(*it) && skipDuplicate)
        {
            continue;
        }

        field1 << m_id;
        field2 << (*it)->getId();
        field3 << songPosition + numSongsAdded;

        ++numSongsAdded;
    }

    if (numSongsAdded == 0)
    {
        return;
    }

    query.prepare("INSERT INTO static_list_song (static_list_id, song_id, song_position) VALUES (?, ?, ?)");
    query.addBindValue(field1);
    query.addBindValue(field2);
    query.addBindValue(field3);

    if (!query.execBatch())
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    // Mise à jour de la table
    int songNum = 0;
    m_automaticSort = false;

    for (QList<CSong *>::ConstIterator it = songs.begin(); it != songs.end(); ++it)
    {
        if (hasSong(*it) && skipDuplicate)
        {
            continue;
        }

        CMediaTableView::addSongToTable(*it, songPosition + songNum);
        emit songAdded(*it);
        ++songNum;
    }

    emit listModified();

    m_automaticSort = true;
    sortByColumn(m_columnSort, m_sortOrder);
}


/**
 * Enlève un morceau de la liste.
 * Toutes les occurences de \a song sont enlevées de la liste.
 *
 * \param song    Pointeur sur le morceau à enlever.
 * \param confirm Demande une confirmation avant d'enlever le morceau de la liste.
 */

void CStaticList::removeSong(CSong * song, bool confirm)
{
    Q_CHECK_PTR(song);

    QList<CMediaTableItem *> songItemList;

    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        CMediaTableItem * songItem = m_model->getSongItem(row);

        if (songItem->getSong() == song)
        {
            songItemList.append(songItem);
        }
    }

    removeSongs(songItemList, confirm);
}


/**
 * Enlève un morceau de la liste.
 *
 * \param songItem Pointeur sur le morceau à enlever.
 * \param confirm  Demande une confirmation avant d'enlever le morceau de la liste.
 */

void CStaticList::removeSong(CMediaTableItem * songItem, bool confirm)
{
    Q_CHECK_PTR(songItem);

    // Confirmation
    if (confirm)
    {
        if (QMessageBox::question(this, QString(), tr("Are you sure you want to remove the selected songs from the list?"), tr("Remove"), tr("Cancel"), 0, 1) == 1)
        {
            return;
        }
    }

    if (m_mainWindow->getCurrentSongItem() == songItem)
    {
        m_mainWindow->stop();
    }

    int row = m_model->getRowForSongItem(songItem);

    if (row >= 0)
    {
        m_model->removeRow(row);
        emit songRemoved(songItem->getSong());
    }
    else
    {
        return;
    }

    QList<CSong *> songs = getSongs();

    removeAllSongsFromTable();

    QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());
    query.prepare("DELETE FROM static_list_song WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    addSongs(songs, false);
}


/**
 * Enlève une liste de morceaux de la liste.
 *
 * \param songs   Liste des morceaux à enlever.
 * \param confirm Demande une confirmation avant d'enlever les morceaux de la liste.
 */

void CStaticList::removeSongs(const QList<CSong *>& songs, bool confirm)
{
    if (songs.isEmpty())
    {
        return;
    }

    QList<CMediaTableItem *> songItemList;

    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        CMediaTableItem * songItem = m_model->getSongItem(row);

        if (songs.contains(songItem->getSong()))
        {
            songItemList.append(songItem);
        }
    }

    removeSongs(songItemList, confirm);
}


/**
 * Enlève une liste de morceaux de la liste de lecture.
 *
 * \param songItemList Liste des morceaux à enlever.
 * \param confirm      Demande une confirmation avant d'enlever les morceaux de la liste.
 */

void CStaticList::removeSongs(const QList<CMediaTableItem *>& songItemList, bool confirm)
{
    if (songItemList.isEmpty())
    {
        return;
    }

    // Confirmation
    if (confirm)
    {
        if (QMessageBox::question(this, QString(), tr("Are you sure you want to remove the selected songs from the list?"), tr("Remove"), tr("Cancel"), 0, 1) == 1)
        {
            return;
        }
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

    // Émission des signaux
    for (QList<CSong *>::ConstIterator it = songList.begin(); it != songList.end(); ++it)
    {
        emit songRemoved(*it);
    }

    QList<CSong *> songs = getSongs();

    removeAllSongsFromTable();

    QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());
    query.prepare("DELETE FROM static_list_song WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    addSongs(songs, false);
}


/**
 * Retire les morceaux sélectionnés de la liste.
 * Affiche une confirmation.
 *
 * \todo Si la liste est en cours de lecture, il faut mettre à jour le pointeur sur le morceau en cours.
 */

void CStaticList::removeSelectedSongs()
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

    removeSongs(songItemList, true);

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
 * Enlève les doublons de la liste.
 *
 * \todo Tester complètement.
 */

void CStaticList::removeDuplicateSongs()
{
    QList<CSong *> songs = getSongs();
    QList<CSong *> songsNew;

    for (QList<CSong *>::ConstIterator it = songs.begin(); it != songs.end(); ++it)
    {
        if (songsNew.contains(*it))
        {
            emit songRemoved(*it);
        }
        else
        {
            songsNew.append(*it);
        }
    }

    // Aucun doublon
    if (songs.size() == songsNew.size())
    {
        return;
    }

    removeAllSongsFromTable();
    //addSongsToTable(songsNew);

    QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());
    query.prepare("DELETE FROM static_list_song WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    addSongs(songsNew, false);
}


/**
 * Met à jour la base de données avec les informations de la liste de lecture.
 * Si la liste n'existe pas en base de données, elle est ajoutée.
 */

bool CStaticList::updateDatabase()
{
    if (!getFolder())
    {
        m_mainWindow->getMediaManager()->logError(tr("the playlist is not in a folder"), __FUNCTION__, __FILE__, __LINE__);
    }

    // Insertion
    if (m_id <= 0)
    {
        int folderId = (getFolder() ? getFolder()->getId() : 0);

        QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());

        // Position dans le dossier
        int position = getFolder()->getPosition(this);
/*
        query.prepare("SELECT MAX(list_position) FROM playlist WHERE folder_id = ?");
        query.bindValue(0, folderId);

        if (!query.exec())
        {
            m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        if (query.next())
        {
            m_position = query.value(0).toInt() + 1;
        }
*/
        // Insertion
        query.prepare("INSERT INTO playlist (playlist_name, folder_id, list_position, list_columns) VALUES (?, ?, ?, ?)");

        query.bindValue(0, m_name);
        query.bindValue(1, folderId);
        query.bindValue(2, position);
        query.bindValue(3, "0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120"); // Disposition par défaut

        if (!query.exec())
        {
            m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        if (m_mainWindow->getMediaManager()->getDataBase().driverName() == "QPSQL")
        {
            query.prepare("SELECT currval('playlist_playlist_id_seq')");

            if (!query.exec())
            {
                m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return false;
            }

            if (query.next())
            {
                m_idPlayList = query.value(0).toInt();
            }
            else
            {
                m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return false;
            }
        }
        else
        {
            m_idPlayList = query.lastInsertId().toInt();
        }

        query.prepare("INSERT INTO static_list (playlist_id) VALUES (?)");
        query.bindValue(0, m_idPlayList);

        if (!query.exec())
        {
            m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        if (m_mainWindow->getMediaManager()->getDataBase().driverName() == "QPSQL")
        {
            query.prepare("SELECT currval('static_list_seq')");

            if (!query.exec())
            {
                m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return false;
            }

            if (query.next())
            {
                m_id = query.value(0).toInt();
            }
            else
            {
                m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return false;
            }
        }
        else
        {
            m_id = query.lastInsertId().toInt();
        }
    }
    // Mise à jour
    else if (m_isStaticListModified)
    {
/*
        QSqlQuery query(m_mainWindow->getDataBase());
        query.prepare("UPDATE playlist SET playlist_name = ? WHERE playlist_id = ?");

        query.bindValue(0, m_name);
        query.bindValue(1, m_idPlayList);

        if (!query.exec())
        {
            m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }
*/
    }

    m_isStaticListModified = false;
    return IPlayList::updateDatabase();
}


/**
 * Supprime la liste de la base de données.
 */

void CStaticList::removeFromDatabase()
{
    if (m_id <= 0)
    {
        m_mainWindow->getMediaManager()->logError(tr("invalid identifier (%1)").arg(m_id), __FUNCTION__, __FILE__, __LINE__);
    }
    else
    {
        QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());

        // Suppression des morceaux
        query.prepare("DELETE FROM static_list_song WHERE static_list_id = ?");
        query.bindValue(0, m_id);

        if (!query.exec())
        {
            m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return;
        }

        // Suppression de la liste statique
        query.prepare("DELETE FROM static_list WHERE static_list_id = ?");
        query.bindValue(0, m_id);

        if (!query.exec())
        {
            m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return;
        }

        m_isStaticListModified = false;
        m_id = -1;
    }

    IPlayList::removeFromDatabase();
}


/**
 * Gestion des touches du clavier.
 * Les touches Entrée et Supprimer sont gérées.
 *
 * \param event Évènement du clavier.
 */

void CStaticList::keyPressEvent(QKeyEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->key() == Qt::Key_Delete)
    {
        event->accept();
        removeSelectedSongs();
        return;
    }

    return IPlayList::keyPressEvent(event);
}


/**
 * Gestion du glisser-déposer.
 * S'occupe d'afficher la ligne indiquant la nouvelle position des items.
 *
 * \param event Évènement de déplacement.
 */

void CStaticList::dragMoveEvent(QDragMoveEvent * event)
{
    IPlayList::dragMoveEvent(event);

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


void CStaticList::dropEvent(QDropEvent * event)
{
    event->ignore();

    IPlayList::dropEvent(event);

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

        QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());
        query.prepare("DELETE FROM static_list_song WHERE static_list_id = ?");
        query.bindValue(0, m_id);

        if (!query.exec())
        {
            m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        addSongs(songs, false);

        // Modification de la sélection
        QItemSelection selection;
        selection.select(m_model->index(row - numRowsBeforeDest, 0), m_model->index(row - numRowsBeforeDest + numSongs - 1, 0));
        selectionModel()->select(selection, QItemSelectionModel::Current | QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}


void CStaticList::paintEvent(QPaintEvent * event)
{
    IPlayList::paintEvent(event);

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

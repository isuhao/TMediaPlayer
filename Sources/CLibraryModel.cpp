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

#include "CLibraryModel.hpp"
#include "CMainWindow.hpp"
#include "CMediaManager.hpp"
#include "CFolder.hpp"
#include "CStaticList.hpp"
#include "CDynamicList.hpp"
#include "CLibrary.hpp"
#include "CCDRomDrive.hpp"
#include "CQueuePlayList.hpp"

#include <QSqlQuery>
#include <QSqlError>
#include <QMimeData>

#include <QtDebug>


/**
 * Constructeur du modèle.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 */

CLibraryModel::CLibraryModel(CMainWindow * mainWindow) :
QStandardItemModel (mainWindow),
m_mainWindow       (mainWindow),
m_rootFolder       (nullptr)
{
    Q_CHECK_PTR(m_mainWindow);
}


/**
 * Détruit le modèle.
 * Enregistre les modifications sur les dossiers et les listes de lecture.
 */

CLibraryModel::~CLibraryModel()
{
    // Met-à-jour tous les dossiers
    for (QList<CFolder *>::ConstIterator it = m_folders.begin(); it != m_folders.end(); ++it)
    {
        (*it)->updateDatabase();
    }

    // Met-à-jour toutes les listes de lecture
    for (QList<IPlayList *>::ConstIterator it = m_playLists.begin(); it != m_playLists.end(); ++it)
    {
        (*it)->updateDatabase();
    }

    // Supprime toutes les listes de lecture
    for (QList<IPlayList *>::ConstIterator it = m_playLists.begin(); it != m_playLists.end(); ++it)
    {
        delete *it;
    }

    // Supprime tous les dossiers
    for (QList<CFolder *>::ConstIterator it = m_folders.begin(); it != m_folders.end(); ++it)
    {
        delete *it;
    }
}


/**
 * Charge le modèle depuis la base de données.
 */

void CLibraryModel::loadFromDatabase()
{
    clear();

    QList<CFolder *> folders;
    QList<IPlayList *> playLists;
    QMap<CFolder *, int> folderPositions;

    QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());

    // Création des dossiers
    if (!query.exec("SELECT folder_id, folder_name, folder_parent, folder_position, folder_expanded "
                    "FROM folder "
                    "ORDER BY folder_position"))
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        while (query.next())
        {
            CFolder * folder = new CFolder(m_mainWindow, query.value(1).toString());
            folder->m_id     = query.value(0).toInt();
            folder->m_folder = reinterpret_cast<CFolder *>(query.value(2).toInt());
            folder->m_open   = query.value(4).toBool();

            folderPositions[folder] = query.value(3).toInt();

            if (query.value(2).toInt() < 0)
            {
                m_mainWindow->getMediaManager()->logError(tr("le dossier parent a un identifiant invalide"), __FUNCTION__, __FILE__, __LINE__);
                folder->m_folder = 0;
            }

            folders.append(folder);

            if (folder->m_id == 0)
            {
                m_rootFolder = folder;
            }
        }
    }

    // On déplace les dossiers dans l'arborescence
    for (QList<CFolder *>::ConstIterator it = folders.begin(); it != folders.end(); ++it)
    {
        long folderId = reinterpret_cast<long>((*it)->m_folder);
        if (folderId >= 0)
        {
            if ((*it)->getId() != 0)
            {
                (*it)->m_folder = getFolderFromId(folderId, folders);

                if ((*it)->m_folder)
                {
                    //(*it)->m_folder->m_folders.append(*it);
                    (*it)->m_folder->addFolderItem(*it, folderPositions[*it]);
                }
                else
                {
                    m_mainWindow->getMediaManager()->logError(tr("invalid identifier (%1)").arg(folderId), __FUNCTION__, __FILE__, __LINE__);
                }
            }
        }
        else
        {
            m_mainWindow->getMediaManager()->logError(tr("le dossier contenant le dossier a un identifiant invalide"), __FUNCTION__, __FILE__, __LINE__);
        }
    }


    // Création des listes de lecture statiques
    if (!query.exec("SELECT static_list_id, playlist_name, list_columns, playlist_id, folder_id, list_position "
                    "FROM static_list NATURAL JOIN playlist ORDER BY list_position"))
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        while (query.next())
        {
            CStaticList * playList = new CStaticList(m_mainWindow, query.value(1).toString());
            playList->m_id = query.value(0).toInt();
            playList->m_idPlayList = query.value(3).toInt();
            playList->initColumns(query.value(2).toString());

            // Dossier contenant la liste
            int folderId = query.value(4).toInt();
            if (folderId >= 0)
            {
                playList->m_folder = getFolderFromId(folderId, folders);
                //playList->m_folder->m_playLists.append(playList);
                playList->m_folder->addPlayListItem(playList, query.value(5).toInt());
            }
            else
            {
                m_mainWindow->getMediaManager()->logError(tr("le dossier contenant la liste statique a un identifiant invalide"), __FUNCTION__, __FILE__, __LINE__);
            }

            // Liste des morceaux de la liste de lecture
            QSqlQuery query2(m_mainWindow->getMediaManager()->getDataBase());
            query2.prepare("SELECT song_id, song_position FROM static_list_song "
                           "WHERE static_list_id = ? ORDER BY song_position");
            query2.bindValue(0, playList->m_id);

            if (!query2.exec())
            {
                m_mainWindow->getMediaManager()->logDatabaseError(query2.lastError().text(), query2.lastQuery(), __FILE__, __LINE__);
                delete playList;
                continue;
            }

            QList<CSong *> songs;

            while (query2.next())
            {
                CSong * song = m_mainWindow->getSongFromId(query2.value(0).toInt());

                if (song)
                {
                    songs.append(song);
                    //playList->addSongToTable(song, query2.value(1).toInt());
                }
            }

            playList->addSongsToTable(songs);

            playLists.append(playList);
            playList->hide();

            connect(playList, SIGNAL(songStarted(CMediaTableItem *)), m_mainWindow, SLOT(playSong(CMediaTableItem *)));
            connect(playList, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(onPlayListRenamed(const QString&, const QString&)));
            connect(playList, SIGNAL(rowCountChanged()), m_mainWindow, SLOT(updateListInformations()));

            connect(playList, SIGNAL(songAdded(CSong *)), m_mainWindow, SLOT(updateListInformations()));
            connect(playList, SIGNAL(songRemoved(CSong *)), m_mainWindow, SLOT(updateListInformations()));
        }
    }


    // Création des listes de lecture dynamiques
    if (!query.exec("SELECT dynamic_list_id, playlist_name, list_columns, playlist_id, folder_id, list_position, auto_update, only_checked "
                    "FROM dynamic_list NATURAL JOIN playlist ORDER BY list_position"))
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        while (query.next())
        {
            CDynamicList * playList = new CDynamicList(m_mainWindow, query.value(1).toString());
            playList->m_id = query.value(0).toInt();
            playList->m_autoUpdate = query.value(6).toBool();
            playList->m_onlyChecked = query.value(7).toBool();
            playList->m_idPlayList = query.value(3).toInt();
            playList->initColumns(query.value(2).toString());

            // Dossier contenant la liste
            int folderId = query.value(4).toInt();
            if (folderId >= 0)
            {
                playList->m_folder = getFolderFromId(folderId, folders);
                //playList->m_folder->m_playLists.append(playList);
                playList->m_folder->addPlayListItem(playList, query.value(5).toInt());
            }
            else
            {
                m_mainWindow->getMediaManager()->logError(tr("le dossier contenant la liste statique a un identifiant invalide"), __FUNCTION__, __FILE__, __LINE__);
            }

            playList->loadFromDatabase();

            playLists.append(playList);
            playList->hide();

            connect(playList, SIGNAL(songStarted(CMediaTableItem *)), m_mainWindow, SLOT(playSong(CMediaTableItem *)));
            connect(playList, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(onPlayListRenamed(const QString&, const QString&)));
            connect(playList, SIGNAL(rowCountChanged()), m_mainWindow, SLOT(updateListInformations()));
            connect(playList, SIGNAL(listUpdated()), m_mainWindow, SLOT(updateListInformations()));
            connect(playList, SIGNAL(listUpdated()), this, SLOT(onPlayListChange()));

            //playList->tryUpdateList();
        }
    }

    // On corrige les éventuelles erreurs de position dans les dossiers
    for (QList<CFolder *>::ConstIterator it = folders.begin(); it != folders.end(); ++it)
    {
        (*it)->fixPositions();
    }

    addFolder(m_rootFolder);

    // Mise à jour des listes dynamiques
    for (QList<IPlayList *>::ConstIterator it = m_playLists.begin(); it != m_playLists.end(); ++it)
    {
        CDynamicList * playList = qobject_cast<CDynamicList *>(*it);

        if (playList)
        {
            playList->tryUpdateList();
        }
    }
}


/**
 * Efface les données du modèle.
 * Les éléments permanents sont recrées (médiathèque, file d'attente, lecteurs de CD-ROM).
 */

void CLibraryModel::clear()
{
    QStandardItemModel::clear();

    m_folderItems.clear();
    m_songTableItems.clear();
    m_folders.clear();
    m_playLists.clear();

    m_rootFolder = nullptr;

    // Ajout de la médiathèque au modèle
    QStandardItem * libraryItem = new QStandardItem(QPixmap(":/icons/library"), tr("Library"));
    libraryItem->setData(QVariant::fromValue(qobject_cast<CMediaTableView *>(m_mainWindow->getLibrary())), Qt::UserRole + 1);

    appendRow(libraryItem);
    m_songTableItems[libraryItem] = m_mainWindow->getLibrary();

    // Ajout de la file d'attente
    QStandardItem * queueItem = new QStandardItem(QPixmap(":/icons/queue"), tr("Queue"));
    queueItem->setData(QVariant::fromValue(qobject_cast<CMediaTableView *>(m_mainWindow->getQueue())), Qt::UserRole + 1);

    appendRow(queueItem);
    m_songTableItems[queueItem] = m_mainWindow->getQueue();

    // Ajout des lecteurs de CD-ROM
    QList<CCDRomDrive *> drives = m_mainWindow->getCDRomDrives();

    for (QList<CCDRomDrive *>::ConstIterator drive = drives.begin(); drive != drives.end(); ++drive)
    {
        QStandardItem * cdDriveItem = new QStandardItem(QPixmap(":/icons/cd"), (*drive)->getDriveName());
        cdDriveItem->setData(QVariant::fromValue(qobject_cast<CMediaTableView *>(*drive)), Qt::UserRole + 1);

        if (!(*drive)->hasCDInDrive())
        {
            cdDriveItem->setEnabled(false);
            cdDriveItem->setSelectable(false);
        }

        appendRow(cdDriveItem);
        m_songTableItems[cdDriveItem] = *drive;
        m_cdRomDrives[cdDriveItem] = *drive;
    }
}


/**
 * Met à jour les informations sur les lecteurs de CD-ROM.
 */

void CLibraryModel::updateCDRomDrives()
{
    QList<CCDRomDrive *> drives = m_mainWindow->getCDRomDrives();

    for (QList<CCDRomDrive *>::ConstIterator drive = drives.begin(); drive != drives.end(); ++drive)
    {
        QStandardItem * cdDriveItem = m_cdRomDrives.key(*drive);

        if (!cdDriveItem)
            continue;

        if ((*drive)->hasCDInDrive())
        {
            cdDriveItem->setEnabled(true);
            cdDriveItem->setSelectable(true);
        }
        else
        {
            cdDriveItem->setEnabled(false);
            cdDriveItem->setSelectable(false);
        }
    }
}


/**
 * Retourne le dossier correspondant à un identifiant.
 *
 * \todo Déplacer dans le classe CMediaManager.
 *
 * \param id Identifiant du dossier.
 * \return Pointeur sur le dossier, ou nullptr si \a id n'est pas valide.
 */

CFolder * CLibraryModel::getFolderFromId(int id) const
{
    return getFolderFromId(id, m_folders);
}


/**
 * Retourne la liste de lecture correspondant à un identifiant.
 *
 * \todo Déplacer dans le classe CMediaManager.
 *
 * \param id Identifiant de la liste.
 * \return Pointeur sur la liste de lecture, ou nullptr si \a id n'est pas valide.
 */

IPlayList * CLibraryModel::getPlayListFromId(int id) const
{
    return getPlayListFromId(id, m_playLists);
}


QModelIndex CLibraryModel::getModelIndex(CFolder * folder) const
{
    QStandardItem * item = m_folderItems.key(folder);

    if (item)
        return item->index();

    return QModelIndex();
}


QModelIndex CLibraryModel::getModelIndex(CMediaTableView * songTable) const
{
    QStandardItem * item = m_songTableItems.key(songTable);

    if (item)
        return item->index();

    return QModelIndex();
}


/**
 * Ouvre ou ferme un dossier.
 *
 * \param index Index du dossier à ouvrir ou fermer.
 * \param open  Indique si le dossier doit être ouvert ou fermé.
 */

void CLibraryModel::openFolder(const QModelIndex& index, bool open)
{
    CFolder * folder = this->data(index, Qt::UserRole + 2).value<CFolder *>();

    if (folder)
    {
        folder->setOpen(open);

        if (open)
            itemFromIndex(index)->setIcon(QPixmap(":/icons/folder_open"));
        else
            itemFromIndex(index)->setIcon(QPixmap(":/icons/folder_close"));
    }
}


/**
 * Ajoute un dossier au modèle.
 *
 * \param folder Dossier à ajouter.
 */

void CLibraryModel::addFolder(CFolder * folder)
{
    Q_CHECK_PTR(folder);

    if (m_folders.contains(folder))
        return;

    m_folders.append(folder);

    if (folder->m_id != 0)
    {
        connect(folder, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(onFolderRenamed(const QString&, const QString&)));
    }

    if (folder->m_folder)
    {
        QStandardItem * folderItem = new QStandardItem(QPixmap(":/icons/folder_close"), folder->getName());
        folderItem->setData(QVariant::fromValue(folder), Qt::UserRole + 2);
        m_folderItems[folderItem] = folder;

        QStandardItem * itemParent = m_folderItems.key(folder->m_folder);

        if (itemParent && folder->m_folder != m_rootFolder)
            itemParent->appendRow(folderItem);
        else
            appendRow(folderItem);

        if (folder->isOpen())
            folderItem->setIcon(QPixmap(":/icons/folder_open"));
        else
            folderItem->setIcon(QPixmap(":/icons/folder_close"));
    }

    // Éléments du dossier
    bool invalidPosition = false;
    QVector<CFolder::TFolderItem *> items = folder->getItems();

    for (QVector<CFolder::TFolderItem *>::ConstIterator it = items.begin(); it != items.end(); ++it)
    {
        if (*it)
        {
            if ((*it)->folder)
            {
                addFolder((*it)->folder);

                if (invalidPosition)
                    (*it)->folder->m_isModified = true;
            }
            else if ((*it)->playList)
            {
                addPlayList((*it)->playList);

                if (invalidPosition)
                    (*it)->playList->m_isPlayListModified = true;
            }
            else
            {
                m_mainWindow->getMediaManager()->logError(tr("l'élément n'est ni un dossier, ni une liste de lecture"), __FUNCTION__, __FILE__, __LINE__);
            }
        }
        else
        {
            m_mainWindow->getMediaManager()->logError(tr("position incorrecte dans un dossier"), __FUNCTION__, __FILE__, __LINE__);
            invalidPosition = true;
        }
    }
}


/**
 * Ajoute une liste de lecture au modèle.
 *
 * \param playList Liste de lecture à ajouter.
 */

void CLibraryModel::addPlayList(IPlayList * playList)
{
    Q_CHECK_PTR(playList);

    if (m_playLists.contains(playList))
        return;

    m_playLists.append(playList);

    QStandardItem * playListItem = new QStandardItem(playList->getName());
    playListItem->setData(QVariant::fromValue(qobject_cast<CMediaTableView *>(playList)), Qt::UserRole + 1);
    m_songTableItems[playListItem] = playList;

    CDynamicList * dynamicList = qobject_cast<CDynamicList *>(playList);

    if (dynamicList)
    {
        playListItem->setIcon(QPixmap(":/icons/dynamic_list"));

        connect(dynamicList, SIGNAL(listUpdated()), m_mainWindow, SLOT(updateListInformations()));
    }
    else
    {
        CStaticList * staticList = qobject_cast<CStaticList *>(playList);

        if (staticList)
        {
            playListItem->setIcon(QPixmap(":/icons/playlist"));

            connect(staticList, SIGNAL(songAdded(CSong *)), m_mainWindow, SLOT(updateListInformations()));
            connect(staticList, SIGNAL(songRemoved(CSong *)), m_mainWindow, SLOT(updateListInformations()));
        }
    }

    connect(playList, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(onPlayListRenamed(const QString&, const QString&)));
    connect(playList, SIGNAL(songStarted(CMediaTableItem *)), m_mainWindow, SLOT(playSong(CMediaTableItem *)));
    connect(playList, SIGNAL(rowCountChanged()), m_mainWindow, SLOT(updateListInformations()));

    QStandardItem * itemParent = m_folderItems.key(playList->m_folder);

    if (itemParent && playList->m_folder != m_rootFolder)
        itemParent->appendRow(playListItem);
    else
        appendRow(playListItem);
}


/**
 * Supprime un dossier.
 *
 * \param folder    Pointeur sur le dossier à supprimer.
 * \param recursive Indique si on doit supprimer le contenu du dossier ou pas.
 */

void CLibraryModel::removeFolder(CFolder * folder, bool recursive)
{
    Q_CHECK_PTR(folder);

    QStandardItem * item = m_folderItems.key(folder);

    CFolder * parentFolder = folder->getFolder();
    if (!parentFolder) parentFolder = m_rootFolder;
    parentFolder->removeFolder(folder);

    // Déplacement des éléments du dossier vers le dossier parent
    if (!recursive)
    {
        QStandardItem * itemParent = m_folderItems.key(parentFolder);

        QList<CFolder *> folders = folder->getFolders();

        for (QList<CFolder *>::ConstIterator it = folders.begin(); it != folders.end(); ++it)
        {
            parentFolder->addFolder(*it);
            QStandardItem * itemFolder = m_folderItems.key(*it);

            if (!itemFolder)
            {
                m_mainWindow->getMediaManager()->logError(tr("invalid folder"), __FUNCTION__, __FILE__, __LINE__);
                continue;
            }

            item->takeRow(itemFolder->row());

            if (itemParent && (*it)->m_folder != m_rootFolder)
                itemParent->appendRow(itemFolder);
            else
                appendRow(itemFolder);
        }

        QList<IPlayList *> playLists = folder->getPlayLists();

        for (QList<IPlayList *>::ConstIterator it = playLists.begin(); it != playLists.end(); ++it)
        {
            parentFolder->addPlayList(*it);
            QStandardItem * itemPlayList = m_songTableItems.key(*it);

            if (!itemPlayList)
            {
                m_mainWindow->getMediaManager()->logError(tr("invalid playlist"), __FUNCTION__, __FILE__, __LINE__);
                continue;
            }

            item->takeRow(itemPlayList->row());

            if (itemParent && (*it)->m_folder != m_rootFolder)
                itemParent->appendRow(itemPlayList);
            else
                appendRow(itemPlayList);
        }
    }

    if (item)
    {
        m_folderItems.remove(item);
        QModelIndex parent = (item->parent() ? item->parent()->index() : QModelIndex());
        removeRow(item->row(), parent);
    }

    folder->removeFromDatabase(recursive);
    m_folders.removeOne(folder);
    delete folder;
}


/**
 * Supprime une liste de lecture.
 *
 * \param playList Pointeur sur la liste de lecture à supprimer.
 */

void CLibraryModel::removePlayList(IPlayList * playList)
{
    Q_CHECK_PTR(playList);

    QStandardItem * item = m_songTableItems.key(playList);

    CFolder * parentFolder = playList->getFolder();
    if (!parentFolder) parentFolder = m_rootFolder;
    parentFolder->removePlayList(playList);

    if (item)
    {
        m_songTableItems.remove(item);
        QModelIndex parent = (item->parent() ? item->parent()->index() : QModelIndex());
        removeRow(item->row(), parent);
    }

    playList->removeFromDatabase();
    m_playLists.removeOne(playList);
    delete playList;
}


bool CLibraryModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    Q_UNUSED(column);
    Q_CHECK_PTR(data);

    if (action == Qt::IgnoreAction)
        return true;

    if (data->hasFormat("application/x-ted-media-list"))
    {
        int playListId, folderId;
        QByteArray encodedData = data->data("application/x-ted-media-list");

        if (!decodeDataList(encodedData, &playListId, &folderId))
        {
            return false;
        }

        CFolder * folderParent = m_rootFolder;
        int position = row;

        if (parent.isValid())
        {
            folderParent = this->data(parent, Qt::UserRole + 2).value<CFolder *>();

            if (!folderParent)
            {
                m_mainWindow->getMediaManager()->logError(tr("invalid folter"), __FUNCTION__, __FILE__, __LINE__);
                return false;
            }
        }

        if (playListId > 0)
        {
            IPlayList * playList = m_mainWindow->getPlayListFromId(playListId);

            if (!playList)
            {
                m_mainWindow->getMediaManager()->logError(tr("invalid playlist"), __FUNCTION__, __FILE__, __LINE__);
                return false;
            }

            emit layoutAboutToBeChanged();

            if (parent.isValid())
                folderParent->addPlayList(playList, position);
            else
                folderParent->addPlayList(playList, position - 1);

            // Modification du modèle
            QStandardItem * item = m_songTableItems.key(playList);
            Q_CHECK_PTR(item);

            if (item->parent())
                item->parent()->takeRow(item->row());
            else
                invisibleRootItem()->takeRow(item->row());

            QStandardItem * itemParent = m_folderItems.key(folderParent);
            if (!itemParent)
                itemParent = invisibleRootItem();

            if (position < 0)
                itemParent->appendRow(item);
            else
                itemParent->insertRow(position, item);

            emit layoutChanged();

            return true;
        }
        else if (folderId > 0)
        {
            CFolder * folder = m_mainWindow->getFolderFromId(folderId);

            if (!folder)
            {
                m_mainWindow->getMediaManager()->logError(tr("invalid folder"), __FUNCTION__, __FILE__, __LINE__);
                return false;
            }

            emit layoutAboutToBeChanged();

            if (parent.isValid())
                folderParent->addFolder(folder, position);
            else
                folderParent->addFolder(folder, position - 1);

            // Modification du modèle
            QStandardItem * item = m_folderItems.key(folder);
            Q_CHECK_PTR(item);

            if (item->parent())
                item->parent()->takeRow(item->row());
            else
                invisibleRootItem()->takeRow(item->row());

            QStandardItem * itemParent = m_folderItems.key(folderParent);
            if (!itemParent)
                itemParent = invisibleRootItem();

            if (position < 0)
                itemParent->appendRow(item);
            else
                itemParent->insertRow(position, item);

            emit layoutChanged();

            return true;
        }

        m_mainWindow->getMediaManager()->logError(tr("l'élément n'est ni un dossier, ni une liste de lecture"), __FUNCTION__, __FILE__, __LINE__);
        return false;
    }
    else if (data->hasFormat("application/x-ted-media-songs"))
    {
        if (parent.isValid())
        {
            CMediaTableView * songTable = this->data(parent, Qt::UserRole + 1).value<CMediaTableView *>();

            if (songTable)
            {
                CStaticList * playList = qobject_cast<CStaticList *>(songTable);
                CQueuePlayList * queue = qobject_cast<CQueuePlayList *>(songTable);

                if (playList)
                {
                    QByteArray encodedData = data->data("application/x-ted-media-songs");
                    QList<CSong *> songList = decodeDataSongs(encodedData);

                    playList->addSongs(songList);
                    return true;
                }
                else if (queue)
                {
                    QByteArray encodedData = data->data("application/x-ted-media-songs");
                    QList<CSong *> songList = decodeDataSongs(encodedData);

                    queue->addSongs(songList);
                    return true;
                }
            }

            return false;
        }

        // Création d'une liste statique
        QByteArray encodedData = data->data("application/x-ted-media-songs");
        QList<CSong *> songs = decodeDataSongs(encodedData);

        m_mainWindow->openDialogCreateStaticList(nullptr, songs);

        return true;
    }

    return false;
}


QList<CSong *> CLibraryModel::decodeDataSongs(const QByteArray& encodedData) const
{
    QDataStream stream(encodedData);

    int numSongs;
    stream >> numSongs;

    QList<CSong *> songList;

    for (int i = 0; i < numSongs; ++i)
    {
        int songId;
        stream >> songId;
        songList << m_mainWindow->getSongFromId(songId);
    }

    return songList;
}


bool CLibraryModel::decodeDataList(const QByteArray& encodedData, int * playList, int * folder)
{
    Q_CHECK_PTR(playList);
    Q_CHECK_PTR(folder);

    *playList = 0;
    *folder = 0;

    QDataStream stream(encodedData);

    if (encodedData.size() != 2 * sizeof(int))
    {
        return false;
    }

    int type, id;
    stream >> type >> id;

    if (type == 0)
    {
        *playList = id;
        return true;
    }
    else if (type == 1)
    {
        *folder = id;
        return true;
    }

    return false;
}


/**
 * Retourne la liste des types MIME acceptés pour le drop.
 *
 * \return Liste de types.
 */

QStringList CLibraryModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-ted-media-songs"; // Liste de morceaux
    types << "application/x-ted-media-list";  // Liste de lecture ou dossier
    return types;
}


QMimeData * CLibraryModel::mimeData(const QModelIndexList& indexes) const
{
#ifdef ENABLE_DRAG_DROP_FOR_LISTS
    if (indexes.size() != 1)
        return nullptr;

    QByteArray data;
    QDataStream streamData(&data, QIODevice::WriteOnly);

    // Médiathèque
    if (indexes[0].row() == 0 && !indexes[0].parent().isValid())
        return nullptr;

    QStandardItem * item = itemFromIndex(indexes[0]);

    if (!item)
        return nullptr;

    if (item->data(Qt::UserRole + 2).value<CFolder *>())
    {
        streamData << 1;
        streamData << item->data(Qt::UserRole + 2).value<CFolder *>()->getId();
    }
    else
    {
        streamData << 0;
        streamData << item->data(Qt::UserRole + 1).value<CMediaTableView *>()->getIdPlayList();
    }

    QMimeData * mimeData = new QMimeData();
    mimeData->setData("application/x-ted-media-list", data);
    return mimeData;
#else
    Q_UNUSED(indexes)
    return nullptr;
#endif
}


/**
 * Retourne la liste des actions acceptées pour le drop.
 *
 * \return Actions acceptées.
 */

Qt::DropActions CLibraryModel::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::MoveAction);
}


CFolder * CLibraryModel::getFolderFromId(int id, const QList<CFolder *> folders) const
{
    if (id < 0)
        return nullptr;

    for (QList<CFolder *>::ConstIterator it = folders.begin(); it != folders.end(); ++it)
    {
        if ((*it)->getId() == id)
            return (*it);
    }

    return nullptr;
}


IPlayList * CLibraryModel::getPlayListFromId(int id, const QList<IPlayList *> playLists) const
{
    if (id <= 0)
        return nullptr;

    for (QList<IPlayList *>::ConstIterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        if ((*it)->getIdPlayList() == id)
            return (*it);
    }

    return nullptr;
}


void CLibraryModel::onPlayListRenamed(const QString& oldName, const QString& newName)
{
    Q_UNUSED(oldName);

    IPlayList * playList = qobject_cast<IPlayList *>(sender());

    if (playList)
    {
        QStandardItem * item = m_songTableItems.key(playList);

        if (item)
            item->setText(newName);
    }
}


void CLibraryModel::onFolderRenamed(const QString& oldName, const QString& newName)
{
    Q_UNUSED(oldName);

    CFolder * folder = qobject_cast<CFolder *>(sender());

    if (folder)
    {
        QStandardItem * item = m_folderItems.key(folder);

        if (item)
            item->setText(newName);
    }
}


void CLibraryModel::onPlayListChange()
{
    IPlayList * playList = qobject_cast<IPlayList *>(sender());

    if (playList)
        m_mainWindow->onPlayListChange(playList);
}

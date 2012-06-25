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

#include "CListModel.hpp"
#include "CApplication.hpp"
#include "CFolder.hpp"
#include "CStaticPlayList.hpp"
#include "CDynamicList.hpp"
#include "CLibrary.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMimeData>

#include <QtDebug>


/**
 * Constructeur du modèle.
 *
 * \param application Pointeur sur l'application.
 */

CListModel::CListModel(CApplication * application) :
    QStandardItemModel (application),
    m_application      (application),
    m_rootFolder       (NULL)
{
    Q_CHECK_PTR(application);
}


/**
 * Détruit le modèle.
 * Enregistre les modifications sur les dossiers et les listes de lecture.
 */

CListModel::~CListModel()
{
    // Met-à-jour et supprime tous les dossiers
    for (QList<CFolder *>::const_iterator it = m_folders.begin(); it != m_folders.end(); ++it)
    {
        (*it)->updateDatabase();
        delete *it;
    }

    // Met-à-jour et supprime toutes les listes de lecture
    for (QList<IPlayList *>::const_iterator it = m_playLists.begin(); it != m_playLists.end(); ++it)
    {
        (*it)->updateDatabase();
        delete *it;
    }
}


/**
 * Charge le modèle depuis la base de données.
 */

void CListModel::loadFromDatabase(void)
{
    clear();

    QList<CFolder *> folders;
    QList<IPlayList *> playLists;

    QSqlQuery query(m_application->getDataBase());

    // Création des dossiers
    if (!query.exec("SELECT folder_id, folder_name, folder_parent, folder_position, folder_expanded "
                    "FROM folder /*WHERE folder_id != 0*/ "
                    "ORDER BY folder_position"))
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        while (query.next())
        {
            CFolder * folder = new CFolder(m_application, query.value(1).toString());
            folder->m_id       = query.value(0).toInt();
            folder->m_folder   = reinterpret_cast<CFolder *>(query.value(2).toInt());
            folder->m_position = query.value(3).toInt();
            folder->m_open     = query.value(4).toBool();

            if (folder->m_folder < 0)
            {
                qWarning() << "CApplication::loadDatabase() : le dossier parent du dossier a un identifiant invalide";
            }

            folders.append(folder);

            if (folder->m_id == 0)
            {
                m_rootFolder = folder;
            }
            else
            {
                connect(folder, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(onFolderRenamed(const QString&, const QString&)));
            }
        }
    }

    // On déplace les dossiers dans l'arborescence
    for (QList<CFolder *>::const_iterator it = folders.begin(); it != folders.end(); ++it)
    {
        long folderId = reinterpret_cast<long>((*it)->m_folder);
        if (folderId >= 0)
        {
            if ((*it)->getId() != 0)
            {
                (*it)->m_folder = getFolderFromId(folderId, folders);

                if ((*it)->m_folder)
                {
                    (*it)->m_folder->m_folders.append(*it);
                }
                else
                {
                    qWarning() << "CListFolder::loadFromDatabase() : identifiant invalide";
                }
            }
        }
        else
        {
            qWarning() << "CListFolder::loadFromDatabase() : le dossier contenant le dossier a un identifiant invalide";
        }
    }


    // Création des listes de lecture statiques
    if (!query.exec("SELECT static_list_id, playlist_name, list_columns, playlist_id, folder_id "
                    "FROM static_list NATURAL JOIN playlist ORDER BY list_position"))
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    while (query.next())
    {
        CStaticPlayList * playList = new CStaticPlayList(m_application, query.value(1).toString());
        playList->m_id = query.value(0).toInt();
        playList->m_idPlayList = query.value(3).toInt();
        playList->initColumns(query.value(2).toString());

        // Dossier contenant la liste
        int folderId = query.value(4).toInt();
        if (folderId >= 0)
        {
            playList->m_folder = getFolderFromId(folderId, folders);
            playList->m_folder->m_playLists.append(playList);
        }
        else
        {
            qWarning() << "CListModel::loadFromDatabase() : le dossier contenant la liste statique a un identifiant invalide";
        }

        // Liste des morceaux de la liste de lecture
        QSqlQuery query2(m_application->getDataBase());
        query2.prepare("SELECT song_id, song_position FROM static_list_song "
                       "WHERE static_list_id = ? ORDER BY song_position");
        query2.bindValue(0, playList->m_id);

        if (!query2.exec())
        {
            m_application->showDatabaseError(query2.lastError().text(), query2.lastQuery(), __FILE__, __LINE__);
            delete playList;
            continue;
        }

        while (query2.next())
        {
            CSong * song = m_application->getSongFromId(query2.value(0).toInt());

            if (song)
            {
                playList->addSongToTable(song, query2.value(1).toInt());
            }
        }

        playLists.append(playList);
        playList->hide();

        connect(playList, SIGNAL(songStarted(CSongTableItem *)), m_application, SLOT(playSong(CSongTableItem *)));
        connect(playList, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(onPlayListRenamed(const QString&, const QString&)));
        connect(playList, SIGNAL(rowCountChanged()), m_application, SLOT(updateListInformations()));

        connect(playList, SIGNAL(songAdded(CSong *)), m_application, SLOT(updateListInformations()));
        connect(playList, SIGNAL(songRemoved(CSong *)), m_application, SLOT(updateListInformations()));
    }


    // Création des listes de lecture dynamiques
    if (!query.exec("SELECT dynamic_list_id, playlist_name, list_columns, playlist_id, folder_id, list_position "
                    "FROM dynamic_list NATURAL JOIN playlist ORDER BY list_position"))
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    while (query.next())
    {
        CDynamicList * playList = new CDynamicList(m_application, query.value(1).toString());
        playList->m_id = query.value(0).toInt();
        playList->m_idPlayList = query.value(3).toInt();
        playList->initColumns(query.value(2).toString());

        // Dossier contenant la liste
        int folderId = query.value(4).toInt();
        if (folderId >= 0)
        {
            playList->m_folder = getFolderFromId(folderId, folders);
            playList->m_folder->m_playLists.append(playList);
        }
        else
        {
            qWarning() << "CApplication::loadDatabase() : le dossier contenant la liste statique a un identifiant invalide";
        }

        playList->loadFromDatabase();

        playLists.append(playList);
        playList->hide();

        connect(playList, SIGNAL(songStarted(CSongTableItem *)), this, SLOT(playSong(CSongTableItem *)));
        connect(playList, SIGNAL(nameChanged(const QString&, const QString&)), this, SLOT(onPlayListRenamed(const QString&, const QString&)));
        connect(playList, SIGNAL(rowCountChanged()), this, SLOT(updateListInformations()));
        connect(playList, SIGNAL(listUpdated()), this, SLOT(updateListInformations()));

        playList->update();
    }


    addFolder(m_rootFolder);
/*
    // Remplissage du modèle
    for (QList<CFolder *>::const_iterator it = folders.begin(); it != folders.end(); ++it)
    {
        if (!(*it)->m_folder)
        {
            addFolder(*it);
        }
    }

    for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        if (!(*it)->m_folder)
        {
            addPlayList(*it);
        }
    }
*/
}


void CListModel::clear(void)
{
    QStandardItemModel::clear();

    m_folderItems.clear();
    m_songTableItems.clear();
    m_folders.clear();
    m_playLists.clear();

    m_rootFolder = NULL;

    // Ajout de la médiathèque au modèle
    QStandardItem * libraryItem = new QStandardItem(QPixmap(":/icons/library"), tr("Library"));
    libraryItem->setData(QVariant::fromValue(qobject_cast<CSongTable *>(m_application->getLibrary())), Qt::UserRole + 1);

    appendRow(libraryItem);

    m_songTableItems[libraryItem] = m_application->getLibrary();
}


/**
 * Retourne le dossier correspondant à un identifiant.
 *
 * \param id Identifiant du dossier.
 * \return Pointeur sur le dossier, ou NULL si \a id n'est pas valide.
 */

CFolder * CListModel::getFolderFromId(int id) const
{
    return getFolderFromId(id, m_folders);
}


/**
 * Retourne la liste de lecture correspondant à un identifiant.
 *
 * \param id Identifiant de la liste.
 * \return Pointeur sur la liste de lecture, ou NULL si \a id n'est pas valide.
 */

IPlayList * CListModel::getPlayListFromId(int id) const
{
    return getPlayListFromId(id, m_playLists);
}


QModelIndex CListModel::getModelIndex(CFolder * folder) const
{
    QStandardItem * item = m_folderItems.key(folder);
    
    if (item)
        return item->index();

    return QModelIndex();
}


QModelIndex CListModel::getModelIndex(CSongTable * songTable) const
{
    QStandardItem * item = m_songTableItems.key(songTable);
    
    if (item)
        return item->index();

    return QModelIndex();
}


/**
 * Ajoute un dossier au modèle.
 *
 * \param folder Dossier à ajouter.
 */

void CListModel::addFolder(CFolder * folder)
{
    Q_CHECK_PTR(folder);

    if (m_folders.contains(folder))
        return;

    m_folders.append(folder);

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

        folder->m_index = folderItem->index();

        if (folder->isOpen())
        {
            //expand(index);
            folderItem->setIcon(QPixmap(":/icons/folder_open"));
        }
        else
        {
            //collapse(index);
            folderItem->setIcon(QPixmap(":/icons/folder_close"));
        }
    }

    // Dossiers enfants
    QList<CFolder *> folders = folder->getFolders();
    for (QList<CFolder *>::const_iterator it = folders.begin(); it != folders.end(); ++it)
    {
        addFolder(*it);
    }

    // Listes enfants
    QList<IPlayList *> playLists = folder->getPlayLists();
    for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        addPlayList(*it);
    }
}


/**
 * Ajoute une liste de lecture au modèle.
 *
 * \param playList Liste de lecture à ajouter.
 */

void CListModel::addPlayList(IPlayList * playList)
{
    Q_CHECK_PTR(playList);

    if (m_playLists.contains(playList))
        return;

    m_playLists.append(playList);

    QStandardItem * playListItem = new QStandardItem(playList->getName());
    playListItem->setData(QVariant::fromValue(qobject_cast<CSongTable *>(playList)), Qt::UserRole + 1);
    m_songTableItems[playListItem] = playList;

    if (qobject_cast<CDynamicList *>(playList))
        playListItem->setIcon(QPixmap(":/icons/dynamic_list"));
    else if (qobject_cast<CStaticPlayList *>(playList))
        playListItem->setIcon(QPixmap(":/icons/playlist"));

    QStandardItem * itemParent = m_folderItems.key(playList->m_folder);

    if (itemParent && playList->m_folder != m_rootFolder)
        itemParent->appendRow(playListItem);
    else
        appendRow(playListItem);

    playList->m_index = playListItem->index();
}


/// \todo Implémentation.
void CListModel::removeFolder(CFolder * folder, bool recursive)
{
    //
}


void CListModel::removePlayList(IPlayList * playList)
{
    QStandardItem * item = m_songTableItems.key(playList);

    if (item)
    {
        m_songTableItems.remove(item);
        delete item;
    }

    playList->romoveFromDatabase();
    m_playLists.removeOne(playList);
    delete playList;
}


bool CListModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    Q_CHECK_PTR(data);

    if (action == Qt::IgnoreAction)
    {
        return true;
    }
    
    if (data->hasFormat("application/x-ted-media-list"))
    {
        qDebug() << "CListModel::dropMimeData() : drop liste";

        int playListId, folderId;
        QByteArray encodedData = data->data("application/x-ted-media-list");

        if (!decodeDataList(encodedData, &playListId, &folderId))
        {
            qDebug() << "CListModel::dropMimeData() : erreur lors du décodage";
            return false;
        }

        CFolder * folderParent = NULL;
        //CFolder * folderParentOld = NULL;
        int position = row;

        if (parent.isValid())
        {
            folderParent = this->data(parent, Qt::UserRole + 2).value<CFolder *>();

            if (!folderParent)
            {
                qDebug() << "CListModel::dropMimeData() : folderParent invalide";
                return false;
            }
        }

        if (playListId > 0)
        {
            IPlayList * playList = m_application->getPlayListFromId(playListId);

            if (!playList)
            {
                qDebug() << "CListModel::dropMimeData() : playList invalide";
                return false;
            }

            //folderParentOld = playList->getFolder();
            
            emit layoutAboutToBeChanged();

            if (folderParent)
            {
                folderParent->addPlayList(playList, position);
            }
            else
            {
                playList->setParent(NULL);
            }

            emit layoutChanged();

            return true;
        }
        else if (folderId > 0)
        {
            CFolder * folder = m_application->getFolderFromId(folderId);

            //itemFromIndex(index(?, 0, ?));

            if (!folder)
            {
                qDebug() << "CListModel::dropMimeData() : folder invalide";
                return false;
            }

            //folderParentOld = folder->getFolder();
            
            emit layoutAboutToBeChanged();

            if (folderParent)
            {
                folderParent->addFolder(folder, position);
            }
            else
            {
                folder->setParent(NULL);
            }

            emit layoutChanged();

            return true;
        }

        qWarning() << "CPlayListModel::dropMimeData() : ni playlist ni folder";
        return false;
    }
    else if (data->hasFormat("application/x-ted-media-songs"))
    {
        if (parent.isValid())
        {
            //qDebug() << "CPlayListModel::dropMimeData() : ajout à une liste...";

            CSongTable * songTable = this->data(parent, Qt::UserRole + 1).value<CSongTable *>();
            CStaticPlayList * playList = qobject_cast<CStaticPlayList *>(songTable);

            if (songTable && playList)
            {
                //qDebug() << "playlist " << playList->getName();

                QByteArray encodedData = data->data("application/x-ted-media-songs");
                QList<CSong *> songList = decodeDataSongs(encodedData);

                playList->addSongs(songList);
                return true;
            }

            //qDebug() << "library ou dynamic list";
            return false;
        }

        // Création d'une liste statique
        qDebug() << "CPlayListModel::dropMimeData() : création d'une nouvelle liste...";

        QByteArray encodedData = data->data("application/x-ted-media-songs");
        QList<CSong *> songs = decodeDataSongs(encodedData);

        m_application->openDialogCreateStaticList(NULL, songs);

        return true;
    }

    return false;
}


QList<CSong *> CListModel::decodeDataSongs(const QByteArray& encodedData) const
{
    QDataStream stream(encodedData);

    int numSongs;
    stream >> numSongs;

    QList<CSong *> songList;

    for (int i = 0; i < numSongs; ++i)
    {
        int songId;
        stream >> songId;
        songList << m_application->getSongFromId(songId);
    }

    return songList;
}


bool CListModel::decodeDataList(const QByteArray& encodedData, int * playList, int * folder) const
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

QStringList CListModel::mimeTypes(void) const
{
    QStringList types;
    types << "application/x-ted-media-songs"; // Liste de morceaux
    types << "application/x-ted-media-list";  // Liste de lecture ou dossier
    return types;
}


QMimeData * CListModel::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.size() != 1)
        return NULL;

    qDebug() << "CListModel::mimeData()";

    QByteArray data;
    QDataStream streamData(&data, QIODevice::WriteOnly);

    // Médiathèque
    if (indexes[0].row() == 0 && !indexes[0].parent().isValid())
        return NULL;

    QStandardItem * item = itemFromIndex(indexes[0]);

    if (!item)
        return NULL;

    if (item->data(Qt::UserRole + 2).value<CFolder *>())
    {
        streamData << 1;
        streamData << item->data(Qt::UserRole + 2).value<CFolder *>()->getId();
    }
    else
    {
        streamData << 0;
        streamData << item->data(Qt::UserRole + 1).value<CSongTable *>()->getIdPlayList();
    }

    QMimeData * mimeData = new QMimeData();
    mimeData->setData("application/x-ted-media-list", data);
    return mimeData;
}


/**
 * Retourne la liste des actions acceptées pour le drop.
 *
 * \return Actions acceptées.
 */

Qt::DropActions CListModel::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::MoveAction);
}


CFolder * CListModel::getFolderFromId(int id, const QList<CFolder *> folders) const
{
    if (id < 0)
        return NULL;

    for (QList<CFolder *>::const_iterator it = folders.begin(); it != folders.end(); ++it)
    {
        if ((*it)->getId() == id)
            return (*it);
    }

    return NULL;
}


IPlayList * CListModel::getPlayListFromId(int id, const QList<IPlayList *> playLists) const
{
    if (id <= 0)
        return NULL;

    for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        if ((*it)->getIdPlayList() == id)
            return (*it);
    }

    return NULL;
}


void CListModel::onPlayListRenamed(const QString& oldName, const QString& newName)
{
    IPlayList * playList = qobject_cast<IPlayList *>(sender());

    if (playList)
    {
        QStandardItem * item = m_songTableItems.key(playList);
        
        if (item)
            item->setText(newName);
    }
}


void CListModel::onFolderRenamed(const QString& oldName, const QString& newName)
{
    CFolder * folder = qobject_cast<CFolder *>(sender());

    if (folder)
    {
        QStandardItem * item = m_folderItems.key(folder);
        
        if (item)
            item->setText(newName);
    }
}

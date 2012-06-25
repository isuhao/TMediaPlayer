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

#include "CFolder.hpp"
#include "IPlayList.hpp"
#include "CApplication.hpp"
#include <QSqlQuery>
#include <QSqlError>

#include <QtDebug>


/**
 * Constructeur du dossier.
 *
 * \param application Pointeur sur l'application.
 * \param name        Nom du dossier.
 */

CFolder::CFolder(CApplication * application, const QString& name) :
    QObject          (application),
    m_application    (application),
    m_id             (-1),
    m_name           (name),
    m_open           (false),
    m_folder         (NULL),
    //m_position       (1),
    m_isModified     (false),
    m_folderChanging (false)
{
    Q_CHECK_PTR(application);
}


/**
 * Détruit le dossier.
 */

CFolder::~CFolder()
{
/*
    foreach (IPlayList * playList, m_playLists)
    {
        playList->updateDatabase();
        delete playList;
    }
*/
}


/**
 * Modifie le nom du dossier.
 *
 * \param name Nouveau nom du dossier.
 */

void CFolder::setName(const QString& name)
{
    if (m_id == 0)
        return;

    const QString oldName = m_name;

    if (name != m_name)
    {
        m_name = name;
        m_isModified = true;
        emit nameChanged(oldName, name);
    }
}


/**
 * Modifie le dossier contenant le dossier.
 *
 * \param folder Pointeur sur le dossier qui contiendra le dossier, ou NULL si le
 *               dossier n'est pas dans un dossier.
 */

void CFolder::setFolder(CFolder * folder)
{
    if (m_id == 0)
        return;

    if (m_folderChanging)
        return;

    CFolder * oldFolder = m_folder;
    m_folderChanging = true;

    if (oldFolder != folder && folder != this)
    {
        m_folder = folder;
        m_isModified = true;

        if (oldFolder) oldFolder->removeFolder(this);
        if (folder) folder->addFolder(this);

        emit folderChanged(oldFolder, folder);
    }

    m_folderChanging = false;
}


/**
 * Indique si les informations ou le contenu du dossier ont été modifiées.
 *
 * \return Booléen.
 */

bool CFolder::isModified(void) const
{
    return m_isModified;
}


/**
 * Retourne la position d'une liste de lecture à l'intérieur du dossier.
 *
 * \param playList Pointeur sur la liste de lecture à rechercher
 * \return Position de la liste de lecture (à partir de 0), ou -1 en cas d'erreur.
 */

int CFolder::getPosition(IPlayList * playList) const
{
    if (!m_playLists0.contains(playList))
    {
        return -1;
    }

    for (int position = 0; position < m_items.size(); ++position)
    {
        if (m_items[position] && m_items[position]->playList == playList)
        {
            return position;
        }
    }

    qWarning() << "CFolder::getPosition() : problème ligne " << __LINE__;
    return -1;
}


/**
 * Retourne la position d'un dossier à l'intérieur du dossier.
 *
 * \param folder Pointeur sur le dossier à rechercher
 * \return Position du dossier (à partir de 0), ou -1 en cas d'erreur.
 */

int CFolder::getPosition(CFolder * folder) const
{
    if (!m_folders0.contains(folder))
    {
        return -1;
    }

    for (int position = 0; position < m_items.size(); ++position)
    {
        if (m_items[position] && m_items[position]->folder == folder)
        {
            return position;
        }
    }

    qWarning() << "CFolder::getPosition() : problème ligne " << __LINE__;
    return -1;
}


/**
 * Ajoute une liste de lecture au dossier.
 *
 * \param playList Liste de lecture à ajouter.
 * \param position Position de la liste dans le dossier.
 */

void CFolder::addPlayList(IPlayList * playList, int position)
{
    Q_CHECK_PTR(playList);

    if (position >= m_items.size())
    {
        m_items.resize(position + 1);
    }
    else if (position < 0)
    {
        position = -1;
    }

    // Déplacement
    if (m_playLists0.contains(playList))
    {
        if (position < 0)
            position = m_items.size() - 1;

        int oldPosition = getPosition(playList);
        TFolderItem * item = m_items[oldPosition];

        if (oldPosition < position)
        {
            for (int pos = oldPosition; pos < position; ++pos)
            {
                m_items[pos] = m_items[pos+1];
            }
        }
        else if (oldPosition > position)
        {
            for (int pos = position; pos < oldPosition; ++pos)
            {
                m_items[pos] = m_items[pos+1];
            }
        }

        m_items[position] = item;

        // Mise à jour des positions des éléments
        for (int pos = 0; pos < m_items.size(); ++pos)
        {
            if (m_items[pos] && m_items[pos]->position != pos)
            {
                m_items[pos]->position = pos;
                if (m_items[pos]->playList) m_items[pos]->playList->m_isModified = true;
                if (m_items[pos]->folder)   m_items[pos]->folder->m_isModified = true;
            }
        }
    }
    // Insertion
    else
    {
        if (position < 0)
            position = m_items.size();

        m_playLists0.append(playList);
        m_items.insert(position, new TFolderItem(-42, playList));
        playList->setFolder(this);

        // Mise à jour des positions des éléments
        for (int pos = 0; pos < m_items.size(); ++pos)
        {
            if (m_items[pos] && m_items[pos]->position != pos)
            {
                m_items[pos]->position = pos;
                if (m_items[pos]->playList) m_items[pos]->playList->m_isModified = true;
                if (m_items[pos]->folder)   m_items[pos]->folder->m_isModified = true;
            }
        }
    }
}


/**
 * Enlève une liste de lecture d'un dossier.
 *
 * \param playList Liste de lecture à enlever.
 */

void CFolder::removePlayList(IPlayList * playList)
{
    Q_CHECK_PTR(playList);

    if (m_playLists0.contains(playList))
    {
        removePlayListItem(playList);
        playList->setFolder(NULL);

        // Mise à jour des positions des listes
        for (int pos = 0; pos < m_items.size(); ++pos)
        {
            if (m_items[pos] && m_items[pos]->position != pos)
            {
                m_items[pos]->position = pos;
                if (m_items[pos]->playList) m_items[pos]->playList->m_isModified = true;
                if (m_items[pos]->folder)   m_items[pos]->folder->m_isModified = true;
            }
        }
    }
}


/**
 * Ajoute un dossier au dossier.
 *
 * \param folder   Dossier à ajouter.
 * \param position Position du dossier dans le dossier.
 */

void CFolder::addFolder(CFolder * folder, int position)
{
    Q_CHECK_PTR(folder);

    if (folder == this)
        return;

    position = qBound(-1, position, m_items.size() - 1);

    // Déplacement
    if (m_folders0.contains(folder))
    {
        Q_ASSERT(folder->m_folder == this);
        
        if (position < 0)
            position = m_items.size() - 1;

        int oldPosition = getPosition(folder);
        TFolderItem * item = m_items[oldPosition];

        if (oldPosition < position)
        {
            for (int pos = oldPosition; pos < position; ++pos)
            {
                m_items[pos] = m_items[pos+1];
            }
        }
        else if (oldPosition > position)
        {
            for (int pos = position; pos < oldPosition; ++pos)
            {
                m_items[pos] = m_items[pos+1];
            }
        }

        m_items[position] = item;

        // Mise à jour des positions des éléments
        for (int pos = 0; pos < m_items.size(); ++pos)
        {
            if (m_items[pos] && m_items[pos]->position != pos)
            {
                m_items[pos]->position = pos;
                if (m_items[pos]->playList) m_items[pos]->playList->m_isModified = true;
                if (m_items[pos]->folder)   m_items[pos]->folder->m_isModified = true;
            }
        }
    }
    // Insertion
    else
    {
        if (position < 0)
            position = m_items.size();

        m_folders0.append(folder);
        m_items.insert(position, new TFolderItem(-42, folder));
        folder->m_folder = this;

        // Mise à jour des positions des listes
        for (int pos = 0; pos < m_items.size(); ++pos)
        {
            if (m_items[pos] && m_items[pos]->position != pos)
            {
                m_items[pos]->position = pos;
                if (m_items[pos]->playList) m_items[pos]->playList->m_isModified = true;
                if (m_items[pos]->folder)   m_items[pos]->folder->m_isModified = true;
            }
        }
    }
}


/**
 * Enlève un dossier d'un dossier.
 *
 * \param folder Dossier à enlever.
 */

void CFolder::removeFolder(CFolder * folder)
{
    Q_CHECK_PTR(folder);

    if (m_folders0.contains(folder))
    {
        removeFolderItem(folder);
        folder->setFolder(NULL);
        //m_isModified = true;

        // Mise à jour des positions des éléments
        for (int pos = 0; pos < m_items.size(); ++pos)
        {
            if (m_items[pos] && m_items[pos]->position != pos)
            {
                m_items[pos]->position = pos;
                if (m_items[pos]->playList) m_items[pos]->playList->m_isModified = true;
                if (m_items[pos]->folder)   m_items[pos]->folder->m_isModified = true;
            }
        }
    }
}


/**
 * Ouvre ou ferme le dossier dans la vue.
 *
 * \param open Booléen indiquant si le dossier est ouvert.
 */

void CFolder::setOpen(bool open)
{
    if (m_id == 0)
        return;

    if (m_open != open)
    {
        m_open = open;
        m_isModified = true;

        if (open) emit folderOpened();
        else      emit folderClosed();
    }
}


/**
 * Met à jour les informations du dossier en base de données.
 *
 * \return Booléen indiquant le succès de l'opération.
 */

bool CFolder::updateDatabase(void)
{
    if (m_id != 0 && !getFolder())
    {
        qWarning() << "CFolder::updateDatabase() : big problème ligne " << __LINE__;
    }

    int folderId = (getFolder() ? getFolder()->getId() : 0);

    // Insertion
    if (m_id < 0)
    {
        QSqlQuery query(m_application->getDataBase());

        // Position dans le dossier
        int position = getFolder()->getPosition(this);
/*
        query.prepare("SELECT MAX(folder_position) FROM folder WHERE folder_parent = ?");
        query.bindValue(0, folderId);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        if (query.next())
        {
            m_position = query.value(0).toInt() + 1;
        }
*/
        // Insertion
        query.prepare("INSERT INTO folder (folder_name, folder_parent, folder_position, folder_expanded) VALUES (?, ?, ?, ?)");

        query.bindValue(0, m_name);
        query.bindValue(1, folderId);
        query.bindValue(2, position);
        query.bindValue(3, (m_open ? 1 : 0));

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        m_id = query.lastInsertId().toInt();
    }
    // Mise à jour
    else if (m_isModified)
    {
        QSqlQuery query(m_application->getDataBase());

        query.prepare("UPDATE folder SET folder_name = ?, folder_parent = ?, folder_position = ?, folder_expanded = ? WHERE folder_id = ?");

        query.bindValue(0, m_name);
        query.bindValue(1, folderId);
        query.bindValue(2, getFolder()->getPosition(this));
        query.bindValue(3, (m_open ? 1 : 0));
        query.bindValue(4, m_id);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }
    }

    m_isModified = false;
    return true;
}


/**
 * Supprime le dossier de la base de données.
 *
 * \todo Gérer le contenu du dossier.
 *
 * \param recursive Indique si on doit supprimer le contenu dossier ou pas.
 */

void CFolder::removeFromDatabase(bool recursive)
{
    if (m_id <= 0)
    {
        qWarning() << "CFolder::romoveFromDatabase() : identifiant invalide";
        return;
    }

    if (recursive)
    {
        //...
    }
    else
    {
        //...
    }

    return; //TMP
    
    // Suppression du dossier
    QSqlQuery query(m_application->getDataBase());
    query.prepare("DELETE FROM folder WHERE folder_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    m_isModified = false;
    m_id = -1;
}


void CFolder::addPlayListItem(IPlayList * playList, int position)
{
    Q_CHECK_PTR(playList);

    if (position >= m_items.size())
    {
        m_items.resize(position + 1);
    }
    else if (position < 0)
    {
        qWarning() << "CFolder::addPlayListItem() : position inconnue";
        position = m_items.size();
        m_items.append(NULL);
    }
    
    m_playLists0.append(playList);
    m_items[position] = new TFolderItem(position, playList);
}

    
void CFolder::addFolderItem(CFolder * folder, int position)
{
    Q_CHECK_PTR(folder);

    if (position >= m_items.size())
    {
        m_items.resize(position + 1);
    }
    else if (position < 0)
    {
        qWarning() << "CFolder::addPlayListItem() : position inconnue";
        position = m_items.size();
        m_items.append(NULL);
    }
    
    m_folders0.append(folder);
    m_items[position] = new TFolderItem(position, folder);
}


void CFolder::removePlayListItem(IPlayList * playList)
{
    Q_CHECK_PTR(playList);

    m_playLists0.removeAll(playList);

    for (QVector<TFolderItem *>::iterator it = m_items.begin(); it != m_items.end(); )
    {
        if ((*it)->playList == playList)
        {
            it = m_items.erase(it);
        }
        else
        {
            ++it;
        }
    }
}


void CFolder::removeFolderItem(CFolder * folder)
{
    Q_CHECK_PTR(folder);

    m_folders0.removeAll(folder);

    for (QVector<TFolderItem *>::iterator it = m_items.begin(); it != m_items.end(); )
    {
        if ((*it)->folder == folder)
        {
            it = m_items.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

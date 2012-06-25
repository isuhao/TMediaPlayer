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

#include "IPlayList.hpp"
#include "CFolder.hpp"
#include "CApplication.hpp"
#include <QSqlQuery>
#include <QSqlError>

#include <QtDebug>


IPlayList::IPlayList(CApplication * application, const QString& name) :
    CSongTable           (application),
    m_name               (name),
    m_position           (1),
    m_folder             (NULL),
    m_isPlayListModified (false),
    m_folderChanging     (false)
{

}


IPlayList::~IPlayList()
{
    //qDebug() << "IPlayList::~IPlayList()";
}


/**
 * Indique si la liste de lecture a été modifiée et doit être mise à jour.
 *
 * \return Booléen.
 */

bool IPlayList::isModified(void) const
{
    return (m_isPlayListModified || CSongTable::isModified());
}


/**
 * Modifie le nom de la liste de lecture.
 *
 * \param name Nouveau nom de la liste.
 */

void IPlayList::setName(const QString& name)
{
    const QString oldName = m_name;

    if (oldName != name)
    {
        m_name = name;
        m_isPlayListModified = true;
        emit nameChanged(oldName, name);
    }
}


/**
 * Modifie le dossier contenant la liste de lecture.
 *
 * \param folder Pointeur sur le dossier qui contiendra la liste, ou NULL si la
 *               liste n'est pas dans un dossier.
 */

void IPlayList::setFolder(CFolder * folder)
{
    if (m_folderChanging)
        return;

    CFolder * oldFolder = m_folder;
    m_folderChanging = true;

    if (oldFolder != folder)
    {
        m_folder = folder;
        m_isPlayListModified = true;

        if (oldFolder) oldFolder->removePlayList(this);
        if (folder) folder->addPlayList(this);

        emit folderChanged(oldFolder, folder);
    }

    m_folderChanging = false;
}


/**
 * Met à jour la base de données si la liste a été modifiée.
 *
 * \return Booléen indiquant le succès de l'opération.
 */

bool IPlayList::updateDatabase(void)
{
    if (m_isPlayListModified)
    {
        if (m_idPlayList <= 0)
        {
            qWarning() << "IPlayList::updateDatabase() : identifiant invalide";
        }
        else
        {
            QSqlQuery query(m_application->getDataBase());
            query.prepare("UPDATE playlist SET playlist_name = ? WHERE playlist_id = ?");

            query.bindValue(0, m_name);
            query.bindValue(1, m_idPlayList);

            if (!query.exec())
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return false;
            }

            m_isPlayListModified = false;
        }
    }

    return CSongTable::updateDatabase();
}


/**
 * Supprime la liste de la base de données.
 */

void IPlayList::romoveFromDatabase(void)
{
    if (m_idPlayList <= 0)
    {
        qWarning() << "IPlayList::romoveFromDatabase() : identifiant invalide";
        return;
    }
    
    // Suppression de la liste
    QSqlQuery query(m_application->getDataBase());
    query.prepare("DELETE FROM playlist WHERE playlist_id = ?");
    query.bindValue(0, m_idPlayList);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    m_isPlayListModified = false;
    m_idPlayList = -1;
}

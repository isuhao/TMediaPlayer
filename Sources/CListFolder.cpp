
#include "CListFolder.hpp"
#include "CPlayList.hpp"
#include "CApplication.hpp"
#include <QSqlQuery>
#include <QSqlError>


CListFolder::CListFolder(CApplication * application, const QString& name) :
    QObject          (application),
    m_application    (application),
    m_id             (-1),
    m_name           (name),
    m_open           (false),
    m_folder         (NULL),
    m_position       (1),
    m_isModified     (false),
    m_folderChanging (false)
{
    Q_CHECK_PTR(application);

    //...
}


CListFolder::~CListFolder()
{
/*
    foreach (CPlayList * playList, m_playLists)
    {
        playList->updateDatabase();
        delete playList;
    }
*/
}


/**
 * Modifie le nom du dossier.
 *
 * \todo Enregistrer les modifications.
 *
 * \param name Nouveau nom du dossier.
 */

void CListFolder::setName(const QString& name)
{
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

void CListFolder::setFolder(CListFolder * folder)
{
    if (m_folderChanging)
        return;

    CListFolder * oldFolder = m_folder;
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

bool CListFolder::isModified(void) const
{
    return m_isModified;
}


/**
 * Ajoute une liste de lecture au dossier.
 *
 * \param playList Liste de lecture à ajouter.
 * \param position Position de la liste dans le dossier.
 */

void CListFolder::addPlayList(CPlayList * playList, int position)
{
    Q_CHECK_PTR(playList);

    position = qBound(-1, position, m_playLists.size());

    if (!m_playLists.contains(playList))
    {
        //m_playLists.append(playList);
        m_playLists.insert(position, playList);
        playList->setFolder(this);
        //m_isModified = true;

        // Mise à jour des positions des listes
        for (int pos = 0; pos < m_playLists.size(); ++pos)
        {
            if (m_playLists[pos]->m_position != pos)
            {
                m_playLists[pos]->m_position = pos;
                m_playLists[pos]->m_isModified = true;
            }
        }
    }
}


/**
 * Enlève une liste de lecture d'un dossier.
 *
 * \todo MAJ position.
 *
 * \param playList Liste de lecture à enlever.
 */

void CListFolder::removePlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    if (m_playLists.contains(playList))
    {
        m_playLists.removeAll(playList);
        playList->setFolder(NULL);
        //m_isModified = true;

        // Mise à jour des positions des listes
        for (int pos = 0; pos < m_playLists.size(); ++pos)
        {
            if (m_playLists[pos]->m_position != pos)
            {
                m_playLists[pos]->m_position = pos;
                m_playLists[pos]->m_isModified = true;
            }
        }
    }
}


/**
 * Ajoute un dossier au dossier.
 *
 * \todo MAJ position.
 *
 * \param folder   Dossier à ajouter.
 * \param position Position de la liste dans le dossier.
 */

void CListFolder::addFolder(CListFolder * folder, int position)
{
    Q_CHECK_PTR(folder);

    position = qBound(-1, position, m_playLists.size());

    if (!m_folders.contains(folder) && folder != this)
    {
        //m_folders.append(folder);
        m_folders.insert(position, folder);
        folder->setFolder(this);
        //m_isModified = true;

        // Mise à jour des positions des listes
        for (int pos = 0; pos < m_folders.size(); ++pos)
        {
            if (m_folders[pos]->m_position != pos)
            {
                m_folders[pos]->m_position = pos;
                m_folders[pos]->m_isModified = true;
            }
        }
    }
}


/**
 * Enlève un dossier d'un dossier.
 *
 * \todo MAJ position.
 *
 * \param folder Dossier à enlever.
 */

void CListFolder::removeFolder(CListFolder * folder)
{
    Q_CHECK_PTR(folder);

    if (m_folders.contains(folder))
    {
        m_folders.removeAll(folder);
        folder->setFolder(NULL);
        //m_isModified = true;

        // Mise à jour des positions des listes
        for (int pos = 0; pos < m_folders.size(); ++pos)
        {
            if (m_folders[pos]->m_position != pos)
            {
                m_folders[pos]->m_position = pos;
                m_folders[pos]->m_isModified = true;
            }
        }
    }
}


/**
 * Ouvre ou ferme le dossier dans la vue.
 *
 * \param open Booléen indiquant si le dossier est ouvert.
 */

void CListFolder::setOpen(bool open)
{
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

bool CListFolder::updateDatabase(void)
{
    // Insertion
    if (m_id <= 0)
    {
        QSqlQuery query(m_application->getDataBase());

        // Position dans le dossier
        query.prepare("SELECT MAX(folder_position) FROM folder WHERE folder_parent = ?");
        query.bindValue(0, m_folder->getId());

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        if (query.next())
        {
            m_position = query.value(0).toInt() + 1;
        }

        // Insertion
        query.prepare("INSERT INTO folder (folder_name, folder_parent, folder_position, folder_expanded) VALUES (?, ?, ?, ?)");

        query.bindValue(0, m_name);
        query.bindValue(1, m_folder->getId());
        query.bindValue(2, m_position);
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
        query.prepare("UPDATE folder SET folder_name = ?, folder_expanded = ? WHERE folder_id = ?");

        query.bindValue(0, m_name);
        query.bindValue(1, (m_open ? 1 : 0));
        query.bindValue(2, m_id);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }
    }

    m_isModified = false;
    return true;
}

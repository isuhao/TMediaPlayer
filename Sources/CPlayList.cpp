
#include "CPlayList.hpp"
#include "CListFolder.hpp"


CPlayList::CPlayList(const QString& name) :
    m_name           (name),
    m_folder         (NULL),
    m_folderChanging (false)
{

}


CPlayList::~CPlayList()
{

}


/**
 * Modifie le nom de la liste de lecture.
 *
 * \param name Nouveau nom de la liste.
 */

void CPlayList::setName(const QString& name)
{
    const QString oldName = m_name;

    if (oldName != name)
    {
        m_name = name;
        emit nameChanged(oldName, name);
    }
}


/**
 * Modifie le dossier contenant la liste de lecture.
 *
 * \param folder Pointeur sur le dossier qui contiendra la liste, ou NULL si la
 *               liste n'est pas dans un dossier.
 */

void CPlayList::setFolder(CListFolder * folder)
{
    if (m_folderChanging)
    {
        return;
    }

    CListFolder * oldFolder = folder;
    m_folderChanging = true;

    if (oldFolder != folder)
    {
        m_folder = folder;

        if (oldFolder) oldFolder->removePlayList(this);
        if (folder) folder->addPlayList(this);

        emit folderChanged(oldFolder, folder);
    }

    m_folderChanging = false;
}

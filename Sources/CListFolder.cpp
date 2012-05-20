
#include "CListFolder.hpp"
#include "CPlayList.hpp"


CListFolder::CListFolder(void) :
    QObject      (),
    m_id         (-1),
    m_isModified (false)
{

}


CListFolder::~CListFolder()
{
    foreach (CPlayList * playList, m_playLists)
    {
        playList->updateDatabase();
        delete playList;
    }
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
    if (name != m_name)
    {
        m_name = name;
        m_isModified = true;
    }
}


bool CListFolder::isModified(void) const
{
    return m_isModified;
}


void CListFolder::addPlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    if (!m_playLists.contains(playList))
    {
        m_playLists.append(playList);
        playList->setFolder(this);
        m_isModified = true;
    }
}


void CListFolder::removePlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    if (m_playLists.contains(playList))
    {
        m_playLists.removeAll(playList);
        playList->setFolder(NULL);
        m_isModified = true;
    }
}

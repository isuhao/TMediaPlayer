
#include "CListFolder.hpp"
#include "CPlayList.hpp"


CListFolder::CListFolder(void)
{

}


CListFolder::~CListFolder()
{

}


void CListFolder::setName(const QString& name)
{
    m_name = name;
}


void CListFolder::addPlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    if (!m_playLists.contains(playList))
    {
        m_playLists.append(playList);
        playList->setFolder(this);
    }
}


void CListFolder::removePlayList(CPlayList * playList)
{
    Q_CHECK_PTR(playList);

    if (m_playLists.contains(playList))
    {
        m_playLists.removeAll(playList);
        playList->setFolder(NULL);
    }
}


#include "CMultiCriterion.hpp"


CMultiCriterion::CMultiCriterion(QObject * parent) :
    ICriteria    (parent),
    m_multi_type (Intersection)
{
    m_type = TypeMultiCriterion;
}


CMultiCriterion::~CMultiCriterion()
{

}


void CMultiCriterion::setMultiCriterionType(TMultiCriterionType type)
{
    m_multi_type = type;
}


void CMultiCriterion::addChild(ICriteria * child)
{
    Q_CHECK_PTR(child);

    if (!m_children.contains(child))
    {
        m_children.append(child);
    }
}


bool CMultiCriterion::matchCriteria(CSong * song) const
{
    Q_CHECK_PTR(song);

    if (m_children.isEmpty())
    {
        return false;
    }

    if (m_multi_type == Union)
    {
        foreach (ICriteria * criteria, m_children)
        {
            if (criteria->matchCriteria(song))
            {
                return true;
            }
        }
    }
    else if (m_multi_type == Intersection)
    {
        foreach (ICriteria * criteria, m_children)
        {
            if (!criteria->matchCriteria(song))
            {
                return false;
            }
        }
        
        return true;
    }

    return false;
}


/**
 *+++
 * La liste est sans doublons, et ne contient que des éléments provenant de \a from.
 *+++
 */
 
QList<CSong *> CMultiCriterion::getSongs(const QList<CSong *>& from, const QList<CSong *>& with) const
{
    if (m_children.isEmpty())
    {
        return with;
    }

    QList<CSong *> songList;

    if (m_multi_type == Union)
    {
        songList = with;

        foreach (ICriteria * criteria, m_children)
        {
            songList = criteria->getSongs(from, songList);
        }
    }
    else if (m_multi_type == Intersection)
    {
        songList = from;

        foreach (ICriteria * criteria, m_children)
        {
            songList = criteria->getSongs(songList, with);
        }
    }

    return songList;
}

/*
QList<int> CMultiCriterion::getValidTypes(void) const
{
    QList<int> validType = ICriteria::getValidTypes();
    validType.append(TypeMultiCriterion);
    return validType;
}


bool CMultiCriterion::isValidType(int type) const
{
    if (type == TypeMultiCriterion) return true;
    return ICriteria::isValidType(type);
}
*/
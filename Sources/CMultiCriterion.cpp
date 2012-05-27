
#include "CMultiCriterion.hpp"
#include "CApplication.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

#include <QtDebug>


CMultiCriterion::CMultiCriterion(QObject * parent) :
    ICriteria    (parent),
    m_multi_type (Intersection)
{
    m_type = TypeMultiCriterion;
}


CMultiCriterion::~CMultiCriterion()
{
    qDebug() << "CMultiCriterion::~CMultiCriterion()";
}


void CMultiCriterion::setMultiCriterionType(TMultiCriterionType type)
{
    m_multi_type = type;
}


/**
 * Ajoute un sous-critère.
 *
 * \param child Sous-critère à ajouter.
 */

void CMultiCriterion::addChild(ICriteria * child)
{
    Q_CHECK_PTR(child);

    if (!m_children.contains(child))
    {
        m_children.append(child);
        child->setParent(this);
        child->m_parent = this;
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


void CMultiCriterion::setPlayList(CDynamicPlayList * playList)
{
    Q_CHECK_PTR(playList);

    foreach (ICriteria * criteria, m_children)
    {
        criteria->setPlayList(playList);
    }

    ICriteria::setPlayList(playList);
}


void CMultiCriterion::insertIntoDatabase(CApplication * application)
{
    Q_CHECK_PTR(application);
    qDebug() << "CMultiCriterion::insertIntoDatabase()";

    // Insertion du critère
    ICriteria::insertIntoDatabase(application);

    // Modification du type
    QSqlQuery query(application->getDataBase());

    query.prepare("UPDATE criteria SET criteria_union = ? WHERE criteria_id = ?");
    query.bindValue(0, m_multi_type);
    query.bindValue(1, m_id);

    if (!query.exec())
    {
        application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    foreach (ICriteria * criteria, m_children)
    {
        criteria->insertIntoDatabase(application);
    }
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
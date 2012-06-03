
#include "CMultiCriterion.hpp"
#include "CApplication.hpp"
#include "CWidgetMultiCriterion.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

#include <QtDebug>


/**
 * Construit le sous-critère d'une liste dynamique.
 */

CMultiCriterion::CMultiCriterion(CApplication * application, QObject * parent) :
    ICriteria (application, parent)
{
    m_type = TypeUnion;
}


/**
 * Détruit le sous-critère.
 */

CMultiCriterion::~CMultiCriterion()
{
    qDebug() << "CMultiCriterion::~CMultiCriterion()";
}


CMultiCriterion::TMultiCriterionType CMultiCriterion::getMultiCriterionType(void) const
{
    switch (m_type)
    {
        default:
        case TypeUnion:        return Union;
        case TypeIntersection: return Intersection;
    }
}


void CMultiCriterion::setMultiCriterionType(TMultiCriterionType type)
{
    switch (type)
    {
        default:
        case Union       : m_type = TypeUnion       ; break;
        case Intersection: m_type = TypeIntersection; break;
    }
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

    if (m_type == TypeUnion)
    {
        for (QList<ICriteria *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
        {
            if ((*it)->matchCriteria(song))
            {
                return true;
            }
        }
    }
    else if (m_type == TypeIntersection)
    {
        for (QList<ICriteria *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
        {
            if (!(*it)->matchCriteria(song))
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

    if (m_type == TypeUnion)
    {
        songList = with;

        for (QList<ICriteria *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
        {
            songList = (*it)->getSongs(from, songList);
        }
    }
    else if (m_type == TypeIntersection)
    {
        songList = from;

        for (QList<ICriteria *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
        {
            songList = (*it)->getSongs(songList, with);
        }
    }

    return songList;
}


void CMultiCriterion::setPlayList(CDynamicPlayList * playList)
{
    Q_CHECK_PTR(playList);

    for (QList<ICriteria *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->setPlayList(playList);
    }

    ICriteria::setPlayList(playList);
}


void CMultiCriterion::insertIntoDatabase(CApplication * application)
{
    Q_CHECK_PTR(application);
    qDebug() << "CMultiCriterion::insertIntoDatabase()";

    // Insertion du critère
    ICriteria::insertIntoDatabase(application);

    for (QList<ICriteria *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->insertIntoDatabase(application);
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


IWidgetCriteria * CMultiCriterion::getWidget(void) const
{
    CWidgetMultiCriterion * widget = new CWidgetMultiCriterion(m_application, NULL);
    widget->setMultiCriterionType(m_type);

    for (QList<ICriteria *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        widget->addCriteria((*it)->getWidget());
    }

    // Suppression du premier critère crée par défaut
    widget->removeCriteria(1);

    return widget;
}

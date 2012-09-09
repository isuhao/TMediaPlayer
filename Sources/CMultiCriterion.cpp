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


ICriteria::TUpdateConditions CMultiCriterion::getUpdateConditions(void) const
{
    if (m_children.isEmpty())
    {
        return UpdateNever;
    }

    TUpdateConditions ret;

    for (QList<ICriteria *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        ret |= (*it)->getUpdateConditions();
    }

    return ret;
}


void CMultiCriterion::setPlayList(CDynamicList * playList)
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

    // Insertion du critère
    ICriteria::insertIntoDatabase(application);

    for (QList<ICriteria *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->insertIntoDatabase(application);
    }
}


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

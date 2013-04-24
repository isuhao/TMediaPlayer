/*
Copyright (C) 2012-2013 Teddy Michel

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

#include "CMultiCriteria.hpp"
#include "CMainWindow.hpp"
#include "CWidgetMultiCriteria.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

#include <QtDebug>


/**
 * Construit le sous-critère d'une liste dynamique.
 */

CMultiCriteria::CMultiCriteria(CMainWindow * mainWindow, QObject * parent) :
ICriterion (mainWindow, parent)
{
    m_type = TypeUnion;
}


/**
 * Détruit le sous-critère.
 */

CMultiCriteria::~CMultiCriteria()
{

}


CMultiCriteria::TMultiCriteriaType CMultiCriteria::getMultiCriteriaType() const
{
    switch (m_type)
    {
        default:
        case TypeUnion:        return Union;
        case TypeIntersection: return Intersection;
    }
}


void CMultiCriteria::setMultiCriteriaType(TMultiCriteriaType type)
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

void CMultiCriteria::addChild(ICriterion * child)
{
    Q_CHECK_PTR(child);

    if (!m_children.contains(child))
    {
        m_children.append(child);
        child->setParent(this);
        child->m_parent = this;
    }
}


bool CMultiCriteria::matchCriterion(CSong * song) const
{
    Q_CHECK_PTR(song);

    if (m_children.isEmpty())
    {
        return false;
    }

    if (m_type == TypeUnion)
    {
        for (QList<ICriterion *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
        {
            if ((*it)->matchCriterion(song))
            {
                return true;
            }
        }
    }
    else if (m_type == TypeIntersection)
    {
        for (QList<ICriterion *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
        {
            if (!(*it)->matchCriterion(song))
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
 
QList<CSong *> CMultiCriteria::getSongs(const QList<CSong *>& from, const QList<CSong *>& with) const
{
    if (m_children.isEmpty())
    {
        return with;
    }

    QList<CSong *> songList;

    if (m_type == TypeUnion)
    {
        songList = with;

        for (QList<ICriterion *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
        {
            songList = (*it)->getSongs(from, songList);
        }
    }
    else if (m_type == TypeIntersection)
    {
        songList = from;

        for (QList<ICriterion *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
        {
            songList = (*it)->getSongs(songList, with);
        }
    }

    return songList;
}


ICriterion::TUpdateConditions CMultiCriteria::getUpdateConditions() const
{
    if (m_children.isEmpty())
    {
        return UpdateNever;
    }

    TUpdateConditions ret;

    for (QList<ICriterion *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        ret |= (*it)->getUpdateConditions();
    }

    return ret;
}


void CMultiCriteria::setPlayList(CDynamicList * playList)
{
    Q_CHECK_PTR(playList);

    for (QList<ICriterion *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->setPlayList(playList);
    }

    ICriterion::setPlayList(playList);
}


void CMultiCriteria::insertIntoDatabase(CMainWindow * mainWindow)
{
    Q_CHECK_PTR(mainWindow);

    // Insertion du critère
    ICriterion::insertIntoDatabase(mainWindow);

    for (QList<ICriterion *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->insertIntoDatabase(mainWindow);
    }
}


IWidgetCriterion * CMultiCriteria::getWidget() const
{
    CWidgetMultiCriteria * widget = new CWidgetMultiCriteria(m_mainWindow, nullptr);
    widget->setMultiCriteriaType(m_type);

    for (QList<ICriterion *>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        widget->addCriterion((*it)->getWidget());
    }

    // Suppression du premier critère crée par défaut
    widget->removeCriterion(1);

    return widget;
}

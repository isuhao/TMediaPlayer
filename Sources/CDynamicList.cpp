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

#include "CDynamicList.hpp"
#include "CApplication.hpp"
#include "CMultiCriterion.hpp"
#include "CWidgetMultiCriterion.hpp"
#include "CCriteria.hpp"
#include "CFolder.hpp"
#include "CLibrary.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

#include <QtDebug>


/**
 * Construit la liste de lecture dynamique.
 *
 * \param application Pointeur sur l'application.
 * \param name        Nom de la liste de lecture.
 */

CDynamicList::CDynamicList(CApplication * application, const QString& name) :
    IPlayList               (application, name),
    m_id                    (-1),
    m_mainCriteria          (NULL),
    m_isDynamicListModified (false)
{
    m_mainCriteria = new CMultiCriterion(m_application, this);
}


/**
 * Détruit la liste de lecture dynamique.
 */

CDynamicList::~CDynamicList()
{
    //qDebug() << "CDynamicList::~CDynamicList()";
}


/**
 * Indique si la liste a été modifiée et doit être mise à jour en base de donnés.
 *
 * \return Booléen.
 */

bool CDynamicList::isModified(void) const
{
    return (m_isDynamicListModified || IPlayList::isModified());
}


/**
 * Retourne le widget permettant l'édition de la liste dynamique.
 *
 * \return Widget.
 */

CWidgetMultiCriterion * CDynamicList::getWidget(void) const
{
    IWidgetCriteria * widget = m_mainCriteria->getWidget();
    return qobject_cast<CWidgetMultiCriterion *>(widget);
}


/**
 * Met à jour la liste des morceaux.
 */

void CDynamicList::update(void)
{
    //qDebug() << "CDynamicList::update()";
    QList<CSong *> songs = m_mainCriteria->getSongs(m_application->getLibrary()->getSongs());

    m_model->setSongs(songs);
    sortByColumn(m_columnSort, m_sortOrder);

    emit listUpdated();
}


/**
 * Met à jour les informations de la liste en base de données.
 *
 * \todo Gérer les critères.
 *
 * \return Booléen indiquant le succès de l'opération.
 */

bool CDynamicList::updateDatabase(void)
{
    QSqlQuery query(m_application->getDataBase());

    if (!getFolder())
    {
        qWarning() << "CDynamicList::updateDatabase() : big problème ligne " << __LINE__;
    }

    // Insertion
    if (m_id <= 0)
    {
        int folderId = (getFolder() ? getFolder()->getId() : 0);

        // Position dans le dossier
        int position = getFolder()->getPosition(this);
/*
        query.prepare("SELECT MAX(list_position) FROM playlist WHERE folder_id = ?");
        query.bindValue(0, folderId);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        if (query.next())
        {
            m_position = query.value(0).toInt() + 1;
        }
*/

        // Insertion
        query.prepare("INSERT INTO playlist (playlist_name, folder_id, list_position, list_columns) VALUES (?, ?, ?, ?)");

        query.bindValue(0, m_name);
        query.bindValue(1, folderId);
        query.bindValue(2, position);
        query.bindValue(3, "0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120"); // Disposition par défaut

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        m_idPlayList = query.lastInsertId().toInt();

        query.prepare("INSERT INTO dynamic_list (criteria_id, playlist_id) VALUES (?, ?)");
        
        query.bindValue(0, 0);
        query.bindValue(1, m_idPlayList);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        m_id = query.lastInsertId().toInt();

        // Insertion des nouveaux critères
        m_mainCriteria->insertIntoDatabase(m_application);

        query.prepare("UPDATE dynamic_list SET criteria_id = ? WHERE dynamic_list_id = ?");
        
        query.bindValue(0, m_mainCriteria->getId());
        query.bindValue(1, m_id);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }
    }
    // Mise à jour
    else if (m_isDynamicListModified)
    {
        query.prepare("UPDATE dynamic_list SET criteria_id = ? WHERE dynamic_list_id = ?");
        
        query.bindValue(0, 0);
        query.bindValue(1, m_id);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        query.prepare("UPDATE playlist SET playlist_name = ? WHERE playlist_id = ?");

        query.bindValue(0, m_name);
        query.bindValue(1, m_idPlayList);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        // Suppression des anciens critères
        query.prepare("DELETE FROM criteria WHERE dynamic_list_id = ?");
        query.bindValue(0, m_id);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        // Insertion des nouveaux critères
        m_mainCriteria->insertIntoDatabase(m_application);

        query.prepare("UPDATE dynamic_list SET criteria_id = ? WHERE dynamic_list_id = ?");
        
        query.bindValue(0, m_mainCriteria->getId());
        query.bindValue(1, m_id);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }
    }

    m_isDynamicListModified = false;
    return IPlayList::updateDatabase();
}


/**
 * Supprime la liste de la base de données.
 */

void CDynamicList::romoveFromDatabase(void)
{
    if (m_id <= 0)
    {
        qWarning() << "CDynamicList::romoveFromDatabase() : identifiant invalide";
        return;
    }

    QSqlQuery query(m_application->getDataBase());

    // Suppression des critères
    query.prepare("DELETE FROM criteria WHERE dynamic_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    // Suppression de la liste dynamique
    query.prepare("DELETE FROM dynamic_list WHERE dynamic_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    m_isDynamicListModified = false;
    m_id = -1;

    IPlayList::removeFromDatabase();
}


/**
 * Charge la liste de lecture dynamique depuis la base de données.
 */

void CDynamicList::loadFromDatabase(void)
{
    if (m_id <= 0)
    {
        return;
    }

    delete m_mainCriteria;
    m_mainCriteria = NULL;

    QSqlQuery query(m_application->getDataBase());

    // Liste des critères
    query.prepare("SELECT criteria_id, criteria_parent, criteria_position, criteria_type, criteria_condition, criteria_value1, criteria_value2 "
                  "FROM criteria WHERE dynamic_list_id = ? ORDER BY criteria_position");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    QMap<int, ICriteria *> criteriaList;

    while (query.next())
    {
        ICriteria * criteria = NULL;

        if (query.value(3).toInt() == ICriteria::TypeUnion)
        {
            CMultiCriterion * multiCriterion = new CMultiCriterion(m_application, this);
            multiCriterion->setMultiCriterionType(CMultiCriterion::Union);
            criteria = multiCriterion;
        }
        else if (query.value(3).toInt() == ICriteria::TypeIntersection)
        {
            CMultiCriterion * multiCriterion = new CMultiCriterion(m_application, this);
            multiCriterion->setMultiCriterionType(CMultiCriterion::Intersection);
            criteria = multiCriterion;
        }
        else
        {
            criteria = new CCriteria(m_application, this);
        }

        criteria->m_id        = query.value(0).toInt();
        criteria->m_parent    = reinterpret_cast<ICriteria *>(query.value(1).toInt());
      //criteria->m_position  = query.value(2).toInt();
        criteria->m_playList  = this;

        criteria->m_type      = query.value(3).toInt();
        criteria->m_condition = query.value(4).toInt();
        criteria->m_value1    = query.value(5);
        criteria->m_value2    = query.value(6);

        criteriaList[criteria->m_id] = criteria;
    }

    if (criteriaList.isEmpty())
    {
        qWarning() << "CDynamicList::loadFromDatabase() : aucun critère défini";
        m_mainCriteria = new CCriteria(m_application, this);
        return;
    }

    // Imbrication des critères
    for (QMap<int, ICriteria *>::const_iterator it = criteriaList.begin(); it != criteriaList.end(); ++it)
    {
        long parentId = reinterpret_cast<long>(it.value()->m_parent);

        if (parentId == 0)
        {
            if (m_mainCriteria)
            {
                qWarning() << "CDynamicList::loadFromDatabase() : plusieurs critères principaux";
                continue;
            }

            m_mainCriteria = it.value();
        }
        else
        {
            if (!criteriaList.contains(parentId))
            {
                qWarning() << "CDynamicList::loadFromDatabase() : l'identifiant du parent ne correspond a aucun critère de la liste";
                continue;
            }

            CMultiCriterion * multiCriterion = qobject_cast<CMultiCriterion *>(criteriaList[parentId]);

            if (!multiCriterion)
            {
                qWarning() << "CDynamicList::loadFromDatabase() : la parent du critère n'est pas un multi-critères";
                continue;
            }

            multiCriterion->addChild(it.value());
        }
    }


    // Conditions de mise à jour
    ICriteria::TUpdateConditions conditions = m_mainCriteria->getUpdateConditions();

    if (conditions.testFlag(ICriteria::UpdateOnSongAdded))
    {
        connect(m_application, SIGNAL(songAdded(CSong *)), this, SLOT(update()), Qt::UniqueConnection);
    }

    if (conditions.testFlag(ICriteria::UpdateOnSongRemoved))
    {
        connect(m_application, SIGNAL(songRemoved(CSong *)), this, SLOT(update()), Qt::UniqueConnection);
    }

    if (conditions.testFlag(ICriteria::UpdateOnSongModified))
    {
        connect(m_application, SIGNAL(songModified(CSong *)), this, SLOT(update()), Qt::UniqueConnection);
    }

    if (conditions.testFlag(ICriteria::UpdateOnSongMoved))
    {
        connect(m_application, SIGNAL(songMoved(CSong *)), this, SLOT(update()), Qt::UniqueConnection);
    }

    if (conditions.testFlag(ICriteria::UpdateOnSongPlayEnd))
    {
        connect(m_application, SIGNAL(songPlayEnd(CSong *)), this, SLOT(update()), Qt::UniqueConnection);
    }

    if (conditions.testFlag(ICriteria::UpdateOnListModified))
    {
        connect(m_application, SIGNAL(listModified()), this, SLOT(update()), Qt::UniqueConnection);
    }
}


/**
 * Change le critère principale utilisé par la liste.
 * L'ancien critère est détruit, et la liste devient le parent du critère.
 *
 * \param criteria Nouveau critère.
 */

void CDynamicList::setCriteria(ICriteria * criteria)
{
    Q_CHECK_PTR(criteria);

    delete m_mainCriteria;
    criteria->setParent(this);
    criteria->setPlayList(this);
    m_mainCriteria = criteria;
    m_isDynamicListModified = true;


    // Conditions de mise à jour
    disconnect(m_application, SIGNAL(songAdded(CSong *)), this, SLOT(update()));
    disconnect(m_application, SIGNAL(songRemoved(CSong *)), this, SLOT(update()));
    disconnect(m_application, SIGNAL(songModified(CSong *)), this, SLOT(update()));
    disconnect(m_application, SIGNAL(songMoved(CSong *)), this, SLOT(update()));
    disconnect(m_application, SIGNAL(songPlayEnd(CSong *)), this, SLOT(update()));
    disconnect(m_application, SIGNAL(listModified()), this, SLOT(update()));

    ICriteria::TUpdateConditions conditions = m_mainCriteria->getUpdateConditions();

    if (conditions.testFlag(ICriteria::UpdateOnSongAdded))
    {
        connect(m_application, SIGNAL(songAdded(CSong *)), this, SLOT(update()), Qt::UniqueConnection);
    }

    if (conditions.testFlag(ICriteria::UpdateOnSongRemoved))
    {
        connect(m_application, SIGNAL(songRemoved(CSong *)), this, SLOT(update()), Qt::UniqueConnection);
    }

    if (conditions.testFlag(ICriteria::UpdateOnSongModified))
    {
        connect(m_application, SIGNAL(songModified(CSong *)), this, SLOT(update()), Qt::UniqueConnection);
    }

    if (conditions.testFlag(ICriteria::UpdateOnSongMoved))
    {
        connect(m_application, SIGNAL(songMoved(CSong *)), this, SLOT(update()), Qt::UniqueConnection);
    }

    if (conditions.testFlag(ICriteria::UpdateOnSongPlayEnd))
    {
        connect(m_application, SIGNAL(songPlayEnd(CSong *)), this, SLOT(update()), Qt::UniqueConnection);
    }

    if (conditions.testFlag(ICriteria::UpdateOnListModified))
    {
        connect(m_application, SIGNAL(listModified()), this, SLOT(update()), Qt::UniqueConnection);
    }

    emit listModified();
}

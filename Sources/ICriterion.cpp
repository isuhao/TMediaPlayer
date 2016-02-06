/*
Copyright (C) 2012-2016 Teddy Michel

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

#include "ICriterion.hpp"
#include "CMainWindow.hpp"
#include "CMediaManager.hpp"
#include "CDynamicList.hpp"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

#include <QtDebug>


/**
 * Crée le critère.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 * \param parent     Pointeur sur l'objet parent, qui sera chargé de détruire le critère.
 */

ICriterion::ICriterion(CMainWindow * mainWindow, QObject * parent) :
QObject      (parent),
m_type       (TypeInvalid),
m_condition  (CondInvalid),
m_value1     (),
m_value2     (),
m_mainWindow (mainWindow),
m_id         (-1),
m_position   (1),
m_parent     (nullptr),
m_playList   (nullptr)
{
    Q_CHECK_PTR(m_mainWindow);
}


/**
 * Détruit le critère.
 */

ICriterion::~ICriterion()
{

}


/**
 * Retourne la liste des morceaux qui vérifient le critère.
 * Cette implémentation de base parcourt la liste \a from et utilise la méthode matchCriterion
 * pour tester si le morceau vérifie le critère.
 *
 * \param from        Liste de morceaux à analyser.
 * \param with        Liste de morceaux à ajouter dans la liste.
 * \param onlyChecked Indique si on doit prendre uniquement les morceaux cochés.
 * \return Liste de morceaux qui vérifient le critère, sans doublons, avec tous les
 *         éléments de \a with.
 */

QList<CSong *> ICriterion::getSongs(const QList<CSong *>& from, const QList<CSong *>& with, bool onlyChecked) const
{
    QList<CSong *> songList = with;

    for (QList<CSong *>::ConstIterator it = from.begin(); it != from.end(); ++it)
    {
        if (matchCriterion(*it) && !songList.contains(*it))
        {
            if (onlyChecked)
            {
                if ((*it)->isEnabled())
                    songList.append(*it);
            }
            else
            {
                songList.append(*it);
            }
        }
    }

    return songList;
}


/**
 * Modifie la liste de lecture dynamique associée au critère.
 *
 * \param playList Pointeur sur la nouvelle liste dynamique.
 */

void ICriterion::setPlayList(CDynamicList * playList)
{
    Q_CHECK_PTR(playList);
    m_playList = playList;
}


/**
 * Ajoute le critère en base de données.
 */

void ICriterion::insertIntoDatabase()
{
    if (m_id != -1)
    {
        m_mainWindow->getMediaManager()->logError(tr("the criteria is already in database"), __FUNCTION__, __FILE__, __LINE__);
        //return;
    }

    if (!m_playList)
    {
        m_mainWindow->getMediaManager()->logError(tr("the criteria has no associated playlist"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    if (m_playList->getId() <= 0)
    {
        m_mainWindow->getMediaManager()->logError(tr("the playlist associated to the criteria has an invalid identifier"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    int parentId = 0;
    if (m_parent)
    {
        parentId = m_parent->getId();

        if (parentId <= 0)
        {
            m_mainWindow->getMediaManager()->logError(tr("the criteria has a parent, but with an invalid identifier"), __FUNCTION__, __FILE__, __LINE__);
            parentId = 0;
        }
    }

    QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());

    // Position du critère
    query.prepare("SELECT MAX(criteria_position) FROM criteria WHERE criteria_parent = ? AND dynamic_list_id = ?");
    query.bindValue(0, parentId);
    query.bindValue(1, m_playList->getId());

    if (!query.exec())
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    if (query.next())
    {
        m_position = query.value(0).toInt() + 1;
    }

    query.prepare("INSERT INTO criteria (dynamic_list_id, criteria_parent, criteria_position, criteria_type, criteria_condition, criteria_value1, criteria_value2) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");

    int numValue = 0;

    query.bindValue(numValue++, m_playList->getId());
    query.bindValue(numValue++, parentId);
    query.bindValue(numValue++, m_position);
    query.bindValue(numValue++, m_type);
    query.bindValue(numValue++, m_condition);
    query.bindValue(numValue++, m_value1);
    query.bindValue(numValue++, m_value2);

    if (!query.exec())
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    if (m_mainWindow->getMediaManager()->getDataBase().driverName() == "QPSQL")
    {
        query.prepare("SELECT currval('criteria_seq')");

        if (!query.exec())
        {
            m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return;
        }

        if (query.next())
        {
            m_id = query.value(0).toInt();
        }
        else
        {
            m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return;
        }
    }
    else
    {
        m_id = query.lastInsertId().toInt();
    }
}

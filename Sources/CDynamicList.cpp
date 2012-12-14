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
#include "CDialogEditSong.hpp"
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
    m_isDynamicListModified (false),
    m_autoUpdate            (true),
    m_onlyChecked           (false),
    m_numItems              (0)
{
    m_mainCriteria = new CMultiCriterion(m_application, this);
}


/**
 * Détruit la liste de lecture dynamique.
 */

CDynamicList::~CDynamicList()
{

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
 *
 * \todo Pouvoir limiter le nombre de morceaux.
 * \todo Si la boite de dialogue "Informations sur un morceau" est affiché, il faut éventuellement la mettre-à-jour.
 */

void CDynamicList::updateList()
{
    // Si on est en train de lire un morceau de la liste, il faut mettre à jour les informations sur le morceau courant
    CSongTableItem * currentItem = m_model->getCurrentSongItem();
    CSong * currentSong = (currentItem ? currentItem->getSong() : NULL);

    CDialogEditSong * dialogEditSong = m_application->getDialogEditSong();
    CSong * currentSongInDialogEditSong = NULL;

    if (dialogEditSong)
    {
        if (dialogEditSong->getSongTable() == this)
            currentSongInDialogEditSong = dialogEditSong->getSongItem()->getSong();
        else
            dialogEditSong = NULL;
    }

    QList<CSong *> songs = m_mainCriteria->getSongs(m_application->getLibrary()->getSongs(), QList<CSong *>(), m_onlyChecked);

    //TODO: nombre d'éléments
    //1) Trier la liste selon le critère demandé
    //2) Conserver les N premiers éléments
    //3) le sortByColumn ci-dessous n'est pas nécessaire si le critère de tri et l'ordre sont les mêmes
    //4) Si seul l'ordre diffère, il suffit de renverser la liste (swap des N/2 premiers éléments avec les N/2 derniers)


    // Liste des morceaux sélectionnés
    const QModelIndexList indexList = selectionModel()->selectedRows();
    QList<CSong *> selectedSongs;

    for (QModelIndexList::const_iterator index = indexList.begin(); index != indexList.end(); ++index)
    {
        CSong * song = getSongItemForRow(index->row())->getSong();

        if (!selectedSongs.contains(song))
            selectedSongs.append(song);
    }

    CSongTableItem * selectedSongItem = getSongItemForRow(selectionModel()->currentIndex().row());
    CSong * selectedSong = (selectedSongItem ? selectedSongItem->getSong() : NULL);


    m_model->setSongs(songs);
    sortByColumn(m_columnSort, m_sortOrder);


    // Sélection des morceaux précédemment sélectionnés
    selectionModel()->clearSelection();

    for (QList<CSong *>::const_iterator song = selectedSongs.begin(); song != selectedSongs.end(); ++song)
    {
        CSongTableItem * songItem = getFirstSongItem(*song);

        if (songItem)
            selectionModel()->select(m_model->index(m_model->getRowForSongItem(songItem), 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    if (selectedSong)
        selectionModel()->setCurrentIndex(m_model->index(m_model->getRowForSongItem(getFirstSongItem(selectedSong)), 0), QItemSelectionModel::Rows);


    // On change le morceau courant affiché dans la liste
    if (currentSong)
    {
        CSongTableItem * currentItemAfter = getFirstSongItem(currentSong);
        m_model->setCurrentSong(currentItemAfter);
        //m_application->m_currentSongItem = currentItemAfter;
        m_application->setCurrentSongItem(currentItemAfter, this);
    }

    if (dialogEditSong)
    {
        dialogEditSong->setSongItem(getFirstSongItem(currentSongInDialogEditSong), this);
    }

    emit listUpdated();
}


/**
 * Met à jour les informations de la liste en base de données.
 *
 * \return Booléen indiquant le succès de l'opération.
 */

bool CDynamicList::updateDatabase(void)
{
    QSqlQuery query(m_application->getDataBase());

    if (!getFolder())
    {
        m_application->logError(tr("the playlist is not in a folder"), __FUNCTION__, __FILE__, __LINE__);
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

        if (m_application->getDataBase().driverName() == "QPSQL")
        {
            query.prepare("SELECT currval('playlist_playlist_id_seq')");

            if (!query.exec())
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return false;
            }

            if (query.next())
            {
                m_idPlayList = query.value(0).toInt();
            }
            else
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return false;
            }
        }
        else
        {
            m_idPlayList = query.lastInsertId().toInt();
        }

        query.prepare("INSERT INTO dynamic_list (criteria_id, playlist_id, auto_update, only_checked) VALUES (?, ?, ?, ?)");

        query.bindValue(0, 0);
        query.bindValue(1, m_idPlayList);
        query.bindValue(2, m_autoUpdate ? 1 : 0);
        query.bindValue(3, m_onlyChecked ? 1 : 0);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        if (m_application->getDataBase().driverName() == "QPSQL")
        {
            query.prepare("SELECT currval('dynamic_list_seq')");

            if (!query.exec())
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return false;
            }

            if (query.next())
            {
                m_id = query.value(0).toInt();
            }
            else
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return false;
            }
        }
        else
        {
            m_id = query.lastInsertId().toInt();
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
    // Mise à jour
    else if (m_isDynamicListModified)
    {
        query.prepare("UPDATE dynamic_list SET criteria_id = ?, auto_update = ?, only_checked = ? WHERE dynamic_list_id = ?");

        query.bindValue(0, 0);
        query.bindValue(1, m_id);
        query.bindValue(2, m_autoUpdate ? 1 : 0);
        query.bindValue(3, m_onlyChecked ? 1 : 0);

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

void CDynamicList::removeFromDatabase(void)
{
    if (m_id <= 0)
    {
        m_application->logError(tr("invalid identifier (%1)").arg(m_id), __FUNCTION__, __FILE__, __LINE__);
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
        m_application->logError(tr("invalid identifier (%1)").arg(m_id), __FUNCTION__, __FILE__, __LINE__);
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
        m_application->logError(tr("dynamic list with no criteria"), __FUNCTION__, __FILE__, __LINE__);
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
                m_application->logError(tr("dynamic list with several main criterion"), __FUNCTION__, __FILE__, __LINE__);
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
    if (m_autoUpdate)
    {
        ICriteria::TUpdateConditions conditions = m_mainCriteria->getUpdateConditions();

        if (conditions.testFlag(ICriteria::UpdateOnSongAdded))
            connect(m_application, SIGNAL(songsAdded()), this, SLOT(updateList()), Qt::UniqueConnection);

        if (conditions.testFlag(ICriteria::UpdateOnSongRemoved))
            connect(m_application, SIGNAL(songRemoved(CSong *)), this, SLOT(updateList()), Qt::UniqueConnection);

        if (conditions.testFlag(ICriteria::UpdateOnSongModified))
            connect(m_application, SIGNAL(songModified(CSong *)), this, SLOT(updateList()), Qt::UniqueConnection);

        if (conditions.testFlag(ICriteria::UpdateOnSongMoved))
            connect(m_application, SIGNAL(songMoved(CSong *)), this, SLOT(updateList()), Qt::UniqueConnection);

        if (conditions.testFlag(ICriteria::UpdateOnSongPlayEnd))
            connect(m_application, SIGNAL(songPlayEnd(CSong *)), this, SLOT(updateList()), Qt::UniqueConnection);

        //if (conditions.testFlag(ICriteria::UpdateOnListModified))
            //connect(m_application, SIGNAL(listModified(IPlayList *)), this, SLOT(updateList()), Qt::UniqueConnection);
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
    if (m_autoUpdate)
    {
        disconnect(m_application, SIGNAL(songsAdded()             ), this, SLOT(updateList()));
        disconnect(m_application, SIGNAL(songRemoved(CSong *)     ), this, SLOT(updateList()));
        disconnect(m_application, SIGNAL(songModified(CSong *)    ), this, SLOT(updateList()));
        disconnect(m_application, SIGNAL(songMoved(CSong *)       ), this, SLOT(updateList()));
        disconnect(m_application, SIGNAL(songPlayEnd(CSong *)     ), this, SLOT(updateList()));
        //disconnect(m_application, SIGNAL(listModified(IPlayList *)), this, SLOT(updateList()));

        ICriteria::TUpdateConditions conditions = m_mainCriteria->getUpdateConditions();

        if (conditions.testFlag(ICriteria::UpdateOnSongAdded))
            connect(m_application, SIGNAL(songsAdded()), this, SLOT(updateList()), Qt::UniqueConnection);

        if (conditions.testFlag(ICriteria::UpdateOnSongRemoved))
            connect(m_application, SIGNAL(songRemoved(CSong *)), this, SLOT(updateList()), Qt::UniqueConnection);

        if (conditions.testFlag(ICriteria::UpdateOnSongModified))
            connect(m_application, SIGNAL(songModified(CSong *)), this, SLOT(updateList()), Qt::UniqueConnection);

        if (conditions.testFlag(ICriteria::UpdateOnSongMoved))
            connect(m_application, SIGNAL(songMoved(CSong *)), this, SLOT(updateList()), Qt::UniqueConnection);

        if (conditions.testFlag(ICriteria::UpdateOnSongPlayEnd))
            connect(m_application, SIGNAL(songPlayEnd(CSong *)), this, SLOT(updateList()), Qt::UniqueConnection);

        //if (conditions.testFlag(ICriteria::UpdateOnListModified))
            //connect(m_application, SIGNAL(listModified(IPlayList *)), this, SLOT(updateList()), Qt::UniqueConnection);
    }

    emit listModified();
}


void CDynamicList::setAutoUpdate(bool autoUpdate)
{
    if (m_autoUpdate != autoUpdate)
    {
        m_autoUpdate = autoUpdate;
        m_isDynamicListModified = true;
        emit listModified();

        if (m_autoUpdate)
            updateList(); // est-ce nécessaire ?
    }
}


void CDynamicList::setOnlyChecked(bool onlyChecked)
{
    if (m_onlyChecked != onlyChecked)
    {
        m_onlyChecked = onlyChecked;
        m_isDynamicListModified = true;
        emit listModified();

        if (m_autoUpdate)
            updateList(); // est-ce nécessaire ?
    }
}

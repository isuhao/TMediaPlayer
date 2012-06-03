
#include "CDynamicPlayList.hpp"
#include "CApplication.hpp"
#include "CMultiCriterion.hpp"
#include "CWidgetMultiCriterion.hpp"
#include "CCriteria.hpp"
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

CDynamicPlayList::CDynamicPlayList(CApplication * application, const QString& name) :
    CPlayList               (application, name),
    m_id                    (-1),
    m_mainCriteria          (NULL),
    m_isDynamicListModified (false)
{
    m_mainCriteria = new CMultiCriterion(m_application, this);
}


/**
 * D�truit la liste de lecture dynamique.
 */

CDynamicPlayList::~CDynamicPlayList()
{
    qDebug() << "CDynamicPlayList::~CDynamicPlayList()";
}


/**
 * Indique si la liste a �t� modifi�e et doit �tre mise � jour en base de donn�s.
 *
 * \return Bool�en.
 */

bool CDynamicPlayList::isModified(void) const
{
    return (m_isDynamicListModified || CPlayList::isModified());
}


/**
 * Retourne le widget permettant l'�dition de la liste dynamique.
 *
 * \return Widget.
 */

CWidgetMultiCriterion * CDynamicPlayList::getWidget(void) const
{
    IWidgetCriteria * widget = m_mainCriteria->getWidget();
    return qobject_cast<CWidgetMultiCriterion *>(widget);
}


/**
 * Met � jour la liste des morceaux.
 */

void CDynamicPlayList::update(void)
{
    qDebug() << "CDynamicPlayList::update()";
    QList<CSong *> songs = m_mainCriteria->getSongs(m_application->getLibrary()->getSongs());

    m_model->setSongs(songs);
    sortByColumn(m_columnSort, m_sortOrder);

    emit listUpdated();
}


/**
 * Met � jour les informations de la liste en base de donn�es.
 *
 * \todo G�rer les crit�res.
 *
 * \return Bool�en indiquant le succ�s de l'op�ration.
 */

bool CDynamicPlayList::updateDatabase(void)
{
    QSqlQuery query(m_application->getDataBase());

    // Insertion
    if (m_id <= 0)
    {
        // Position dans le dossier
        query.prepare("SELECT MAX(list_position) FROM playlist WHERE folder_id = ?");
        query.bindValue(0, 0);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        if (query.next())
        {
            m_position = query.value(0).toInt() + 1;
        }

        // Insertion
        query.prepare("INSERT INTO playlist (playlist_name, folder_id, list_position, list_columns) VALUES (?, ?, ?, ?)");

        query.bindValue(0, m_name);
        query.bindValue(1, 0);
        query.bindValue(2, m_position);
        query.bindValue(3, "0:40;1:100;2:100;3:100"); // Disposition par d�faut \todo => settings

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

        // Insertion des nouveaux crit�res
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
    // Mise � jour
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

        // Suppression des anciens crit�res
        query.prepare("DELETE FROM criteria WHERE dynamic_list_id = ?");
        query.bindValue(0, m_id);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        // Insertion des nouveaux crit�res
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
    return CPlayList::updateDatabase();
}


/**
 * Supprime la liste de la base de donn�es.
 */

void CDynamicPlayList::romoveFromDatabase(void)
{
    if (m_id <= 0)
    {
        qWarning() << "CDynamicPlayList::romoveFromDatabase() : identifiant invalide";
        return;
    }

    QSqlQuery query(m_application->getDataBase());

    // Suppression des crit�res
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

    CPlayList::romoveFromDatabase();
}


/**
 * Charge la liste de lecture dynamique depuis la base de donn�es.
 */

void CDynamicPlayList::loadFromDatabase(void)
{
    if (m_id <= 0)
    {
        return;
    }

    delete m_mainCriteria;
    m_mainCriteria = NULL;

    QSqlQuery query(m_application->getDataBase());

    // Liste des crit�res
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
        qWarning() << "CDynamicPlayList::loadFromDatabase() : aucun crit�re d�fini";
        m_mainCriteria = new CCriteria(m_application, this);
        return;
    }

    // Imbrication des crit�res
    for (QMap<int, ICriteria *>::const_iterator it = criteriaList.begin(); it != criteriaList.end(); ++it)
    {
        int parentId = reinterpret_cast<int>(it.value()->m_parent);

        if (parentId == 0)
        {
            if (m_mainCriteria)
            {
                qWarning() << "CDynamicPlayList::loadFromDatabase() : plusieurs crit�res principaux";
                continue;
            }

            m_mainCriteria = it.value();
        }
        else
        {
            if (!criteriaList.contains(parentId))
            {
                qWarning() << "CDynamicPlayList::loadFromDatabase() : l'identifiant du parent ne correspond a aucun crit�re de la liste";
                continue;
            }

            CMultiCriterion * multiCriterion = qobject_cast<CMultiCriterion *>(criteriaList[parentId]);

            if (!multiCriterion)
            {
                qWarning() << "CDynamicPlayList::loadFromDatabase() : la parent du crit�re n'est pas un multi-crit�res";
                continue;
            }

            multiCriterion->addChild(it.value());
        }
    }
}


/**
 * Change le crit�re principale utilis� par la liste.
 * L'ancien crit�re est d�truit, et la liste devient le parent du crit�re.
 *
 * \param criteria Nouveau crit�re.
 */

void CDynamicPlayList::setCriteria(ICriteria * criteria)
{
    Q_CHECK_PTR(criteria);

    delete m_mainCriteria;
    criteria->setParent(this);
    criteria->setPlayList(this);
    m_mainCriteria = criteria;
    m_isDynamicListModified = true;

    emit listModified();
}

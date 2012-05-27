
#include "ICriteria.hpp"
#include "CApplication.hpp"
#include "CDynamicPlayList.hpp"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

#include <QtDebug>


/**
 * Crée le critère.
 *
 * \param parent Pointeur sur l'objet parent, qui sera chargé de détruire le critère.
 */

ICriteria::ICriteria(QObject * parent) :
    QObject     (parent),
    m_type      (TypeInvalid),
    m_condition (CondInvalid),
    m_value1    (),
    m_value2    (),
    m_id        (-1),
    m_position  (1),
    m_parent    (NULL),
    m_playList  (NULL)
{

}


/**
 * Détruit le critère.
 */

ICriteria::~ICriteria()
{
    qDebug() << "ICriteria::~ICriteria()";
}


/**
 * Retourne la liste des morceaux qui vérifient le critère.
 * Cette implémentation de base parcourt la liste \a from est utilise la méthode matchCriteria
 * pour tester si le morceau vérifie le critère.
 *
 * \param from Liste de morceaux à analyser.
 * \param with Liste de morceaux à ajouter dans la liste.
 * \return Liste de morceaux qui vérifient le critère, sans doublons, avec tous les
 *         éléments de \a with.
 */

QList<CSong *> ICriteria::getSongs(const QList<CSong *>& from, const QList<CSong *>& with) const
{
    QList<CSong *> songList = with;

    foreach (CSong * song, from)
    {
        if (matchCriteria(song) && !songList.contains(song))
        {
            songList.append(song);
        }
    }

    return songList;
}


void ICriteria::setPlayList(CDynamicPlayList * playList)
{
    Q_CHECK_PTR(playList);

    m_playList = playList;
}


void ICriteria::insertIntoDatabase(CApplication * application)
{
    Q_CHECK_PTR(application);
    qDebug() << "ICriteria::insertIntoDatabase()";

    if (m_id != -1)
    {
        qWarning() << "ICriteria::insertIntoDatabase() : le critère a déjà été inséré dans la base de données";
        //return;
    }

    if (!m_playList)
    {
        qWarning() << "ICriteria::insertIntoDatabase() : le critère n'a pas de liste associée";
        return;
    }

    if (m_playList->getId() <= 0)
    {
        qWarning() << "ICriteria::insertIntoDatabase() : la liste associée au critère a un identifiant invalide";
        return;
    }

    int parentId = 0;
    if (m_parent)
    {
        parentId = m_parent->getId();

        if (parentId <= 0)
        {
            qWarning() << "ICriteria::insertIntoDatabase() : le critère a un parent, mais son identifiant est incorrect";
            parentId = 0;
        }
    }

    QSqlQuery query(application->getDataBase());

    // Position du critère
    query.prepare("SELECT MAX(criteria_position) FROM criteria WHERE criteria_parent = ? AND dynamic_list_id = ?");
    query.bindValue(0, parentId);
    query.bindValue(1, m_playList->getId());

    if (!query.exec())
    {
        application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    if (query.next())
    {
        m_position = query.value(0).toInt() + 1;
    }

    query.prepare("INSERT INTO criteria (dynamic_list_id, criteria_parent, criteria_position, criteria_union, criteria_type, criteria_condition, criteria_value1, criteria_value2) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    int numValue = 0;

    query.bindValue(numValue++, m_playList->getId());
    query.bindValue(numValue++, parentId);
    query.bindValue(numValue++, m_position);
    query.bindValue(numValue++, 0);
    query.bindValue(numValue++, m_type);
    query.bindValue(numValue++, m_condition);
    query.bindValue(numValue++, m_value1);
    query.bindValue(numValue++, m_value2);

    if (!query.exec())
    {
        application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    m_id = query.lastInsertId().toInt();
}

/*
QList<int> ICriteria::getValidTypes(void) const
{
    return QList<int>();
}


bool ICriteria::isValidType(int type) const
{
    return false;
}


QList<int> ICriteria::getValidConditions(void) const
{
    return QList<int>();
}
*/
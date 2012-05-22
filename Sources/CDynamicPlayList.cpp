
#include "CDynamicPlayList.hpp"
#include "CApplication.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>


CDynamicPlayList::CDynamicPlayList(CApplication * application) :
    CPlayList (application),
    m_id      (-1)
{

}


CDynamicPlayList::~CDynamicPlayList()
{

}


/// \todo Implémentation
void CDynamicPlayList::update(void)
{

/*
    QList<CSong *> songs = m_mainCriteria->getSongs();

    m_model->setSongs(songs);
*/

}


/**
 * Met à jour les informations de la liste en base de données.
 *
 * \todo Gérer les critères.
 *
 * \return Booléen indiquant le succès de l'opération.
 */

bool CDynamicPlayList::updateDatabase(void)
{
    QSqlQuery query(m_application->getDataBase());

    // Insertion
    if (m_id <= 0)
    {
        // Position dans le dossier
        query.prepare("SELECT MAX(list_position) FROM dynamic_list WHERE folder_id = ?");
        query.bindValue(0, 0);

        if (!query.exec())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
            return false;
        }

        if (query.next())
        {
            m_position = query.value(0).toInt() + 1;
        }

        // Insertion
        query.prepare("INSERT INTO playlist (playlist_name, folder_id, list_position) VALUES (?, ?, ?)");

        query.bindValue(0, m_name);
        query.bindValue(2, 0);
        query.bindValue(3, m_position);

        if (!query.exec())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
            return false;
        }

        int idPlayList = query.lastInsertId().toInt();

        query.prepare("INSERT INTO dynamic_list (playlist_id, dynamic_list_union) VALUES (?, ?)");

        query.bindValue(0, idPlayList);
        query.bindValue(1, 1);

        if (!query.exec())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
            return false;
        }

        m_id = query.lastInsertId().toInt();
    }
    // Mise à jour
    else
    {
        query.prepare("SELECT playlist_id FROM dynamic_list WHERE dynamic_list_id = ?");
        query.bindValue(0, m_id);

        if (!query.exec())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
            return false;
        }

        if (!query.next())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
            return false;
        }

        int idPlayList = query.value(0).toInt();

        query.prepare("UPDATE playlist SET playlist_name = ? WHERE playlist_id = ?");

        query.bindValue(0, m_name);
        query.bindValue(1, idPlayList);

        if (!query.exec())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
            return false;
        }
    }

    return true;
}



#include "CStaticPlayList.hpp"
#include "CApplication.hpp"
#include "CSongTableModel.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>


CStaticPlayList::CStaticPlayList(CApplication * application, const QString& name) :
    CPlayList (application, name),
    m_id      (-1)
{
    m_model->setCanDrop(true);
    horizontalHeader()->showSection(0); // Position

    // Glisser-déposer
    setDropIndicatorShown(true);
    setAcceptDrops(true);
}


CStaticPlayList::~CStaticPlayList()
{

}


/**
 * Ajoute une chanson à la liste.
 *
 * \todo MAJ base
 *
 * \param song Pointeur sur la chanson à ajouter.
 * \param pos  Position où ajouter la chanson. Si négatif, la chanson est ajoutée à la fin de la liste.
 */

void CStaticPlayList::addSong(CSong * song, int pos)
{
    Q_CHECK_PTR(song);
    CSongTable::addSong(song, pos);
    emit songAdded(song);
}


/**
 * Enlève une chanson de la liste.
 * Toutes les occurences de \a song sont enlevées de la liste.
 *
 * \todo MAJ base
 *
 * \param song Pointeur sur la chanson à enlever.
 */

void CStaticPlayList::removeSong(CSong * song)
{
    Q_CHECK_PTR(song);

    if (hasSong(song))
    {
        CSongTable::removeSong(song);
        emit songRemoved(song);
        emit listModified();
    }
}


/**
 * Enlève une chanson de la liste.
 *
 * \todo MAJ base
 *
 * \param pos Position de la chanson dans la liste (à partir de 0).
 */

void CStaticPlayList::removeSong(int pos)
{
    Q_ASSERT(pos >= 0 && pos < getNumSongs());
    CSongTableModel::TSongItem * song = getSongItemForIndex(pos);

    if (song)
    {
        //TODO...
        CSongTable::removeSong(pos);
        emit songRemoved(song->song);
        emit listModified();
    }
}


/**
 * Enlève les doublons de la liste.
 *
 * \todo MAJ base
 *
 * \todo Implémentation.
 */

void CStaticPlayList::removeDuplicateSongs(void)
{
    //TODO...
}


bool CStaticPlayList::updateDatabase(void)
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
        query.bindValue(1, 0);
        query.bindValue(2, m_position);

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
        query.prepare("UPDATE playlist SET playlist_name = ? WHERE playlist_id = ?");

        query.bindValue(0, m_name);
        query.bindValue(1, m_id);

        if (!query.exec())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
            return false;
        }
    }

    return true;
}

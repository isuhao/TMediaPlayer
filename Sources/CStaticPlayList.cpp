
#include "CStaticPlayList.hpp"
#include "CApplication.hpp"
#include "CSongTableModel.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>


CStaticPlayList::CStaticPlayList(CApplication * application, const QString& name) :
    CPlayList              (application, name),
    m_id                   (-1),
    m_isStaticListModified (false)
{
    m_model->setCanDrop(true);

    // Glisser-déposer
    setDropIndicatorShown(true);
    setAcceptDrops(true);
}


CStaticPlayList::~CStaticPlayList()
{

}


bool CStaticPlayList::isModified(void) const
{
    return (m_isStaticListModified || CPlayList::isModified());
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

    m_isStaticListModified = true; // Hum...
    //...
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
        m_isStaticListModified = true; // Hum...
        //TODO...
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
        m_isStaticListModified = true; // Hum...
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
        query.prepare("INSERT INTO playlist (playlist_name, folder_id, list_position, list_columns) VALUES (?, ?, ?, ?)");

        query.bindValue(0, m_name);
        query.bindValue(1, 0);
        query.bindValue(2, m_position);
        query.bindValue(3, "1:100;2:100;3:100"); // Disposition par défaut \todo => settings

        if (!query.exec())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
            return false;
        }

        m_idPlayList = query.lastInsertId().toInt();

        query.prepare("INSERT INTO static_list (playlist_id) VALUES (?)");
        query.bindValue(0, m_idPlayList);

        if (!query.exec())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
            return false;
        }

        m_id = query.lastInsertId().toInt();
        m_isStaticListModified = false;
    }
    // Mise à jour
    else if (m_isStaticListModified)
    {
        query.prepare("UPDATE playlist SET playlist_name = ? WHERE playlist_id = ?");

        query.bindValue(0, m_name);
        query.bindValue(1, m_idPlayList);

        if (!query.exec())
        {
            QString error = query.lastError().text();
            QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
            return false;
        }

        m_isStaticListModified = false;
    }

    return CPlayList::updateDatabase();
}


void CStaticPlayList::initColumns(const QString& str)
{
    CSongTable::initColumns(str);

    // Colonne "Position"
    showColumn(0);
}

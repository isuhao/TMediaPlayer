
#include "CStaticPlayList.hpp"
#include "CApplication.hpp"
#include "CSongTableModel.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

#include <QtDebug>


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
    CSongTable::addSongToTable(song, pos);
    emit songAdded(song);
}


/**
 * Ajoute plusieurs morceaux à la liste de lecture.
 * Si certains morceaux sont déjà présents dans la liste, une confirmation est demandée.
 *
 * \todo Ne faire qu'une seule requête.
 *
 * \param songs Liste des morceaux à ajouter.
 */

void CStaticPlayList::addSongs(const QList<CSong *>& songs)
{
    Q_ASSERT(m_id > 0);

    if (songs.isEmpty())
    {
        return;
    }

    // Recherche des doublons
    bool hasDuplicate = false;
    foreach (CSong * song, songs)
    {
        if (hasSong(song))
        {
            hasDuplicate = true;
            break;
        }
    }

    bool skipDuplicate = false;

    if (hasDuplicate)
    {
        QMessageBox::StandardButton ret = QMessageBox::question(this, QString(), tr("There had duplicates being added to the playlist.\nWould you like to add them?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (ret == QMessageBox::No)
        {
            skipDuplicate = true;
        }
        else if (ret == QMessageBox::Cancel)
        {
            return;
        }
    }
    
    QSqlQuery query(m_application->getDataBase());

    // Position des morceaux dans la liste
    query.prepare("SELECT MAX(song_position) FROM static_list_song WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec() || !query.next())
    {
        QString error = query.lastError().text();
        QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
        return;
    }

    int songPosition = query.value(0).toInt() + 1;

    // Préparation de la requête SQL
    QVariantList field1;
    QVariantList field2;
    QVariantList field3;

    int numSongsAdded = 0;

    foreach (CSong * song, songs)
    {
        Q_CHECK_PTR(song);

        if (hasSong(song) && skipDuplicate)
        {
            continue;
        }

        field1 << m_id;
        field2 << song->getId();
        field3 << songPosition + numSongsAdded;

        ++numSongsAdded;
    }

    if (numSongsAdded == 0)
    {
        qDebug() << "Aucun morceau à ajouter";
        return;
    }
    
    query.prepare("INSERT INTO static_list_song (static_list_id, song_id, song_position) VALUES (?, ?, ?)");
    query.addBindValue(field1);
    query.addBindValue(field2);
    query.addBindValue(field3);

    if (!query.execBatch())
    {
        QString error = query.lastError().text();
        QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
        return;
    }

    // Mise à jour de la table
    int songNum = 0;
    m_automaticSort = false;

    foreach (CSong * song, songs)
    {
        if (hasSong(song) && skipDuplicate)
        {
            continue;
        }
        
        CSongTable::addSongToTable(song, songPosition + songNum);
        emit songAdded(song);
    }

    m_automaticSort = true;
    sortByColumn(m_columnSort, m_sortOrder);
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
        CSongTable::removeSongFromTable(song);
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
        CSongTable::removeSongFromTable(pos);
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


/**
 * Met à jour la base de données avec les informations de la liste de lecture.
 * Si la liste n'existe pas en base de données, elle est ajoutée.
 */

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

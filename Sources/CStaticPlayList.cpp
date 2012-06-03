
#include "CStaticPlayList.hpp"
#include "CApplication.hpp"
#include "CSongTableModel.hpp"
#include "CDynamicPlayList.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QKeyEvent>

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
    qDebug() << "CStaticPlayList::~CStaticPlayList()";
}


/**
 * Indique si la liste de lecture a été modifiée et doit être mise à jour.
 *
 * \return Booléen.
 */

bool CStaticPlayList::isModified(void) const
{
    return (m_isStaticListModified || CPlayList::isModified());
}


/**
 * Ajoute une chanson à la liste.
 *
 * \todo Implémentation
 *
 * \param song Pointeur sur la chanson à ajouter.
 * \param pos  Position où ajouter la chanson. Si négatif, la chanson est ajoutée à la fin de la liste.
 */

void CStaticPlayList::addSong(CSong * song, int pos)
{
    Q_CHECK_PTR(song);

    //...

    CSongTable::addSongToTable(song, pos);
    emit songAdded(song);
}


/**
 * Ajoute plusieurs morceaux à la liste de lecture.
 * Si certains morceaux sont déjà présents dans la liste, une confirmation est demandée.
 *
 * \param songs Liste des morceaux à ajouter.
 * \param confirm Indique si on doit demander une confirmation 
 */

void CStaticPlayList::addSongs(const QList<CSong *>& songs, bool confirm)
{
    Q_ASSERT(m_id > 0);

    if (songs.isEmpty())
    {
        return;
    }

    bool skipDuplicate = false;

    if (confirm)
    {
        // Recherche des doublons
        bool hasDuplicate = false;

        for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
        {
            if (hasSong(*it))
            {
                hasDuplicate = true;
                break;
            }
        }

        if (hasDuplicate)
        {
            /// \todo Créer la boite de dialogue à la main pour gérer les boutons en français
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
    }
    
    QSqlQuery query(m_application->getDataBase());

    // Position des morceaux dans la liste
    query.prepare("SELECT MAX(song_position) FROM static_list_song WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec() || !query.next())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    int songPosition = query.value(0).toInt() + 1;

    // Préparation de la requête SQL
    QVariantList field1;
    QVariantList field2;
    QVariantList field3;

    int numSongsAdded = 0;

    for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
    {
        Q_CHECK_PTR(*it);

        if (hasSong(*it) && skipDuplicate)
        {
            continue;
        }

        field1 << m_id;
        field2 << (*it)->getId();
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
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    // Mise à jour de la table
    int songNum = 0;
    m_automaticSort = false;

    for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
    {
        if (hasSong(*it) && skipDuplicate)
        {
            continue;
        }
        
        CSongTable::addSongToTable(*it, songPosition + songNum);
        emit songAdded(*it);
        ++songNum;
    }

    emit listModified();

    m_automaticSort = true;
    sortByColumn(m_columnSort, m_sortOrder);
}


/**
 * Enlève un morceau de la liste.
 * Toutes les occurences de \a song sont enlevées de la liste.
 *
 * \param song    Pointeur sur le morceau à enlever.
 * \param confirm Demande une confirmation avant d'enlever le morceau de la liste.
 */

void CStaticPlayList::removeSong(CSong * song, bool confirm)
{
    Q_CHECK_PTR(song);

    QList<CSongTableItem *> songItemList;

    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        CSongTableItem * songItem = m_model->getSongItem(row);

        if (songItem->getSong() == song)
        {
            songItemList.append(songItem);
        }
    }

    removeSongs(songItemList, confirm);
}


/**
 * Enlève un morceau de la liste.
 *
 * \param songItem Pointeur sur le morceau à enlever.
 * \param confirm  Demande une confirmation avant d'enlever le morceau de la liste.
 */

void CStaticPlayList::removeSong(CSongTableItem * songItem, bool confirm)
{
    Q_CHECK_PTR(songItem);

    // Confirmation
    if (confirm)
    {
        if (QMessageBox::question(this, QString(), tr("Are you sure you want to remove the selected songs from the list?"), tr("Remove"), tr("Cancel"), 0, 1) == 1)
        {
            return;
        }
    }

    if (m_application->getCurrentSongItem() == songItem)
    {
        m_application->stop();
    }

    int row = m_model->getRowForSongItem(songItem);

    if (row >= 0)
    {
        m_model->removeRow(row);
        emit songRemoved(songItem->getSong());
    }
    else
    {
        return;
    }

    QList<CSong *> songs = getSongs();

    removeAllSongsFromTable();

    QSqlQuery query(m_application->getDataBase());
    query.prepare("DELETE FROM static_list_song WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    addSongs(songs, false);
}


/**
 * Enlève une liste de morceaux de la liste.
 *
 * \param songs   Liste des morceaux à enlever.
 * \param confirm Demande une confirmation avant d'enlever les morceaux de la liste.
 */

void CStaticPlayList::removeSongs(const QList<CSong *>& songs, bool confirm)
{
    if (songs.isEmpty())
    {
        return;
    }

    QList<CSongTableItem *> songItemList;

    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        CSongTableItem * songItem = m_model->getSongItem(row);

        if (songs.contains(songItem->getSong()))
        {
            songItemList.append(songItem);
        }
    }

    removeSongs(songItemList, confirm);
}


/**
 * Enlève une liste de morceaux de la liste.
 *
 * \param songItemList Liste des morceaux à enlever.
 * \param confirm      Demande une confirmation avant d'enlever les morceaux de la liste.
 */

void CStaticPlayList::removeSongs(const QList<CSongTableItem *>& songItemList, bool confirm)
{
    if (songItemList.isEmpty())
    {
        return;
    }

    // Confirmation
    if (confirm)
    {
        if (QMessageBox::question(this, QString(), tr("Are you sure you want to remove the selected songs from the list?"), tr("Remove"), tr("Cancel"), 0, 1) == 1)
        {
            return;
        }
    }

    if (songItemList.contains(m_application->getCurrentSongItem()))
    {
        m_application->stop();
    }

    QList<CSong *> songList;

    for (QList<CSongTableItem *>::const_iterator it = songItemList.begin(); it != songItemList.end(); ++it)
    {
        int row = m_model->getRowForSongItem(*it);

        if (row >= 0)
        {
            m_model->removeRow(row);

            if (!songList.contains((*it)->getSong()))
            {
                songList.append((*it)->getSong());
            }
        }
    }

    // Émission des signaux
    for (QList<CSong *>::const_iterator it = songList.begin(); it != songList.end(); ++it)
    {
        emit songRemoved(*it);
    }

    QList<CSong *> songs = getSongs();

    removeAllSongsFromTable();

    QSqlQuery query(m_application->getDataBase());
    query.prepare("DELETE FROM static_list_song WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    addSongs(songs, false);
}


/**
 * Retire les morceaux sélectionnés de la liste.
 * Affiche une confirmation, qui pourra par la suite être désactivée.
 */

void CStaticPlayList::removeSelectedSongs(void)
{
    qDebug() << "CStaticPlayList::removeSongs()";

    // Liste des morceaux sélectionnés
    QModelIndexList indexList = selectionModel()->selectedRows();

    if (indexList.isEmpty())
    {
        return;
    }

    QList<CSongTableItem *> songItemList;

    for (QModelIndexList::const_iterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        CSongTableItem * songItem = m_model->getSongItem(*it);

        if (m_application->getCurrentSongItem() == songItem)
        {
            m_application->stop();
        }

        emit songRemoved(songItem->getSong());
        songItemList.append(songItem);
    }

    removeSongs(songItemList, true);

    selectionModel()->clearSelection();
}


/**
 * Enlève les doublons de la liste.
 *
 * \todo Tester complètement.
 */

void CStaticPlayList::removeDuplicateSongs(void)
{
    qDebug() << "CStaticPlayList::removeDuplicateSongs()";

    QList<CSong *> songs = getSongs();
    QList<CSong *> songsNew;

    for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
    {
        if (songsNew.contains(*it))
        {
            emit songRemoved(*it);
        }
        else
        {
            songsNew.append(*it);
        }
    }

    // Aucun doublon
    if (songs.size() == songsNew.size())
    {
        return;
    }

    removeAllSongsFromTable();
    //addSongsToTable(songsNew);

    QSqlQuery query(m_application->getDataBase());
    query.prepare("DELETE FROM static_list_song WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    addSongs(songsNew, false);
}


/**
 * Met à jour la base de données avec les informations de la liste de lecture.
 * Si la liste n'existe pas en base de données, elle est ajoutée.
 */

bool CStaticPlayList::updateDatabase(void)
{
    // Insertion
    if (m_id <= 0)
    {
        QSqlQuery query(m_application->getDataBase());

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
        query.bindValue(3, "0:40;1:100;2:100;3:100"); // Disposition par défaut \todo => settings

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        m_idPlayList = query.lastInsertId().toInt();

        query.prepare("INSERT INTO static_list (playlist_id) VALUES (?)");
        query.bindValue(0, m_idPlayList);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        m_id = query.lastInsertId().toInt();
    }
    // Mise à jour
    else if (m_isStaticListModified)
    {
/*
        QSqlQuery query(m_application->getDataBase());
        query.prepare("UPDATE playlist SET playlist_name = ? WHERE playlist_id = ?");

        query.bindValue(0, m_name);
        query.bindValue(1, m_idPlayList);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }
*/
    }

    m_isStaticListModified = false;
    return CPlayList::updateDatabase();
}


/**
 * Supprime la liste de la base de données.
 */

void CStaticPlayList::romoveFromDatabase(void)
{
    if (m_id <= 0)
    {
        qWarning() << "CStaticPlayList::romoveFromDatabase() : identifiant invalide";
        return;
    }

    QSqlQuery query(m_application->getDataBase());

    // Suppression des morceaux
    query.prepare("DELETE FROM static_list_song WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    // Suppression de la liste statique
    query.prepare("DELETE FROM static_list WHERE static_list_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    m_isStaticListModified = false;
    m_id = -1;

    CPlayList::romoveFromDatabase();
}


/**
 * Affiche le menu contextuel.
 *
 * \param point Position du clic.
 */

void CStaticPlayList::openCustomMenuProject(const QPoint& point)
{
    qDebug() << "CStaticPlayList::openCustomMenuProject()";

    QModelIndex index = indexAt(point);

    if (index.isValid())
    {
        bool severalSongs = (selectionModel()->selectedRows().size() > 1);

        if (!severalSongs)
        {
            m_selectedItem = m_model->getSongItem(index);
        }

        // Menu contextuel
        QMenu * menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        
        if (!severalSongs)
        {
            menu->addAction(tr("Play")); // TODO...
            menu->addSeparator();
        }

        menu->addAction(tr("Informations"), m_application, SLOT(openDialogSongInfos()));
        if (!severalSongs) menu->addAction(tr("Edit metadata"), m_application, SLOT(openDialogEditMetadata()));
        if (!severalSongs) menu->addAction(tr("Show in explorer"), m_application, SLOT(openSongInExplorer()));
        menu->addSeparator();
        menu->addAction(tr("Remove from playlist"), this, SLOT(removeSelectedSongs()));
        menu->addAction(tr("Remove from library"), this, SLOT(removeSongsFromLibrary()));
        menu->addAction(tr("Check selection"), this, SLOT(checkSelection()));
        menu->addAction(tr("Uncheck selection"), this, SLOT(uncheckSelection()));
        menu->addSeparator();

        if (!severalSongs)
        {
            // Listes de lecture contenant le morceau
            //TODO: gérer les dossiers
            QMenu * menuPlayList = menu->addMenu(tr("Playlists"));
            CSongTable * library = m_application->getLibrary();
            m_actionGoToSongTable[library] = menuPlayList->addAction(QPixmap(":/icons/library"), tr("Library"));
            connect(m_actionGoToSongTable[library], SIGNAL(triggered()), this, SLOT(goToSongTable()));

            QList<CPlayList *> playLists = m_application->getPlayListsWithSong(m_selectedItem->getSong());

            if (playLists.size() > 0)
            {
                menuPlayList->addSeparator();

                for (QList<CPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
                {
                    m_actionGoToSongTable[*it] = menuPlayList->addAction((*it)->getName());
                    connect(m_actionGoToSongTable[*it], SIGNAL(triggered()), this, SLOT(goToSongTable()));

                    if (qobject_cast<CDynamicPlayList *>(*it))
                    {
                        m_actionGoToSongTable[*it]->setIcon(QPixmap(":/icons/dynamic_list"));
                    }
                    else if (qobject_cast<CStaticPlayList *>(*it))
                    {
                        m_actionGoToSongTable[*it]->setIcon(QPixmap(":/icons/playlist"));
                    }
                }
            }
        }

        // Ajouter à la liste de lecture
        //TODO: gérer les dossiers
        QMenu * menuAddToPlayList = menu->addMenu(tr("Add to playlist"));
        QList<CPlayList *> playLists = m_application->getAllPlayLists();

        if (playLists.isEmpty())
        {
            QAction * actionNoPlayList = menuAddToPlayList->addAction(tr("There are no playlist"));
            actionNoPlayList->setEnabled(false);
        }
        else
        {
            for (QList<CPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
            {
                CStaticPlayList * staticList = qobject_cast<CStaticPlayList *>(*it);
                if (staticList)
                {
                    m_actionAddToPlayList[staticList] = menuAddToPlayList->addAction(QPixmap(":/icons/playlist"), (*it)->getName());
                    connect(m_actionAddToPlayList[staticList], SIGNAL(triggered()), this, SLOT(addToPlayList()));
                }
            }
        }

        menu->addSeparator();
        menu->addAction(tr("Remove duplicates"), this, SLOT(removeDuplicateSongs()));

        menu->move(mapToGlobal(point));
        menu->show();
    }
}


/**
 * Gestion des touches du clavier.
 * Les touches Entrée et Supprimer sont gérées.
 *
 * \param event Évènement du clavier.
 */

void CStaticPlayList::keyPressEvent(QKeyEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->key() == Qt::Key_Delete)
    {
        event->accept();
        removeSelectedSongs();
        return;
    }

    return CPlayList::keyPressEvent(event);
}

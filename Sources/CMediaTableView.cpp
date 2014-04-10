/*
Copyright (C) 2012-2014 Teddy Michel

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

#include "CMediaTableView.hpp"
#include "CMediaTableModel.hpp"
#include "CSong.hpp"
#include "CMainWindow.hpp"
#include "CMediaManager.hpp"
#include "IPlayList.hpp"
#include "CMediaTableHeader.hpp"
#include "CStaticList.hpp"
#include "CDynamicList.hpp"
#include "CRatingDelegate.hpp"
#include "CLibrary.hpp"
#include "CQueuePlayList.hpp"
#include "Utils.hpp"

#include <QStringList>
#include <QMouseEvent>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QPainter>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>
#include <QDrag>

#include <QtDebug>


/**
 * Constructeur de la vue.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 */

CMediaTableView::CMediaTableView(CMainWindow * mainWindow) :
QTableView        (mainWindow),
m_mainWindow      (mainWindow),
m_model           (nullptr),
m_idPlayList      (-1),
m_columnSort      (ColArtist),
m_sortOrder       (Qt::AscendingOrder),
m_automaticSort   (true),
m_selectedItem    (nullptr),
m_isModified      (false),
m_isColumnMoving  (false),
m_currentItem     (nullptr),
m_menu            (this)
{
    Q_CHECK_PTR(m_mainWindow);

    setItemDelegate(new CRatingDelegate());
    setEditTriggers(QAbstractItemView::SelectedClicked);

    m_model = new CMediaTableModel(m_mainWindow, this);
    setModel(m_model);

    connect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(onRowCountChange(const QModelIndex&, int, int)));
    connect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(onRowCountChange(const QModelIndex&, int, int)));
    connect(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(sort()));

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    setSizePolicy(sizePolicy);

    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSortingEnabled(true);
    setShowGrid(false);

    // Glisser-déposer
    setDragEnabled(true);

    // Modification des colonnes
    CMediaTableHeader * header = new CMediaTableHeader(this);
    setHorizontalHeader(header);

#if QT_VERSION >= 0x050000
    header->setSectionsMovable(true);
#else
    header->setMovable(true);
#endif

    connect(header, SIGNAL(columnShown(int, bool)), this, SLOT(showColumn(int, bool)));

    connect(m_model, SIGNAL(columnAboutToBeSorted(int, Qt::SortOrder)), this, SLOT(onSortAboutToChange()));
    connect(m_model, SIGNAL(columnSorted(int, Qt::SortOrder)), this, SLOT(sortColumn(int, Qt::SortOrder)));

    verticalHeader()->hide();
    verticalHeader()->setDefaultSectionSize(m_mainWindow->getRowHeight());

    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(onSelectionChange()));

    initColumns("0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120");
}


/**
 * Détruit la liste de morceaux.
 */

CMediaTableView::~CMediaTableView()
{

}


/**
 * Retourne la liste des morceaux de la liste.
 *
 * \return Liste des morceaux.
 */

QList<CSong *> CMediaTableView::getSongs() const
{
    return m_model->getSongs();
}


/**
 * Retourne le premier item correspond à un morceau dans la liste.
 *
 * \param song Morceau à rechercher.
 * \return Pointeur sur l'item, ou nullptr si le morceau n'est pas dans la liste.
 */

CMediaTableItem * CMediaTableView::getFirstSongItem(CSong * song) const
{
    if (!song)
        return nullptr;

    for (QList<CMediaTableItem *>::ConstIterator it = m_model->m_data.begin(); it != m_model->m_data.end(); ++it)
    {
        if ((*it)->getSong() == song)
        {
            return *it;
        }
    }

    return nullptr;
}


/**
 * Retourne le pointeur sur l'item à une ligne donnée.
 *
 * \param row Numéro de la ligne (à partir de 0).
 * \return Pointeur sur l'item, ou nullptr.
 */

CMediaTableItem * CMediaTableView::getSongItemForRow(int row) const
{
    return m_model->getSongItem(row);
}


int CMediaTableView::getRowForSongItem(CMediaTableItem * songItem) const
{
    if (songItem)
        return m_model->getRowForSongItem(songItem);

    return -1;
}


/**
 * Retourne le premier item sélectionné dans la table.
 *
 * \return Pointeur sur l'item, ou nullptr si aucun n'est sélectionné.
 */

CMediaTableItem * CMediaTableView::getSelectedSongItem() const
{
    // On cherche la première ligne sélectionnée
    int row = -1;
    QModelIndexList indexList = selectionModel()->selectedIndexes();

    for (QModelIndexList::ConstIterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        if (row == -1 || it->row() < row)
        {
            row = it->row();
        }
    }

    return (row < 0 ? nullptr : m_model->getSongItem(row));
    //return m_model->getSongItem(selectionModel()->currentIndex());
}


/**
 * Retourne la liste des éléments sélectionnés.
 *
 * \return Liste des éléments sélectionnés.
 */

QList<CMediaTableItem *> CMediaTableView::getSelectedSongItems() const
{
    QList<CMediaTableItem *> songItemList;
    QModelIndexList indexList = selectionModel()->selectedRows();

    for (QModelIndexList::ConstIterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        CMediaTableItem * songItem = m_model->getSongItem(*it);
        if (songItem)
            songItemList.append(songItem);
    }

    return songItemList;
}


/**
 * Retourne la liste des morceaux sélectionnés.
 *
 * \return Liste des morceaux sélectionnés.
 */

QList<CSong *> CMediaTableView::getSelectedSongs() const
{
    QList<CSong *> songs;
    QModelIndexList indexList = selectionModel()->selectedRows();

    for (QModelIndexList::ConstIterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        songs.append(m_model->m_dataFiltered.at(it->row())->getSong());
    }

    return songs;
}


/**
 * Chercher le morceau précédant un autre morceau.
 *
 * \param songItem Morceau actuel, ou nullptr.
 * \param shuffle  Indique si la lecture est aléatoire.
 * \return Morceau précédant, ou nullptr.
 */

CMediaTableItem * CMediaTableView::getPreviousSong(CMediaTableItem * songItem, bool shuffle) const
{
    return m_model->getPreviousSong(songItem, shuffle);
}


/**
 * Chercher le morceau suivant un autre morceau.
 *
 * \param songItem Morceau actuel, ou nullptr.
 * \param shuffle  Indique si la lecture est aléatoire.
 * \return Morceau suivant, ou nullptr.
 */

CMediaTableItem * CMediaTableView::getNextSong(CMediaTableItem * songItem, bool shuffle) const
{
    return m_model->getNextSong(songItem, shuffle);
}


/**
 * Retourne le dernier morceau de la liste.
 *
 * \param shuffle Indique si la lecture est aléatoire.
 * \return Dernier morceau de la liste, ou nullptr si la liste est vide.
 */

CMediaTableItem * CMediaTableView::getLastSong(bool shuffle) const
{
    return m_model->getLastSong(shuffle);
}


/**
 * Calcule la durée totale des morceaux présents dans la liste.
 *
 * \return Durée totale en millisecondes.
 */

qlonglong CMediaTableView::getTotalDuration(bool filtered) const
{
    qlonglong duration = 0;

    QList<CMediaTableItem *> data = (filtered ? m_model->m_dataFiltered : m_model->m_data);

    for (QList<CMediaTableItem *>::ConstIterator songItem = data.begin(); songItem != data.end(); ++songItem)
    {
        duration += (*songItem)->getSong()->getDuration();
    }

    return duration;
}


/**
 * Applique un filtre de recherche à la liste de lecture.
 * Les morceaux qui ne correspondent pas au filtre sont masqués.
 *
 * \param filter Filtre de recherche.
 */

void CMediaTableView::applyFilter(const QString& filter)
{
    m_model->applyFilter(filter);
}


/**
 * Ajoute un morceau à la table.
 * Si le morceau est déjà présent, il est quand même ajouté.
 *
 * \param song Morceau à ajouter.
 * \param pos  Position où placer le morceau. Si négatif, le morceau est ajouté à la fin de la liste.
 */

void CMediaTableView::addSongToTable(CSong * song, int pos)
{
    Q_CHECK_PTR(song);

    m_model->insertRow(song, pos);

    if (m_automaticSort)
    {
        sortByColumn(m_columnSort, m_sortOrder);
    }
}


/**
 * Ajoute plusieurs morceaux à la table.
 * Si l'un des morceaux est déjà présent, il est quand même ajouté.
 *
 * \param songs Liste des morceaux à ajouter.
 */

void CMediaTableView::addSongsToTable(const QList<CSong *>& songs)
{
    for (QList<CSong *>::ConstIterator song = songs.begin(); song != songs.end(); ++song)
    {
        Q_CHECK_PTR(*song);
        m_model->insertRow(*song);
    }

    if (m_automaticSort)
    {
        sortByColumn(m_columnSort, m_sortOrder);
    }
}


/**
 * Enlève une chanson de la liste.
 * Toutes les occurences de \a song sont enlevées de la liste.
 *
 * \param song Pointeur sur la chanson à enlever.
 */

void CMediaTableView::removeSongFromTable(CSong * song)
{
    if (!song)
    {
        m_mainWindow->getMediaManager()->logError(tr("invalid pointer"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    int row = 0;

    for (QList<CMediaTableItem *>::ConstIterator it = m_model->m_data.begin(); it != m_model->m_data.end(); ++it, ++row)
    {
        if ((*it)->getSong() == song)
        {
            //int row = getRowForSongItem(*it);
            m_model->removeRow(row);
        }
    }

    if (m_automaticSort)
    {
        sortByColumn(m_columnSort, m_sortOrder);
    }
}


/**
 * Enlève une chanson de la liste.
 *
 * \param row Position de la chanson dans la liste (à partir de 0).
 */

void CMediaTableView::removeSongFromTable(int row)
{
    if (row < 0)
        return;

    if (row >= m_model->m_data.size())
    {
        m_mainWindow->getMediaManager()->logError(tr("invalid argument"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    m_model->removeRow(row);

    if (m_automaticSort)
    {
        sortByColumn(m_columnSort, m_sortOrder);
    }
}


void CMediaTableView::removeSongsFromTable(const QList<CSong *>& songs)
{
    m_model->removeSongs(songs);

    if (m_automaticSort)
    {
        sortByColumn(m_columnSort, m_sortOrder);
    }
}


/**
 * Enlève tous les morceaux de la table.
 * Aucune modification n'a lieu en base de données, et aucun signal n'est envoyé.
 */

void CMediaTableView::removeAllSongsFromTable()
{
    m_model->clear();
}


/**
 * Initialise la liste des morceaux aléatoires.
 *
 * \param firstSong Morceau à placer au début de la liste.
 */

void CMediaTableView::initShuffle(CMediaTableItem * firstSong)
{
    m_model->initShuffle(firstSong);
}


/**
 * Initialiser la disposition des colonnes de la table.
 *
 * Le format de la chaine est le suivant :
 * Les informations de chaque colonne sont placées dans leur ordre d'affichage, avec un point-virgule comme séparateur.
 * Pour une colonne, on trouve le type de colonne (entier défini dans l'énumération TColumnType), suivant éventuellement
 * d'un signe plus ou moins pour le classement ascendant ou descendant, suivi du signe deux-point, suivi de la
 * largeur de la colonnes en pixels.
 *
 * Par exemple : "1:100;2:100;6+:40", ce qui signifie "Titre" (100px), "Artist" (100px), "Année" (40px), classé par
 * année ascendante.
 *
 * \param str Chaine de caractères contenant la disposition des colonnes.
 */

void CMediaTableView::initColumns(const QString& str)
{
    bool isValid = false;

    for (int col = 0; col < ColNumber; ++col)
    {
        m_columns[col].pos     = -1;
        m_columns[col].width   = horizontalHeader()->defaultSectionSize();
        m_columns[col].visible = false;
    }

    if (!str.isEmpty())
    {
        isValid = true;
        m_columnSort = ColArtist;
        m_sortOrder = Qt::AscendingOrder;
        int colPosition = 0;

        QStringList columnList = str.split(';', QString::SkipEmptyParts);

        for (QStringList::ConstIterator columnInfos = columnList.begin(); columnInfos != columnList.end(); ++columnInfos)
        {
            QStringList columnInfosPart = columnInfos->split(':');

            if (columnInfosPart.size() != 2)
            {
                isValid = false;
                break;
            }

            columnInfosPart[0] = columnInfosPart[0].trimmed();
            QRegExp regExp("([0-9]{1,2})([-+])?");

            if (regExp.indexIn(columnInfosPart[0]) != 0)
            {
                isValid = false;
                break;
            }

            int colType = regExp.capturedTexts()[1].toInt();
            if (colType < 0 || colType >= ColNumber)
            {
                isValid = false;
                break;
            }

            if (regExp.capturedTexts()[2] == "+")
            {
                m_columnSort = colType;
                m_sortOrder = Qt::AscendingOrder;
            }
            else if (regExp.capturedTexts()[2] == "-")
            {
                m_columnSort = colType;
                m_sortOrder = Qt::DescendingOrder;
            }

            QStringList list = regExp.capturedTexts();

            bool ok;
            int width = columnInfosPart[1].toInt(&ok);

            if (!ok)
            {
                isValid = false;
                break;
            }

            // Largeur par défaut
            if (width < 0) width = horizontalHeader()->defaultSectionSize();

            m_columns[colType].pos     = colPosition++;
            m_columns[colType].width   = width;
            m_columns[colType].visible = true;
        }

        for (int col = 0; col < ColNumber; ++col)
        {
            if (!m_columns[col].visible)
            {
                m_columns[col].pos = colPosition++;
            }
        }
    }

    // Disposition par défaut
    //0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120
    if (!isValid)
    {
        for (int col = 0; col < ColNumber; ++col)
        {
            m_columns[col].pos     = col;
            m_columns[col].width   = horizontalHeader()->defaultSectionSize();
            m_columns[col].visible = true;
        }

        m_columnSort = ColArtist;
        m_sortOrder = Qt::AscendingOrder;
    }

    // La colonne 0 est toujours visible, c'est elle qui contient l'indicateur de lecture, et la position du
    // morceau pour les listes statiques. De plus, cela garantie qu'il y a toujours au moins une colonne
    // d'affichée, sans quoi on pourrait faire disparaitre les données et le header en masquant toutes les colonnes.
    m_columns[0].visible = true;

    m_isColumnMoving = true;
    CMediaTableHeader * header = qobject_cast<CMediaTableHeader *>(horizontalHeader());
/*
    QString debugStr1 = "";
    QString debugStr2 = "";
    for (int col = 0; col < ColNumber; ++col)
    {
        debugStr1 += QString::number(horizontalHeader()->visualIndex(col)) + ',';
        debugStr2 += QString::number(m_columns[col].pos) + ',';
    }
    qDebug() << "Colonnes Qt (avant) :" << debugStr1;
    qDebug() << "Colonnes TT (avant) :" << debugStr2;
*/
    for (int col = 0; col < ColNumber; ++col)
    {
        // Redimensionnement
        if (m_columns[col].width <= 0) m_columns[col].width = header->defaultSectionSize();
        header->resizeSection(col, m_columns[col].width);

        // Affichage ou masquage de la colonne
        if (m_columns[col].visible)
        {
            header->showSection(col);
        }
        else
        {
            header->hideSection(col);
        }

        if (col > 0 && col < ColNumber)
        {
            header->m_actionShowCol[col]->setChecked(m_columns[col].visible);
        }

        // Déplacement de la colonne
        int logical = -1;
        for (int j = 0; j < ColNumber; ++j)
        {
            if (m_columns[j].pos == col)
            {
                logical = j;
                break;
            }
        }

        int visualIndex = header->visualIndex(logical);
        header->moveSection(visualIndex, col);
    }
/*
    debugStr1 = "";
    debugStr2 = "";
    for (int col = 0; col < ColNumber; ++col)
    {
        debugStr1 += QString::number(horizontalHeader()->visualIndex(col)) + ',';
        debugStr2 += QString::number(m_columns[col].pos) + ',';
    }
    qDebug() << "Colonnes Qt (apres) :" << debugStr1;
    qDebug() << "Colonnes TT (apres) :" << debugStr2;
*/
    m_isColumnMoving = false;

    sortByColumn(m_columnSort, m_sortOrder);

    //horizontalHeader()->hideSection(0);
}


/**
 * Affiche ou masque une colonne.
 *
 * \param column Numéro de la colonne.
 * \param show   Indique si la colonne doit être affiché ou masqué.
 */

void CMediaTableView::showColumn(int column, bool show)
{
    if (column < 0 || column >= ColNumber)
        return;

    if (m_columns[column].visible != show)
    {
        m_columns[column].visible = show;

        int numColumns;

        if (show)
        {
            numColumns = -1;
            horizontalHeader()->showSection(column);

        }
        else
        {
            numColumns = 0;
            horizontalHeader()->hideSection(column);
        }

        //QString debugStr1 = "";
        //QString debugStr2 = "";
        for (int col = 0; col < ColNumber; ++col)
        {
            if (m_columns[col].visible)
            {
                ++numColumns;
            }

            //debugStr1 += QString::number(horizontalHeader()->visualIndex(col)) + ',';
            //debugStr2 += QString::number(m_columns[col].pos) + ',';
        }
        //qDebug() << "Colonnes Qt (avant) :" << debugStr1;
        //qDebug() << "Colonnes TT (avant) :" << debugStr2;

        int visualIndex = horizontalHeader()->visualIndex(column);

        // Déplacement de la colonne
        horizontalHeader()->moveSection(visualIndex, numColumns);
/*
        debugStr1 = "";
        debugStr2 = "";
        for (int col = 0; col < ColNumber; ++col)
        {
            debugStr1 += QString::number(horizontalHeader()->visualIndex(col)) + ',';
            debugStr2 += QString::number(m_columns[col].pos) + ',';
        }
        qDebug() << "Colonnes Qt (apres) :" << debugStr1;
        qDebug() << "Colonnes TT (apres) :" << debugStr2;
*/
        m_isModified = true;
    }
}


/**
 * Méthode appellée lorsque le tri des colonnes va changé.
 * Mémorise les lignes sélectionnées.
 */

void CMediaTableView::onSortAboutToChange()
{
    m_selectedItems = getSelectedSongItems();
    m_currentItem = m_model->getSongItem(selectionModel()->currentIndex());
}


/**
 * Tri la table selon une colonne.
 *
 * \param column Numéro de la colonne.
 * \param order  Ordre croissant ou décroissant.
 */

void CMediaTableView::sortColumn(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= ColNumber)
        return;

    if (m_columnSort != column || m_sortOrder != order)
    {
        m_columnSort = column;
        m_sortOrder = order;

        m_isModified = true;

        // Mise-à-jour de la sélection
        selectionModel()->clearSelection();

        for (QList<CMediaTableItem *>::ConstIterator songItem = m_selectedItems.begin(); songItem != m_selectedItems.end(); ++songItem)
        {
            selectionModel()->select(m_model->index(m_model->getRowForSongItem(*songItem), 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }

        if (m_currentItem)
            selectionModel()->setCurrentIndex(m_model->index(m_model->getRowForSongItem(m_currentItem), 0), QItemSelectionModel::Rows);
    }
}


/**
 * Tri la liste des morceaux selon la colonne et l'ordre choisis par l'utilisateur.
 */

void CMediaTableView::sort()
{
    m_model->sort(m_columnSort, m_sortOrder);
}


/**
 * Affiche le morceau sélectionné dans une des listes de lecture qui le contient.
 */

void CMediaTableView::goToSongTable()
{
    QAction * action = qobject_cast<QAction *>(sender());

    if (action)
    {
        CMediaTableView * songTable = m_actionGoToSongTable.key(action);

        if (songTable)
        {
            m_mainWindow->selectSong(songTable, songTable->getFirstSongItem(m_selectedItem->getSong()));
        }
    }
}


/**
 * Ajoute les morceaux sélectionnés à une liste de lecture.
 */

void CMediaTableView::addToPlayList()
{
    QAction * action = qobject_cast<QAction *>(sender());

    if (action)
    {
        CStaticList * playList = m_actionAddToPlayList.key(action);

        if (playList)
        {
            // Liste des morceaux sélectionnés
            QModelIndexList indexList = selectionModel()->selectedRows();

            if (indexList.isEmpty())
                return;

            QList<CSong *> songList;

            for (QModelIndexList::ConstIterator index = indexList.begin(); index != indexList.end(); ++index)
            {
                CMediaTableItem * songItem = m_model->getSongItem(*index);

                if (!songList.contains(songItem->getSong()))
                {
                    songList.append(songItem->getSong());
                }
            }

            playList->addSongs(songList);

            //m_mainWindow->selectSong(songTable, songTable->getFirstSongItem(m_selectedItem->getSong()));
        }
    }
}


/**
 * Retire les morceaux sélectionnés de la médiathèque.
 */

void CMediaTableView::removeSongsFromLibrary()
{
    // Liste des morceaux sélectionnés
    QModelIndexList indexList = selectionModel()->selectedRows();

    QList<CSong *> songList;
    bool needStop = false;

    for (QModelIndexList::ConstIterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        CMediaTableItem * songItem = m_model->getSongItem(*it);

        if (m_mainWindow->getCurrentSongItem() == songItem)
            needStop = true;

        if (!songList.contains(songItem->getSong()))
            songList.append(songItem->getSong());
    }

    if (songList.isEmpty())
        return;

    // Confirmation
    if (QMessageBox::question(this, QString(), tr("Are you sure you want to remove the selected songs from the library?\nThe files will not be deleted."), tr("Remove"), tr("Cancel"), 0, 1) == 1)
        return;

    if (needStop)
        m_mainWindow->stop();

    m_mainWindow->removeSongs(songList);

    selectionModel()->clearSelection();
}


/**
 * Renomme automatiquement les fichiers des morceaux sélectionnés.
 */

void CMediaTableView::moveSongs()
{
    // Liste des morceaux sélectionnés
    QModelIndexList indexList = selectionModel()->selectedRows();

    QList<CSong *> songList;

    for (QModelIndexList::ConstIterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        CMediaTableItem * songItem = m_model->getSongItem(*it);
        CSong * song = songItem->getSong();

        // Un même morceau peut se trouver plusieurs fois dans une liste statique
        if (!songList.contains(song))
        {
            songList.append(song);
        }
    }

    if (songList.isEmpty())
    {
        return;
    }

    for (QList<CSong *>::ConstIterator it = songList.begin(); it != songList.end(); ++it)
    {
        (*it)->moveFile();
    }
}


/**
 * Analyse les métadonnées des morceaux sélectionnés.
 */

void CMediaTableView::analyzeSongs()
{
    // Liste des morceaux sélectionnés
    QModelIndexList indexList = selectionModel()->selectedRows();

    QList<CSong *> songList;

    for (QModelIndexList::ConstIterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        CMediaTableItem * songItem = m_model->getSongItem(*it);
        CSong * song = songItem->getSong();

        // Un même morceau peut se trouver plusieurs fois dans une liste statique
        if (!songList.contains(song))
        {
            songList.append(song);
        }
    }

    if (songList.isEmpty())
    {
        return;
    }

    // Boite de dialogue
    QProgressDialog progress(tr("Analysis files..."), tr("Abort"), 0, songList.size(), this);
    progress.setModal(true);
    progress.setMinimumDuration(2000);
    int i = 0;

    for (QList<CSong *>::ConstIterator it = songList.begin(); it != songList.end(); ++it)
    {
        progress.setValue(i++);

        (*it)->loadTags();
        qApp->processEvents();

        (*it)->updateDatabase();
        qApp->processEvents();

        (*it)->writeTags();
        qApp->processEvents();

        if (progress.wasCanceled())
        {
            break;
        }
    }
}


/**
 * Coche tous les morceaux sélectionnés dans la liste.
 */

void CMediaTableView::checkSelection()
{
    // Liste des morceaux sélectionnés
    QModelIndexList indexList = selectionModel()->selectedRows();

    if (indexList.isEmpty())
    {
        return;
    }

    for (QModelIndexList::ConstIterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        m_model->m_dataFiltered.at(it->row())->getSong()->setEnabled(true);
    }
}


/**
 * Décoche tous les morceaux sélectionnés dans la liste.
 */

void CMediaTableView::uncheckSelection()
{
    // Liste des morceaux sélectionnés
    QModelIndexList indexList = selectionModel()->selectedRows();

    if (indexList.isEmpty())
    {
        return;
    }

    for (QModelIndexList::ConstIterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        m_model->m_dataFiltered.at(it->row())->getSong()->setEnabled(false);
    }
}


void CMediaTableView::changeCurrentSongRating(int rating)
{
    // Liste des morceaux sélectionnés
    QModelIndexList indexList = selectionModel()->selectedRows();

    if (indexList.size() != 1)
    {
        return;
    }

    CSong * song = m_model->m_dataFiltered.at(indexList.at(0).row())->getSong();

    if (song != nullptr)
    {
        song->setRating(rating);
    }
}


void CMediaTableView::changeCurrentSongRatingTo0()
{
    changeCurrentSongRating(0);
}


void CMediaTableView::changeCurrentSongRatingTo1()
{
    changeCurrentSongRating(1);
}


void CMediaTableView::changeCurrentSongRatingTo2()
{
    changeCurrentSongRating(2);
}


void CMediaTableView::changeCurrentSongRatingTo3()
{
    changeCurrentSongRating(3);
}


void CMediaTableView::changeCurrentSongRatingTo4()
{
    changeCurrentSongRating(4);
}


void CMediaTableView::changeCurrentSongRatingTo5()
{
    changeCurrentSongRating(5);
}


void CMediaTableView::onRowCountChange(const QModelIndex& parent, int start, int end)
{
    emit rowCountChanged();
}


/**
 * Méthode appellée lorsque la sélection change.
 */

void CMediaTableView::onSelectionChange()
{
    // On vérifie que cette table est actuellement affichée
    if (m_mainWindow->getDisplayedSongTable() != this)
    {
        return;
    }

    QList<CMediaTableItem *> songItems = getSelectedSongItems();

    const int numSongs = songItems.size();

    if (numSongs <= 0)
    {
        return;
    }

    // Durée totale sélectionnée
    qlonglong durationMS = 0;

    for (QList<CMediaTableItem *>::ConstIterator it = songItems.begin(); it != songItems.end(); ++it)
    {
        durationMS += (*it)->getSong()->getDuration();
    }

    m_mainWindow->setSelectionInformations(numSongs, durationMS);
}


void CMediaTableView::addSongToQueueBegining()
{
    // Liste des morceaux sélectionnés
    QList<CSong *> songs = getSelectedSongs();

    if (songs.isEmpty())
    {
        return;
    }

    CQueuePlayList * queue = m_mainWindow->getQueue();
    queue->addSongs(songs, 0);
}


void CMediaTableView::addSongToQueueEnd()
{
    // Liste des morceaux sélectionnés
    QList<CSong *> songs = getSelectedSongs();

    if (songs.isEmpty())
    {
        return;
    }

    CQueuePlayList * queue = m_mainWindow->getQueue();
    queue->addSongs(songs, -1);
}


QString CMediaTableView::getColumnsInfos() const
{
    QString str;
    int currentPos = 0;

    for (int i = 0; i < ColNumber && currentPos < ColNumber; ++i)
    {
        if (m_columns[i].pos == currentPos && m_columns[i].visible)
        {
            if (currentPos > 0) str += ";";

            if (m_columnSort == i)
            {
                if (m_sortOrder == Qt::AscendingOrder)
                {
                    str += QString("%1+:%2").arg(i).arg(m_columns[i].width);
                }
                else
                {
                    str += QString("%1-:%2").arg(i).arg(m_columns[i].width);
                }
            }
            else
            {
                str += QString("%1:%2").arg(i).arg(m_columns[i].width);
            }

            ++currentPos;
            i = -1;
        }
    }

    if (str.isEmpty())
    {
        //m_mainWindow->getSettings()->value("Preferences/ColumnsDefault", "0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120").toString();
        str = "0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120"; // Disposition par défaut
    }

    return str;
}


/*
void CMediaTableView::mousePressEvent(QMouseEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->button() == Qt::RightButton)
    {
        QPoint pt = event->pos();
        QModelIndex index = this->indexAt(pt);

        if (index.isValid())
        {
            qDebug() << "Clic droit sur la table";
            selectRow(index.row());
        }
        else
        {
            qDebug() << "Clic droit en dehors de la table";
        }
    }
    else if (event->button() == Qt::LeftButton)
    {
        QPoint pt = event->pos();
        QModelIndex index = this->indexAt(pt);

        if (index.isValid())
        {
            qDebug() << "Clic droit sur la table";
            selectRow(index.row());
            emit songSelected(index.row());
        }
        else
        {
            qDebug() << "Clic droit en dehors de la table";
        }
    }

    QTableView::mousePressEvent(event);
}
*/

void CMediaTableView::columnMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    if (!m_isColumnMoving)
    {
        if (oldVisualIndex < newVisualIndex)
        {
            for (int i = 0; i < ColNumber; ++i)
            {
                if (m_columns[i].pos > oldVisualIndex && m_columns[i].pos <= newVisualIndex)
                {
                    --(m_columns[i].pos);
                }
            }
        }
        else
        {
            for (int i = 0; i < ColNumber; ++i)
            {
                if (m_columns[i].pos >= newVisualIndex && m_columns[i].pos < oldVisualIndex)
                {
                    ++(m_columns[i].pos);
                }
            }
        }

        m_columns[logicalIndex].pos = newVisualIndex;

        m_isModified = true;
    }

    emit columnChanged();

    QTableView::columnMoved(logicalIndex, oldVisualIndex, newVisualIndex);
}


void CMediaTableView::columnResized(int logicalIndex, int oldSize, int newSize)
{
    m_columns[logicalIndex].width = newSize;

    m_isModified = true;
    emit columnChanged();

    QTableView::columnResized(logicalIndex, oldSize, newSize);
}


/**
 * Met à jour la base de données avec les informations de la table.
 *
 * \return Booléen indiquant le succès de l'opération.
 */

bool CMediaTableView::updateDatabase()
{
    if (m_isModified)
    {
        if (m_idPlayList < 0)
        {
            m_mainWindow->getMediaManager()->logError(tr("invalid identifier (%1)").arg(m_idPlayList), __FUNCTION__, __FILE__, __LINE__);
            return false;
        }

        QString colInfos = getColumnsInfos();

        QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());

        query.prepare("UPDATE playlist SET list_columns = ? WHERE playlist_id = ?");
        query.bindValue(0, colInfos);
        query.bindValue(1, m_idPlayList);

        if (!query.exec())
        {
            m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        m_isModified = false;
        initColumns(colInfos);
    }

    return true;
}


void CMediaTableView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();
    QList<int> rows;

    for (int i = indexes.count() - 1 ; i >= 0; --i)
    {
        if (!rows.contains(indexes.at(i).row()))
        {
            rows.append(indexes.at(i).row());
        }

        if (!(m_model->flags(indexes.at(i)) & Qt::ItemIsDragEnabled)) // utiliser m_dataFiltered ?
        {
            indexes.removeAt(i);
        }
    }

    if (indexes.count() > 0)
    {
        QMimeData * data = m_model->mimeData(indexes); // utiliser m_dataFiltered ?
        if (!data) return;

        QPixmap pixmap(64, 32);
        pixmap.fill(QColor(0, 0, 0, 0));
        {
            QPainter painter(&pixmap);
            painter.drawImage(QRect(QPoint(0, 0), QPoint(32, 32)), QImage(":/icons/song"));
            painter.drawText(QPoint(34, 26), QString::number(rows.size()));
        }

        QDrag * drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);

        //drag->setDragCursor(QPixmap(":/icons/play"), Qt::CopyAction);

        // Curseur...
        //drag->setDragCursor(const QPixmap& cursor, Qt::DropAction action);
        //CopyAction, MoveAction or LinkAction. All other values of DropAction

        //drag->setHotSpot(m_pressedPosition - rect.topLeft());
        drag->setHotSpot(QPoint(16, 16)); // Décalage vers le haut à gauche.

        //Qt::DropAction defaultDropAction0 = Qt::IgnoreAction;
        Qt::DropAction defaultDropAction = this->defaultDropAction();
/*
        if (defaultDropAction != Qt::IgnoreAction && (supportedActions & defaultDropAction))
            defaultDropAction0 = defaultDropAction;
        else if (supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove)
            defaultDropAction0 = Qt::CopyAction;
*/
        drag->exec(supportedActions, defaultDropAction);

        //if (drag->exec(supportedActions, defaultDropAction) == Qt::MoveAction)
        //    d->clearOrRemove();
    }

    //QAbstractItemView::startDrag(supportedActions);
}


/**
 * Gestion des touches du clavier.
 * Les touches Entrée et Supprimer sont gérées.
 *
 * \param event Évènement du clavier.
 */

void CMediaTableView::keyPressEvent(QKeyEvent * event)
{
    Q_CHECK_PTR(event);

    // Lancer la lecture du morceau courant
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        event->accept();

        // Un morceau est en cours de lecture
        if (!m_mainWindow->isStopped())
        {
            // Le morceau sélectionné est en cours de lecture
            if (getSelectedSongItem() == m_mainWindow->getCurrentSongItem())
            {
                // La lecture n'est pas en pause, rien à faire
                if (m_mainWindow->isPlaying())
                    return;
            }
            else
            {
                // Un autre morceau est en cours de lecture, on l'arrête
                m_mainWindow->stop();
            }

        }

        // On démarre la lecture
        m_mainWindow->play();
        return;
    }

    // Espace : changer l'état de la lecture
    // Normalement ce raccourci est géré par l'action actionTogglePlay définie dans TMediaPlayer.ui
    if (event->key() == Qt::Key_Space)
    {
        event->accept();
        m_mainWindow->togglePlay();
        return;
    }

    // Suppression du morceau courant
    if (event->key() == Qt::Key_Delete)
    {
        event->accept();
        removeSongsFromLibrary();
        return;
    }

    return QTableView::keyPressEvent(event);
}


/**
 * Gestion des double-clics.
 *
 * \param event Évènement de la souris.
 */

void CMediaTableView::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->button() == Qt::LeftButton)
    {
        QPoint pt = event->pos();
        QModelIndex index = indexAt(pt);

        if (index.isValid())
        {
            selectRow(index.row());
            emit songStarted(m_model->getSongItem(index));
            event->accept();
        }
    }

    QTableView::mouseDoubleClickEvent(event);
}


void CMediaTableView::contextMenuEvent(QContextMenuEvent * event)
{
    Q_CHECK_PTR(event);

    QModelIndex index = indexAt(event->pos());
    m_selectedItem = nullptr;

    if (index.isValid())
    {
        int numSelectedSongs = selectionModel()->selectedRows().size();
        bool severalSongs = (numSelectedSongs > 1);

        if (!severalSongs)
        {
            m_selectedItem = m_model->getSongItem(index);
        }

        m_menu.clear();

        if (!severalSongs)
        {
            QAction * actionPlay;

            CMediaTableItem * currentSong = m_mainWindow->getCurrentSongItem();

            // Le morceau est en cours de lecture
            if (currentSong && currentSong == m_selectedItem)
            {
                if (m_mainWindow->isPaused())
                {
                    actionPlay = m_menu.addAction(tr("Play"), m_mainWindow, SLOT(play()));
                }
                else
                {
                    actionPlay = m_menu.addAction(tr("Pause"), m_mainWindow, SLOT(pause()));
                }
            }
            // Le morceau est en cours de lecture dans une autre liste
            else if (currentSong && currentSong->getSong() == m_selectedItem->getSong())
            {
                actionPlay = m_menu.addAction(tr("Play in this playlist"), this, SLOT(changeCurrentSongList()));
            }
            else
            {
                actionPlay = m_menu.addAction(tr("Play"), this, SLOT(playSelectedSong()));
            }

            m_menu.setDefaultAction(actionPlay);
            m_menu.addSeparator();
        }

        m_menu.addAction(tr("Informations..."), m_mainWindow, SLOT(openDialogSongInfos()));

        if (!severalSongs && canEditSongs())
        {
            // Note du morceau
            QMenu * menuRating = m_menu.addMenu(tr("Rating"));

            // TODO: utiliser des pixmaps à la place de texte
            QAction * actionRating0 = menuRating->addAction(QString::fromUtf8("☆☆☆☆☆"), this, SLOT(changeCurrentSongRatingTo0()));
            QAction * actionRating1 = menuRating->addAction(QString::fromUtf8("★☆☆☆☆"), this, SLOT(changeCurrentSongRatingTo1()));
            QAction * actionRating2 = menuRating->addAction(QString::fromUtf8("★★☆☆☆"), this, SLOT(changeCurrentSongRatingTo2()));
            QAction * actionRating3 = menuRating->addAction(QString::fromUtf8("★★★☆☆"), this, SLOT(changeCurrentSongRatingTo3()));
            QAction * actionRating4 = menuRating->addAction(QString::fromUtf8("★★★★☆"), this, SLOT(changeCurrentSongRatingTo4()));
            QAction * actionRating5 = menuRating->addAction(QString::fromUtf8("★★★★★"), this, SLOT(changeCurrentSongRatingTo5()));

            actionRating0->setCheckable(true);
            actionRating1->setCheckable(true);
            actionRating2->setCheckable(true);
            actionRating3->setCheckable(true);
            actionRating4->setCheckable(true);
            actionRating5->setCheckable(true);

            QActionGroup * group = new QActionGroup(menuRating);
            group->addAction(actionRating0);
            group->addAction(actionRating1);
            group->addAction(actionRating2);
            group->addAction(actionRating3);
            group->addAction(actionRating4);
            group->addAction(actionRating5);

            int rating = m_selectedItem->getSong()->getRating();

                 if (rating == 0) actionRating0->setChecked(true);
            else if (rating == 1) actionRating1->setChecked(true);
            else if (rating == 2) actionRating2->setChecked(true);
            else if (rating == 3) actionRating3->setChecked(true);
            else if (rating == 4) actionRating4->setChecked(true);
            else if (rating == 5) actionRating5->setChecked(true);
        }

        if (canImportSongs())
        {
            m_menu.addAction(tr("Import..."), this, SLOT(importSelectedSongs()))->setEnabled(false);
            //m_menu.addAction(tr("Import..."), this, SLOT(importSelectedSongs()));
        }

        if (!severalSongs)
        {
            if (canEditSongs())
            {
                m_menu.addAction(tr("Edit metadata..."), m_mainWindow, SLOT(openDialogEditMetadata()));
            }

            m_menu.addAction(tr("Show in explorer"), m_mainWindow, SLOT(openSongInExplorer()));

            if (canEditSongs() && m_selectedItem->getSong()->getFileStatus() == false)
            {
                m_menu.addAction(tr("Relocate"), m_mainWindow, SLOT(relocateSong()));
            }
        }

        m_menu.addSeparator();

        if (canEditPlayList())
        {
            m_menu.addAction(tr("Remove from playlist"), this, SLOT(removeSelectedSongs()));
        }

        if (canEditSongs())
        {
            m_menu.addAction(tr("Remove from library"), this, SLOT(removeSongsFromLibrary()));
            m_menu.addAction(tr("Rename file(s) automatically", "", numSelectedSongs), this, SLOT(moveSongs()));
            m_menu.addAction(tr("Analyze file(s)", "", numSelectedSongs), this, SLOT(analyzeSongs()));

/*
            if (severalSongs)
            {
                m_menu.addAction(tr("Rename file(s) automatically", "", numSelectedSongs), this, SLOT(moveSongs()));
                m_menu.addAction(tr("Analyze files"), this, SLOT(analyzeSongs()));
            }
            else
            {
                m_menu.addAction(tr("Rename file automatically"), this, SLOT(moveSongs()));
                m_menu.addAction(tr("Analyze file"), this, SLOT(analyzeSongs()));
            }
*/
        }

        if (!severalSongs)
        {
            QAction * actionCheck = m_menu.addAction(tr("Check song"), this, SLOT(checkSelection()));
            QAction * actionUncheck = m_menu.addAction(tr("Uncheck song"), this, SLOT(uncheckSelection()));

            bool songIsChecked = m_selectedItem->getSong()->isEnabled();

            if (songIsChecked)
            {
                actionCheck->setEnabled(false);
            }
            else
            {
                actionUncheck->setEnabled(false);
            }
        }
        else
        {
            m_menu.addAction(tr("Check selection"), this, SLOT(checkSelection()));
            m_menu.addAction(tr("Uncheck selection"), this, SLOT(uncheckSelection()));
        }

        if (canMoveToPlayList())
        {
            m_menu.addSeparator();

            if (!severalSongs)
            {
                // File d'attente
                QMenu * menuQueue = m_menu.addMenu(tr("Queue"));
                menuQueue->addAction(tr("Add at the beginning"), this, SLOT(addSongToQueueBegining()));
                menuQueue->addAction(tr("Add at the end"), this, SLOT(addSongToQueueEnd()));

                // Listes de lecture contenant le morceau
                //TODO: gérer les dossiers
                QMenu * menuPlayList = m_menu.addMenu(tr("Playlists"));
                CMediaTableView * library = m_mainWindow->getLibrary();
                m_actionGoToSongTable[library] = menuPlayList->addAction(QPixmap(":/icons/library"), tr("Library"));
                connect(m_actionGoToSongTable[library], SIGNAL(triggered()), this, SLOT(goToSongTable()));

                QList<IPlayList *> playLists = m_mainWindow->getPlayListsWithSong(m_selectedItem->getSong());

                if (playLists.size() > 0)
                {
                    menuPlayList->addSeparator();

                    for (QList<IPlayList *>::ConstIterator it = playLists.begin(); it != playLists.end(); ++it)
                    {
                        m_actionGoToSongTable[*it] = menuPlayList->addAction((*it)->getName());
                        connect(m_actionGoToSongTable[*it], SIGNAL(triggered()), this, SLOT(goToSongTable()));

                        if (qobject_cast<CDynamicList *>(*it))
                        {
                            m_actionGoToSongTable[*it]->setIcon(QPixmap(":/icons/dynamic_list"));
                        }
                        else if (qobject_cast<CStaticList *>(*it))
                        {
                            m_actionGoToSongTable[*it]->setIcon(QPixmap(":/icons/playlist"));
                        }
                    }
                }
            }

            // Ajouter à la liste de lecture
            //TODO: gérer les dossiers
            QMenu * menuAddToPlayList = m_menu.addMenu(tr("Add to playlist"));
            QList<IPlayList *> playLists = m_mainWindow->getAllPlayLists();

            if (playLists.isEmpty())
            {
                QAction * actionNoPlayList = menuAddToPlayList->addAction(tr("There are no playlist"));
                actionNoPlayList->setEnabled(false);
            }
            else
            {
                for (QList<IPlayList *>::ConstIterator it = playLists.begin(); it != playLists.end(); ++it)
                {
                    CStaticList * staticList = qobject_cast<CStaticList *>(*it);
                    if (staticList)
                    {
                        m_actionAddToPlayList[staticList] = menuAddToPlayList->addAction(QPixmap(":/icons/playlist"), (*it)->getName());
                        connect(m_actionAddToPlayList[staticList], SIGNAL(triggered()), this, SLOT(addToPlayList()));
                    }
                }
            }
        }

        if (canEditPlayList())
        {
            m_menu.addSeparator();
            m_menu.addAction(tr("Remove duplicates"), this, SLOT(removeDuplicateSongs()));
        }

        m_menu.move(getCorrectMenuPosition(&m_menu, event->globalPos()));
        m_menu.show();
    }
}


/**
 * Indique si la liste de morceaux a été modifiée.
 *
 * \return Booléen.
 */

bool CMediaTableView::isModified() const
{
    return m_isModified;
}


void CMediaTableView::replaceSong(CSong * oldSong, CSong * newSong)
{
    m_model->replaceSong(oldSong, newSong);
}


/**
 * Sélectionne un morceau dans la liste.
 *
 * \param songItem Morceau à sélectionner.
 */

void CMediaTableView::selectSongItem(CMediaTableItem * songItem)
{
    selectionModel()->clear();

    if (songItem)
    {
        QModelIndex index = m_model->index(m_model->getRowForSongItem(songItem), 0);
        setCurrentIndex(index);
        scrollTo(index/*, QAbstractItemView::PositionAtTop*/);
    }
}


void CMediaTableView::changeCurrentSongList()
{
    m_mainWindow->changeCurrentSongList(m_selectedItem, this);
}


/**
 * Slot utilisé pour lancer la lecture du morceau sélectionné.
 */

void CMediaTableView::playSelectedSong()
{
    m_mainWindow->playSong(m_selectedItem);
}

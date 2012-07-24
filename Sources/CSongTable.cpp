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

#include "CSongTable.hpp"
#include "CSongTableModel.hpp"
#include "CSong.hpp"
#include "CApplication.hpp"
#include "IPlayList.hpp"
#include "CSongTableHeader.hpp"
#include "CStaticPlayList.hpp"
#include "CDynamicList.hpp"
#include "CRatingDelegate.hpp"
#include "CLibrary.hpp"
#include <QStringList>
#include <QMouseEvent>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QMenu>
#include <QPainter>
#include <QMessageBox>

#include <QtDebug>


/**
 * Constructeur de la vue.
 *
 * \param application Pointeur sur l'application.
 */

CSongTable::CSongTable(CApplication * application) :
    QTableView        (application),
    m_application     (application),
    m_model           (NULL),
    m_idPlayList      (-1),
    m_columnSort      (ColArtist),
    m_sortOrder       (Qt::AscendingOrder),
    m_automaticSort   (true),
    m_selectedItem    (NULL),
    m_isModified      (false),
    m_isColumnMoving  (false)
{
    Q_CHECK_PTR(application);

    setItemDelegate(new CRatingDelegate());
    setEditTriggers(QAbstractItemView::SelectedClicked);

    m_model = new CSongTableModel(m_application, this);
    setModel(m_model);

    connect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(onRowCountChange(const QModelIndex&, int, int)));
    connect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(onRowCountChange(const QModelIndex&, int, int)));
    connect(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(sort()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openCustomMenuProject(const QPoint&)));

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
    CSongTableHeader * header = new CSongTableHeader(this);
    setHorizontalHeader(header);
    header->setMovable(true);
    connect(header, SIGNAL(columnShown(int, bool)), this, SLOT(showColumn(int, bool)));
    connect(m_model, SIGNAL(columnSorted(int, Qt::SortOrder)), this, SLOT(sortColumn(int, Qt::SortOrder)));

    verticalHeader()->hide();
    verticalHeader()->setDefaultSectionSize(m_application->getRowHeight());

    initColumns("0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120");
}


/**
 * Détruit la liste de morceaux.
 */

CSongTable::~CSongTable()
{
    //qDebug() << "CSongTable::~CSongTable()";
}


/**
 * Retourne la liste des morceaux de la liste.
 *
 * \return Liste des morceaux.
 */

QList<CSong *> CSongTable::getSongs(void) const
{
    return m_model->getSongs();
}


/**
 * Retourne le premier item correspond à un morceau dans la liste.
 *
 * \param song Morceau à rechercher.
 * \return Pointeur sur l'item, ou NULL si le morceau n'est pas dans la liste.
 */

CSongTableItem * CSongTable::getFirstSongItem(CSong * song) const
{
    if (!song)
    {
        return NULL;
    }

    for (QList<CSongTableItem *>::const_iterator it = m_model->m_data.begin(); it != m_model->m_data.end(); ++it)
    {
        if ((*it)->getSong() == song)
        {
            return *it;
        }
    }

    return NULL;
}


/**
 * Retourne le pointeur sur l'item à une ligne donnée.
 *
 * \param pos Numéro de la ligne (à partir de 0).
 * \return Pointeur sur l'item, ou NULL.
 */

CSongTableItem * CSongTable::getSongItemForRow(int row) const
{
    return m_model->getSongItem(row);
}


int CSongTable::getRowForSongItem(CSongTableItem * songItem) const
{
    Q_CHECK_PTR(songItem);
    return m_model->getRowForSongItem(songItem);
}


/**
 * Retourne le premier item sélectionné dans la table.
 *
 * \return Pointeur sur l'item, ou NULL si aucun n'est sélectionné.
 */

CSongTableItem * CSongTable::getSelectedSongItem(void) const
{
    // On cherche la première ligne sélectionnée
    int row = -1;
    QModelIndexList indexList = selectionModel()->selectedIndexes();

    for (QModelIndexList::const_iterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        if (row == -1 || it->row() < row)
        {
            row = it->row();
        }
    }

    return (row < 0 ? NULL : m_model->getSongItem(row));
    //return m_model->getSongItem(selectionModel()->currentIndex());
}


QList<CSongTableItem *> CSongTable::getSelectedSongItems(void) const
{
    QList<CSongTableItem *> songItemList;
    QModelIndexList indexList = selectionModel()->selectedRows();

    foreach (QModelIndex index, indexList)
    {
        CSongTableItem * songItem = m_model->getSongItem(index);
        if (songItem) songItemList.append(songItem);
    }

    return songItemList;
}


/**
 * Chercher le morceau précédant un autre morceau.
 *
 * \param songItem Morceau actuel, ou NULL.
 * \param shuffle  Indique si la lecture est aléatoire.
 * \return Morceau précédant, ou NULL.
 */

CSongTableItem * CSongTable::getPreviousSong(CSongTableItem * songItem, bool shuffle) const
{
    return m_model->getPreviousSong(songItem, shuffle);
}


/**
 * Chercher le morceau suivant un autre morceau.
 *
 * \param songItem Morceau actuel, ou NULL.
 * \param shuffle  Indique si la lecture est aléatoire.
 * \return Morceau suivant, ou NULL.
 */

CSongTableItem * CSongTable::getNextSong(CSongTableItem * songItem, bool shuffle) const
{
    return m_model->getNextSong(songItem, shuffle);
}


/**
 * Calcule la durée totale des morceaux présents dans la liste.
 *
 * \return Durée totale en millisecondes.
 */

int CSongTable::getTotalDuration(void) const
{
    int duration = 0;

    foreach (CSongTableItem * songItem, m_model->m_data)
    {
        duration += songItem->getSong()->getDuration();
    }

    return duration;
}


/**
 * Ajoute un morceau à la table.
 * Si le morceau est déjà présent, il est quand même ajouté.
 *
 * \param song Morceau à ajouter.
 * \param pos  Position où placer le morceau. Si négatif, le morceau est ajouté à la fin de la liste.
 */

void CSongTable::addSongToTable(CSong * song, int pos)
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

void CSongTable::addSongsToTable(const QList<CSong *>& songs)
{
    for (QList<CSong *>::const_iterator song = songs.begin(); song != songs.end(); ++song)
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
 * \todo Mettre à jour le modèle.
 *
 * \param song Pointeur sur la chanson à enlever.
 */

void CSongTable::removeSongFromTable(CSong * song)
{
    Q_CHECK_PTR(song);

    int row = 0;

    for (QList<CSongTableItem *>::const_iterator it = m_model->m_data.begin(); it != m_model->m_data.end(); ++it, ++row)
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

void CSongTable::removeSongFromTable(int row)
{
    Q_ASSERT(row >= 0 && row < m_model->m_data.size());

    m_model->removeRow(row);

    if (m_automaticSort)
    {
        sortByColumn(m_columnSort, m_sortOrder);
    }
}


void CSongTable::removeSongsFromTable(const QList<CSong *>& songs)
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

void CSongTable::removeAllSongsFromTable(void)
{
    m_model->clear();
}


/**
 * Initialise la liste des morceaux aléatoires.
 *
 * \param firstSong Morceau à placer au début de la liste.
 */

void CSongTable::initShuffle(CSongTableItem * firstSong)
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

void CSongTable::initColumns(const QString& str)
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

        foreach (QString columnInfos, columnList)
        {
            QStringList columnInfosPart = columnInfos.split(':');

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

    //qDebug() << "Modification des colonnes :";

    m_isColumnMoving = true;
    CSongTableHeader * header = qobject_cast<CSongTableHeader *>(horizontalHeader());
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
/*
        switch (col)
        {
            default:
                qWarning() << "CSongTable::initColumns() : Invalid column index";
                break;

            case ColPosition        : break;
            case ColTitle           : header->m_actColTitle           ->setChecked(m_columns[col].visible); break;
            case ColArtist          : header->m_actColArtist          ->setChecked(m_columns[col].visible); break;
            case ColAlbum           : header->m_actColAlbum           ->setChecked(m_columns[col].visible); break;
            case ColAlbumArtist     : header->m_actColAlbumArtist     ->setChecked(m_columns[col].visible); break;
            case ColComposer        : header->m_actColComposer        ->setChecked(m_columns[col].visible); break;
            case ColYear            : header->m_actColYear            ->setChecked(m_columns[col].visible); break;
            case ColTrackNumber     : header->m_actColTrackNumber     ->setChecked(m_columns[col].visible); break;
            case ColDiscNumber      : header->m_actColDiscNumber      ->setChecked(m_columns[col].visible); break;
            case ColGenre           : header->m_actColGenre           ->setChecked(m_columns[col].visible); break;
            case ColRating          : header->m_actColRating          ->setChecked(m_columns[col].visible); break;
            case ColComments        : header->m_actColComments        ->setChecked(m_columns[col].visible); break;
            case ColPlayCount       : header->m_actColPlayCount       ->setChecked(m_columns[col].visible); break;
            case ColLastPlayTime    : header->m_actColLastPlayTime    ->setChecked(m_columns[col].visible); break;
            case ColFileName        : header->m_actColFileName        ->setChecked(m_columns[col].visible); break;
            case ColBitRate         : header->m_actColBitRate         ->setChecked(m_columns[col].visible); break;
            case ColFormat          : header->m_actColFormat          ->setChecked(m_columns[col].visible); break;
            case ColDuration        : header->m_actColDuration        ->setChecked(m_columns[col].visible); break;
            case ColSampleRate      : header->m_actColSampleRate      ->setChecked(m_columns[col].visible); break;
            case ColCreationDate    : header->m_actColCreationDate    ->setChecked(m_columns[col].visible); break;
            case ColModificationDate: header->m_actColModificationDate->setChecked(m_columns[col].visible); break;
        }
*/
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

        //int visualIndex = header->visualIndex(col);
        //header->moveSection(visualIndex, m_columns[col].pos);
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

void CSongTable::showColumn(int column, bool show)
{
    Q_ASSERT(column >= 0 && column < ColNumber);

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
 * Tri la table selon une colonne.
 *
 * \param column Numéro de la colonne.
 * \param order  Ordre croissant ou décroissant.
 */

void CSongTable::sortColumn(int column, Qt::SortOrder order)
{
    Q_ASSERT(column >= 0 && column < ColNumber);

    if (m_columnSort != column || m_sortOrder != order)
    {
        m_columnSort = column;
        m_sortOrder = order;

        m_isModified = true;
    }
}


void CSongTable::sort(void)
{
    m_model->sort(m_columnSort, m_sortOrder);
}


void CSongTable::goToSongTable(void)
{
    //qDebug() << "CSongTable::goToSongTable";

    QAction * action = qobject_cast<QAction *>(sender());

    if (action)
    {
        CSongTable * songTable = m_actionGoToSongTable.key(action);

        if (songTable)
        {
            m_application->selectSong(songTable, songTable->getFirstSongItem(m_selectedItem->getSong()));
        }
    }
}


void CSongTable::addToPlayList(void)
{
    //qDebug() << "CSongTable::addToPlayList";

    QAction * action = qobject_cast<QAction *>(sender());

    if (action)
    {
        CStaticPlayList * playList = m_actionAddToPlayList.key(action);

        if (playList)
        {
            // Liste des morceaux sélectionnés
            QModelIndexList indexList = selectionModel()->selectedRows();

            if (indexList.isEmpty())
            {
                return;
            }

            QList<CSong *> songList;

            foreach (QModelIndex index, indexList)
            {
                CSongTableItem * songItem = m_model->getSongItem(index);

                if (!songList.contains(songItem->getSong()))
                {
                    songList.append(songItem->getSong());
                }
            }

            playList->addSongs(songList);

            //m_application->selectSong(songTable, songTable->getFirstSongItem(m_selectedItem->getSong()));
        }
    }
}


/**
 * Retire les morceaux sélectionnés de la médiathèque.
 */

void CSongTable::removeSongsFromLibrary(void)
{
    //qDebug() << "CSongTable::removeSongsFromLibrary()";

    // Liste des morceaux sélectionnés
    QModelIndexList indexList = selectionModel()->selectedRows();

    if (indexList.isEmpty())
    {
        return;
    }

    // Confirmation
    if (QMessageBox::question(this, QString(), tr("Are you sure you want to remove the selected songs from the library?\nThe files will not be deleted."), tr("Remove"), tr("Cancel"), 0, 1) == 1)
    {
        return;
    }

    QList<CSong *> songList;

    for (QModelIndexList::const_iterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        CSongTableItem * songItem = m_model->getSongItem(*it);

        if (m_application->getCurrentSongItem() == songItem)
        {
            m_application->stop();
        }

        if (!songList.contains(songItem->getSong()))
        {
            songList.append(songItem->getSong());
        }
    }

    m_application->removeSongs(songList);

    selectionModel()->clearSelection();
}


/**
 * Coche tous les morceaux sélectionnés dans la liste.
 */

void CSongTable::checkSelection(void)
{
    //qDebug() << "CSongTable::checkSelection()";

    // Liste des morceaux sélectionnés
    QModelIndexList indexList = selectionModel()->selectedRows();

    if (indexList.isEmpty())
    {
        return;
    }

    for (QModelIndexList::const_iterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        m_model->m_data.at(it->row())->getSong()->setEnabled(true);
    }
}


/**
 * Décoche tous les morceaux sélectionnés dans la liste.
 */

void CSongTable::uncheckSelection(void)
{
    //qDebug() << "CSongTable::uncheckSelection()";

    // Liste des morceaux sélectionnés
    QModelIndexList indexList = selectionModel()->selectedRows();

    if (indexList.isEmpty())
    {
        return;
    }

    for (QModelIndexList::const_iterator it = indexList.begin(); it != indexList.end(); ++it)
    {
        m_model->m_data.at(it->row())->getSong()->setEnabled(false);
    }
}


void CSongTable::onRowCountChange(const QModelIndex& parent, int start, int end)
{
    emit rowCountChanged();
}


QString CSongTable::getColumnsInfos(void) const
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
        //m_application->getSettings()->value("Preferences/ColumnsDefault", "0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120").toString();
        str = "0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120"; // Disposition par défaut
    }

    return str;
}


/*
void CSongTable::mousePressEvent(QMouseEvent * event)
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

    //TODO

    QTableView::mousePressEvent(event);
}
*/

void CSongTable::columnMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    //qDebug() << "columnMoved("<<logicalIndex<<""<<oldVisualIndex<<""<<newVisualIndex<<")";

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


void CSongTable::columnResized(int logicalIndex, int oldSize, int newSize)
{
    //qDebug() << "columnResized("<<logicalIndex<<""<<oldSize<<""<<newSize<<")";

    m_columns[logicalIndex].width = newSize;

    m_isModified = true;
    emit columnChanged();

    QTableView::columnResized(logicalIndex, oldSize, newSize);
}


bool CSongTable::updateDatabase(void)
{
    if (m_isModified)
    {
        if (m_idPlayList < 0)
        {
            qWarning() << "CSongTable::updateDatabase() : id négatif";
            return false;
        }

        QString colInfos = getColumnsInfos();

        QSqlQuery query(m_application->getDataBase());

        query.prepare("UPDATE playlist SET list_columns = ? WHERE playlist_id = ?");
        query.bindValue(0, colInfos);
        query.bindValue(1, m_idPlayList);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return false;
        }

        m_isModified = false;
        initColumns(colInfos);
    }

    return true;
}


void CSongTable::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();
    QList<int> rows;

    for (int i = indexes.count() - 1 ; i >= 0; --i)
    {
        if (!rows.contains(indexes.at(i).row()))
        {
            rows.append(indexes.at(i).row());
        }

        if (!(m_model->flags(indexes.at(i)) & Qt::ItemIsDragEnabled))
        {
            indexes.removeAt(i);
        }
    }

    if (indexes.count() > 0)
    {
        QMimeData * data = m_model->mimeData(indexes);
        if (!data) return;

        //QRect rect;
        //QPixmap pixmap = d->renderToPixmap(indexes, &rect);

        //QPixmap pixmap = QPixmap(":/icons/song").scaledToHeight(32);
        QPixmap pixmap(64, 32);
        pixmap.fill(QColor(0, 0, 0, 0));
        {
            QPainter painter(&pixmap);
            painter.drawImage(QRect(QPoint(0, 0), QPoint(32, 32)), QImage(":/icons/song"));
            painter.drawText(QPoint(34, 26), QString::number(rows.size()));
        }

        //rect.adjust(horizontalOffset(), verticalOffset(), 0, 0);

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

void CSongTable::keyPressEvent(QKeyEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        event->accept();
        m_application->play();
        return;
    }

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

void CSongTable::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->button() == Qt::LeftButton)
    {
        QPoint pt = event->pos();
        QModelIndex index = indexAt(pt);

        if (index.isValid())
        {
            //qDebug() << "Double-clic sur un item...";
            selectRow(index.row());
            emit songStarted(m_model->getSongItem(index));
            event->accept();
        }
        else
        {
            //qDebug() << "Double-clic en dehors de la table";
        }
    }

    QTableView::mouseDoubleClickEvent(event);
}


/**
 * Indique si la liste de morceaux a été modifiée.
 *
 * \return Booléen.
 */

bool CSongTable::isModified(void) const
{
    return m_isModified;
}


void CSongTable::selectSongItem(CSongTableItem * songItem)
{
    Q_CHECK_PTR(songItem);

    selectionModel()->clearSelection();
    selectionModel()->setCurrentIndex(m_model->index(m_model->getRowForSongItem(songItem), 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
}


void CSongTable::playSelectedSong(void)
{
    m_application->playSong(m_selectedItem);
}


/**
 * Affiche le menu contextuel.
 *
 * \param point Position du clic.
 */

void CSongTable::openCustomMenuProject(const QPoint& point)
{
    //qDebug() << "CSongTable::openCustomMenuProject()";

    QModelIndex index = indexAt(point);
    m_selectedItem = NULL;

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
            menu->addAction(tr("Play"), this, SLOT(playSelectedSong()));
            menu->addSeparator();
        }

        menu->addAction(tr("Informations..."), m_application, SLOT(openDialogSongInfos()));
        
        if (!severalSongs)
        {
            menu->addAction(tr("Edit metadata..."), m_application, SLOT(openDialogEditMetadata()));
            menu->addAction(tr("Show in explorer"), m_application, SLOT(openSongInExplorer()));

            if (m_selectedItem->getSong()->getFileStatus() == false)
            {
                menu->addAction(tr("Relocate"), m_application, SLOT(relocateSong()));
            }
        }

        menu->addSeparator();
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

            QList<IPlayList *> playLists = m_application->getPlayListsWithSong(m_selectedItem->getSong());

            if (playLists.size() > 0)
            {
                menuPlayList->addSeparator();

                for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
                {
                    m_actionGoToSongTable[*it] = menuPlayList->addAction((*it)->getName());
                    connect(m_actionGoToSongTable[*it], SIGNAL(triggered()), this, SLOT(goToSongTable()));

                    if (qobject_cast<CDynamicList *>(*it))
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
        QList<IPlayList *> playLists = m_application->getAllPlayLists();

        if (playLists.isEmpty())
        {
            QAction * actionNoPlayList = menuAddToPlayList->addAction(tr("There are no playlist"));
            actionNoPlayList->setEnabled(false);
        }
        else
        {
            for (QList<IPlayList *>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
            {
                CStaticPlayList * staticList = qobject_cast<CStaticPlayList *>(*it);
                if (staticList)
                {
                    m_actionAddToPlayList[staticList] = menuAddToPlayList->addAction(QPixmap(":/icons/playlist"), (*it)->getName());
                    connect(m_actionAddToPlayList[staticList], SIGNAL(triggered()), this, SLOT(addToPlayList()));
                }
            }
        }

        menu->move(mapToGlobal(point));
        menu->show();
    }
}

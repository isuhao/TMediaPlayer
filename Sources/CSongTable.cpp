
#include "CSongTable.hpp"
#include "CSongTableModel.hpp"
#include "CSong.hpp"
#include "CApplication.hpp"
#include "CPlayList.hpp"
#include "CSongTableHeader.hpp"
#include <QStringList>
#include <QMouseEvent>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QMenu>

#include <QtDebug>


CSongTable::CSongTable(CApplication * application) :
    QTableView        (application),
    m_model           (NULL),
    m_application     (application),
    m_idPlayList      (-1),
    m_columnSort      (ColArtist),
    m_isModified      (false),
    m_sortOrder       (Qt::AscendingOrder),
    m_isColumnMoving  (false),
    m_automaticSort   (true)
{
    Q_CHECK_PTR(application);

    m_model = new CSongTableModel();
    setModel(m_model);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openCustomMenuProject(const QPoint&)));

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    setSizePolicy(sizePolicy);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
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
    verticalHeader()->setDefaultSectionSize(19); /// \todo => paramètres

    initColumns("");
}


CSongTable::~CSongTable()
{

}


/**
 * Retourne le pointeur sur l'item à une ligne donnée.
 *
 * \param pos Numéro de la ligne (à partir de 0).
 * \return Pointeur sur l'item, ou NULL.
 */

CSongTableModel::TSongItem * CSongTable::getSongItemForIndex(int pos) const
{
    return (pos < 0 ? NULL : m_model->getSongItem(pos));
}


/**
 * Chercher la position du morceau précédant un autre morceau.
 *
 * \param pos     Position du morceau actuel.
 * \param shuffle Indique si la lecture est aléatoire.
 * \return Position du morceau précédant.
 */

int CSongTable::getPreviousSong(int pos, bool shuffle) const
{
    Q_ASSERT(pos == -1 || (pos >= 0 && pos < m_songs.size()));

    if (pos < 0)
    {
        if (shuffle)
        {
            //TODO...
            return -1;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if (shuffle)
        {
            //TODO...
            return -1;
        }
        else
        {
            return (pos > 0 ? pos - 1 : -1);
        }
    }
}


/**
 * Chercher la position du morceau suivant un autre morceau.
 *
 * \param pos     Position du morceau actuel.
 * \param shuffle Indique si la lecture est aléatoire.
 * \return Position du morceau suivant.
 */

int CSongTable::getNextSong(int pos, bool shuffle) const
{
    Q_ASSERT(pos == -1 || (pos >= 0 && pos < m_songs.size()));

    if (pos < 0)
    {
        if (shuffle)
        {
            //TODO...
            return -1;
        }
        else
        {
            return (m_songs.size() > 0 ? 0 : -1);
        }
    }
    else
    {
        if (shuffle)
        {
            //TODO...
            return -1;
        }
        else
        {
            return (pos == m_songs.size() - 1 ? -1 : pos + 1);
        }
    }
}


/**
 * Calcule la durée totale des morceaux présents dans la liste.
 *
 * \return Durée totale en millisecondes.
 */

int CSongTable::getTotalDuration(void) const
{
    int duration = 0;

    foreach (CSong * song, m_songs)
    {
        duration += song->getDuration();
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

    pos = qBound(-1, pos, m_songs.size());

    m_songs.insert(pos, song);
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
    foreach (CSong * song, songs)
    {
        Q_CHECK_PTR(song);
        m_songs.append(song);
        m_model->insertRow(song);
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

    m_songs.removeAll(song);

    if (m_automaticSort)
    {
        sortByColumn(m_columnSort, m_sortOrder);
    }
}


/**
 * Enlève une chanson de la liste.
 *
 * \param pos Position de la chanson dans la liste (à partir de 0).
 */

void CSongTable::removeSongFromTable(int pos)
{
    Q_ASSERT(pos >= 0 && pos < m_songs.size());

    m_songs.removeAt(pos);
    m_model->removeRow(pos);

    if (m_automaticSort)
    {
        sortByColumn(m_columnSort, m_sortOrder);
    }
}


/**
 * Supprime tous les morceaux de la table.
 */

void CSongTable::deleteSongs(void)
{
    foreach (CSong * song, m_songs)
    {
        delete song;
    }

    m_songs.clear();
    m_model->clear();

    if (m_automaticSort)
    {
        sortByColumn(m_columnSort, m_sortOrder);
    }
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
 * \todo Implémentation.
 *
 * \param str Chaine de caractères contenant la disposition des colonnes.
 */

void CSongTable::initColumns(const QString& str)
{
    bool isValid = false;

    for (int i = 0; i < ColNumber; ++i)
    {
        m_columns[i].pos     = -1;
        m_columns[i].width   = horizontalHeader()->defaultSectionSize();
        m_columns[i].visible = false;
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

        for (int i = 0; i < ColNumber; ++i)
        {
            if (!m_columns[i].visible)
            {
                m_columns[i].pos = colPosition++;
            }
        }
    }

    // Disposition par défaut
    if (!isValid)
    {
        for (int i = 0; i < ColNumber; ++i)
        {
            m_columns[i].pos     = i;
            m_columns[i].width   = horizontalHeader()->defaultSectionSize();
            m_columns[i].visible = true;
        }

        m_columns[0].visible = false;
        m_columnSort = ColArtist;
        m_sortOrder = Qt::AscendingOrder;
    }

    qDebug() << "Modification des colonnes :";
    
    m_isColumnMoving = true;
    CSongTableHeader * header = qobject_cast<CSongTableHeader *>(horizontalHeader());

    for (int i = 0; i < ColNumber; ++i)
    {
        // Affichage ou masquage de la colonne
        if (m_columns[i].visible)
        {
            header->showSection(i);
        }
        else
        {
            header->hideSection(i);
        }

        switch (i)
        {
            default:
                qWarning() << "CSongTable::initColumns() : Invalid column index";
                break;

            case  0: break;
            case  1: header->m_actColTitle       ->setChecked(m_columns[i].visible); break;
            case  2: header->m_actColArtist      ->setChecked(m_columns[i].visible); break;
            case  3: header->m_actColAlbum       ->setChecked(m_columns[i].visible); break;
            case  4: header->m_actColAlbumArtist ->setChecked(m_columns[i].visible); break;
            case  5: header->m_actColComposer    ->setChecked(m_columns[i].visible); break;
            case  6: header->m_actColYear        ->setChecked(m_columns[i].visible); break;
            case  7: header->m_actColTrackNumber ->setChecked(m_columns[i].visible); break;
            case  8: header->m_actColDiscNumber  ->setChecked(m_columns[i].visible); break;
            case  9: header->m_actColGenre       ->setChecked(m_columns[i].visible); break;
            case 10: header->m_actColRating      ->setChecked(m_columns[i].visible); break;
            case 11: header->m_actColComments    ->setChecked(m_columns[i].visible); break;
            case 12: header->m_actColPlayCount   ->setChecked(m_columns[i].visible); break;
            case 13: header->m_actColLastPlayTime->setChecked(m_columns[i].visible); break;
            case 14: header->m_actColFileName    ->setChecked(m_columns[i].visible); break;
            case 15: header->m_actColBitRate     ->setChecked(m_columns[i].visible); break;
            case 16: header->m_actColFormat      ->setChecked(m_columns[i].visible); break;
            case 17: header->m_actColDuration    ->setChecked(m_columns[i].visible); break;
        }

        // Déplacement de la colonne
        int visualIndex = header->visualIndex(i);
        header->moveSection(visualIndex, m_columns[i].pos);

        // Redimensionnement
        if (m_columns[i].width <= 0) m_columns[i].width = header->defaultSectionSize();
        header->resizeSection(i, m_columns[i].width);
    }

    m_isColumnMoving = false;

    sortByColumn(m_columnSort, m_sortOrder);

    //horizontalHeader()->hideSection(0);
}


void CSongTable::showColumn(int column, bool show)
{
    Q_ASSERT(column >= 0 && column < ColNumber);

    if (m_columns[column].visible != show)
    {
        m_columns[column].visible = show;

        if (show)
        {
            horizontalHeader()->showSection(column);

            int numColumns = -1;
            for (int i = 0; i < ColNumber; ++i)
            {
                if (m_columns[i].visible)
                {
                    ++numColumns;
                }
            }
        
            // Déplacement de la colonne
            int visualIndex = horizontalHeader()->visualIndex(column);
            horizontalHeader()->moveSection(visualIndex, numColumns);
        }
        else
        {
            horizontalHeader()->hideSection(column);
        }

        m_isModified = true;
    }
}


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


void CSongTable::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->button() == Qt::LeftButton) 
    {
        QPoint pt = event->pos();
        QModelIndex index = this->indexAt(pt);

        if (index.isValid())
        {
            qDebug() << "Double-clic sur la table";
            selectRow(index.row());
            emit songStarted(index.row());

        }
        else
        {
            qDebug() << "Double-clic en dehors de la table";
        }
    }

    QTableView::mouseDoubleClickEvent(event);
}
*/

void CSongTable::columnMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    qDebug() << "columnMoved("<<logicalIndex<<""<<oldVisualIndex<<""<<newVisualIndex<<")";

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
    qDebug() << "columnResized("<<logicalIndex<<""<<oldSize<<""<<newSize<<")";

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

        QSqlQuery query(m_application->getDataBase());

        query.prepare("UPDATE playlist SET list_columns = ? WHERE playlist_id = ?");
        query.bindValue(0, getColumnsInfos());
        query.bindValue(1, m_idPlayList);

        if (!query.exec())
        {
            qWarning() << "CSongTable::updateDatabase() : Erreur SQL";
            return false;
        }

        m_isModified = false;
    }

    return true;
}


void CSongTable::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();
    for (int i = indexes.count() - 1 ; i >= 0; --i)
    {
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
        //QPixmap pixmap(":/icons/play");//TODO: afficher le nombre d'éléments...
        QPixmap pixmap = QPixmap(":/icons/song").scaledToHeight(32);
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

        Qt::DropAction defaultDropAction0 = Qt::IgnoreAction;
        Qt::DropAction defaultDropAction = this->defaultDropAction();

        if (defaultDropAction != Qt::IgnoreAction && (supportedActions & defaultDropAction))
            defaultDropAction0 = defaultDropAction;
        else if (supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove)
            defaultDropAction0 = Qt::CopyAction;

        drag->exec(supportedActions, defaultDropAction);

        //if (drag->exec(supportedActions, defaultDropAction) == Qt::MoveAction)
        //    d->clearOrRemove();
    }

    //QAbstractItemView::startDrag(supportedActions);
}


bool CSongTable::isModified(void) const
{
    return m_isModified;
}


void CSongTable::addSong(CSong * song, int pos)
{
    addSongToTable(song, pos);
}


void CSongTable::addSongs(const QList<CSong *>& songs)
{
    addSongsToTable(songs);
}


void CSongTable::removeSong(CSong * song)
{
    removeSongFromTable(song);
}


void CSongTable::removeSong(int pos)
{
    removeSongFromTable(pos);
}


/// \todo Implémentation.
void CSongTable::openCustomMenuProject(const QPoint& point)
{
    qDebug() << "CSongTable::openCustomMenuProject()";

    QModelIndex index = indexAt(point);

    if (index.isValid())
    {
        bool severalSongs = (selectionModel()->selectedRows().size() > 1);

        // Menu contextuel
        QMenu * menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);

        menu->addAction(tr("Informations"), m_application, SLOT(openDialogSongInfos()));
        if (!severalSongs) menu->addAction(tr("Show in explorer"), m_application, SLOT(openSongInExplorer()));
        menu->addSeparator();
        menu->addAction(tr("Remove from library"));
        menu->addSeparator();

        if (!severalSongs)
        {
            // Listes de lecture contenant le morceau
            QMenu * menuPlayList = menu->addMenu(tr("Playlists"));
            menuPlayList->addAction(tr("Library"));

            QList<CPlayList *> playLists = m_application->getPlayListsWithSong(m_model->getSongItem(index)->song);

            if (playLists.size() > 0)
            {
                menuPlayList->addSeparator();

                foreach (CPlayList * playList, playLists)
                {
                    menuPlayList->addAction(playList->getName());
                }
            }
        }

        // Toutes les listes de lecture
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
            foreach (CPlayList * playList, playLists)
            {
                menuAddToPlayList->addAction(playList->getName());
            }
        }

        menu->move(mapToGlobal(point));
        menu->show();
    }
}

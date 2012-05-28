
#include "CSongTable.hpp"
#include "CSongTableModel.hpp"
#include "CSong.hpp"
#include "CApplication.hpp"
#include "CPlayList.hpp"
#include "CSongTableHeader.hpp"
#include "CStaticPlayList.hpp"
#include "CDynamicPlayList.hpp"
#include <QStringList>
#include <QMouseEvent>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QMenu>
#include <QPainter>
#include <QMessageBox>

#include <QtDebug>


CSongTable::CSongTable(CApplication * application) :
    QTableView       (application),
    m_application    (application),
    m_model          (NULL),
    m_idPlayList     (-1),
    m_columnSort     (ColArtist),
    m_isModified     (false),
    m_sortOrder      (Qt::AscendingOrder),
    m_isColumnMoving (false),
    m_selectedItem   (NULL),
    m_automaticSort  (true)
{
    Q_CHECK_PTR(application);

    m_model = new CSongTableModel(this);
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
    verticalHeader()->setDefaultSectionSize(m_application->getRowHeight());

    initColumns("");
}


/**
 * Détruit la liste de morceaux.
 */

CSongTable::~CSongTable()
{
    qDebug() << "CSongTable::~CSongTable()";
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


CSongTableItem * CSongTable::getFirstSongItem(CSong * song) const
{
    if (!song)
    {
        return NULL;
    }

    foreach (CSongTableItem * songItem, m_model->m_data)
    {
        if (songItem->getSong() == song)
        {
            return songItem;
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


CSongTableItem * CSongTable::getSelectedSongItem(void) const
{
    QItemSelectionModel * selection = selectionModel();
    return m_model->getSongItem(selection->currentIndex());
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
    foreach (CSong * song, songs)
    {
        Q_CHECK_PTR(song);
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

    foreach (CSongTableItem * songItem, m_model->m_data)
    {
        if (songItem->getSong() == song)
        {
            int row = getRowForSongItem(songItem);
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


/**
 * Enlève tous les morceaux de la table.
 * Aucune modification n'a lieu en base de données, et aucun signal n'est envoyé.
 */

void CSongTable::removeAllSongsFromTable(void)
{
    m_model->clear();
}


/**
 * Supprime tous les morceaux de la table. La mémoire est libérée.
 */

void CSongTable::deleteSongs(void)
{
    foreach (CSongTableItem * songItem, m_model->m_data)
    {
        delete songItem->getSong();
    }

    removeAllSongsFromTable();
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
    if (!isValid)
    {
        for (int col = 0; col < ColNumber; ++col)
        {
            m_columns[col].pos     = col;
            m_columns[col].width   = horizontalHeader()->defaultSectionSize();
            m_columns[col].visible = true;
        }

        m_columns[0].visible = false;
        m_columnSort = ColArtist;
        m_sortOrder = Qt::AscendingOrder;
    }

    // La colonne 0 est toujours visible, c'est elle qui contient l'indicateur de lecture, et la position du
    // morceau pour les listes statiques. De plus, cela garantie qu'il y a toujours au moins une colonne
    // d'affichée, sans quoi on pourrait faire disparaitre les données et le header en masquant toutes les colonnes.
    m_columns[0].visible = true;

    qDebug() << "Modification des colonnes :";
    
    m_isColumnMoving = true;
    CSongTableHeader * header = qobject_cast<CSongTableHeader *>(horizontalHeader());
        
    QString debugStr1 = "";
    QString debugStr2 = "";
    for (int col = 0; col < ColNumber; ++col)
    {
        debugStr1 += QString::number(horizontalHeader()->visualIndex(col)) + ',';
        debugStr2 += QString::number(m_columns[col].pos) + ',';
    }
    qDebug() << "Colonnes Qt (avant) :" << debugStr1;
    qDebug() << "Colonnes TT (avant) :" << debugStr2;

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
        
    debugStr1 = "";
    debugStr2 = "";
    for (int col = 0; col < ColNumber; ++col)
    {
        debugStr1 += QString::number(horizontalHeader()->visualIndex(col)) + ',';
        debugStr2 += QString::number(m_columns[col].pos) + ',';
    }
    qDebug() << "Colonnes Qt (apres) :" << debugStr1;
    qDebug() << "Colonnes TT (apres) :" << debugStr2;

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
        
        QString debugStr1 = "";
        QString debugStr2 = "";
        for (int col = 0; col < ColNumber; ++col)
        {
            if (m_columns[col].visible)
            {
                ++numColumns;
            }
            
            debugStr1 += QString::number(horizontalHeader()->visualIndex(col)) + ',';
            debugStr2 += QString::number(m_columns[col].pos) + ',';
        }
        qDebug() << "Colonnes Qt (avant) :" << debugStr1;
        qDebug() << "Colonnes TT (avant) :" << debugStr2;

        int visualIndex = horizontalHeader()->visualIndex(column);

        // Déplacement de la colonne
        horizontalHeader()->moveSection(visualIndex, numColumns);

        debugStr1 = "";
        debugStr2 = "";
        for (int col = 0; col < ColNumber; ++col)
        {
            debugStr1 += QString::number(horizontalHeader()->visualIndex(col)) + ',';
            debugStr2 += QString::number(m_columns[col].pos) + ',';
        }
        qDebug() << "Colonnes Qt (apres) :" << debugStr1;
        qDebug() << "Colonnes TT (apres) :" << debugStr2;

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


void CSongTable::goToSongTable(void)
{
    qDebug() << "CSongTable::goToSongTable";

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
        str = "1:100;2+:100;3:100"; // Valeur par défaut, \todo dans SETTINGS
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


/**
 * Gestion des touches du clavier.
 * Les touches Entrée et Supprimer sont gérées.
 *
 * \todo Gérer la touche Supprimer.
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
        //delete
        event->accept();
        return;
    }

    //...

    return QTableView::keyPressEvent(event);
}


void CSongTable::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->button() == Qt::LeftButton) 
    {
        QPoint pt = event->pos();
        QModelIndex index = indexAt(pt);

        if (index.isValid())
        {
            qDebug() << "Double-clic sur un item...";
            selectRow(index.row());
            emit songStarted(m_model->getSongItem(index));
            event->accept();
        }
        else
        {
            qDebug() << "Double-clic en dehors de la table";
        }
    }

    QTableView::mouseDoubleClickEvent(event);
}


bool CSongTable::isModified(void) const
{
    return m_isModified;
}


/**
 * Ajoute un morceau à la table.
 *
 * \todo Supprimer cette méthode ?
 */

void CSongTable::addSong(CSong * song, int pos)
{
    addSongToTable(song, pos);
}


/**
 * Ajoute une liste de morceaux à la table.
 *
 * \todo Supprimer cette méthode ?
 */

void CSongTable::addSongs(const QList<CSong *>& songs)
{
    addSongsToTable(songs);
}


/**
 * Enlève un morceau de la table.
 *
 * \todo Supprimer cette méthode ?
 */

void CSongTable::removeSong(CSong * song)
{
    removeSongFromTable(song);
}


/**
 * Enlène un morceau de la table à partir de sa position.
 *
 * \todo Supprimer cette méthode ?
 */

void CSongTable::removeSong(int pos)
{
    removeSongFromTable(pos);
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
    qDebug() << "CSongTable::openCustomMenuProject()";

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

        menu->addAction(tr("Informations"), m_application, SLOT(openDialogSongInfos()));
        if (!severalSongs) menu->addAction(tr("Show in explorer"), m_application, SLOT(openSongInExplorer()));
        menu->addSeparator();
        menu->addAction(tr("Remove from library")); //TODO
        menu->addAction(tr("Check selection")); //TODO
        menu->addAction(tr("Uncheck selection")); //TODO
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

                foreach (CPlayList * playList, playLists)
                {
                    m_actionGoToSongTable[playList] = menuPlayList->addAction(playList->getName());
                    connect(m_actionGoToSongTable[playList], SIGNAL(triggered()), this, SLOT(goToSongTable()));

                    if (qobject_cast<CDynamicPlayList *>(playList))
                    {
                        m_actionGoToSongTable[playList]->setIcon(QPixmap(":/icons/dynamic_list"));
                    }
                    else if (qobject_cast<CStaticPlayList *>(playList))
                    {
                        m_actionGoToSongTable[playList]->setIcon(QPixmap(":/icons/playlist"));
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
            foreach (CPlayList * playList, playLists)
            {
                if (qobject_cast<CStaticPlayList *>(playList))
                {
                    menuAddToPlayList->addAction(QPixmap(":/icons/playlist"), playList->getName());
                }
            }
        }

        menu->move(mapToGlobal(point));
        menu->show();
    }
}


#include "CPlayListView.hpp"
#include "CPlayListModel.hpp"
#include "CPlayList.hpp"
#include "CStaticPlayList.hpp"
#include "CDynamicPlayList.hpp"
#include "CApplication.hpp"
#include <QHeaderView>
#include <QMenu>
#include <QDragMoveEvent>

#include <QtDebug>


CPlayListView::CPlayListView(CApplication * application) :
    QTreeView      (application),
    m_application  (application),
    m_model        (NULL),
    m_menuPlaylist (NULL),
    m_menuDefault  (NULL)
{
    Q_CHECK_PTR(application);

    header()->setVisible(false);
    setUniformRowHeights(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    m_model = new CPlayListModel(m_application);
    setModel(m_model);

    // Glisser-déposer
    setDropIndicatorShown(false);
    setAcceptDrops(true);
    //viewport()->setAcceptDrops(true);

    // Menus contextuels
    m_menuPlaylist = new QMenu(this);
    //m_menuPlaylist->addAction(tr("Open")); //TODO: ouvrir dans une nouvelle fenêtre
    m_menuPlaylist->addAction(tr("Edit..."), m_application, SLOT(editSelectedPlayList()));
    m_menuPlaylist->addAction(tr("Remove"), m_application, SLOT(removeSelectedPlayList()));

    m_menuDefault = new QMenu(this);
    //TODO: gérer le dossier
    m_menuDefault->addAction(tr("New playlist..."), m_application, SLOT(openDialogAddStaticPlayList()));
    m_menuDefault->addAction(tr("New dynamic playlist..."), m_application, SLOT(openDialogAddDynamicList()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openCustomMenuProject(const QPoint&)));
}


QModelIndex CPlayListView::addSongTable(CSongTable * songTable)
{
    Q_CHECK_PTR(songTable);

    QStandardItem * playListItem;
    CPlayList * playList = qobject_cast<CPlayList *>(songTable);

    if (playList)
    {
        playListItem = new QStandardItem(playList->getName());
        CDynamicPlayList * dynamicList = qobject_cast<CDynamicPlayList *>(playList);

        if (dynamicList)
        {
            playListItem->setIcon(QPixmap(":/icons/dynamic_list"));
        }
        else if (qobject_cast<CStaticPlayList *>(playList))
        {
            playListItem->setIcon(QPixmap(":/icons/playlist"));
        }
    }
    else
    {
        playListItem = new QStandardItem(QPixmap(":/icons/library"), tr("Library"));
    }

    playListItem->setData(QVariant::fromValue(songTable), Qt::UserRole + 1);
    m_model->appendRow(playListItem);
    return playListItem->index();
}


void CPlayListView::removeSongTable(CSongTable * songTable)
{
    Q_CHECK_PTR(songTable);

    QModelIndex index = getModelIndex(songTable);
    m_model->removeRow(index.row(), index.parent());
}


/**
 * Retourne la liste de morceaux à partir d'un index.
 *
 * \param index Index de la liste.
 * \return Liste de morceaux, ou NULL.
 */

CSongTable * CPlayListView::getSongTable(const QModelIndex& index) const
{
    return m_model->data(index, Qt::UserRole + 1).value<CSongTable *>();
}


/**
 * Retourne la liste de morceaux actuellement sélectionnée.
 *
 * \return Liste de morceaux.
 */

CSongTable * CPlayListView::getSelectedSongTable(void) const
{
    QModelIndex index = selectionModel()->currentIndex();

    if (index.isValid())
    {
        return getSongTable(index);
    }

    return NULL;
}


QModelIndex CPlayListView::getModelIndex(CSongTable * songTable) const
{
    if (!songTable)
    {
        return QModelIndex();
    }

    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        const QStandardItem * item = m_model->item(row);
        if (item && item->data(Qt::UserRole + 1).value<CSongTable *>() == songTable)
        {
            return m_model->indexFromItem(item);
        }
    }

    return QModelIndex();
}


void CPlayListView::onPlayListRenamed(const QString& oldName, const QString& newName)
{
    CPlayList * playList = qobject_cast<CPlayList *>(sender());

    if (playList)
    {
        QStandardItem * item = m_model->itemFromIndex(playList->m_index);
        
        if (item)
        {
            item->setText(newName);
        }
    }
}


/**
 * Gestion des touches du clavier.
 * La touche Supprimer est gérée.
 *
 * \param event Évènement du clavier.
 */

void CPlayListView::keyPressEvent(QKeyEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->key() == Qt::Key_Delete)
    {
        event->accept();
        m_application->removeSelectedPlayList();
        return;
    }

    return QTreeView::keyPressEvent(event);
}


/**
 * Gestion du glisser-déposer.
 * Cette méthode est appelée à chaque fois qu'un objet QDrag est déplacé à l'intérieur de la vue.
 *
 * \param event Évènement.
 */

void CPlayListView::dragMoveEvent(QDragMoveEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->keyboardModifiers() & Qt::ControlModifier)
    {
        // Traitement différent si Ctrl+Glisser (par exemple pour copier OU déplacer)
        // on verra plus tard...
    }

    event->setDropAction(Qt::CopyAction);

    QPoint point = event->pos();
    //qDebug() << point << indexAt(point).row();

    QModelIndex index = indexAt(point);

    if (index.isValid())
    {
        CSongTable * songTable = m_model->data(index, Qt::UserRole + 1).value<CSongTable *>();
        CStaticPlayList * playList = qobject_cast<CStaticPlayList *>(songTable);

        if (songTable && playList)
        {
            event->accept();
            event->acceptProposedAction();
        }
        else
        {
            event->ignore();
            return;
        }
    }
    else
    {
        event->accept();
        event->acceptProposedAction();
    }

/*

    event->accept();
    event->acceptProposedAction(); //causes cursor icon to change and target item to get highlighted
*/
    QTreeView::dragMoveEvent(event);
}


/**
 * Ouvre le menu contextuel de la vue.
 *
 * \param point Position du clic.
 */

void CPlayListView::openCustomMenuProject(const QPoint& point)
{
    //qDebug() << "CPlayListView::openCustomMenuProject()";

    QModelIndex index = indexAt(point);

    if (index.isValid())
    {
        m_menuPlaylist->move(mapToGlobal(point));
        m_menuPlaylist->show();
    }
    else
    {
        m_menuDefault->move(mapToGlobal(point));
        m_menuDefault->show();
    }
}

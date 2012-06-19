
#include "CPlayListView.hpp"
#include "CPlayListModel.hpp"
#include "CPlayList.hpp"
#include "CStaticPlayList.hpp"
#include "CDynamicPlayList.hpp"
#include "CApplication.hpp"
#include "CListFolder.hpp"
#include <QHeaderView>
#include <QMenu>
#include <QDragMoveEvent>

#include <QtDebug>


/**
 * Constructeur de la vue.
 *
 * \param application Pointeur sur l'application.
 */

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
    setEditTriggers(QAbstractItemView::NoEditTriggers); // TODO: pouvoir renommer directement dans la vue
    
    m_model = new CPlayListModel(m_application);
    setModel(m_model);

    // Glisser-déposer
    setDropIndicatorShown(false);
    setAcceptDrops(true);
    //setDragEnabled(true);
    //viewport()->setAcceptDrops(true);

    connect(this, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(onItemCollapsed(const QModelIndex&)));
    connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(onItemExpanded(const QModelIndex&)));

    // Menus contextuels
    m_menuPlaylist = new QMenu(this);
    //m_menuPlaylist->addAction(tr("Open")); //TODO: ouvrir dans une nouvelle fenêtre
    m_menuPlaylist->addAction(tr("Edit..."), m_application, SLOT(editSelectedItem()));
    m_menuPlaylist->addAction(tr("Remove"), m_application, SLOT(removeSelectedItem()));
    //TODO: Pouvoir exporter une liste de lecture

    m_menuFolder = new QMenu(this);
    m_menuFolder->addAction(tr("Edit..."), m_application, SLOT(editSelectedItem()));
    m_menuFolder->addAction(tr("Remove"), m_application, SLOT(removeSelectedItem()));
    m_menuFolder->addSeparator();
    //m_menuFolder->addAction(tr("New playlist..."), m_application, SLOT(openDialogCreateStaticList()));
    //m_menuFolder->addAction(tr("New dynamic playlist..."), m_application, SLOT(openDialogCreateDynamicList()));
    //m_menuFolder->addAction(tr("New folder..."), m_application, SLOT(openDialogCreateFolder()));
    m_menuFolder->addAction(tr("New playlist..."), this, SLOT(createStaticList()));
    m_menuFolder->addAction(tr("New dynamic playlist..."), this, SLOT(createDynamicList()));
    m_menuFolder->addAction(tr("New folder..."), this, SLOT(createFolder()));

    m_menuDefault = new QMenu(this);
    //TODO: gérer le dossier
    m_menuDefault->addAction(tr("New playlist..."), m_application, SLOT(openDialogCreateStaticList()));
    m_menuDefault->addAction(tr("New dynamic playlist..."), m_application, SLOT(openDialogCreateDynamicList()));
    m_menuDefault->addAction(tr("New folder..."), m_application, SLOT(openDialogCreateFolder()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openCustomMenuProject(const QPoint&)));
}


QModelIndex CPlayListView::addSongTable(CSongTable * songTable, const QModelIndex& parent)
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
    
    QStandardItem * itemParent = m_model->itemFromIndex(parent);

    if (parent.isValid() && itemParent)
    {
        itemParent->appendRow(playListItem);
    }
    else
    {
        m_model->appendRow(playListItem);
    }

    QModelIndex index = playListItem->index();
    
    if (playList)
    {
        playList->m_index = index;
    }

    return index;
}


QModelIndex CPlayListView::addFolder(CListFolder * folder, const QModelIndex& parent)
{
    Q_CHECK_PTR(folder);

    connect(folder, SIGNAL(folderOpened()), this, SLOT(onFolderOpen()));
    connect(folder, SIGNAL(folderClosed()), this, SLOT(onFolderClose()));

    QStandardItem * folderItem = new QStandardItem(QPixmap(":/icons/folder_close"), folder->getName());
    folderItem->setData(QVariant::fromValue(folder), Qt::UserRole + 2);

    QStandardItem * itemParent = m_model->itemFromIndex(parent);

    if (parent.isValid() && itemParent)
    {
        itemParent->appendRow(folderItem);
    }
    else
    {
        m_model->appendRow(folderItem);
    }

    QModelIndex index = folderItem->index();

    if (folder->isOpen())
    {
        expand(index);
        folderItem->setIcon(QPixmap(":/icons/folder_open"));
    }
    else
    {
        collapse(index);
        folderItem->setIcon(QPixmap(":/icons/folder_close"));
    }

    // Dossiers enfants
    QList<CListFolder *> folders = folder->getFolders();
    foreach (CListFolder * child, folders)
    {
        child->m_index = addFolder(child, index);
    }

    // Listes enfants
    QList<CPlayList *> playLists = folder->getPlayLists();
    foreach (CPlayList * child, playLists)
    {
        child->m_index = addSongTable(child, index);
    }

    folder->m_index = index;
    return index;
}


void CPlayListView::removeSongTable(CSongTable * songTable)
{
    Q_CHECK_PTR(songTable);

    QModelIndex index = getSongTableModelIndex(songTable);
    m_model->removeRow(index.row(), index.parent());
}


/**
 * Retourne la liste de morceaux correspondant à un index.
 *
 * \param index Index de la liste.
 * \return Liste de morceaux, ou NULL.
 */

CSongTable * CPlayListView::getSongTable(const QModelIndex& index) const
{
    return m_model->data(index, Qt::UserRole + 1).value<CSongTable *>();
}


/**
 * Retourne le dossier correspondant à un index.
 *
 * \param index Index du dossier.
 * \return Dossier, ou NULL.
 */

CListFolder * CPlayListView::getFolder(const QModelIndex& index) const
{
    return m_model->data(index, Qt::UserRole + 2).value<CListFolder *>();
}


/**
 * Retourne la liste de morceaux actuellement sélectionnée.
 *
 * \return Liste de morceaux sélectionnée, ou NULL.
 */

CSongTable * CPlayListView::getSelectedSongTable(void) const
{
    QModelIndex index = selectionModel()->currentIndex();
    return (index.isValid() ? getSongTable(index) : NULL);
}


/**
 * Retourne le dossier actuellement sélectionnée.
 *
 * \return Dossier sélectionné, ou NULL.
 */

CListFolder * CPlayListView::getSelectedFolder(void) const
{
    QModelIndex index = selectionModel()->currentIndex();
    return (index.isValid() ? getFolder(index) : NULL);
}


/**
 * Retourne l'index du modèle correspondant à une liste de morceaux.
 *
 * \param songTable Liste de morceaux à rechercher.
 * \return Index de la liste de morceaux.
 */

QModelIndex CPlayListView::getSongTableModelIndex(CSongTable * songTable) const
{
    if (!songTable)
        return QModelIndex();

    return getModelIndex(songTable, QModelIndex());
}


/**
 * Retourne l'index du modèle correspondant à un dossier.
 *
 * \param folder Dossier à rechercher.
 * \return Index du dossier.
 */

QModelIndex CPlayListView::getFolderModelIndex(CListFolder * folder) const
{
    if (!folder)
        return QModelIndex();

    return getModelIndex(folder, QModelIndex());
}


/**
 * Retourne l'index du modèle correspondant à une liste de morceaux.
 *
 * \param songTable Liste de morceaux à rechercher.
 * \param parent    Index du dossier parent.
 * \return Index de la liste de morceaux.
 */

QModelIndex CPlayListView::getModelIndex(CSongTable * songTable, const QModelIndex& parent) const
{
    if (!songTable)
        return QModelIndex();

    for (int row = 0; row < m_model->rowCount(parent); ++row)
    {
        QModelIndex child = m_model->index(row, 0, parent);
        const QStandardItem * item = m_model->itemFromIndex(child);

        if (item)
        {
            CSongTable * itemSongTable = item->data(Qt::UserRole + 1).value<CSongTable *>();

            if (itemSongTable)
            {
                if (itemSongTable == songTable)
                {
                    return m_model->indexFromItem(item);
                }
            }
            else
            {
                CListFolder * itemFolder = item->data(Qt::UserRole + 2).value<CListFolder *>();

                if (itemFolder)
                {
                    QModelIndex index = getModelIndex(songTable, child);

                    if (index.isValid())
                        return index;
                }
            }
        }
    }

    return QModelIndex();
}


/**
 * Retourne l'index du modèle correspondant à un dossier.
 *
 * \param folder Dossier à rechercher.
 * \param parent Index du dossier parent.
 * \return Index du dossier.
 */

QModelIndex CPlayListView::getModelIndex(CListFolder * folder, const QModelIndex& parent) const
{
    if (!folder)
        return QModelIndex();

    for (int row = 0; row < m_model->rowCount(parent); ++row)
    {
        QModelIndex child = m_model->index(row, 0, parent);
        const QStandardItem * item = m_model->itemFromIndex(child);

        if (item)
        {
            CListFolder * itemFolder = item->data(Qt::UserRole + 2).value<CListFolder *>();

            if (itemFolder)
            {
                if (itemFolder == folder)
                {
                    return m_model->indexFromItem(item);
                }

                QModelIndex index = getModelIndex(folder, child);

                if (index.isValid())
                    return index;
            }
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


void CPlayListView::onFolderRenamed(const QString& oldName, const QString& newName)
{
    CListFolder * folder = qobject_cast<CListFolder *>(sender());

    if (folder)
    {
        QStandardItem * item = m_model->itemFromIndex(folder->m_index);
        
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
        m_application->removeSelectedItem();
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
        // Ctrl: Copier
        // Shift: Déplacer
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
        QStandardItem * item = m_model->itemFromIndex(index);

        if (item)
        {
            CSongTable * songTable = item->data(Qt::UserRole + 1).value<CSongTable *>();

            if (songTable)
            {
                if (qobject_cast<CPlayList *>(songTable))
                {
                    m_menuPlaylist->move(mapToGlobal(point));
                    m_menuPlaylist->show();
                    return;
                }
            }
            else if (item->data(Qt::UserRole + 2).value<CListFolder *>())
            {
                m_menuFolder->move(mapToGlobal(point));
                m_menuFolder->show();
                return;
            }
            else
            {
                qWarning() << "CPlayListView::openCustomMenuProject() : l'item n'est ni une liste ni un dossier";
            }
        }
    }

    m_menuDefault->move(mapToGlobal(point));
    m_menuDefault->show();
}


void CPlayListView::onItemCollapsed(const QModelIndex& index)
{
    CListFolder * folder = getFolder(index);

    if (folder)
    {
        folder->setOpen(false);
    }
}


void CPlayListView::onItemExpanded(const QModelIndex& index)
{
    CListFolder * folder = getFolder(index);

    if (folder)
    {
        folder->setOpen(true);
    }
}


void CPlayListView::onFolderOpen(void)
{
    CListFolder * folder = qobject_cast<CListFolder *>(sender());

    if (folder)
    {
        QModelIndex index = folder->getModelIndex();
        expand(index);

        QStandardItem * item = m_model->itemFromIndex(index);
        if (item)
        {
            item->setIcon(QPixmap(":/icons/folder_open"));
        }
    }
}


void CPlayListView::onFolderClose(void)
{
    CListFolder * folder = qobject_cast<CListFolder *>(sender());

    if (folder)
    {
        QModelIndex index = folder->getModelIndex();
        collapse(index);

        QStandardItem * item = m_model->itemFromIndex(index);
        if (item)
        {
            item->setIcon(QPixmap(":/icons/folder_close"));
        }
    }
}


void CPlayListView::createStaticList(void)
{
    CListFolder * folder = getSelectedFolder();
    m_application->openDialogCreateStaticList(folder);
}


void CPlayListView::createDynamicList(void)
{
    CListFolder * folder = getSelectedFolder();
    m_application->openDialogCreateDynamicList(folder);
}


void CPlayListView::createFolder(void)
{
    CListFolder * folder = getSelectedFolder();
    m_application->openDialogCreateFolder(folder);
}

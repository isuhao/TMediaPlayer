
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
    header()->setVisible(false);
    setUniformRowHeights(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    m_model = new CPlayListModel(m_application);
    setModel(m_model);

    // Glisser-déposer
    setDropIndicatorShown(true);
    setAcceptDrops(true);
    //viewport()->setAcceptDrops(true);

    // Menus contextuels
    m_menuPlaylist = new QMenu(this);
    m_menuPlaylist->addAction(tr("Open"));
    m_menuPlaylist->addAction(tr("Edit..."));
    m_menuPlaylist->addAction(tr("Remove"));

    m_menuDefault = new QMenu(this);
    m_menuDefault->addAction(tr("New playlist..."));
    m_menuDefault->addAction(tr("New dynamic playlist..."));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openCustomMenuProject(const QPoint&)));
}


CPlayListView::~CPlayListView()
{

}


QModelIndex CPlayListView::addSongTable(CSongTable * songTable)
{
    Q_CHECK_PTR(songTable);

    QStandardItem * playListItem;
    CPlayList * playList = qobject_cast<CPlayList *>(songTable);

    if (playList)
    {
        playListItem = new QStandardItem(playList->getName());

        if (qobject_cast<CDynamicPlayList *>(playList))
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
        playListItem = new QStandardItem(tr("Library"));
        //playListItem->setIcon(QPixmap(":/icons/library"));
    }

    playListItem->setData(QVariant::fromValue(songTable));
    m_model->appendRow(playListItem);
    return playListItem->index();
}


CSongTable * CPlayListView::getSongTable(const QModelIndex& index) const
{
    return m_model->data(index, Qt::UserRole + 1).value<CSongTable *>();
}


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


/// \todo Implémentation.
void CPlayListView::openCustomMenuProject(const QPoint& point)
{
    qDebug() << "CPlayListView::openCustomMenuProject()";

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

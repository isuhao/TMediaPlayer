
#include "CPlayListView.hpp"
#include <QHeaderView>


CPlayListView::CPlayListView(QWidget * parent) :
    QTreeView (parent)
{
    header()->setVisible(false);
    setUniformRowHeights(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}


CPlayListView::~CPlayListView()
{

}

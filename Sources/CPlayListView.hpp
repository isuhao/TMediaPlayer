
#ifndef FILE_CPLAYLIST_VIEW
#define FILE_CPLAYLIST_VIEW

#include <QTreeView>


class CPlayListView : public QTreeView
{
    Q_OBJECT

public:

    CPlayListView(QWidget * parent = NULL);
    ~CPlayListView();
};

#endif // FILE_CPLAYLIST_VIEW

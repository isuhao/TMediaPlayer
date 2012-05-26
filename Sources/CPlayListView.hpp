
#ifndef FILE_CPLAYLIST_VIEW
#define FILE_CPLAYLIST_VIEW

#include <QTreeView>


class CPlayListModel;
class CSongTable;
class CApplication;


class CPlayListView : public QTreeView
{
    Q_OBJECT

public:

    CPlayListView(CApplication * application);
    ~CPlayListView();

    QModelIndex addSongTable(CSongTable * songTable);
    CSongTable * getSongTable(const QModelIndex& index) const;
    QModelIndex getModelIndex(CSongTable * songTable) const;

protected:

    virtual void dragMoveEvent(QDragMoveEvent * event);

protected slots:

    virtual void openCustomMenuProject(const QPoint& point);

private:
    
    CApplication * m_application;
    CPlayListModel * m_model; ///< Modèle utilisé pour afficher les listes de lecture.
    QMenu * m_menuPlaylist;   ///< Menu contextuel pour les listes de lecture.
    QMenu * m_menuDefault;    ///< Menu contextuel par défaut.
};

#endif // FILE_CPLAYLIST_VIEW

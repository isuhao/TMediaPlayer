
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

protected slots:

    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void openCustomMenuProject(const QPoint& point);

private:
    
    CPlayListModel * m_model;     ///< Modèle utilisé pour afficher les listes de lecture.
    CApplication * m_application; ///< Pointeur sur l'application;
    QMenu * m_menuPlaylist;       ///< Menu contextuel pour les listes de lecture.
    QMenu * m_menuDefault;        ///< Menu contextuel par défaut.
};

#endif // FILE_CPLAYLIST_VIEW

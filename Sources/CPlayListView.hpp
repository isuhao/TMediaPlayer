
#ifndef FILE_C_PLAYLIST_VIEW
#define FILE_C_PLAYLIST_VIEW

#include <QTreeView>


class CListModel;
class CSongTable;
class CApplication;
class CFolder;


/**
 * Vue utilisée pour afficher les listes de lecture sous forme d'un arbre.
 */

class CPlayListView : public QTreeView
{
    Q_OBJECT

public:

    explicit CPlayListView(CApplication * application);

    //QModelIndex addSongTable(CSongTable * songTable, const QModelIndex& parent = QModelIndex());
    //QModelIndex addFolder(CFolder * folder, const QModelIndex& parent = QModelIndex());
    //void removeSongTable(CSongTable * songTable);
    CSongTable * getSongTable(const QModelIndex& index) const;
    CFolder * getFolder(const QModelIndex& index) const;
    CSongTable * getSelectedSongTable(void) const;
    CFolder * getSelectedFolder(void) const;
    QModelIndex getSongTableModelIndex(CSongTable * songTable) const;
    QModelIndex getFolderModelIndex(CFolder * folder) const;
    void setModel(CListModel * model);

protected:
    
    //QModelIndex getModelIndex(CSongTable * songTable, const QModelIndex& parent = QModelIndex()) const;
    //QModelIndex getModelIndex(CFolder * folder, const QModelIndex& parent = QModelIndex()) const;
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void dragMoveEvent(QDragMoveEvent * event);

protected slots:

    virtual void openCustomMenuProject(const QPoint& point);
    void onItemCollapsed(const QModelIndex& index);
    void onItemExpanded(const QModelIndex& index);
    void onFolderOpen(void);
    void onFolderClose(void);
    void createStaticList(void);
    void createDynamicList(void);
    void createFolder(void);

private:
    
    CApplication * m_application; ///< Pointeur sur l'application.
    CListModel * m_model;         ///< Modèle utilisé pour afficher les listes de lecture.
    QMenu * m_menuPlaylist;       ///< Menu contextuel pour les listes de lecture.
    QMenu * m_menuFolder;         ///< Menu contextuel pour les dossiers.
    QMenu * m_menuDefault;        ///< Menu contextuel par défaut.
};

#endif // FILE_C_PLAYLIST_VIEW

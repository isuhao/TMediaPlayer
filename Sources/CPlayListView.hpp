
#ifndef FILE_C_PLAYLIST_VIEW
#define FILE_C_PLAYLIST_VIEW

#include <QTreeView>


class CPlayListModel;
class CSongTable;
class CApplication;
class CListFolder;


/**
 * Vue utilisée pour afficher les listes de lecture sous forme d'un arbre.
 */

class CPlayListView : public QTreeView
{
    Q_OBJECT

public:

    explicit CPlayListView(CApplication * application);

    QModelIndex addSongTable(CSongTable * songTable, const QModelIndex& parent = QModelIndex());
    QModelIndex addFolder(CListFolder * folder, const QModelIndex& parent = QModelIndex());
    void removeSongTable(CSongTable * songTable);
    CSongTable * getSongTable(const QModelIndex& index) const;
    CListFolder * getFolder(const QModelIndex& index) const;
    CSongTable * getSelectedSongTable(void) const;
    CListFolder * getSelectedFolder(void) const;
    QModelIndex getModelIndex(CSongTable * songTable) const;
    
public slots:

    void onPlayListRenamed(const QString& oldName, const QString& newName);
    void onFolderRenamed(const QString& oldName, const QString& newName);

protected:
    
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void dragMoveEvent(QDragMoveEvent * event);

protected slots:

    virtual void openCustomMenuProject(const QPoint& point);
    void onItemCollapsed(const QModelIndex& index);
    void onItemExpanded(const QModelIndex& index);
    void onFolderOpen(void);
    void onFolderClose(void);

private:
    
    CApplication * m_application;
    CPlayListModel * m_model; ///< Modèle utilisé pour afficher les listes de lecture.
    QMenu * m_menuPlaylist;   ///< Menu contextuel pour les listes de lecture.
    QMenu * m_menuFolder;     ///< Menu contextuel pour les dossiers.
    QMenu * m_menuDefault;    ///< Menu contextuel par défaut.
};

#endif // FILE_C_PLAYLIST_VIEW


#ifndef FILE_C_PLAYLIST_VIEW
#define FILE_C_PLAYLIST_VIEW

#include <QTreeView>


class CPlayListModel;
class CSongTable;
class CApplication;


class CPlayListView : public QTreeView
{
    Q_OBJECT

public:

    explicit CPlayListView(CApplication * application);

    QModelIndex addSongTable(CSongTable * songTable);
    CSongTable * getSongTable(const QModelIndex& index) const;
    CSongTable * getSelectedSongTable(void) const;
    QModelIndex getModelIndex(CSongTable * songTable) const;
    
public slots:

    void onPlayListRenamed(const QString& oldName, const QString& newName);

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

#endif // FILE_C_PLAYLIST_VIEW

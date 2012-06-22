
#ifndef FILE_C_LIST_MODEL
#define FILE_C_LIST_MODEL

#include <QStandardItemModel>

class CApplication;
class CFolder;
class CSong;
class CSongTable;
class IPlayList;


/**
 * Modèle permettant de stocker les listes de lecture et les dossiers.
 */

class CListModel : public QStandardItemModel
{
    Q_OBJECT

public:

    explicit CListModel(CApplication * application);
    virtual ~CListModel();

    void loadFromDatabase(void);
    virtual void clear(void);

    //inline CFolder * getRootFolder(void) const;
    inline QList<CFolder *> getFolders(void) const;
    inline QList<IPlayList *> getPlayLists(void) const;
    inline CFolder * getRootFolder(void) const;
    CFolder * getFolderFromId(int id) const;
    IPlayList * getPlayListFromId(int id) const;
    QModelIndex getModelIndex(CFolder * folder) const;
    QModelIndex getModelIndex(CSongTable * songTable) const;

    void addFolder(CFolder * folder);
    void addPlayList(IPlayList * playList);
    void removeFolder(CFolder * folder, bool recursive = false);
    void removePlayList(IPlayList * playList);

    // Glisser-déposer
    QMimeData * mimeData(const QModelIndexList& indexes) const;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    QStringList mimeTypes(void) const;
    Qt::DropActions supportedDropActions() const;
    
private slots:

    void onPlayListRenamed(const QString& oldName, const QString& newName);
    void onFolderRenamed(const QString& oldName, const QString& newName);

private:

    CFolder * getFolderFromId(int id, const QList<CFolder *> folders) const;
    IPlayList * getPlayListFromId(int id, const QList<IPlayList *> playLists) const;
    
    // Glisser-déposer
    QList<CSong *> decodeDataSongs(const QByteArray& encodedData) const;
    bool decodeDataList(const QByteArray& encodedData, int * playList, int * folder) const;

    CApplication * m_application; ///< Pointeur sur l'application.
    CFolder * m_rootFolder;       ///< Dossier principal.
    QMap<QStandardItem *, CFolder *> m_folderItems;
    QMap<QStandardItem *, CSongTable *> m_songTableItems;
    QList<CFolder *> m_folders;
    QList<IPlayList *> m_playLists;
};


inline QList<CFolder *> CListModel::getFolders(void) const
{
    return m_folders;
}


inline QList<IPlayList *> CListModel::getPlayLists(void) const
{
    return m_playLists;
}


inline CFolder * CListModel::getRootFolder(void) const
{
    return m_rootFolder;
}

#endif // FILE_C_LIST_MODEL

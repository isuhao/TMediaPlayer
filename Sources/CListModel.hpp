/*
Copyright (C) 2012 Teddy Michel

This file is part of TMediaPlayer.

TMediaPlayer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TMediaPlayer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TMediaPlayer. If not, see <http://www.gnu.org/licenses/>.
*/

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
    void openFolder(const QModelIndex& index, bool open);

    void addFolder(CFolder * folder);
    void addPlayList(IPlayList * playList);
    void removeFolder(CFolder * folder, bool recursive = false);
    void removePlayList(IPlayList * playList);

    // Glisser-déposer
    QMimeData * mimeData(const QModelIndexList& indexes) const;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    QStringList mimeTypes(void) const;
    Qt::DropActions supportedDropActions() const;
    
    QList<CSong *> decodeDataSongs(const QByteArray& encodedData) const;
    static bool decodeDataList(const QByteArray& encodedData, int * playList, int * folder);
    
private slots:

    void onPlayListRenamed(const QString& oldName, const QString& newName);
    void onFolderRenamed(const QString& oldName, const QString& newName);

private:

    CFolder * getFolderFromId(int id, const QList<CFolder *> folders) const;
    IPlayList * getPlayListFromId(int id, const QList<IPlayList *> playLists) const;

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

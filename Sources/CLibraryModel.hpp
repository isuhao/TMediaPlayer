/*
Copyright (C) 2012-2014 Teddy Michel

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

#ifndef FILE_C_LIBRARY_MODEL_HPP_
#define FILE_C_LIBRARY_MODEL_HPP_

#include <QStandardItemModel>

class CMainWindow;
class CFolder;
class CSong;
class CMediaTableView;
class CCDRomDrive;
class IPlayList;


/**
 * Modèle permettant de stocker les listes de lecture et les dossiers.
 */

class CLibraryModel : public QStandardItemModel ///TODO: hériter de QAbstractItemModel
{
    Q_OBJECT

public:

    explicit CLibraryModel(CMainWindow * mainWindow);
    virtual ~CLibraryModel();

    void loadFromDatabase();
    virtual void clear();
    void updateCDRomDrives();

    inline QList<CFolder *> getFolders() const;
    inline QList<IPlayList *> getPlayLists() const;
    inline CFolder * getRootFolder() const;
    CFolder * getFolderFromId(int id) const;
    IPlayList * getPlayListFromId(int id) const;
    QModelIndex getModelIndex(CFolder * folder) const;
    QModelIndex getModelIndex(CMediaTableView * songTable) const;
    void openFolder(const QModelIndex& index, bool open);

    void addFolder(CFolder * folder);
    void addPlayList(IPlayList * playList);
    void removeFolder(CFolder * folder, bool recursive = false);
    void removePlayList(IPlayList * playList);

    // Glisser-déposer
    QMimeData * mimeData(const QModelIndexList& indexes) const;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;

    QList<CSong *> decodeDataSongs(const QByteArray& encodedData) const;
    static bool decodeDataList(const QByteArray& encodedData, int * playList, int * folder);

private slots:

    void onPlayListRenamed(const QString& oldName, const QString& newName);
    void onFolderRenamed(const QString& oldName, const QString& newName);
    void onPlayListChange();

private:

    CFolder * getFolderFromId(int id, const QList<CFolder *> folders) const;
    IPlayList * getPlayListFromId(int id, const QList<IPlayList *> playLists) const;

    CMainWindow * m_mainWindow;    ///< Pointeur sur la fenêtre principale de l'application.
    CFolder * m_rootFolder;        ///< Dossier principal.
    QMap<QStandardItem *, CFolder *> m_folderItems;            ///< Tableau des items associés aux dossiers.
    QMap<QStandardItem *, CMediaTableView *> m_songTableItems; ///< Tableau des items associés aux listes de lecture.
    QMap<QStandardItem *, CCDRomDrive *> m_cdRomDrives;        ///< Tableau des items associés aux lecteurs de CD-ROM.
    QList<CFolder *> m_folders;         ///< Liste des dossiers.
    QList<IPlayList *> m_playLists;     ///< Liste des listes de lecture.
};


inline QList<CFolder *> CLibraryModel::getFolders() const
{
    return m_folders;
}


inline QList<IPlayList *> CLibraryModel::getPlayLists() const
{
    return m_playLists;
}


/**
 * Retourne le dossier principal, qui contient les dossiers et les listes de lecture.
 *
 * \return Pointeur sur le dossier principal.
 */

inline CFolder * CLibraryModel::getRootFolder() const
{
    return m_rootFolder;
}

#endif // FILE_C_LIBRARY_MODEL_HPP_

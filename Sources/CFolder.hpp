/*
Copyright (C) 2012-2015 Teddy Michel

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

#ifndef FILE_C_LIST_FOLDER
#define FILE_C_LIST_FOLDER

#include <QObject>
#include <QString>
#include <QList>
#include <QVector>
#include <QModelIndex>


class CMainWindow;
class IPlayList;
class CLibraryModel;
class CLibraryView;
class CDialogEditFolder;
class CDialogRemoveFolder;


/**
 * Dossier contenant des listes de lecture.
 */

class CFolder : public QObject
{
    Q_OBJECT

    friend class CMainWindow;
    friend class CLibraryModel;
    friend class CLibraryView;
    friend class CDialogEditFolder;
    friend class CDialogRemoveFolder;

public:

    struct TFolderItem
    {
        int position;         ///< Position sauvegardée.
        IPlayList * playList; ///< Pointeur sur la liste de lecture.
        CFolder * folder;     ///< Pointeur sur le dossier.

        inline TFolderItem(int pposition, IPlayList * pplayList) : position(pposition), playList(pplayList), folder(nullptr) { }
        inline TFolderItem(int pposition, CFolder * pfolder) : position(pposition), playList(nullptr), folder(pfolder) { }
    };

    explicit CFolder(CMainWindow * mainWindow, const QString& name = QString());
    virtual ~CFolder();

    inline int getId() const;
    inline QString getName() const;
    inline CFolder * getFolder() const;
    bool hasAncestor(CFolder * folder) const;
    inline bool isOpen() const;
    inline QList<IPlayList *> getPlayLists() const;
    inline QList<CFolder *> getFolders() const;
    inline QVector<TFolderItem *> getItems() const;
    inline int getNumPlayLists() const;
    inline int getNumFolders() const;
    bool isModified() const;
    int getPosition(IPlayList * playList) const;
    int getPosition(CFolder * folder) const;

signals:

    /**
     * Signal émis lorsque le nom du dossier change.
     * Utilisez la méthode sender() dans le slot pour connaitre le dossier.
     *
     * \param oldName Ancien nom du dossier.
     * \param newName Nouveau nom du dossier.
     */

    void nameChanged(const QString& oldName, const QString& newName);

    /**
     * Signal émis lorsque le dossier contenant le dossier change.
     * Utilisez la méthode sender() dans le slot pour connaitre le dossier.
     *
     * \param oldFolder Pointeur sur l'ancien dossier.
     * \param newFolder Pointeur sur le nouveau dossier.
     */

    void folderChanged(CFolder * oldFolder, CFolder * newFolder);

    void folderOpened(); ///< Signal émis lorsque le dossier est ouvert.
    void folderClosed(); ///< Signal émis lorsque le dossier est fermé.

public slots:

    void setName(const QString& name);
    void setFolder(CFolder * folder);
    void addPlayList(IPlayList * playList, int position = -1);
    void removePlayList(IPlayList * playList);
    void addFolder(CFolder * folder, int position = -1);
    void removeFolder(CFolder * folder);
    void setOpen(bool open = true);

protected:

    virtual bool updateDatabase();
    virtual void removeFromDatabase(bool recursive = false);
    void fixPositions();

private:

    void addPlayListItem(IPlayList * playList, int position = -1);
    void addFolderItem(CFolder * folder, int position = -1);
    void removePlayListItem(IPlayList * playList);
    void removeFolderItem(CFolder * folder);

    CMainWindow * m_mainWindow;      ///< Pointeur sur la fenêtre principale de l'application.
    int m_id;                        ///< Identifiant du dossier en base de données.
    QString m_name;                  ///< Nom du dossier.
    bool m_open;                     ///< Indique si le dossier est ouvert ou fermé.
    CFolder * m_folder;              ///< Dossier parent.
  //int m_position;                  ///< Position dans le dossier.
    bool m_isModified;               ///< Indique si le dossier a été modifié.
    bool m_folderChanging;           ///< Indique si le dossier parent est en train d'être changé.
  //QModelIndex m_index;             ///< Index du dossier dans la vue.
    QList<IPlayList *> m_playLists0; ///< Liste des listes de lecture du dossier.
    QList<CFolder *> m_folders;      ///< Liste des dossiers du dossier.
    QVector<TFolderItem *> m_items;  ///< Liste des éléments du dossier.
};

Q_DECLARE_METATYPE(CFolder *)


/**
 * Retourne l'identifiant du dossier en base de données.
 *
 * \return Identifiant du dossier.
 */

inline int CFolder::getId() const
{
    return m_id;
}


/**
 * Retourne le nom du dossier.
 *
 * \return Nom du dossier.
 */

inline QString CFolder::getName() const
{
    return m_name;
}


/**
 * Retourne le dossier contenant le dossier.
 *
 * \return Pointeur sur le dossier parent.
 */

inline CFolder * CFolder::getFolder() const
{
    return m_folder;
}


/**
 * Indique si le dossier est ouvert ou fermé.
 *
 * \return Booléen.
 */

inline bool CFolder::isOpen() const
{
    return m_open;
}


/**
 * Retourne la liste des listes de lecture du dossier.
 *
 * \return Liste des listes de lecture.
 */

inline QList<IPlayList *> CFolder::getPlayLists() const
{
    return m_playLists0;
}


/**
 * Retourne la liste des dossiers du dossier.
 *
 * \return Liste des dossiers.
 */

inline QList<CFolder *> CFolder::getFolders() const
{
    return m_folders;
}


inline QVector<CFolder::TFolderItem *> CFolder::getItems() const
{
    return m_items;
}


/**
 * Retourne le nombre de liste de lectures du dossier.
 *
 * \return Nombre de listes de lecture.
 */

inline int CFolder::getNumPlayLists() const
{
    return m_playLists0.size();
}


/**
 * Retourne le nombre de dossiers du dossier.
 *
 * \return Nombre de dossiers.
 */

inline int CFolder::getNumFolders() const
{
    return m_folders.size();
}

#endif // FILE_C_LIST_FOLDER

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

#ifndef FILE_C_LIST_FOLDER
#define FILE_C_LIST_FOLDER

#include <QObject>
#include <QString>
#include <QList>
#include <QVector>
#include <QModelIndex>


class CApplication;
class IPlayList;
class CListModel;
class CPlayListView;
class CDialogEditFolder;
class CDialogRemoveFolder;


/**
 * Dossier contenant des listes de lecture.
 */

class CFolder : public QObject
{
    Q_OBJECT

    friend class CApplication;
    friend class CListModel;
    friend class CPlayListView;
    friend class CDialogEditFolder;
    friend class CDialogRemoveFolder;

public:

    struct TFolderItem
    {
        int position;         ///< Position sauvegardée.
        IPlayList * playList; ///< Pointeur sur la liste de lecture.
        CFolder * folder;     ///< Pointeur sur le dossier.

        inline TFolderItem(int pposition, IPlayList * pplayList) : position(pposition), playList(pplayList), folder(NULL) { }
        inline TFolderItem(int pposition, CFolder * pfolder) : position(pposition), playList(NULL), folder(pfolder) { }
    };

    explicit CFolder(CApplication * application, const QString& name = QString());
    virtual ~CFolder();

    inline int getId(void) const;
    inline QString getName(void) const;
    inline CFolder * getFolder(void) const;
    bool hasAncestor(CFolder * folder) const;
    inline bool isOpen(void) const;
    inline QList<IPlayList *> getPlayLists(void) const;
    inline QList<CFolder *> getFolders(void) const;
    inline QVector<TFolderItem *> getItems(void) const;
    inline int getNumPlayLists(void) const;
    inline int getNumFolders(void) const;
    bool isModified(void) const;
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

    void folderOpened(void); ///< Signal émis lorsque le dossier est ouvert.
    void folderClosed(void); ///< Signal émis lorsque le dossier est fermé.

public slots:

    void setName(const QString& name);
    void setFolder(CFolder * folder);
    void addPlayList(IPlayList * playList, int position = -1);
    void removePlayList(IPlayList * playList);
    void addFolder(CFolder * folder, int position = -1);
    void removeFolder(CFolder * folder);
    void setOpen(bool open = true);

protected:
    
    virtual bool updateDatabase(void);
    virtual void removeFromDatabase(bool recursive = false);
    void fixPositions(void);

private:
    
    void addPlayListItem(IPlayList * playList, int position = -1);
    void addFolderItem(CFolder * folder, int position = -1);
    void removePlayListItem(IPlayList * playList);
    void removeFolderItem(CFolder * folder);

    CApplication * m_application;   ///< Pointeur sur l'application.
    int m_id;                       ///< Identifiant du dossier en base de données.
    QString m_name;                 ///< Nom du dossier.
    bool m_open;                    ///< Indique si le dossier est ouvert ou fermé.
    CFolder * m_folder;             ///< Dossier parent.
    //int m_position;                 ///< Position dans le dossier.
    bool m_isModified;              ///< Indique si le dossier a été modifié.
    bool m_folderChanging;          ///< Indique si le dossier parent est en train d'être changé.
    //QModelIndex m_index;            ///< Index du dossier dans la vue.
    QList<IPlayList *> m_playLists0; ///< Liste des listes de lecture du dossier.
    QList<CFolder *> m_folders0;     ///< Liste des dossiers du dossier.
    QVector<TFolderItem *> m_items;  ///< Liste des éléments du dossier.
};

Q_DECLARE_METATYPE(CFolder *)


/**
 * Retourne l'identifiant du dossier en base de données.
 *
 * \return Identifiant du dossier.
 */

inline int CFolder::getId(void) const
{
    return m_id;
}


/**
 * Retourne le nom du dossier.
 *
 * \return Nom du dossier.
 */

inline QString CFolder::getName(void) const
{
    return m_name;
}


/**
 * Retourne le dossier contenant le dossier.
 *
 * \return Pointeur sur le dossier parent.
 */

inline CFolder * CFolder::getFolder(void) const
{
    return m_folder;
}


/**
 * Indique si le dossier est ouvert ou fermé.
 *
 * \return Booléen.
 */

inline bool CFolder::isOpen(void) const
{
    return m_open;
}


/**
 * Retourne la liste des listes de lecture du dossier.
 *
 * \return Liste des listes de lecture.
 */

inline QList<IPlayList *> CFolder::getPlayLists(void) const
{
    return m_playLists0;
}


/**
 * Retourne la liste des dossiers du dossier.
 *
 * \return Liste des dossiers.
 */

inline QList<CFolder *> CFolder::getFolders(void) const
{
    return m_folders0;
}


inline QVector<CFolder::TFolderItem *> CFolder::getItems(void) const
{
    return m_items;
}


/**
 * Retourne le nombre de liste de lectures du dossier.
 *
 * \return Nombre de listes de lecture.
 */

inline int CFolder::getNumPlayLists(void) const
{
    return m_playLists0.size();
}


/**
 * Retourne le nombre de dossiers du dossier.
 *
 * \return Nombre de dossiers.
 */

inline int CFolder::getNumFolders(void) const
{
    return m_folders0.size();
}

#endif // FILE_C_LIST_FOLDER

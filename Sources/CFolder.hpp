
#ifndef FILE_C_LIST_FOLDER
#define FILE_C_LIST_FOLDER

#include <QObject>
#include <QString>
#include <QList>
#include <QModelIndex>


class CApplication;
class IPlayList;
class CPlayListView;
class CDialogEditFolder;


/**
 * Dossier contenant des listes de lecture.
 */

class CFolder : public QObject
{
    Q_OBJECT

    friend class CApplication;
    friend class CDialogEditFolder;
    friend class CPlayListView;
    friend class CListModel;

public:

    explicit CFolder(CApplication * application, const QString& name = QString());
    virtual ~CFolder();

    inline int getId(void) const;
    inline QString getName(void) const;
    inline CFolder * getFolder(void) const;
    inline bool isOpen(void) const;
    inline QList<IPlayList *> getPlayLists(void) const;
    inline QList<CFolder *> getFolders(void) const;
    inline int getNumPlayLists(void) const;
    inline int getNumFolders(void) const;
    bool isModified(void) const;
    inline QModelIndex getModelIndex(void) const;
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

protected slots:

    virtual bool updateDatabase(void);

private:

    CApplication * m_application;   ///< Pointeur sur l'application.
    int m_id;                       ///< Identifiant du dossier en base de données.
    QString m_name;                 ///< Nom du dossier.
    bool m_open;                    ///< Indique si le dossier est ouvert ou fermé.
    CFolder * m_folder;             ///< Dossier parent.
    int m_position;                 ///< Position dans le dossier.
    bool m_isModified;              ///< Indique si le dossier a été modifié.
    bool m_folderChanging;          ///< Indique si le dossier parent est en train d'être changé.
    QModelIndex m_index;            ///< Index du dossier dans la vue.
    QList<IPlayList *> m_playLists; ///< Liste des listes de lecture du dossier (l'ordre est le même que l'affichage).
    QList<CFolder *> m_folders;     ///< Liste des dossiers du dossier (l'ordre est le même que l'affichage).
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
    return m_playLists;
}


/**
 * Retourne la liste des dossiers du dossier.
 *
 * \return Liste des dossiers.
 */

inline QList<CFolder *> CFolder::getFolders(void) const
{
    return m_folders;
}


/**
 * Retourne le nombre de liste de lectures du dossier.
 *
 * \return Nombre de listes de lecture.
 */

inline int CFolder::getNumPlayLists(void) const
{
    return m_playLists.size();
}


/**
 * Retourne le nombre de dossiers du dossier.
 *
 * \return Nombre de dossiers.
 */

inline int CFolder::getNumFolders(void) const
{
    return m_folders.size();
}


inline QModelIndex CFolder::getModelIndex(void) const
{
    return m_index;
}

#endif // FILE_C_LIST_FOLDER

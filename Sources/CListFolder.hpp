
#ifndef FILE_C_LIST_FOLDER
#define FILE_C_LIST_FOLDER

#include <QObject>
#include <QString>
#include <QList>
#include <QModelIndex>


class CApplication;
class CPlayList;
class CPlayListView;
class CDialogEditFolder;


/**
 * Dossier contenant des listes de lecture.
 */

class CListFolder : public QObject
{
    Q_OBJECT

    friend class CApplication;
    friend class CDialogEditFolder;
    friend class CPlayListView;

public:

    explicit CListFolder(CApplication * application, const QString& name = QString());
    virtual ~CListFolder();

    inline int getId(void) const;
    inline QString getName(void) const;
    inline bool isOpen(void) const;
    inline QList<CPlayList *> getPlayLists(void) const;
    inline QList<CListFolder *> getFolders(void) const;
    inline int getNumPlayLists(void) const;
    bool isModified(void) const;
    inline QModelIndex getModelIndex(void) const;

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

    void folderChanged(CListFolder * oldFolder, CListFolder * newFolder);

    void folderOpened(void); ///< Signal émis lorsque le dossier est ouvert.
    void folderClosed(void); ///< Signal émis lorsque le dossier est fermé.

public slots:

    void setName(const QString& name);
    void setFolder(CListFolder * folder);
    void addPlayList(CPlayList * playList);
    void removePlayList(CPlayList * playList);
    void addFolder(CListFolder * folder);
    void removeFolder(CListFolder * folder);
    void setOpen(bool open = true);

protected slots:

    virtual bool updateDatabase(void);

private:

    CApplication * m_application;   ///< Pointeur sur l'application.
    int m_id;                       ///< Identifiant du dossier en base de données.
    QString m_name;                 ///< Nom du dossier.
    bool m_open;                    ///< Indique si le dossier est ouvert ou fermé.
    CListFolder * m_folder;         ///< Dossier parent.
    int m_position;                 ///< Position dans le dossier.
    bool m_isModified;              ///< Indique si le dossier a été modifié.
    bool m_folderChanging;          ///< Indique si le dossier est en train d'être changé.
    QModelIndex m_index;            ///< Index du dossier dans la vue.
    QList<CPlayList *> m_playLists; ///< Liste des listes de lecture du dossier (l'ordre est le même que l'affichage).
    QList<CListFolder *> m_folders; ///< Liste des dossiers du dossier (l'ordre est le même que l'affichage).
};

Q_DECLARE_METATYPE(CListFolder *)


inline int CListFolder::getId(void) const
{
    return m_id;
}


/**
 * Retourne le nom du dossier.
 *
 * \return Nom du dossier.
 */

inline QString CListFolder::getName(void) const
{
    return m_name;
}


/**
 * Indique si le dossier est ouvert ou fermé.
 *
 * \return Booléen.
 */

inline bool CListFolder::isOpen(void) const
{
    return m_open;
}


inline QList<CPlayList *> CListFolder::getPlayLists(void) const
{
    return m_playLists;
}


inline QList<CListFolder *> CListFolder::getFolders(void) const
{
    return m_folders;
}


inline int CListFolder::getNumPlayLists(void) const
{
    return m_playLists.size();
}


inline QModelIndex CListFolder::getModelIndex(void) const
{
    return m_index;
}

#endif // FILE_C_LIST_FOLDER

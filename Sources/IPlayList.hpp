
#ifndef FILE_I_PLAYLIST
#define FILE_I_PLAYLIST

#include <QString>
#include "CSongTable.hpp"


class CFolder;


/**
 * Classe de base des listes de lecture.
 * Gère les paramètres communs aux listes : le nom, le dossier, et la position dans le dossier.
 */

class IPlayList : public CSongTable
{
    Q_OBJECT

    friend class CApplication;
    friend class CFolder;
    friend class CPlayListView;
    friend class CListModel;

public:

    explicit IPlayList(CApplication * application, const QString& name = QString());
    virtual ~IPlayList() = 0;

    inline QString getName(void) const;
    inline CFolder * getFolder(void) const;
    virtual bool isModified(void) const;

public slots:

    void setName(const QString& name);
    void setFolder(CFolder * folder);

signals:

    /**
     * Signal émis lorsque le nom de la liste change.
     * Utilisez la méthode sender() dans le slot pour connaitre la liste de lecture.
     *
     * \param oldName Ancien nom de la liste.
     * \param newName Nouveau nom de la liste.
     */

    void nameChanged(const QString& oldName, const QString& newName);
    
    /**
     * Signal émis lorsque le dossier contenant la liste change.
     * Utilisez la méthode sender() dans le slot pour connaitre la liste de lecture.
     *
     * \param oldFolder Pointeur sur l'ancien dossier.
     * \param newFolder Pointeur sur le nouveau dossier.
     */

    void folderChanged(CFolder * oldFolder, CFolder * newFolder);

    /**
     * Signal émis lorsque le contenu de la liste change.
     * Utilisez la méthode sender() dans le slot pour connaitre la liste de lecture.
     */

    void listModified();

protected:

    virtual bool updateDatabase(void);
    virtual void romoveFromDatabase(void);

    QString m_name;            ///< Nom de la liste de lecture.
    int m_position;            ///< Position de la liste dans le dossier.

private:

    CFolder * m_folder;    ///< Dossier contenant la liste.
    bool m_isPlayListModified; ///< Indique si la liste de lecture a été modifiée.
    bool m_folderChanging;     ///< Indique si le dossier est en train d'être changé.
    QModelIndex m_index;       ///< Index de la liste dans la vue.
};


/**
 * Retourne le nom de la liste de lecture.
 *
 * \return Nom de la liste.
 */

inline QString IPlayList::getName(void) const
{
    return m_name;
}


/**
 * Retourne le dossier contenant la liste de lecture.
 *
 * \return Pointeur sur le dossier, ou NULL si la liste est à la racine.
 */

inline CFolder * IPlayList::getFolder(void) const
{
    return m_folder;
}

#endif // FILE_I_PLAYLIST

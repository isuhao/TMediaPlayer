
#ifndef FILE_CPLAYLIST
#define FILE_CPLAYLIST

#include <QString>
#include "CSongTable.hpp"


class CListFolder;


/**
 * Classe de base des listes de lecture.
 */

class CPlayList : public CSongTable
{
    Q_OBJECT

    friend class CApplication;
    friend class CListFolder;

public:

    explicit CPlayList(CApplication * application, const QString& name = QString());
    virtual ~CPlayList() = 0;

    inline QString getName(void) const;
    inline CListFolder * getFolder(void) const;
    virtual bool isModified(void) const;

public slots:

    void setName(const QString& name);
    void setFolder(CListFolder * folder);

signals:

    void nameChanged(const QString& oldName, const QString& newName); ///< Signal émis lorsque le nom de la liste change.
    void folderChanged(CListFolder * oldFolder, CListFolder * newFolder); ///< Signal émis lorsque le dossier contenant la liste change.
    void listModified(); ///< Signal émis lorsque le contenu de la liste change.

protected:

    virtual bool updateDatabase(void);

    QString m_name;            ///< Nom de la liste de lecture.
    int m_position;            ///< Position de la liste dans le dossier.

private:

    CListFolder * m_folder;    ///< Dossier contenant la liste.
    bool m_isPlayListModified; ///< Indique si la liste de lecture a été modifiée.
    bool m_folderChanging;     ///< Indique si le dossier est en train d'être changé.
};


inline QString CPlayList::getName(void) const
{
    return m_name;
}


inline CListFolder * CPlayList::getFolder(void) const
{
    return m_folder;
}

#endif

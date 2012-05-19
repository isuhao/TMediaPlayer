
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

public:

    CPlayList(const QString& name = QString());
    virtual ~CPlayList() = 0;

    inline QString getName(void) const;
    inline CListFolder * getFolder(void) const;

public slots:

    void setName(const QString& name);
    void setFolder(CListFolder * folder);

signals:

    void nameChanged(const QString& oldName, const QString& newName); ///< Signal émis lorsque le nom de la liste change.
    void folderChanged(CListFolder * oldFolder, CListFolder * newFolder); ///< Signal émis lorsque le dossier contenant la liste change.
    void listModified(); ///< Signal émis lorsque le contenu de la liste change.

private:

    QString m_name;         ///< Nom de la liste de lecture.
    CListFolder * m_folder; ///< Dossier contenant la liste.
    bool m_folderChanging;
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

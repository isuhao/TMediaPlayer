
#ifndef FILE_CLISTFOLDER
#define FILE_CLISTFOLDER

#include <QObject>
#include <QString>
#include <QList>


class CPlayList;


/**
 * Dossier contenant des listes de lecture.
 */

class CListFolder : public QObject
{
    Q_OBJECT

public:

    CListFolder(void);
    ~CListFolder();

    inline QString getName(void) const;
    inline QList<CPlayList *> getPlayLists(void) const;
    inline int getNumPlayLists(void) const;
    bool isModified(void) const;

public slots:

    void setName(const QString& name);
    void addPlayList(CPlayList * playList);
    void removePlayList(CPlayList * playList);

private:

    int m_id;                       ///< Identifiant du dossier en base de donn�es.
    QString m_name;                 ///< Nom du dossier.
    bool m_isModified;              ///< Indique si le dossier a �t� modifi�.
    QList<CPlayList *> m_playLists; ///< Liste des listes de lecture du dossier (l'ordre est le m�me que l'affichage).
};


/**
 * Retourne le nom du dossier.
 *
 * \return Nom du dossier.
 */

inline QString CListFolder::getName(void) const
{
    return m_name;
}


inline QList<CPlayList *> CListFolder::getPlayLists(void) const
{
    return m_playLists;
}


inline int CListFolder::getNumPlayLists(void) const
{
    return m_playLists.size();
}

#endif
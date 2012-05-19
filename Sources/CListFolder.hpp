
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

public slots:

    void setName(const QString& name);
    void addPlayList(CPlayList * playList);
    void removePlayList(CPlayList * playList);

private:

    QString m_name;
    QList<CPlayList *> m_playLists;
};


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

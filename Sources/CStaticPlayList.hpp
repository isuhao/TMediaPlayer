
#ifndef FILE_CSTATICPLAYLIST
#define FILE_CSTATICPLAYLIST

#include <QString>
#include "CPlayList.hpp"


class CListFolder;


class CStaticPlayList : public CPlayList
{
    Q_OBJECT

    friend class CDialogEditStaticPlayList;
    friend class CApplication;
    friend class CListFolder;

public:

    CStaticPlayList(CApplication * application, const QString& name = QString());
    ~CStaticPlayList();

    virtual bool isModified(void) const;

public slots:

    void addSong(CSong * song, int pos = -1);
    void addSongs(const QList<CSong *>& songs);
    void removeSong(CSong * song);
    void removeSong(int pos);
    void removeDuplicateSongs(void);

signals:

    void songAdded(CSong * song);   ///< Signal émis lorsqu'une chanson est ajoutée à la liste.
    void songRemoved(CSong * song); ///< Signal émis lorsqu'une chanson est enlevée de la liste.
    void songMoved(CSong * song);   ///< Signal émis lorsqu'une chanson est déplacée dans la liste.

protected slots:

    virtual bool updateDatabase(void);
    //virtual void openCustomMenuProject(const QPoint& point);

protected:

    virtual void initColumns(const QString& str);

private:

    int m_id;                    ///< Identifiant de la liste en base de données.
    bool m_isStaticListModified; ///< Indique si la liste a été modifiée.
};

#endif

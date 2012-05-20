
#ifndef FILE_CSTATICPLAYLIST
#define FILE_CSTATICPLAYLIST

#include <QString>
#include "CPlayList.hpp"


class CListFolder;


class CStaticPlayList : public CPlayList
{
    Q_OBJECT

    friend class CDialogEditStaticPlayList;

public:

    CStaticPlayList(CApplication * application, const QString& name = QString());
    ~CStaticPlayList();

public slots:

    virtual void addSong(CSong * song, int pos = -1);
    virtual void removeSong(CSong * song);
    virtual void removeSong(int pos);
    virtual void removeDuplicateSongs(void);

signals:

    void songAdded(CSong * song);   ///< Signal émis lorsqu'une chanson est ajoutée à la liste.
    void songRemoved(CSong * song); ///< Signal émis lorsqu'une chanson est enlevée de la liste.
    void songMoved(CSong * song);   ///< Signal émis lorsqu'une chanson est déplacée dans la liste.

protected slots:

    bool updateDatabase(void);

private:

    int m_id; ///< Identifiant de la liste en base de données.
};

#endif

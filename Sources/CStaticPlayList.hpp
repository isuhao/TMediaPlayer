
#ifndef FILE_C_STATIC_PLAYLIST
#define FILE_C_STATIC_PLAYLIST

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

    explicit CStaticPlayList(CApplication * application, const QString& name = QString());
    ~CStaticPlayList();

    virtual bool isModified(void) const;

public slots:

    void addSong(CSong * song, int pos = -1);
    void addSongs(const QList<CSong *>& songs, bool confirm = true);
    //void removeSong(CSong * song);
    //void removeSong(int row);
    void removeSongs(void);
    void removeDuplicateSongs(void);

signals:

    void songAdded(CSong * song);   ///< Signal émis lorsqu'une chanson est ajoutée à la liste.
    void songRemoved(CSong * song); ///< Signal émis lorsqu'une chanson est enlevée de la liste.
    void songMoved(CSong * song);   ///< Signal émis lorsqu'une chanson est déplacée dans la liste.

protected slots:

    virtual bool updateDatabase(void);
    virtual void openCustomMenuProject(const QPoint& point);

protected:

    virtual void keyPressEvent(QKeyEvent * event);

private:

    int m_id;                    ///< Identifiant de la liste en base de données.
    bool m_isStaticListModified; ///< Indique si la liste a été modifiée.
};

#endif // FILE_C_STATIC_PLAYLIST

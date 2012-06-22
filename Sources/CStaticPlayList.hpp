
#ifndef FILE_C_STATIC_PLAYLIST
#define FILE_C_STATIC_PLAYLIST

#include <QString>
#include "IPlayList.hpp"


class CFolder;


class CStaticPlayList : public IPlayList
{
    Q_OBJECT

    friend class CDialogEditStaticPlayList;
    friend class CApplication;
    friend class CFolder;
    friend class CListModel;

public:

    explicit CStaticPlayList(CApplication * application, const QString& name = QString());
    virtual ~CStaticPlayList();

    virtual bool isModified(void) const;

public slots:

    void addSong(CSong * song, int pos = -1);
    void addSongs(const QList<CSong *>& songs, bool confirm = true);
    void removeSong(CSong * song, bool confirm = true);
    void removeSong(CSongTableItem * songItem, bool confirm = true);
    void removeSongs(const QList<CSong *>& songs, bool confirm = true);
    void removeSongs(const QList<CSongTableItem *>& songItemList, bool confirm = true);
    void removeSelectedSongs(void);
    void removeDuplicateSongs(void);

signals:

    void songAdded(CSong * song);   ///< Signal émis lorsqu'une chanson est ajoutée à la liste.     \todo Remplacer CSong par CSongTableItem ?
    void songRemoved(CSong * song); ///< Signal émis lorsqu'une chanson est enlevée de la liste.    \todo Remplacer CSong par CSongTableItem ?
    void songMoved(CSong * song);   ///< Signal émis lorsqu'une chanson est déplacée dans la liste. \todo Remplacer CSong par CSongTableItem ?

protected slots:

    virtual bool updateDatabase(void);
    virtual void romoveFromDatabase(void);
    virtual void openCustomMenuProject(const QPoint& point);

protected:

    virtual void keyPressEvent(QKeyEvent * event);
    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void dropEvent(QDropEvent * event);
    virtual void paintEvent(QPaintEvent * event);

private:

    int m_id;                    ///< Identifiant de la liste en base de données.
    bool m_isStaticListModified; ///< Indique si la liste a été modifiée.
    QRect m_dropIndicatorRect;
};

#endif // FILE_C_STATIC_PLAYLIST

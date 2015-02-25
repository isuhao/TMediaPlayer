/*
Copyright (C) 2012-2015 Teddy Michel

This file is part of TMediaPlayer.

TMediaPlayer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TMediaPlayer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TMediaPlayer. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FILE_C_STATIC_PLAYLIST
#define FILE_C_STATIC_PLAYLIST

#include <QString>
#include "IPlayList.hpp"


class CFolder;


class CStaticList : public IPlayList
{
    Q_OBJECT

    friend class CDialogEditStaticPlayList;
    friend class CMainWindow;
    friend class CFolder;
    friend class CLibraryModel;

public:

    explicit CStaticList(CMainWindow * mainWindow, const QString& name = QString());
    virtual ~CStaticList();

    virtual bool isModified() const;

public slots:

    void addSong(CSong * song, int pos = -1);
    void addSongs(const QList<CSong *>& songs, bool confirm = true);
    void removeSong(CSong * song, bool confirm = true);
    void removeSong(CMediaTableItem * songItem, bool confirm = true);
    void removeSongs(const QList<CSong *>& songs, bool confirm = true);
    void removeSongs(const QList<CMediaTableItem *>& songItemList, bool confirm = true);
    void removeSelectedSongs();
    virtual void removeDuplicateSongs();

signals:

    void songAdded(CSong * song);   ///< Signal émis lorsqu'une chanson est ajoutée à la liste.     \todo Remplacer CSong par CMediaTableItem ?
    void songRemoved(CSong * song); ///< Signal émis lorsqu'une chanson est enlevée de la liste.    \todo Remplacer CSong par CMediaTableItem ?
    void songMoved(CSong * song);   ///< Signal émis lorsqu'une chanson est déplacée dans la liste. \todo Remplacer CSong par CMediaTableItem ?

protected slots:

    virtual bool updateDatabase();
    virtual void removeFromDatabase();

protected:

    virtual void keyPressEvent(QKeyEvent * event);
    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void dropEvent(QDropEvent * event);
    virtual void paintEvent(QPaintEvent * event);

private:

    virtual bool canEditPlayList() const
    {
        return true;
    }

    int m_id;                    ///< Identifiant de la liste en base de données.
    bool m_isStaticListModified; ///< Indique si la liste a été modifiée.
    QRect m_dropIndicatorRect;
};

#endif // FILE_C_STATIC_PLAYLIST

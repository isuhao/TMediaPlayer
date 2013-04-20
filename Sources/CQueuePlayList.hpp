/*
Copyright (C) 2012-2013 Teddy Michel

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

#ifndef FILE_C_QUEUE_PLAYLIST
#define FILE_C_QUEUE_PLAYLIST

#include "CSongTable.hpp"


class CQueuePlayList : public CSongTable
{
    Q_OBJECT

public:

    explicit CQueuePlayList(CApplication * application);
    virtual ~CQueuePlayList();

    virtual bool isModified() const;

    void addSongs(const QList<CSong *>& songs, int position = -1);

protected slots:

    virtual bool updateDatabase();
    virtual void openCustomMenuProject(const QPoint& point);
    void removeSelectedSongs();
    void removeSongs(const QList<CSongTableItem *>& songItemList);

protected:

    virtual void keyPressEvent(QKeyEvent * event);
    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void dropEvent(QDropEvent * event);
    virtual void paintEvent(QPaintEvent * event);

private:

    QRect m_dropIndicatorRect;
};

#endif // FILE_C_QUEUE_PLAYLIST

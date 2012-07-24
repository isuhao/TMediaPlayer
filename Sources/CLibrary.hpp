/*
Copyright (C) 2012 Teddy Michel

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

#ifndef FILE_C_LIBRARY
#define FILE_C_LIBRARY

#include "CSongTable.hpp"

class CSong;


class CLibrary : public CSongTable
{
    Q_OBJECT

public:

    explicit CLibrary(CApplication * application);
    
    void deleteSongs(void);
    inline QMap<int, CSong *> getSongsMap(void) const;
    
public slots:

    virtual void addSong(CSong * song, int pos = -1);
    virtual void addSongs(const QList<CSong *>& songs);
    virtual void removeSong(CSong * song);
    virtual void removeSong(int pos);

private:

    QMap<int, CSong *> m_songs;
};


inline QMap<int, CSong *> CLibrary::getSongsMap(void) const
{
    return m_songs;
}

#endif // FILE_C_LIBRARY

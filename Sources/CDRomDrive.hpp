/*
Copyright (C) 2012-2014 Teddy Michel

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

/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2007 Lukas Lalinsky

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

     $Id: disc_win32_new.c 9184 2007-06-17 15:46:00Z luks $

----------------------------------------------------------------------------*/

#ifndef FILE_CDROM_DRIVE
#define FILE_CDROM_DRIVE

#include <QString>
#include <QtDebug>

#ifdef Q_OS_WIN32
#   include <windows.h>
#endif


struct TDiscInfos
{
    int firstTrack;
    int lastTrack;
    int trackOffsets[100];

    TDiscInfos() : firstTrack(0), lastTrack(0)
    {
        for (int track = 0; track < 100; ++track)
            trackOffsets[track] = 0;
    }
};


class CDiscInfos
{
public:

    explicit CDiscInfos(const QString& deviceName);
    ~CDiscInfos();

    bool hasDiscInDrive();
    bool readInfos();
    void ejectDisc();

    TDiscInfos infos;

private:

    QString m_deviceName;

#ifdef Q_OS_WIN32
    HANDLE m_device;
#endif
};

#endif // FILE_CDROM_DRIVE

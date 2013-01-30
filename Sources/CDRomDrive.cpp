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

#include "CDRomDrive.hpp"


#if defined(Q_OS_WIN32)

#include <windows.h>
#include <string.h>
#include <stdio.h>


#define CD_FRAMES       75
#define XA_INTERVAL     ((60 + 90 + 2) * CD_FRAMES)

#define IOCTL_CDROM_READ_TOC         0x24000
#define IOCTL_CDROM_GET_LAST_SESSION 0x24038


struct TRACK_DATA
{
    UCHAR  Reserved;
    UCHAR  Control : 4;
    UCHAR  Adr : 4;
    UCHAR  TrackNumber;
    UCHAR  Reserved1;
    UCHAR  Address[4];
};

struct CDROM_TOC
{
    UCHAR  Length[2];
    UCHAR  FirstTrack;
    UCHAR  LastTrack;
    TRACK_DATA  TrackData[100];
};

typedef struct _CDROM_TOC_SESSION_DATA
{
    UCHAR  Length[2];
    UCHAR  FirstCompleteSession;
    UCHAR  LastCompleteSession;
    TRACK_DATA  TrackData[1];
} CDROM_TOC_SESSION_DATA;


int AddressToSectors(UCHAR address[4])
{
	return (address[1] * 4500 + address[2] * 75 + address[3]);
}


/// \todo Utiliser les méthodes de QString
CDiscInfos::CDiscInfos(const QString& deviceName) :
m_device (INVALID_HANDLE_VALUE)
{
    int colon = deviceName.indexOf(':');

    if (colon < 0)
    {
        m_deviceName = "\\\\.\\" + deviceName + ":";
    }
    else if (colon > 0)
    {
        m_deviceName = "\\\\.\\" + deviceName.mid(colon - 1, 2);
    }
    else
    {
        m_deviceName = "\\\\.\\D:";
    }
}


CDiscInfos::~CDiscInfos()
{
    if (m_device != INVALID_HANDLE_VALUE)
        CloseHandle(m_device);
}


bool CDiscInfos::hasDiscInDrive()
{
	if (m_device == INVALID_HANDLE_VALUE)
    {
	    m_device = CreateFileA(qPrintable(m_deviceName), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	    if (m_device == INVALID_HANDLE_VALUE)
        {
            qWarning() << "couldn't open the CD audio device";
		    return false;
	    }
    
	    CDROM_TOC_SESSION_DATA session;
	    DWORD dwReturned;
	    BOOL bResult = DeviceIoControl(m_device, IOCTL_CDROM_GET_LAST_SESSION, NULL, 0, &session, sizeof(session), &dwReturned, NULL);

	    if (bResult == FALSE)
        {
            qWarning() << "error while reading the CD TOC";
		    CloseHandle(m_device);
            m_device = INVALID_HANDLE_VALUE;
		    return false;
	    }

        return true;
	}
    else
    {
        DWORD cbBytesReturned;
        BOOL bResult = DeviceIoControl(m_device, IOCTL_STORAGE_CHECK_VERIFY2, NULL, 0, NULL, 0, &cbBytesReturned, NULL);
        return (bResult != FALSE);
    }
}


bool CDiscInfos::readInfos()
{
	m_device = CreateFileA(qPrintable(m_deviceName), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (m_device == INVALID_HANDLE_VALUE)
    {
        qWarning() << "couldn't open the CD audio device";
		return false;
	}
    
	CDROM_TOC_SESSION_DATA session;
	DWORD dwReturned;
	BOOL bResult = DeviceIoControl(m_device, IOCTL_CDROM_GET_LAST_SESSION, NULL, 0, &session, sizeof(session), &dwReturned, NULL);

	if (bResult == FALSE)
    {
        qWarning() << "error while reading the CD TOC";
		CloseHandle(m_device);
        m_device = INVALID_HANDLE_VALUE;
		return false;
	}
    
	CDROM_TOC toc;
	bResult = DeviceIoControl(m_device, IOCTL_CDROM_READ_TOC, NULL, 0, &toc, sizeof(toc), &dwReturned, NULL);

	if (bResult == FALSE)
    {
        qWarning() << "error while reading the CD TOC";
		CloseHandle(m_device);
        m_device = INVALID_HANDLE_VALUE;
		return false;
	}
/*
	CloseHandle(m_device);
    m_device = INVALID_HANDLE_VALUE;
*/
	infos.firstTrack = toc.FirstTrack;
	infos.lastTrack = toc.LastTrack;

	// Multi-session disc
	if (session.FirstCompleteSession != session.LastCompleteSession)
    {
		infos.lastTrack = session.TrackData[0].TrackNumber - 1;
		infos.trackOffsets[0] = AddressToSectors(toc.TrackData[infos.lastTrack].Address) - XA_INTERVAL;
	}
	else
    {
		infos.trackOffsets[0] = AddressToSectors(toc.TrackData[infos.lastTrack].Address);
	}

	for (int i = infos.firstTrack; i <= infos.lastTrack; ++i)
    {
		infos.trackOffsets[i] = AddressToSectors(toc.TrackData[i - 1].Address);
	}

	return true;
}


void CDiscInfos::ejectDisc()
{
    if (m_device == INVALID_HANDLE_VALUE)
    {
	    m_device = CreateFileA(qPrintable(m_deviceName), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	    if (m_device == INVALID_HANDLE_VALUE)
        {
            qWarning() << "couldn't open the CD audio device";
		    return;
	    }
	}

    DWORD cbBytesReturned;
    BOOL bResult = DeviceIoControl(m_device, IOCTL_STORAGE_EJECT_MEDIA, NULL, 0, NULL, 0, &cbBytesReturned, NULL);
}


#elif defined(Q_OS_LINUX)


#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>


#define XA_INTERVAL ((60 + 90 + 2) * CD_FRAMES)


static int read_toc_header(int fd, int * first, int * last)
{
	struct cdrom_tochdr th;
	struct cdrom_multisession ms;

	int ret = ioctl(fd, CDROMREADTOCHDR, &th);

	if (ret < 0)
		return ret; // error

	*first = th.cdth_trk0;
	*last = th.cdth_trk1;

	/*
	 * Hide the last track if this is a multisession disc. Note that
	 * currently only dual-session discs with one track in the second
	 * session are handled correctly.
	 */
	ms.addr_format = CDROM_LBA;
	ret = ioctl(fd, CDROMMULTISESSION, &ms);

	if (ms.xa_flag)
		--(*last);

	return ret;
}


static int read_toc_entry(int fd, int track_num, unsigned long * lba)
{
	struct cdrom_tocentry te;

	te.cdte_track = track_num;
	te.cdte_format = CDROM_LBA;

	int ret = ioctl(fd, CDROMREADTOCENTRY, &te);
	Q_ASSERT(te.cdte_format == CDROM_LBA);

	// in case the ioctl() was successful
	if (ret == 0)
		*lba = te.cdte_addr.lba;

	return ret;
}


static int read_leadout(int fd, unsigned long * lba)
{
	struct cdrom_multisession ms;

	ms.addr_format = CDROM_LBA;
	int ret = ioctl(fd, CDROMMULTISESSION, &ms);

	if (ms.xa_flag)
    {
		*lba = ms.addr.lba - XA_INTERVAL;
		return ret;
	}

	return read_toc_entry(fd, CDROM_LEADOUT, lba);
}


CDiscInfos::CDiscInfos(const QString& deviceName) :
m_deviceName (deviceName)
{

}


CDiscInfos::~CDiscInfos()
{

}


bool CDiscInfos::hasDiscInDrive()
{
	int fd = open(qPrintable(m_deviceName), O_RDONLY | O_NONBLOCK);

	if (fd < 0)
		return false;

	close(fd);
	return true;
}


bool CDiscInfos::readInfos()
{
	int fd = open(qPrintable(m_deviceName), O_RDONLY | O_NONBLOCK);

	if (fd < 0)
    {
        qWarning() << "cannot open device " << m_deviceName;
		return false;
	}
    
	int first, last;

	// Find the numbers of the first track (usually 1) and the last track
	if (read_toc_header(fd, &first, &last) < 0)
    {
        qWarning() << "cannot read table of contents";
		close(fd);
		return false;
	}

	// Basic error checking
	if (last == 0)
    {
        qWarning() << "this disc has no tracks";
		close(fd);
		return false;
	}

	infos.firstTrack = first;
	infos.lastTrack = last;

	/*
	 * Get the logical block address (lba) for the end of the audio data.
	 * The "LEADOUT" track is the track beyond the final audio track, so
	 * we're looking for the block address of the LEADOUT track.
	 */
	unsigned long lba = 0;
	read_leadout(fd, &lba);
	infos.trackOffsets[0] = lba + 150;

	for (int i = first; i <= last; ++i)
    {
		read_toc_entry(fd, i, &lba);
		infos.trackOffsets[i] = lba + 150;
	}

	close(fd);
	return true;
}


void CDiscInfos::ejectDisc()
{
    //...
}

#endif

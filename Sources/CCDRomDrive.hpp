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

#ifndef FILE_C_CDROM_DRIVE
#define FILE_C_CDROM_DRIVE

#include "CMediaTableView.hpp"
#include "CDRomDrive.hpp"


class CMainWindow;
class CSong;

namespace FMOD
{
    class Sound;
}


/**
 * Classe utilisée pour gérer un lecteur de CD-ROM.
 * Les pistes du lecteur sont vues par l'application comme des morceaux normaux, mais
 * sont gérées différement par FMOD.
 */

class CCDRomDrive : public CMediaTableView
{
    Q_OBJECT

public:

    CCDRomDrive(const QString& driveName, CMainWindow * mainWindow, const QString& SCSIName = QString(), const QString& deviceName = QString());
    virtual ~CCDRomDrive();

    inline QString getDriveName() const;
    inline QString getSCSIName() const;
    inline QString getDeviceName() const;
    inline quint32 getDiscId() const;
    inline QString getMusicBrainzDiscId() const;
    bool hasCDInDrive();
    inline CSong * getSong(int trackNumber) const;

    virtual bool isModified() const;

public slots:

    void ejectDisc();

protected slots:

    virtual bool updateDatabase();

private:

    virtual bool canImportSongs() const
    {
        return true;
    }

    virtual bool canEditSongs() const
    {
        return false;
    }

    virtual bool canMoveToPlayList() const
    {
        return false;
    }

    QString m_driveName;     ///< Nom du lecteur.
    QString m_SCSIName;
    QString m_deviceName;    ///< Nom du périphérique.
    quint32 m_discId;        ///< DiscId.
    QString m_musicBrainzId; ///< Identifiant du disque pour MusicBrainz.
    FMOD::Sound * m_sound;   ///< Pointeur sur la structure de FMOD.
    CDiscInfos m_disc;
    CSong * m_songs[100];    ///< Liste des morceaux du CD-ROM.
};


/**
 * Donne le nom du lecteur.
 *
 * \return Nom du lecteur.
 */

inline QString CCDRomDrive::getDriveName() const
{
    return m_driveName;
}


inline QString CCDRomDrive::getSCSIName() const
{
    return m_SCSIName;
}


/**
 * Retourne le nom du périphérique.
 *
 * \return Nom du périphérique.
 */

inline QString CCDRomDrive::getDeviceName() const
{
    return m_deviceName;
}


/**
 * Retourne l'identifiant DiscId.
 *
 * \return Identifiant DiscId.
 */

inline quint32 CCDRomDrive::getDiscId() const
{
    return m_discId;
}


/**
 * Retourne l'identifiant du disque pour MusicBrainz.
 *
 * \return Identifiant du disque pour MusicBrainz.
 */

inline QString CCDRomDrive::getMusicBrainzDiscId() const
{
    return m_musicBrainzId;
}


/**
 * Retourne le morceau correspondant à un numéro de piste.
 *
 * \param trackNumber Numéro de piste (entre 0 et 99).
 * \return Pointeur sur le morceau, ou nullptr.
 */

inline CSong * CCDRomDrive::getSong(int trackNumber) const
{
    if (trackNumber < 0 || trackNumber >= 100)
        return nullptr;

    return m_songs[trackNumber];
}

#endif // FILE_C_CDROM_DRIVE

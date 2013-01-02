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

#ifndef FILE_C_CDROM_DRIVE
#define FILE_C_CDROM_DRIVE

#include "CSongTable.hpp"
#include "CDRomDrive.hpp"


class CApplication;
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

class CCDRomDrive : public CSongTable
{
    Q_OBJECT

public:

    CCDRomDrive(const QString& driveName, CApplication * application, const QString& SCSIName = QString(), const QString& deviceName = QString());
    virtual ~CCDRomDrive();

    inline QString getDriveName() const;
    inline QString getSCSIName() const;
    inline QString getDeviceName() const;
    inline qint32 getDiscId() const;
    inline QString getMusicBrainzDiscId() const;
    bool hasCDInDrive();

    virtual bool isModified() const;

public slots:

    void ejectDisc();

protected:

    virtual bool updateDatabase();

private:

    QString m_driveName;     ///< Nom du lecteur.
    QString m_SCSIName;
    QString m_deviceName;    ///< Nom du périphérique.
    qint32 m_discId;         ///< DiscId.
    QString m_musicBrainzId; ///< Identifiant du disque pour MusicBrainz.
    FMOD::Sound * m_sound;   ///< Pointeur sur la structure de FMOD.
    CDiscInfos m_disc;
    QList<CSong *> m_songs;  ///< Liste des morceaux du CD-ROM.
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


inline QString CCDRomDrive::getDeviceName() const
{
    return m_deviceName;
}


inline qint32 CCDRomDrive::getDiscId() const
{
    return m_discId;
}


inline QString CCDRomDrive::getMusicBrainzDiscId() const
{
    return m_musicBrainzId;
}

#endif // FILE_C_CDROM_DRIVE

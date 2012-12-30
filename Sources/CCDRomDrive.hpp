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

#ifndef FILE_C_CDROM_DRIVE
#define FILE_C_CDROM_DRIVE

#include "CSongTable.hpp"


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
    bool hasCDInDrive();
    inline qint32 getDiscId() const;

    virtual bool isModified() const;

protected:

    virtual bool updateDatabase();

private:

    QString m_driveName;    ///< Nom du lecteur.
    QString m_SCSIName;
    QString m_deviceName;   ///< Nom du périphérique.
    qint32 m_discId;        ///< DiscId.
    QString m_musicBrainzId;
    FMOD::Sound * m_sound;  ///< Pointeur sur la structure de FMOD.
    QList<CSong *> m_songs; ///< Liste des morceaux du CD-ROM.
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


inline qint32 CCDRomDrive::getDiscId() const
{
    return m_discId;
}

#endif // FILE_C_CDROM_DRIVE

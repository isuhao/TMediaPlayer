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

#ifndef FILE_C_MUSIC_BRAINZ_LOOKUP
#define FILE_C_MUSIC_BRAINZ_LOOKUP

#include <QObject>
#include <QString>


class CMainWindow;
class CCDRomDrive;
class QNetworkReply;
class QNetworkAccessManager;
class QDomElement;


class CMusicBrainzLookup : public QObject
{
    Q_OBJECT

public:

    CMusicBrainzLookup(CCDRomDrive * cdRomDrive, CMainWindow * application);
    virtual ~CMusicBrainzLookup();

protected slots:

    void replyFinished(QNetworkReply * reply);

protected:

    QString getArtistName(const QDomElement& node) const;

    static const QString m_lookupUrl; ///< URL pour effectuer les requêtes sur MusicBrainz.

    CMainWindow * m_mainWindow; ///< Pointeur sur la classe principale de l'application.
    CCDRomDrive * m_cdRomDrive;   ///< Pointeur sur le lecteur de CD-ROM.
    QString m_musicBrainzId;      ///< Identifiant du disque pour MusicBrainz.
    QNetworkAccessManager * m_networkManager;
};

#endif // FILE_C_MUSIC_BRAINZ_LOOKUP

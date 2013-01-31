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

#ifndef FILE_C_SCROBBLE
#define FILE_C_SCROBBLE

#include "ILastFmService.hpp"


class CSong;


class CScrobble : public ILastFmService
{
    Q_OBJECT

public:

    struct TScrobbleInfos
    {
        QString title;       ///< Titre du morceau.
        QString artist;      ///< Artiste.
        QString album;       ///< Titre de l'album.
        QString albumArtist; ///< Artiste de l'album, si différent de celui du morceau.
        int duration;        ///< Durée en secondes.
        int timestamp;       ///< Timestamp de fin de lecture.
        int trackNumber;     ///< Numéro de piste.

        /// Constructeur par défaut.
        inline TScrobbleInfos() : duration(0), timestamp(0), trackNumber(0) { }
    };

    CScrobble(CApplication * application, const QByteArray& sessionKey, CSong * song);
    CScrobble(CApplication * application, const QByteArray& sessionKey, const TScrobbleInfos& song);

protected:

    void sendRequest();

protected slots:

    virtual void replyFinished(QNetworkReply * reply);

private:

    TScrobbleInfos m_song; ///< Informations sur le morceau à scrobbler.
};

#endif // FILE_C_SCROBBLE

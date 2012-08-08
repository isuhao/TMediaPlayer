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

#include "CScrobble.hpp"
#include "CSong.hpp"
#include "CApplication.hpp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextStream>


CScrobble::CScrobble(CApplication * application, const QByteArray& sessionKey, CSong * song) :
    ILastFmService (application, sessionKey),
    m_song         (song)
{
    Q_CHECK_PTR(song);

    // Arguments de la requête
    QMap<QByteArray, QByteArray> args;

    args["method"]    = "track.scrobble";
    args["artist"]    = song->getArtistName().toUtf8();
    args["track"]     = song->getTitle().toUtf8();
    args["api_key"]   = m_apiKey;
    args["duration"]  = QString::number(song->getDuration() / 1000).toUtf8();
    args["sk"]        = m_sessionKey;
    args["timestamp"] = QString::number(QDateTime::currentDateTime().toTime_t()).toUtf8();

    QByteArray albumTitle = song->getAlbumTitle().toUtf8();

    if (!albumTitle.isEmpty())
    {
        args["album"] = albumTitle;
        QByteArray albumArtist = song->getAlbumArtist().toUtf8();

        if (!albumArtist.isEmpty() && albumArtist != args["artist"])
        {
            args["albumArtist"] = albumArtist;
        }
    }

    if (song->getTrackNumber() > 0)
    {
        args["trackNumber"] = QString::number(song->getTrackNumber()).toUtf8();
    }

    QByteArray content = getLastFmQuery(args);

    // Log
    QFile * logFile = m_application->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   Request 'Scrobble'\n";
    stream << "----------------------------------------\n";
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString() << "\n";
    stream << tr("Title:") << ' ' << "'" << song->getTitle() << "'\n";
    stream << tr("Artist:") << ' ' << "'" << song->getArtistName() << "'\n";
    stream << tr("Album:") << ' ' << "'" << song->getAlbumTitle() << "'\n";
    stream << tr("URL:") << ' ' << "'" << m_lastFmUrl << "'\n";
    stream << tr("Content:") << ' ' << "'" << content << "'\n";

    QUrl url(m_lastFmUrl);
    QNetworkRequest request(url);
    //QNetworkRequest request(QUrl(m_lastFmUrl)); // Ne compile pas !
    //QNetworkRequest request(QUrl("http://ws.audioscrobbler.com/2.0/")); // Compile !
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");


    //return; // Pas de scrobble

    m_networkManager->post(request, content);
}


void CScrobble::replyFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    // Log
    QFile * logFile = m_application->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   Reply 'Scrobble'\n";
    stream << "----------------------------------------\n";
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString() << "\n";
    stream << tr("Code:") << ' ' << reply->error() << "\n";
    stream << tr("Content:") << ' ' << "'" << reply->readAll() << "'\n";

    if (reply->error() != QNetworkReply::NoError)
    {
        stream << "Erreur HTTP : " << reply->error() << "\n";
    }

    m_application->notifyInformation(tr("Song scrobbled to Last.fm"));

    reply->deleteLater();
    deleteLater();
}

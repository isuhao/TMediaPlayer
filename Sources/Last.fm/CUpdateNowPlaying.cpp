/*
Copyright (C) 2012-2016 Teddy Michel

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

#include "CUpdateNowPlaying.hpp"
#include "../CSong.hpp"
#include "../CMediaManager.hpp"

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
//#include <QUrl>

#include <QtDebug>


CUpdateNowPlaying::CUpdateNowPlaying(CMediaManager * mediaManager, const QByteArray& sessionKey, CSong * song) :
ILastFmService (mediaManager, sessionKey),
m_song         (song)
{
    Q_CHECK_PTR(song);

    // Arguments de la requête
    QMap<QByteArray, QByteArray> args;

    QString artistName = song->getArtistName();

    if (artistName.isEmpty())
    {
        deleteLater();
        return;
    }

    args["method"]   = "track.updateNowPlaying";
    args["artist"]   = artistName.toUtf8();
    args["track"]    = song->getTitle().toUtf8();
    args["api_key"]  = m_apiKey;
    args["duration"] = QString::number(song->getDuration() / 1000).toUtf8();
    args["sk"]       = m_sessionKey;

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
    QFile * logFile = m_mediaManager->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream.setFieldAlignment(QTextStream::AlignLeft);

    stream << "========================================\n";
    stream << "   Request 'Update Now Playing'\n";
    stream << "----------------------------------------\n";
    stream << qSetFieldWidth(9) << tr("Date:")    << qSetFieldWidth(0) << ' ' << QDateTime::currentDateTime().toString() << "\n";
    stream << qSetFieldWidth(9) << tr("Title:")   << qSetFieldWidth(0) << ' ' << song->getTitle() << "\n";
    stream << qSetFieldWidth(9) << tr("Artist:")  << qSetFieldWidth(0) << ' ' << song->getArtistName() << "\n";
    stream << qSetFieldWidth(9) << tr("Album:")   << qSetFieldWidth(0) << ' ' << song->getAlbumTitle() << "\n";
    stream << qSetFieldWidth(9) << tr("URL:")     << qSetFieldWidth(0) << ' ' << m_lastFmUrl << "\n";
    stream << qSetFieldWidth(9) << tr("Content:") << qSetFieldWidth(0) << ' ' << content << "\n";

    QUrl url(m_lastFmUrl);
    QNetworkRequest request(url);
    //QNetworkRequest request(QUrl(m_lastFmUrl)); // Ne compile pas !
    //QNetworkRequest request(QUrl("http://ws.audioscrobbler.com/2.0/")); // Compile !
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    m_networkManager->post(request, content);
}


void CUpdateNowPlaying::replyFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    // Log
    QFile * logFile = m_mediaManager->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream.setFieldAlignment(QTextStream::AlignLeft);

    stream << "========================================\n";
    stream << "   Reply 'Update Now Playing'\n";
    stream << "----------------------------------------\n";
    stream << qSetFieldWidth(9) << tr("Date:")    << qSetFieldWidth(0) << ' ' << QDateTime::currentDateTime().toString() << "\n";
    stream << qSetFieldWidth(9) << tr("Code:")    << qSetFieldWidth(0) << ' ' << reply->error() << "\n";
    stream << qSetFieldWidth(9) << tr("Content:") << qSetFieldWidth(0) << ' ' << reply->readAll() << "\n";

    if (reply->error() != QNetworkReply::NoError)
    {
        stream << tr("HTTP error: ") << reply->error() << "\n";
    }

    reply->deleteLater();
    deleteLater();
}

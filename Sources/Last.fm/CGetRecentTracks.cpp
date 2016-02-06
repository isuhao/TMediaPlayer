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

#include "CGetRecentTracks.hpp"
#include "../CSong.hpp"
#include "../CMediaManager.hpp"

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextStream>
#include <QDomDocument>
#include <QSqlQuery>
#include <QSqlError>

#include <QtDebug>


/**
 * Construit la requête.
 *
 * \param application Pointeur sur la classe principale de l'application.
 * \param sessionKey  Clé d'identification pour la session.
 */

CGetRecentTracks::CGetRecentTracks(CMediaManager * mediaManager, const QByteArray& sessionKey) :
ILastFmService (mediaManager, sessionKey)
{
    getTracks(0);
}


void CGetRecentTracks::getTracks(int page)
{
    // Arguments de la requête
    QMap<QByteArray, QByteArray> args;

    args["method"]    = "user.getRecentTracks";
    args["api_key"]   = m_apiKey;
    args["sk"]        = m_sessionKey;
    args["user"]      = "Doch54"; // TODO: variable
    args["page"]      = QByteArray::number(page);
    args["limit"]     = QByteArray::number(100);

    QByteArray content = getLastFmQuery(args);

    // Log
    QFile * logFile = m_mediaManager->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream.setFieldAlignment(QTextStream::AlignLeft);

    stream << "========================================\n";
    stream << "   Request 'Get recent tracks'\n";
    stream << "----------------------------------------\n";
    stream << qSetFieldWidth(9) << tr("Date:")    << qSetFieldWidth(0) << ' ' << QDateTime::currentDateTime().toString() << "\n";
    stream << qSetFieldWidth(9) << tr("URL:")     << qSetFieldWidth(0) << ' ' << m_lastFmUrl << "\n";
    stream << qSetFieldWidth(9) << tr("Content:") << qSetFieldWidth(0) << ' ' << content << "\n";

    QUrl url(m_lastFmUrl);
    QNetworkRequest request(url);
    //QNetworkRequest request(QUrl(m_lastFmUrl)); // Ne compile pas !
    //QNetworkRequest request(QUrl("http://ws.audioscrobbler.com/2.0/")); // Compile !
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    m_networkManager->post(request, content);
}


/**
 * Réception de la réponse.
 *
 * \param reply Réponse.
 */

void CGetRecentTracks::replyFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    QByteArray data = reply->readAll();

    // Log
    QFile * logFile = m_mediaManager->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream.setFieldAlignment(QTextStream::AlignLeft);

    stream << "========================================\n";
    stream << "   Reply 'Get recent tracks'\n";
    stream << "----------------------------------------\n";
    stream << qSetFieldWidth(9) << tr("Date:")    << qSetFieldWidth(0) << ' ' << QDateTime::currentDateTime().toString() << "\n";
    stream << qSetFieldWidth(9) << tr("Code:")    << qSetFieldWidth(0) << ' ' << reply->error() << "\n";
    stream << qSetFieldWidth(9) << tr("Content:") << qSetFieldWidth(0) << ' ' << data << "\n";

    if (reply->error() != QNetworkReply::NoError)
    {
        stream << tr("HTTP error: ") << reply->error() << "\n";
    }

    QDomDocument doc;
    QString error;
    bool needDelete = true;

    if (doc.setContent(data, &error))
    {
        QDomElement racine = doc.documentElement();

        if (racine.tagName() == "lfm")
        {
            QDomElement racine2 = racine.firstChildElement();

            if (racine2.tagName() == "recenttracks")
            {
                int currentPage = racine2.attribute("page", 0).toInt();
                int totalPages = racine2.attribute("totalPages", 0).toInt();

                // Pour chaque piste lue
                for (QDomElement nodeTracks = racine2.firstChildElement(); !nodeTracks.isNull(); nodeTracks = nodeTracks.nextSibling().toElement())
                {
                    if (nodeTracks.tagName() != "track")
                    {
                        stream << tr("Réponse XML incorrecte (élément 'track' attendu)\n");
                        continue;
                    }

                    qDebug() << "Lecture :";

                    // Pour chaque attribut de la piste
                    for (QDomElement nodeTrack = nodeTracks.firstChildElement(); !nodeTrack.isNull(); nodeTrack = nodeTrack.nextSibling().toElement())
                    {
                        if (nodeTrack.tagName() == "artist")
                        {
                            qDebug() << " - " << nodeTrack.text();
                            //...
                        }
                        else if (nodeTrack.tagName() == "name")
                        {
                            qDebug() << " - " << nodeTrack.text();
                            //...
                        }
                        else if (nodeTrack.tagName() == "album")
                        {
                            qDebug() << " - " << nodeTrack.text();
                            //...
                        }
                        else if (nodeTrack.tagName() == "date")
                        {
                            qDebug() << " - " << nodeTrack.text() << ", UTC = " << nodeTrack.attribute("uts");
                            //...
                        }
                    }
                }
/*
        <track>
            <artist mbid="d614b0ad-fe3a-4927-b413-48cb831a814b">Frou Frou</artist>                      <!========
            <name>Shh</name>                                                                            <!========
            <streamable>0</streamable>
            <mbid>542c047a-fa87-4252-803d-4e1a84a6371a</mbid>
            <album mbid="a07b8c39-6eb8-4822-853a-3aabbbafed8d">Details</album>                          <!========
            <url>http://www.last.fm/music/Frou+Frou/_/Shh</url>
            <image size="small">http://userserve-ak.last.fm/serve/34s/44480663.png</image>
            <image size="medium">http://userserve-ak.last.fm/serve/64s/44480663.png</image>
            <image size="large">http://userserve-ak.last.fm/serve/126/44480663.png</image>
            <image size="extralarge">http://userserve-ak.last.fm/serve/300x300/44480663.png</image>
            <date uts="1150983870">22 Jun 2006, 13:44</date>                                            <!========
        </track>
*/
                if (currentPage < totalPages)
                {
                    needDelete = false;
                    getTracks(currentPage + 1);
                }
            }
            else
            {
                stream << tr("Réponse XML incorrecte (élément 'recenttracks' attendu)\n");
            }
        }
        else
        {
            stream << tr("Réponse XML incorrecte (élément 'lfm' attendu)\n");
        }
    }
    else
    {
        stream << tr("Invalid XML document (%1)").arg(error);
    }

    if (needDelete)
    {
        deleteLater();
    }

    reply->deleteLater();
}

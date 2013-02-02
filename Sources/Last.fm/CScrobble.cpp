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

#include "CScrobble.hpp"
#include "../CSong.hpp"
#include "../CApplication.hpp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextStream>
#include <QDomDocument>
#include <QSqlQuery>
#include <QSqlError>

#include <QtDebug>


/**
 * Construit la requête avec un morceau.
 * La date de lecture est la date actuelle.
 *
 * \param application Pointeur sur la classe principale de l'application.
 * \param sessionKey  Clé d'identification pour la session.
 * \param song        Pointeur sur le morceau écouté.
 */

CScrobble::CScrobble(CApplication * application, const QByteArray& sessionKey, CSong * song) :
ILastFmService (application, sessionKey)
{
    Q_CHECK_PTR(song);

    // Informations sur le morceau
    m_song.title        = song->getTitle();
    m_song.artist       = song->getArtistName();
    m_song.album        = song->getAlbumTitle();
    m_song.albumArtist  = song->getAlbumArtist();
    m_song.duration     = (song->getDuration() / 1000);
    m_song.timestamp    = QDateTime::currentDateTime().toTime_t();
    m_song.trackNumber  = song->getTrackNumber();

    sendRequest();
}


/**
 * Construit la requête avec des informations déjà définie.
 * Permet de scrobbler un morceau anciennement écouté.
 *
 * \param application Pointeur sur la classe principale de l'application.
 * \param sessionKey  Clé d'identification pour la session.
 * \param song        Informations sur le scrobble.
 */

CScrobble::CScrobble(CApplication * application, const QByteArray& sessionKey, const TScrobbleInfos& song) :
ILastFmService (application, sessionKey),
m_song         (song)
{
    sendRequest();
}


/**
 * Envoi la requête à Last.fm.
 */

void CScrobble::sendRequest()
{
    if (m_song.artist.isEmpty())
    {
        deleteLater();
        return;
    }

    // Arguments de la requête
    QMap<QByteArray, QByteArray> args;

    args["method"]    = "track.scrobble";
    args["artist"]    = m_song.artist.toUtf8();
    args["track"]     = m_song.title.toUtf8();
    args["api_key"]   = m_apiKey;
    args["duration"]  = QString::number(m_song.duration).toUtf8();
    args["sk"]        = m_sessionKey;
    args["timestamp"] = QString::number(m_song.timestamp).toUtf8();

    QByteArray albumTitle = m_song.album.toUtf8();

    if (!albumTitle.isEmpty())
    {
        args["album"] = albumTitle;
        QByteArray albumArtist = m_song.albumArtist.toUtf8();

        if (!albumArtist.isEmpty() && albumArtist != args["artist"])
        {
            args["albumArtist"] = albumArtist;
        }
    }

    if (m_song.trackNumber > 0)
    {
        args["trackNumber"] = QString::number(m_song.trackNumber).toUtf8();
    }

    QByteArray content = getLastFmQuery(args);

    // Log
    QFile * logFile = m_application->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream.setFieldAlignment(QTextStream::AlignLeft);

    stream << "========================================\n";
    stream << "   Request 'Scrobble'\n";
    stream << "----------------------------------------\n";
    stream << qSetFieldWidth(9) << tr("Date:")    << qSetFieldWidth(0) << ' ' << QDateTime::currentDateTime().toString() << "\n";
    stream << qSetFieldWidth(9) << tr("Title:")   << qSetFieldWidth(0) << ' ' << m_song.title << "\n";
    stream << qSetFieldWidth(9) << tr("Artist:")  << qSetFieldWidth(0) << ' ' << m_song.artist << "\n";
    stream << qSetFieldWidth(9) << tr("Album:")   << qSetFieldWidth(0) << ' ' << m_song.album << "\n";
    stream << qSetFieldWidth(9) << tr("URL:")     << qSetFieldWidth(0) << ' ' << m_lastFmUrl << "\n";
    stream << qSetFieldWidth(9) << tr("Content:") << qSetFieldWidth(0) << ' ' << content << "\n";

    QUrl url(m_lastFmUrl);
    QNetworkRequest request(url);
    //QNetworkRequest request(QUrl(m_lastFmUrl)); // Ne compile pas !
    //QNetworkRequest request(QUrl("http://ws.audioscrobbler.com/2.0/")); // Compile !
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    //return; // Pas de scrobble
    m_networkManager->post(request, content);
}


void CScrobble::logError()
{
    QSqlDatabase dataBase = QSqlDatabase::addDatabase("QSQLITE", "lastfm");
    dataBase.setDatabaseName(m_application->getApplicationPath() + "lastfm.sqlite");

    if (!dataBase.open())
    {
        qWarning() << "Erreur d'ouverture de la base lastfm.sqlite\n";
        return;
    }

    QSqlQuery query(dataBase);
    QStringList tables = dataBase.tables(QSql::Tables);

    if (!tables.contains("scrobbles"))
    {
        if (!query.exec("CREATE TABLE scrobbles ("
                            "time TIMESTAMP NOT NULL,"
                            "title VARCHAR(512) NOT NULL,"
                            "artist VARCHAR(512) NOT NULL,"
                            "album VARCHAR(512),"
                            "albumArtist VARCHAR(512),"
                            "duration INTEGER,"
                            "trackNumber INTEGER"
                        ")"))
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            dataBase.close();
            return;
        }
    }

    query.prepare("INSERT INTO scrobbles(time, title, artist, album, albumArtist, duration, trackNumber) VALUES(?, ?, ?, ?, ?, ?, ?)");

    query.bindValue(0, m_song.timestamp);
    query.bindValue(1, m_song.title);
    query.bindValue(2, m_song.artist);
    query.bindValue(3, m_song.album);
    query.bindValue(4, m_song.albumArtist);
    query.bindValue(5, m_song.duration);
    query.bindValue(6, m_song.trackNumber);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        dataBase.close();
        return;
    }

    dataBase.close();
}


/**
 * Réception de la réponse au scrobble.
 *
 * \todo Gérer les erreurs.
 *
 * \param reply Réponse.
 */

void CScrobble::replyFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    QByteArray data = reply->readAll();

    // Log
    QFile * logFile = m_application->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream.setFieldAlignment(QTextStream::AlignLeft);

    stream << "========================================\n";
    stream << "   Reply 'Scrobble'\n";
    stream << "----------------------------------------\n";
    stream << qSetFieldWidth(9) << tr("Date:")    << qSetFieldWidth(0) << ' ' << QDateTime::currentDateTime().toString() << "\n";
    stream << qSetFieldWidth(9) << tr("Code:")    << qSetFieldWidth(0) << ' ' << reply->error() << "\n";
    stream << qSetFieldWidth(9) << tr("Content:") << qSetFieldWidth(0) << ' ' << data << "\n";

    if (reply->error() != QNetworkReply::NoError)
    {
        stream << tr("Erreur HTTP : ") << reply->error() << "\n";

        if (reply->error() == QNetworkReply::HostNotFoundError)
        {
            logError();
        }
    }

    QDomDocument doc;
    QString error;
    bool status = true;

    if (doc.setContent(data, &error))
    {
        QDomElement racine = doc.documentElement();

        if (racine.tagName() == "lfm")
        {
            // Erreur
            if (racine.attribute("status", "failed") == "failed")
            {
                status = false;
                logError();
            }
        }
        else
        {
            stream << "Réponse XML incorrecte (élément 'lfm' attendu)\n";
            status = false;
        }
    }
    else
    {
        stream << "Document XML invalide (" << error << ")\n";
        status = false;
    }

    if (status)
    {
        m_application->notifyInformation(tr("Song scrobbled to Last.fm"));
    }
    else
    {
        m_application->notifyInformation(tr("Can't scrobble the song to Last.fm"));
    }

    reply->deleteLater();
    deleteLater();
}

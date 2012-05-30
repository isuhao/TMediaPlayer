
#include "CUpdateNowPlaying.hpp"
#include "CSong.hpp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
//#include <QUrl>

#include <QtDebug>


CUpdateNowPlaying::CUpdateNowPlaying(CApplication * application, const QByteArray& sessionKey, CSong * song) :
    ILastFmService (application, sessionKey),
    m_song         (song)
{
    Q_CHECK_PTR(song);

    // Arguments de la requête
    QMap<QByteArray, QByteArray> args;
    
    args["method"]   = "track.updateNowPlaying";
    args["artist"]   = song->getArtistName().toUtf8();
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
    logLastFmRequest(m_lastFmUrl, content);

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

    qDebug() << "CUpdateNowPlaying::replyFinished()";

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "CUpdateNowPlaying::replyFinished() : erreur HTTP avec Last.fm (" << reply->error() << ")";
    }

    QByteArray data = reply->readAll();
    logLastFmResponse(reply->error(), data);

    reply->deleteLater();
}

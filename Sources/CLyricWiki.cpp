/*
Copyright (C) 2012-2014 Teddy Michel

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

#include "CLyricWiki.hpp"
#include "CMediaManager.hpp"
#include "CSong.hpp"

// Qt
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <QUrl>
#include <QWebFrame>
#include <QWebElementCollection>

#include <QtDebug>


CLyricWiki::CLyricWiki(CMediaManager * mediaManager, CSong * song) :
QObject        (mediaManager),
m_mediaManager (mediaManager),
m_song         (song)
{
    Q_CHECK_PTR(m_mediaManager);
    Q_CHECK_PTR(m_song);

    QNetworkAccessManager * networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));

    QUrl url(QString("http://lyrics.wikia.com/api.php?action=lyrics&artist=%1&song=%2&albumName=%3&fmt=xml&func=getSong").arg(m_song->getArtistName()).arg(m_song->getTitle()).arg(m_song->getAlbumTitle()));
    QNetworkRequest request(url);
    //request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    networkManager->get(request);
}


CLyricWiki::~CLyricWiki()
{

}


void CLyricWiki::replyFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    if (reply->error() != QNetworkReply::NoError)
    {
        m_mediaManager->logError(tr("HTTP error: %1").arg(reply->error()), __FUNCTION__, __FILE__, __LINE__);
    }

    QByteArray data = reply->readAll();
    QDomDocument doc;
    QString error;

    if (!doc.setContent(data, &error))
    {
        m_mediaManager->logError(tr("invalid XML document (%1)").arg(error), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    QDomElement racine = doc.documentElement();

    if (racine.tagName() != "LyricsResult")
    {
        m_mediaManager->logError(tr("invalid XML response (expected element '%1')").arg("LyricsResult"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    QDomElement elemURL = racine.firstChildElement("url");

    if (elemURL.isNull())
    {
        m_mediaManager->logError(tr("invalid XML response (expected element '%1')").arg("url"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    QString url = elemURL.text();

    QDomElement elemPageId = racine.firstChildElement("page_id");

    if (elemPageId.isNull())
    {
        m_mediaManager->logError(tr("invalid XML response (expected element '%1')").arg("page_id"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    if (elemPageId.text().toInt() <= 0)
    {
        m_mediaManager->logError(tr("invalid XML response (invalid element '%1')").arg("page_id"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    m_frame = page.mainFrame();
    m_frame->load(QUrl::fromPercentEncoding(qPrintable(url)));
    connect(m_frame, SIGNAL(loadFinished(bool)), this, SLOT(onPageFinished()));

    reply->deleteLater();
}


void CLyricWiki::replyFinished2(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    if (reply->error() != QNetworkReply::NoError)
    {
        m_mediaManager->logError(tr("HTTP error: %1").arg(reply->error()), __FUNCTION__, __FILE__, __LINE__);
    }

    QByteArray data = reply->readAll();
    m_frame->setHtml(data);

    QWebElementCollection elements = m_frame->findAllElements(".lyricbox");

    for (QWebElementCollection::iterator element = elements.begin(); element != elements.end(); ++element)
    {
        (*element).removeAllChildren();
    }

    reply->deleteLater();
    deleteLater();
}


void CLyricWiki::onPageFinished()
{
    QString lyrics;

    QWebElementCollection elements = m_frame->findAllElements(".lyricbox");

    for (QWebElementCollection::iterator element = elements.begin(); element != elements.end(); ++element)
    {
        (*element).firstChild().removeFromDocument();
        (*element).lastChild().removeFromDocument();
        (*element).lastChild().removeFromDocument();
        lyrics = (*element).toPlainText().trimmed();
    }

    emit lyricsFound(lyrics);

    deleteLater();
}

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

#include "CLyricWiki.hpp"
#include "CApplication.hpp"
#include "CSong.hpp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <QUrl>

#include <QWebFrame>
#include <QWebElementCollection>

#include <QtDebug>


CLyricWiki::CLyricWiki(CApplication * application, CSong * song) :
    QObject       (application),
    m_application (application),
    m_song        (song)
{
    Q_CHECK_PTR(application);
    Q_CHECK_PTR(song);

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
        qDebug() << "Erreur HTTP : " << reply->error() << "\n";
    }

    QByteArray data = reply->readAll();

    QDomDocument doc;
    
    QString error;
    if (!doc.setContent(data, &error))
    {
        qDebug() << "Document XML invalide (" << error << ")\n";
        return;
    }

    QDomElement racine = doc.documentElement();

    if (racine.tagName() != "LyricsResult")
    {
        qDebug() << "Réponse XML incorrecte (élément 'LyricsResult' attendu)\n";
        return;
    }
    
    //racine = racine.firstChildElement();

    QDomElement elemURL = racine.firstChildElement("url");

    if (elemURL.isNull())
    {
        qDebug() << "Réponse XML incorrecte (élément 'url' attendu)\n";
        return;
    }
QString url = elemURL.text();
qDebug() << url;

    QDomElement elemPageId = racine.firstChildElement("page_id");

    if (elemPageId.isNull())
    {
        qDebug() << "Réponse XML incorrecte (élément 'page_id' attendu)\n";
        return;
    }

    if (elemPageId.text().toInt() <= 0)
    {
        qDebug() << "Réponse XML incorrecte (élément 'page_id' invalide)\n";
        return;
    }
int pageId = elemPageId.text().toInt();
qDebug() << pageId;

/*
<?xml version="1.0" encoding="UTF-8"?>
<LyricsResult>
	<artist>Nazareth</artist>
	<song>Day At The Beach</song>
	<lyrics>[...]</lyrics>
   *<url>http://lyrics.wikia.com/Nazareth:Day_At_The_Beach</url>
	<page_namespace>0</page_namespace>
   *<page_id>834495</page_id>
	<isOnTakedownList>0</isOnTakedownList>
</LyricsResult>
*/

    m_frame = page.mainFrame();
    m_frame->load(QUrl::fromPercentEncoding(qPrintable(url)));
    connect(m_frame, SIGNAL(loadFinished(bool)), this, SLOT(onPageFinished()));
/*
    QNetworkAccessManager * networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished2(QNetworkReply *)));

    QNetworkRequest request(QUrl::fromPercentEncoding(qPrintable(url)));
    //request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    networkManager->get(request);
*/
    reply->deleteLater();
}


void CLyricWiki::replyFinished2(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Erreur HTTP : " << reply->error() << "\n";
    }

    QByteArray data = reply->readAll();
    m_frame->setHtml(data);

    QWebElementCollection elements = m_frame->findAllElements(".lyricbox");

    foreach (QWebElement element, elements)
    {
        element.removeAllChildren();
        qDebug() << element.toPlainText();
    }



/*
    QDomDocument doc;
    
    QString error;
    if (!doc.setContent(data, &error))
    {
        qDebug() << "Document XML invalide (" << error << ")\n";
        return;
    }

    QDomElement elem = doc.documentElement();

    while (true)
    {
        elem = elem.nextSiblingElement("div");

        if (elem.attribute("class") == "lyricbox")
        {
            qDebug() << elem.text();
            //...
            break;
        }
    }
*/
//<div class="lyricbox">

    //...

    //QString lyrics;
    //emit lyricsFound(lyrics);

    reply->deleteLater();
    deleteLater();
}


void CLyricWiki::onPageFinished(void)
{
    qDebug() << "onPageFinished" << this;
    QString lyrics;

    QWebElementCollection elements = m_frame->findAllElements(".lyricbox");

    foreach (QWebElement element, elements)
    {
        element.firstChild().removeFromDocument();
        element.lastChild().removeFromDocument();
        element.lastChild().removeFromDocument();
        lyrics = element.toPlainText().trimmed();
    }

    emit lyricsFound(lyrics);

    deleteLater();
}

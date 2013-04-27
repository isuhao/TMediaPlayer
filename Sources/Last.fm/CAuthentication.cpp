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

#include "CAuthentication.hpp"
#include "../CMainWindow.hpp"
#include "../CMediaManager.hpp"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QtXml>


CAuthentication::CAuthentication(CMainWindow * mainWindow) :
ILastFmService (mainWindow, ""),
m_timerLastFm  (nullptr),
m_numRequests  (0)
{
    QMap<QByteArray, QByteArray> args;
    args["method"]  = "auth.getToken";
    args["api_key"] = m_apiKey;
    QByteArray content = getLastFmQuery(args);
    
    QString url = QString("%1?%2").arg(m_lastFmUrl).arg(QString(content));

    // Log
    QFile * logFile = m_mainWindow->getMediaManager()->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   Request 'Get Token'\n";
    stream << "----------------------------------------\n";
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString() << '\n';
    stream << tr("URL:") << ' ' << "'" << url << "'\n";

    m_networkManager->get(QNetworkRequest(QUrl(url)));
}


void CAuthentication::replyFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    QByteArray data = reply->readAll();

    // Log
    QFile * logFile = m_mainWindow->getMediaManager()->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   Reply 'Get Token'\n";
    stream << "----------------------------------------\n";
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString() << '\n';
    stream << tr("Code:") << ' ' << reply->error() << '\n';
    stream << tr("Content:") << ' ' << "'" << data << "'\n";

    if (reply->error() != QNetworkReply::NoError)
    {
        stream << tr("HTTP error: ") << reply->error() << "\n";
    }

    QDomDocument doc;
    
    QString error;
    if (!doc.setContent(data, &error))
    {
        stream << tr("Document XML invalide (") << error << ")\n";
        return;
    }

    QDomElement racine = doc.documentElement();

    if (racine.tagName() != "lfm")
    {
        stream << tr("Réponse XML incorrecte (élément 'lfm' attendu)\n");
        return;
    }

    if (racine.attribute("status", "failed") == "failed")
    {
        stream << tr("La requête Last.fm a echouée\n");
        return;
    }

    racine = racine.firstChildElement();

    if (racine.tagName() != "token")
    {
        stream << tr("Réponse XML incorrecte (élément 'token' attendu)\n");
        return;
    }
    
    m_lastFmToken = racine.text().toLatin1();

    // Ouverture du navigateur
    QDesktopServices::openUrl(QUrl(QString("http://www.last.fm/api/auth/?api_key=%1&token=%2").arg(QString(m_apiKey)).arg(QString(m_lastFmToken))));

    if (m_timerLastFm)
    {
        m_timerLastFm->stop();
        delete m_timerLastFm;
    }

    m_timerLastFm = new QTimer(this);
    connect(m_timerLastFm, SIGNAL(timeout()), this, SLOT(getLastFmSession()));
    m_timerLastFm->start(5000);

    reply->deleteLater();
}


void CAuthentication::getLastFmSession()
{
    Q_CHECK_PTR(m_timerLastFm);

    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyLastFmFinished(QNetworkReply *)));

    QMap<QByteArray, QByteArray> args;

    args["method"]  = "auth.getSession";
    args["api_key"] = m_apiKey;
    args["token"]   = m_lastFmToken;

    QByteArray content = getLastFmQuery(args);
    QString url = QString("%1?%2").arg(m_lastFmUrl).arg(QString(content));

    // Log
    QFile * logFile = m_mainWindow->getMediaManager()->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   Request 'Get Session key'\n";
    stream << "----------------------------------------\n";
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString() << '\n';
    stream << tr("URL:") << ' ' << "'" << url << "'\n";

    manager->get(QNetworkRequest(QUrl(url)));

    if (++m_numRequests >= 10)
    {
        m_timerLastFm->stop();
        delete m_timerLastFm;
        m_timerLastFm = nullptr;
        m_numRequests = 0;
        return;
    }
}


void CAuthentication::replyLastFmFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);
    Q_CHECK_PTR(m_timerLastFm);

    QByteArray data = reply->readAll();

    // Log
    QFile * logFile = m_mainWindow->getMediaManager()->getLogFile("lastFm");
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   Reply 'Get Session key'\n";
    stream << "----------------------------------------\n";
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString() << "\n";
    stream << tr("Code:") << ' ' << reply->error() << "\n";
    stream << tr("Content:") << ' ' << data << "\n";

    if (reply->error() != QNetworkReply::NoError)
    {
        stream << tr("Erreur HTTP : ") << reply->error() << "\n";
    }

    QDomDocument doc;
    
    QString error;
    if (!doc.setContent(data, &error))
    {
        stream << tr("Document XML invalide (") << error << ")\n";
        return;
    }

    QDomElement racine = doc.documentElement();

    if (racine.tagName() != "lfm")
    {
        stream << tr("Réponse XML incorrecte (élément 'lfm' attendu)\n");
        return;
    }

    if (racine.attribute("status", "failed") == "failed")
    {
        stream << tr("La requête Last.fm a echouée\n");
        return;
    }
    
    racine = racine.firstChildElement();

    if (racine.tagName() != "session")
    {
        stream << tr("Réponse XML incorrecte (élément 'session' attendu)\n");
        return;
    }

    racine = racine.firstChildElement();
    racine = racine.nextSiblingElement("key");

    if (racine.isNull())
    {
        stream << tr("Réponse XML incorrecte (élément 'key' attendu)\n");
        return;
    }

    m_sessionKey = racine.text().toLatin1();

    // Enregistrement de la clé
    m_mainWindow->getMediaManager()->getSettings()->setValue("LastFm/SessionKey", m_sessionKey);

    reply->deleteLater();
    deleteLater();

    m_timerLastFm->stop();
    delete m_timerLastFm;
    m_timerLastFm = nullptr;
    m_numRequests = 0;
}

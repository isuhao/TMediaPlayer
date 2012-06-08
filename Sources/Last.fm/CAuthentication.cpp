
#include "CAuthentication.hpp"
#include "CApplication.hpp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QtXml>

#include <QtDebug>


CAuthentication::CAuthentication(CApplication * application) :
    ILastFmService (application, ""),
    m_timerLastFm  (NULL),
    m_numRequests  (0)
{
    QMap<QByteArray, QByteArray> args;
    args["method"]  = "auth.getToken";
    args["api_key"] = m_apiKey;
    QByteArray content = getLastFmQuery(args);
    
    QString url = QString("%1?%2").arg(m_lastFmUrl).arg(QString(content));
    logLastFmRequest(url);

    m_networkManager->get(QNetworkRequest(QUrl(url)));
}


void CAuthentication::replyFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    //qDebug() << "CAuthentication::replyFinished()";

    Q_CHECK_PTR(reply);

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "CApplication::replyLastFmGetToken() : erreur HTTP avec Last.fm (" << reply->error() << ")";
    }

    QByteArray data = reply->readAll();
    logLastFmResponse(reply->error(), data);

    QDomDocument doc;
    
    QString error;
    if (!doc.setContent(data, &error))
    {
        qWarning() << "CApplication::replyLastFmGetToken() : document XML invalide (" << error << ")";
        return;
    }

    QDomElement racine = doc.documentElement();

    if (racine.tagName() != "lfm")
    {
        qWarning() << "CApplication::replyLastFmGetToken() : réponse XML incorrecte (élément 'lfm' attendu)";
        return;
    }

    if (racine.attribute("status", "failed") == "failed")
    {
        qWarning() << "CApplication::replyLastFmGetToken() : la requête Last.fm a echouée";
        return;
    }

    racine = racine.firstChildElement();

    if (racine.tagName() != "token")
    {
        qWarning() << "CApplication::replyLastFmGetToken() : réponse XML incorrecte (élément 'token' attendu)";
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


void CAuthentication::getLastFmSession(void)
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
    logLastFmRequest(url);

    manager->get(QNetworkRequest(QUrl(url)));

    if (++m_numRequests >= 10)
    {
        m_timerLastFm->stop();
        delete m_timerLastFm;
        m_timerLastFm = NULL;
        m_numRequests = 0;
        return;
    }
}


void CAuthentication::replyLastFmFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);
    Q_CHECK_PTR(m_timerLastFm);

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "CApplication::replyLastFmFinished() : erreur HTTP avec Last.fm (" << reply->error() << ")";
    }

    QByteArray data = reply->readAll();
    logLastFmResponse(reply->error(), data);

    QDomDocument doc;
    
    QString error;
    if (!doc.setContent(data, &error))
    {
        qWarning() << "CApplication::replyLastFmFinished() : document XML invalide (" << error << ")";
        return;
    }

    QDomElement racine = doc.documentElement();

    if (racine.tagName() != "lfm")
    {
        qWarning() << "CApplication::replyLastFmFinished() : réponse XML incorrecte (élément 'lfm' attendu)";
        return;
    }

    if (racine.attribute("status", "failed") == "failed")
    {
        qWarning() << "CApplication::replyLastFmFinished() : la requête Last.fm a echouée";
        return;
    }
    
    racine = racine.firstChildElement();

    if (racine.tagName() != "session")
    {
        qWarning() << "CApplication::replyLastFmGetToken() : réponse XML incorrecte (élément 'session' attendu)";
        return;
    }

    racine = racine.firstChildElement();
    racine = racine.nextSiblingElement("key");

    if (racine.isNull())
    {
        qWarning() << "CApplication::replyLastFmFinished() : réponse XML incorrecte (élément key attendu)";
        return;
    }

    m_sessionKey = racine.text().toLatin1();

    // Enregistrement de la clé
    m_application->getSettings()->setValue("LastFm/SessionKey", m_sessionKey);

    reply->deleteLater();

    m_timerLastFm->stop();
    delete m_timerLastFm;
    m_timerLastFm = NULL;
    m_numRequests = 0;
}

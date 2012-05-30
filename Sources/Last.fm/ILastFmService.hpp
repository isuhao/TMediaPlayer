
#ifndef FILE_I_LAST_FM_SERVICE
#define FILE_I_LAST_FM_SERVICE

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QString>


class CApplication;
class QNetworkReply;
class QNetworkAccessManager;


class ILastFmService : public QObject
{
    Q_OBJECT

public:

    ILastFmService(CApplication * application, const QByteArray& sessionKey);
    virtual ~ILastFmService();
    
    QByteArray getLastFmQuery(const QMap<QByteArray, QByteArray>& args) const;
    QByteArray getLastFmSignature(const QMap<QByteArray, QByteArray>& args) const;
    void logLastFmRequest(const QString& url, const QString& content = QString());
    void logLastFmResponse(int code, const QString& content);

    static QByteArray encodeString(const QByteArray& str);

protected slots:

    virtual void replyFinished(QNetworkReply * reply) = 0;

protected:
    
    static const QByteArray m_apiKey;
    static const QByteArray m_secret;
    static const QString m_lastFmUrl;

    CApplication * m_application;
    QByteArray m_sessionKey;
    QNetworkAccessManager * m_networkManager;
};

#endif // FILE_I_LAST_FM_SERVICE

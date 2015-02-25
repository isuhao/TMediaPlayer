/*
Copyright (C) 2012-2015 Teddy Michel

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

#ifndef FILE_I_LAST_FM_SERVICE
#define FILE_I_LAST_FM_SERVICE

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QString>


class CMediaManager;
class QNetworkReply;
class QNetworkAccessManager;


class ILastFmService : public QObject
{
    Q_OBJECT

public:

    ILastFmService(CMediaManager * mediaManager, const QByteArray& sessionKey);
    virtual ~ILastFmService();

    QByteArray getLastFmQuery(const QMap<QByteArray, QByteArray>& args) const;
    QByteArray getLastFmSignature(const QMap<QByteArray, QByteArray>& args) const;

    static QByteArray encodeString(const QByteArray& str);

protected slots:

    virtual void replyFinished(QNetworkReply * reply) = 0;

protected:

    static const QByteArray m_apiKey;
    static const QByteArray m_secret;
    static const QString m_lastFmUrl;

    CMediaManager * m_mediaManager;
    QByteArray m_sessionKey;
    QNetworkAccessManager * m_networkManager;
};

#endif // FILE_I_LAST_FM_SERVICE

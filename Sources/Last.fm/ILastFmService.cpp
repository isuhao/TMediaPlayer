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

#include "ILastFmService.hpp"
#include "../CMainWindow.hpp"
#include <QCryptographicHash>
#include <QDateTime>
#include <QNetworkAccessManager>

#include <QtDebug>


const QByteArray ILastFmService::m_apiKey = "20478fcc23bae9e1e2396a2b1cc52338";
const QByteArray ILastFmService::m_secret = "b2ed8ec840ec1995003bb99fb02ace44";
const QString ILastFmService::m_lastFmUrl = "http://ws.audioscrobbler.com/2.0/";


ILastFmService::ILastFmService(CMainWindow * mainWindow, const QByteArray& sessionKey) :
QObject      (mainWindow),
m_mainWindow (mainWindow),
m_sessionKey (sessionKey)
{
    Q_CHECK_PTR(m_mainWindow);

    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
}


ILastFmService::~ILastFmService()
{

}


QByteArray ILastFmService::getLastFmQuery(const QMap<QByteArray, QByteArray>& args) const
{
    QByteArray content;

    for (QMap<QByteArray, QByteArray>::ConstIterator it = args.begin(); it != args.end(); ++it)
    {
        if (it != args.begin())
        {
            content.append("&");
        }

        content.append(it.key());
        content.append("=");
        content.append(encodeString(it.value()));
    }

    content.append("&api_sig=");
    content.append(getLastFmSignature(args));

    return content;
}


/**
 * Calcule la signature d'une méthode pour envoyer une requête à Last.fm.
 *
 * \param args Tableau associatif des arguments (de la forme clé => valeur), avec la méthode.
 */

QByteArray ILastFmService::getLastFmSignature(const QMap<QByteArray, QByteArray>& args) const
{
    QCryptographicHash crypto(QCryptographicHash::Md5);

    for (QMap<QByteArray, QByteArray>::ConstIterator it = args.begin(); it != args.end(); ++it)
    {
        crypto.addData(it.key());
        crypto.addData(it.value());
    }

    crypto.addData(m_secret);
    return crypto.result().toHex();
}


/**
 * Convertit une chaine de caractère pour l'utiliser comme URL d'une requête à Last.fm
 *
 * \param str Chaine à convertir.
 * \return Chaine convertie (les caractères & et = sont remplacés par leur code hexadécimal).
 */

QByteArray ILastFmService::encodeString(const QByteArray& str)
{
    QByteArray res;

    // Encodage de la chaine
    for (int i = 0; i < str.size(); ++i)
    {
        if (str[i] == '&')
        {
            res.append("%26");
        }
        else if (str[i] == '=')
        {
            res.append("%3D");
        }
        else
        {
            res.append(str[i]);
        }
    }

    return res;
}

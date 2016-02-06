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

#ifndef FILE_C_AUTHENTICATION
#define FILE_C_AUTHENTICATION

#include "ILastFmService.hpp"


class QTimer;


/**
 * Lance le processus d'authentication avec Last.fm.
 * Le navigateur doit s'ouvrir pour que l'utilisateur puisse se connecter.
 */

class CAuthentication : public ILastFmService
{
    Q_OBJECT

public:

    explicit CAuthentication(CMediaManager * mediaManager);

protected slots:

    virtual void replyFinished(QNetworkReply * reply);
    void getLastFmSession();
    void replyLastFmFinished(QNetworkReply * reply);

private:

    QByteArray m_lastFmToken; ///< Token utilisé pour la connexion à Last.fm.
    QTimer * m_timerLastFm;   ///< Timer utilisé pour récupérer la clé.
    int m_numRequests;
};

#endif // FILE_C_AUTHENTICATION

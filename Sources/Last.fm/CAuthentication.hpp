
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

    CAuthentication(CApplication * application);

protected slots:

    virtual void replyFinished(QNetworkReply * reply);
    void getLastFmSession(void);
    void replyLastFmFinished(QNetworkReply * reply);

private:

    QByteArray m_lastFmToken; ///< Token utilisé pour la connexion à Last.fm.
    QTimer * m_timerLastFm;   ///< Timer utilisé pour récupérer la clé.
    int m_numRequests;
};

#endif // FILE_C_AUTHENTICATION

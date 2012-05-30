
#ifndef FILE_C_SCROBBLE
#define FILE_C_SCROBBLE

#include "ILastFmService.hpp"


class CSong;


class CScrobble : public ILastFmService
{
    Q_OBJECT

public:

    CScrobble(CApplication * application, const QByteArray& sessionKey, CSong * song);

protected slots:

    virtual void replyFinished(QNetworkReply * reply);

private:

    CSong * m_song;
};

#endif // FILE_C_SCROBBLE

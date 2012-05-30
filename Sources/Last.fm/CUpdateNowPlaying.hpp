
#ifndef FILE_C_UPDATE_NOW_PLAYING
#define FILE_C_UPDATE_NOW_PLAYING

#include "ILastFmService.hpp"


class CSong;


class CUpdateNowPlaying : public ILastFmService
{
    Q_OBJECT

public:

    CUpdateNowPlaying(CApplication * application, const QByteArray& sessionKey, CSong * song);

protected slots:

    virtual void replyFinished(QNetworkReply * reply);

private:

    CSong * m_song;
};

#endif // FILE_C_UPDATE_NOW_PLAYING

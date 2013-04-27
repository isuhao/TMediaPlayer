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

#ifndef FILE_C_UPDATE_NOW_PLAYING
#define FILE_C_UPDATE_NOW_PLAYING

#include "ILastFmService.hpp"


class CSong;


class CUpdateNowPlaying : public ILastFmService
{
    Q_OBJECT

public:

    CUpdateNowPlaying(CMainWindow * mainWindow, const QByteArray& sessionKey, CSong * song);

protected slots:

    virtual void replyFinished(QNetworkReply * reply);

private:

    CSong * m_song;
};

#endif // FILE_C_UPDATE_NOW_PLAYING

/*
Copyright (C) 2012-2014 Teddy Michel

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

#ifndef FILE_C_LYRIC_WIKI
#define FILE_C_LYRIC_WIKI

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QString>
#include <QWebPage>

class CMediaManager;
class CSong;
class QNetworkReply;
class QWebFrame;


class CLyricWiki : public QObject
{
    Q_OBJECT

public:

    CLyricWiki(CMediaManager * mediaManager, CSong * song);
    virtual ~CLyricWiki();

    inline CSong * getSong() const
    {
        return m_song;
    }

signals:

    void lyricsFound(const QString& lyrics);

protected slots:

    void replyFinished(QNetworkReply * reply);
    void replyFinished2(QNetworkReply * reply);
    void onPageFinished();

protected:

    CMediaManager * m_mediaManager;
    CSong * m_song;
    QWebPage page;
    QWebFrame * m_frame;
};

#endif // FILE_C_LYRIC_WIKI

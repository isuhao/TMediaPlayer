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

#include <QApplication>
#include <QTextCodec>
#include <QLocalSocket>
#include <ctime>

#include "CMediaManager.hpp"
#include "CMainWindow.hpp"


void myMessageOutput(QtMsgType type, const char * msg)
{
    const char symbols[] = { 'I', 'E', '!', 'X' };
    QString output = QString("%3 [%1] %2\r\n").arg(symbols[type]).arg(msg).arg(QDateTime::currentDateTimeUtc().toString("dd/MM/yyyy hh:mm:ss"));

    QFile file("TMediaPlayer.log");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    file.write(output.toAscii());
}


int main(int argc, char * argv[])
{
    qInstallMsgHandler(myMessageOutput);
    srand(time(nullptr));

    QApplication app(argc, argv);
    app.setWindowIcon(QPixmap(":/icons/TMediaPlayer"));

    QCoreApplication::setOrganizationName("Ted");
    QCoreApplication::setApplicationName("TMediaPlayer");

    QTextCodec * codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

#ifndef T_NO_SINGLE_APP
	QLocalSocket socket;
	socket.connectToServer("tmediaplayer-" + CMediaManager::getAppVersion());

	if (socket.waitForConnected(250))
	{
		// L'application est déjà lancée
		return 0;
	}
#endif // T_NO_SINGLE_APP

    CMediaManager mediaManager;

    CMainWindow window(&mediaManager);

    if (!window.initWindow())
    {
        return EXIT_FAILURE;
    }

    window.show();

    return app.exec();
}

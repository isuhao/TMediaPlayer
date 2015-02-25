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

    // Arguments du programme
    QStringList args = app.arguments();
    args.removeFirst();

    // Numéro de version
    if (args.contains("-V") || args.contains("--version"))
    {
        QTextStream(stdout) << QString("TMediaPlayer %1").arg(CMediaManager::getAppVersion()) << endl;
        QTextStream(stdout) << QString("Copyright (C) 2012-2015 Teddy Michel") << endl;
        QTextStream(stdout) << QString("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>") << endl;
        return EXIT_SUCCESS;
    }

#ifndef T_NO_SINGLE_APP
    QLocalSocket socket;
    socket.connectToServer("tmediaplayer-" + CMediaManager::getAppVersion());

    // L'application est déjà lancée
    if (socket.waitForConnected(100))
    {
        // Transmission des arguments
        if (!args.isEmpty())
        {
            QByteArray data;
            QDataStream out(&data, QIODevice::WriteOnly);
            out << args;

            socket.write(data);
            socket.waitForBytesWritten(100);
        }

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

    // Liste des morceaux à ajouter
    if (!args.isEmpty())
    {
        window.loadFiles(args);
    }

    return app.exec();
}

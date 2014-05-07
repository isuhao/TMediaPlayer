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

    // Numéro de version
    if (args.contains("-v") || args.contains("--version"))
    {
        QTextStream(stdout) << QString("TMediaPlayer %1").arg(CMediaManager::getAppVersion()) << endl;
        QTextStream(stdout) << QString("Copyright (C) 2012-2014 Teddy Michel") << endl;
        QTextStream(stdout) << QString("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html") << endl;
        return EXIT_SUCCESS;
    }

    //TODO: analyser les arguments pour récupérer la liste des morceaux à lire

#ifndef T_NO_SINGLE_APP
    QLocalSocket socket;
    socket.connectToServer("tmediaplayer-" + CMediaManager::getAppVersion());

    if (socket.waitForConnected(250))
    {
        // L'application est déjà lancée
        //TODO: si il y a une liste de morceaux, on doit les transmettre à l'application lancée
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

    //TODO: parcourir la liste des morceaux à lire
    //TODO: ajouter le morceau à la médiathèque si nécessaire
    //TODO: lire le dernier morceau de la liste

    return app.exec();
}

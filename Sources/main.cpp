/*
Copyright (C) 2012 Teddy Michel

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
#include <QTranslator>
#include <ctime>
#include "CApplication.hpp"


int main(int argc, char * argv[])
{
    srand(time(NULL));

    QApplication app(argc, argv);
    app.setWindowIcon(QPixmap(":/icons/TMediaPlayer"));

    QCoreApplication::setOrganizationName("Ted");
    QCoreApplication::setApplicationName("TMediaPlayer");

    // Internationnalisation
    QString locale = QLocale::system().name();
    QTranslator translator;
    translator.load(QString("Lang/TMediaPlayer_") + locale);
    app.installTranslator(&translator);

    CApplication lecteur;

    if (!lecteur.initWindow())
    {
        return EXIT_FAILURE;
    }

    lecteur.show();

    return app.exec();
}

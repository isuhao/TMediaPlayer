
#include <QApplication>
#include <QTranslator>
#include "CApplication.hpp"


int main(int argc, char * argv[])
{
    QApplication app(argc, argv);

    // Internationnalisation
    QString locale = QLocale::system().name();
    QTranslator translator;
    translator.load(QString("Lang/TMediaPlayer_") + locale);
    app.installTranslator(&translator);

    CApplication lecteur;
    lecteur.show();
    return app.exec();
}

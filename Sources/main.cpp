
#include <QApplication>
#include <QTranslator>
#include <ctime>
#include "CApplication.hpp"


int main(int argc, char * argv[])
{
    srand(time(NULL));

    QApplication app(argc, argv);
    app.setWindowIcon(QPixmap(":/icons/TMediaPlayer"));

    // Internationnalisation
    QString locale = QLocale::system().name();
    QTranslator translator;
    translator.load(QString("Lang/TMediaPlayer_") + locale);
    app.installTranslator(&translator);

    CApplication lecteur;
    lecteur.initWindow();
    lecteur.show();

    return app.exec();
}

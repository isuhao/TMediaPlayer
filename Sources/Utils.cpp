
#include "Utils.hpp"

#include <QMenu>
#include <QPoint>
#include <QApplication>
#include <QDesktopWidget>
#include <QTime>
#include <QDir>


/**
 * Détermine la position correcte pour un menu contextuel.
 *
 * \param menu         Pointeur sur le menu à positionner.
 * \param menuPosition Position du menu (par rapport à l'écran).
 * \return Position corrigée du menu, pour éviter qu'il ne sorte de l'écran.
 */

QPoint getCorrectMenuPosition(QMenu * menu, const QPoint& menuPosition)
{
    QPoint menuCorrectPos = menuPosition;

    if (!menu)
        return menuCorrectPos;

    QDesktopWidget * desktopWidget = QApplication::desktop();
    Q_CHECK_PTR(desktopWidget);

    const QRect screen = desktopWidget->screenGeometry(menu);
    const QSize size = menu->sizeHint();

    // Correction en hauteur
    const int screenHeight = screen.height();
    const int menuHeight = size.height();

    if (menuCorrectPos.y() + menuHeight > screenHeight)
    {
        if (menuHeight <= screenHeight)
            menuCorrectPos.setY(screenHeight - menuHeight);
        else
            menuCorrectPos.setY(0); // Le menu est plus haut que l'écran !
    }

    // Correction en largeur
    const int screenWidth = screen.width();
    const int menuWidth = size.width();

    if (menuCorrectPos.x() + menuWidth > screenWidth)
    {
        if (menuWidth <= screenWidth)
            menuCorrectPos.setX(screenWidth - menuWidth);
        else
            menuCorrectPos.setX(0); // Le menu est plus large que l'écran !
    }

    return menuCorrectPos;
}


/**
 * Convertit une durée en millisecondes en un texte affichable.
 *
 * \param durationMS Durée en millisecondes.
 * \return Texte.
 */

QString durationToString(qlonglong durationMS)
{
    QTime duration(0, 0);
    duration = duration.addMSecs(static_cast<int>(durationMS % 86400000));

    if (durationMS > 86400000)
    {
        int numDays = static_cast<int>(durationMS / 86400000);
        return QObject::tr("%n day(s) %1", "", numDays).arg(duration.toString());
    }
    else
    {
        return duration.toString();
    }
}


/**
 * Convertit un nombre d'octets en chaine de caractères plus compréhensible.
 *
 * \param fileSize Nombre d'octets.
 * \return Chaine de caractère utilisant les préfixes binaires (Kio, Mio, Gio, pour
 *         respectivement 1024, 1024*1024, et 1024*1024*1024 octets).
 */

QString getFileSize(qlonglong fileSize)
{
    Q_ASSERT(fileSize >= 0);

    // Plus de 1 Tio
    if (fileSize >= 1099511627776L/*1024 * 1024 * 1024 * 1024*/)
    {
        float fileSizeDisplay = static_cast<float>(static_cast<int>(static_cast<float>(10 * fileSize) / (1099511627776L/*1024 * 1024 * 1024 * 1024*/))) / 10;
        return QObject::tr("%1 Tio").arg(fileSizeDisplay);
    }
    // Plus de 1 Gio
    else if (fileSize >= 1073741824L/*1024 * 1024 * 1024*/)
    {
        float fileSizeDisplay = static_cast<float>(static_cast<int>(static_cast<float>(10 * fileSize) / (1073741824L/*1024 * 1024 * 1024*/))) / 10;
        return QObject::tr("%1 Gio").arg(fileSizeDisplay);
    }
    // Plus de 1 Mio
    else if (fileSize >= 1048576L/*1024 * 1024*/)
    {
        float fileSizeDisplay = static_cast<float>(static_cast<int>(static_cast<float>(10 * fileSize) / (1048576L/*1024 * 1024*/))) / 10;
        return QObject::tr("%1 Mio").arg(fileSizeDisplay);
    }
    // Moins de 1 Kio
    else if (fileSize >= 1024)
    {
        float fileSizeDisplay = static_cast<float>(static_cast<int>(static_cast<float>(10 * fileSize) / 1024)) / 10;
        return QObject::tr("%1 Kio").arg(fileSizeDisplay);
    }
    // Moins de 1 Kio
    else
    {
        return QObject::tr("%n byte(s)", "", fileSize);
    }
}


/**
 * Liste les morceaux contenus dans un répertoire.
 *
 * \param pathName Nom du répertoire à parcourir récursivement.
 * \return Liste des fichiers du répertoire.
 */

QStringList listFilesInFolder(const QString& pathName)
{
    QStringList fileList;
    QDir dir(pathName);

    QStringList dirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (QStringList::const_iterator it = dirList.begin(); it != dirList.end(); ++it)
    {
        fileList.append(listFilesInFolder(dir.absoluteFilePath(*it)));
    }

    QStringList fileDirList = dir.entryList(QDir::Files | QDir::Readable, QDir::Name);

    for (QStringList::const_iterator it = fileDirList.begin(); it != fileDirList.end(); ++it)
    {
        fileList.append(dir.absoluteFilePath(*it).replace('\\', '/'));
    }

    return fileList;
}

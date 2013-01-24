
#include "Utils.hpp"
#include <QMenu>
#include <QPoint>
#include <QPoint>
#include <QApplication>
#include <QDesktopWidget>


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

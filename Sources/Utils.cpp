
#include "Utils.hpp"
#include <QMenu>
#include <QPoint>
#include <QPoint>
#include <QApplication>
#include <QDesktopWidget>


/**
 * D�termine la position correcte pour un menu contextuel.
 *
 * \param menu         Pointeur sur le menu � positionner.
 * \param menuPosition Position du menu (par rapport � l'�cran).
 * \return Position corrig�e du menu, pour �viter qu'il ne sorte de l'�cran.
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
            menuCorrectPos.setY(0); // Le menu est plus haut que l'�cran !
    }

    // Correction en largeur
    const int screenWidth = screen.width();
    const int menuWidth = size.width();

    if (menuCorrectPos.x() + menuWidth > screenWidth)
    {
        if (menuWidth <= screenWidth)
            menuCorrectPos.setX(screenWidth - menuWidth);
        else
            menuCorrectPos.setX(0); // Le menu est plus large que l'�cran !
    }

    return menuCorrectPos;
}

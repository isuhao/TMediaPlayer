
#include "CRating.hpp"
#include <QtGui>
#include <cmath>


const int PaintingScaleFactor = 15;


CRating::CRating(int rating)
{
    setRating(rating);

    for (int i = 0; i < 5; ++i)
    {
        double angle = (0.8 * i - 0.5) * 3.14;
        m_starPolygon << QPointF(0.5 + 0.5 * cos(angle), 0.5 + 0.5 * sin(angle));
    }

    m_diamondPolygon << QPointF(0.4, 0.5) << QPointF(0.5, 0.4) << QPointF(0.6, 0.5) << QPointF(0.5, 0.6) << QPointF(0.4, 0.5);
}


QSize CRating::sizeHint() const
{
    return PaintingScaleFactor * QSize(5, 1);
}


void CRating::paint(QPainter *painter, const QRect &rect, const QPalette &palette, EditMode mode) const
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);

    if (mode == Editable)
    {
        painter->setBrush(palette.highlight());
    }
    else
    {
        painter->setBrush(palette.foreground());
    }

    int yOffset = (rect.height() - PaintingScaleFactor) / 2;
    painter->translate(rect.x(), rect.y() + yOffset);
    painter->scale(PaintingScaleFactor, PaintingScaleFactor);

    for (int i = 0; i < 5; ++i)
    {
        if (i < m_rating)
        {
            painter->drawPolygon(m_starPolygon, Qt::WindingFill);
        }
        else if (mode == Editable)
        {
            painter->drawPolygon(m_diamondPolygon, Qt::WindingFill);
        }

        painter->translate(1.0, 0.0);
    }

    painter->restore();
}

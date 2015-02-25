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

#include "CRating.hpp"
#include <QtGui>
#include <cmath>


const int PaintingScaleFactor = 15;


QPolygonF CRating::m_starPolygon;
QPolygonF CRating::m_diamondPolygon;
bool CRating::m_polygonInitialized = false;


CRating::CRating(int rating)
{
    setRating(rating);

    if (!m_polygonInitialized)
    {
        for (int i = 0; i < 5; ++i)
        {
            double angle = (0.8 * i - 0.5) * 3.14;
            m_starPolygon << QPointF(0.5 + 0.5 * cos(angle), 0.5 + 0.5 * sin(angle));
        }

        m_diamondPolygon << QPointF(0.4, 0.5) << QPointF(0.5, 0.4) << QPointF(0.6, 0.5) << QPointF(0.5, 0.6) << QPointF(0.4, 0.5);

        m_polygonInitialized = true;
    }
}


CRating::CRating(const CRating& other) :
m_rating (other.m_rating)
{
    if (!m_polygonInitialized)
    {
        for (int i = 0; i < 5; ++i)
        {
            double angle = (0.8 * i - 0.5) * 3.14;
            m_starPolygon << QPointF(0.5 + 0.5 * cos(angle), 0.5 + 0.5 * sin(angle));
        }

        m_diamondPolygon << QPointF(0.4, 0.5) << QPointF(0.5, 0.4) << QPointF(0.6, 0.5) << QPointF(0.5, 0.6) << QPointF(0.4, 0.5);

        m_polygonInitialized = true;
    }
}


QSize CRating::sizeHint() const
{
    return PaintingScaleFactor * QSize(5, 1);
}


QSize CRating::minimumSizeHint() const
{
    return sizeHint();
}


/**
 * Dessine la note sous forme d'étoiles.
 *
 * \todo Gérer correctement la couleur de fond.
 */

void CRating::paint(QPainter * painter, const QRect& rect, const QPalette& palette, EditMode mode) const
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

/*
Copyright (C) 2012-2013 Teddy Michel

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

#ifndef FILE_C_RATING
#define FILE_C_RATING

#include <QMetaType>
#include <QPointF>
#include <QVector>
#include <QPolygonF>


class QPainter;


class CRating
{
public:

    enum EditMode
    {
        Editable,
        ReadOnly
    };

    CRating(int rating = 0);
    CRating(const CRating& other);

    void paint(QPainter * painter, const QRect& rect, const QPalette& palette, EditMode mode) const;
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    inline int getRating() const;
    inline void setRating(int rating);

    inline CRating& operator=(const CRating& other)
    {
        m_rating = other.m_rating;
        return *this;
    }

private:

    static QPolygonF m_starPolygon;
    static QPolygonF m_diamondPolygon;
    static bool m_polygonInitialized;
    int m_rating;
};

Q_DECLARE_METATYPE(CRating)


inline int CRating::getRating() const
{
    return m_rating;
}


inline void CRating::setRating(int rating)
{
    if (rating < 0 || rating > 5) return;
    m_rating = rating;
}

#endif // FILE_C_RATING

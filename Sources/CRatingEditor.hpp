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

#ifndef FILE_C_RATING_EDITOR
#define FILE_C_RATING_EDITOR

#include <QWidget>
#include "CRating.hpp"


class CRatingEditor : public QWidget
{
    Q_OBJECT

public:

    CRatingEditor(QWidget * parent = NULL);

    QSize sizeHint() const;
    inline void setRating(const CRating& rating);
    inline CRating getRating(void);

signals:

    void editingFinished(void);

protected:

     void paintEvent(QPaintEvent * event);
     void mouseMoveEvent(QMouseEvent * event);
     void mouseReleaseEvent(QMouseEvent * event);

private:

     int starAtPosition(int x) const;

     CRating m_rating;
};


inline void CRatingEditor::setRating(const CRating& rating)
{
    m_rating = rating;
}


inline CRating CRatingEditor::getRating()
{
    return m_rating;
}

#endif // FILE_C_RATING_EDITOR

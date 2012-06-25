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

#include "CRatingEditor.hpp"
#include <QtGui>


CRatingEditor::CRatingEditor(QWidget * parent) :
    QWidget (parent)
{
    setMouseTracking(true);
    setAutoFillBackground(true);
}


QSize CRatingEditor::sizeHint() const
{
    return m_rating.sizeHint();
}


void CRatingEditor::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    m_rating.paint(&painter, rect(), this->palette(), CRating::Editable);
}


void CRatingEditor::mouseMoveEvent(QMouseEvent * event)
{
    int star = starAtPosition(event->x());

    if (star != m_rating.getRating())
    {
        m_rating.setRating(star);
        update();
    }
}


void CRatingEditor::mouseReleaseEvent(QMouseEvent * event )
{
    emit editingFinished();
}


int CRatingEditor::starAtPosition(int x) const
{
    double star = (5 * static_cast<double>(x) / m_rating.sizeHint().width());
    return qBound(0, qRound(star), 5);
}

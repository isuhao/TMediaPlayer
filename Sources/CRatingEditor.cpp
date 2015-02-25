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

#include "CRatingEditor.hpp"
#include <QtGui>


CRatingEditor::CRatingEditor(QWidget * parent) :
    QWidget    (parent),
    m_delegate (true),
    m_editMode (CRating::Editable)
{
    setMouseTracking(true);
    setAutoFillBackground(true);
}


QSize CRatingEditor::sizeHint() const
{
    return m_rating.sizeHint();
}


QSize CRatingEditor::minimumSizeHint() const
{
    return m_rating.minimumSizeHint();
}


void CRatingEditor::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    m_rating.paint(&painter, rect(), this->palette(), m_editMode);
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


void CRatingEditor::mouseReleaseEvent(QMouseEvent * event)
{
    m_saveRating = m_rating;
    emit editingFinished();
}


void CRatingEditor::enterEvent(QEvent * event)
{
    if (m_delegate)
        return;

    m_saveRating = m_rating;
    m_editMode = CRating::Editable;
    update();
}


void CRatingEditor::leaveEvent(QEvent * event)
{
    if (m_delegate)
        return;

    m_rating = m_saveRating;
    m_editMode = CRating::ReadOnly;
    update();
}


int CRatingEditor::starAtPosition(int x) const
{
    double star = (5 * static_cast<double>(x) / m_rating.sizeHint().width());
    return qBound(0, qRound(star), 5);
}

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

    explicit CRatingEditor(QWidget * parent = NULL);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    inline CRating getRating() const;
    inline int getRatingValue() const;
    inline void setRating(const CRating& rating);
    inline void setRatingValue(int value);

    inline void setDelegate(bool delegate)
    {
        m_delegate = delegate;

        if (!delegate)
        {
            m_saveRating = m_rating;
            m_editMode = CRating::ReadOnly;
        }
    }

signals:

    void editingFinished();

protected:

     virtual void paintEvent(QPaintEvent * event);
     virtual void mouseMoveEvent(QMouseEvent * event);
     virtual void mouseReleaseEvent(QMouseEvent * event);
     virtual void enterEvent(QEvent * event);
     virtual void leaveEvent(QEvent * event);

private:

     int starAtPosition(int x) const;

     CRating m_rating;
     CRating m_saveRating;
     bool m_delegate;              ///< Indique si le widget a été crée par un délégué dans une vue.
     CRating::EditMode m_editMode; ///< Mode d'édition.
};


inline CRating CRatingEditor::getRating() const
{
    return m_rating;
}


inline int CRatingEditor::getRatingValue() const
{
    return m_rating.getRating();
}


inline void CRatingEditor::setRating(const CRating& rating)
{
    m_rating = rating;
}


inline void CRatingEditor::setRatingValue(int value)
{
    m_rating.setRating(value);
}

#endif // FILE_C_RATING_EDITOR

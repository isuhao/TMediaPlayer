
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

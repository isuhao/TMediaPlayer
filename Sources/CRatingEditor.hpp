
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

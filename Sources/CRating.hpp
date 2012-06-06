
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

    void paint(QPainter * painter, const QRect &rect, const QPalette &palette, EditMode mode) const;
    QSize sizeHint(void) const;
    inline int getRating(void) const;
    inline void setRating(int rating);

private:

    QPolygonF m_starPolygon;
    QPolygonF m_diamondPolygon;
    int m_rating;
};

Q_DECLARE_METATYPE(CRating)


inline int CRating::getRating() const
{
    return m_rating;
}


inline void CRating::setRating(int rating)
{
    if (rating < 0 ||rating > 5) return;
    m_rating = rating;
}

#endif // FILE_C_RATING

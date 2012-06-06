
#ifndef FILE_C_RATING_DELEGATE
#define FILE_C_RATING_DELEGATE

#include <QStyledItemDelegate>


class CRatingDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    inline CRatingDelegate(QWidget * parent = NULL);

    void paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void setEditorData(QWidget * editor, const QModelIndex& index) const;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex& index) const;

private slots:

    void commitAndCloseEditor();
};


inline CRatingDelegate::CRatingDelegate(QWidget * parent) :
    QStyledItemDelegate (parent)
{

}

#endif // FILE_C_RATING_DELEGATE

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

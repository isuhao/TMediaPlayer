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

#include "CRatingDelegate.hpp"
#include "CRatingEditor.hpp"
#include <QtGui>


void CRatingDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_CHECK_PTR(painter);

    if (index.data().canConvert<CRating>())
    {
        CRating starRating = qvariant_cast<CRating>(index.data());

        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        starRating.paint(painter, option.rect, option.palette,
        CRating::ReadOnly);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}


QSize CRatingDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data().canConvert<CRating>())
    {
        CRating starRating = qvariant_cast<CRating>(index.data());
        return starRating.sizeHint();
    }
    else
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}


QWidget * CRatingDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data().canConvert<CRating>())
    {
        CRatingEditor * editor = new CRatingEditor(parent);
        connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
        return editor;
    }
    else
    {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}


void CRatingDelegate::setEditorData(QWidget * editor, const QModelIndex& index) const
{
    if (index.data().canConvert<CRating>())
    {
        CRating starRating = qvariant_cast<CRating>(index.data());
        CRatingEditor * ratingEditor = qobject_cast<CRatingEditor *>(editor);
        ratingEditor->setRating(starRating);
    }
    else
    {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}


void CRatingDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex& index) const
{
    if (index.data().canConvert<CRating>())
    {
        CRatingEditor * ratingEditor = qobject_cast<CRatingEditor *>(editor);
        model->setData(index, QVariant::fromValue(ratingEditor->getRating()));
    }
    else
    {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}


void CRatingDelegate::commitAndCloseEditor()
{
    CRatingEditor * editor = qobject_cast<CRatingEditor *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

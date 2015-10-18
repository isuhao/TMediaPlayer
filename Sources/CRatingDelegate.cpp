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

#include "CRatingDelegate.hpp"
#include "CRatingEditor.hpp"
//#include "CSongTitle.hpp"
#include <QtGui>


void CRatingDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_CHECK_PTR(painter);

    painter->save();

    if (index.data().canConvert<CRating>())
    {
        CRating starRating = qvariant_cast<CRating>(index.data());

        if (option.state & QStyle::State_Selected)
        {
            if (option.state & QStyle::State_Active)
                painter->fillRect(option.rect, option.palette.highlight());
            else
                painter->fillRect(option.rect, option.palette.brush(QPalette::Inactive, QPalette::Highlight));
        }

        starRating.paint(painter, option.rect, option.palette, CRating::ReadOnly);
    }
/*
    else if (index.data().canConvert<CSongTitle>())
    {
        QStyleOptionViewItemV4 optionV4 = option;
        initStyleOption(&optionV4, index);

        CSongTitle songTitle = qvariant_cast<CSongTitle>(index.data());
        songTitle.paint(painter, optionV4);
    }
*/
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }

    painter->restore();
}


QSize CRatingDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data().canConvert<CRating>())
    {
        CRating starRating = qvariant_cast<CRating>(index.data());
        return starRating.sizeHint();
    }
/*
    if (index.data().canConvert<CSongTitle>())
    {
        QStyleOptionViewItemV4 optionV4 = option;
        initStyleOption(&optionV4, index);

        CSongTitle songTitle = qvariant_cast<CSongTitle>(index.data());
        return songTitle.sizeHint(optionV4);
    }
*/
    return QStyledItemDelegate::sizeHint(option, index);
}


QWidget * CRatingDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data().canConvert<CRating>())
    {
        CRatingEditor * editor = new CRatingEditor(parent);
        connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
        return editor;
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}


void CRatingDelegate::setEditorData(QWidget * editor, const QModelIndex& index) const
{
    if (index.data().canConvert<CRating>())
    {
        CRating starRating = qvariant_cast<CRating>(index.data());
        CRatingEditor * ratingEditor = qobject_cast<CRatingEditor *>(editor);
        ratingEditor->setRating(starRating);
        return;
    }

    QStyledItemDelegate::setEditorData(editor, index);
}


void CRatingDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex& index) const
{
    if (index.data().canConvert<CRating>())
    {
        CRatingEditor * ratingEditor = qobject_cast<CRatingEditor *>(editor);
        model->setData(index, QVariant::fromValue(ratingEditor->getRating()));
        return;
    }

    QStyledItemDelegate::setModelData(editor, model, index);
}


void CRatingDelegate::commitAndCloseEditor()
{
    CRatingEditor * editor = qobject_cast<CRatingEditor *>(sender());

    if (editor != nullptr)
    {
        emit commitData(editor);
        emit closeEditor(editor);
    }
}

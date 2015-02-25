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

#ifndef FILE_C_WIDGET_LINE_EDIT_MULTIPLE_HPP_
#define FILE_C_WIDGET_LINE_EDIT_MULTIPLE_HPP_

#include <QWidget>

class QLineEdit;
class QComboBox;
class QHBoxLayout;


class CWidgetLineEditMultiple : public QWidget
{
    Q_OBJECT

public:

	CWidgetLineEditMultiple(QWidget * parent = nullptr);
	virtual ~CWidgetLineEditMultiple();

	void setContent(const QStringList& values, const QString& placeHolder);
	void setContent(const QString& value);

	QString text() const;
    
    QString placeholderText() const;
    void setPlaceholderText(const QString& placeHolder);

	//virtual QSize minimumSizeHint() const;
	//virtual QSize sizeHint() const;

public slots:

	//virtual void setVisible(bool visible);
    void setText(const QString& text);

signals:

	void textChanged(const QString& text);

private:

	QLineEdit * m_lineEdit;
	QComboBox * m_comboBox;
    QHBoxLayout * m_layout;
};

#endif // FILE_C_WIDGET_LINE_EDIT_MULTIPLE_HPP_

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

#include "CWidgetLineEditMultiple.hpp"
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>


CWidgetLineEditMultiple::CWidgetLineEditMultiple(QWidget * parent) :
QWidget    (parent),
m_lineEdit (nullptr),
m_comboBox (nullptr),
m_layout   (nullptr)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

	setContent(QString());
}

CWidgetLineEditMultiple::~CWidgetLineEditMultiple()
{
	delete m_lineEdit;
	delete m_comboBox;
}

void CWidgetLineEditMultiple::setContent(const QStringList& values, const QString& placeHolder)
{
	if (values.isEmpty())
	{
		setContent(QString());
		return;
	}

	if (values.size() == 1)
	{
		setContent(values.at(0));
		return;
	}

	if (m_lineEdit != nullptr)
	{
		delete m_lineEdit;
		m_lineEdit = nullptr;
	}

	if (m_comboBox == nullptr)
	{
		m_comboBox = new QComboBox();
		//m_comboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
		m_comboBox->setEditable(true);
		m_comboBox->setInsertPolicy(QComboBox::NoInsert);
        m_layout->addWidget(m_comboBox);

		connect(m_comboBox, SIGNAL(editTextChanged(const QString&)), this, SIGNAL(textChanged(const QString&)));
		connect(m_comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SIGNAL(textChanged(const QString&)));
		//connect(m_comboBox->lineEdit(), SIGNAL(textEdited(const QString&)), this, SIGNAL(textChanged(const QString&)));
	}
	else
	{
		m_comboBox->clear();
	}

	for (QStringList::ConstIterator it = values.begin(); it != values.end(); ++it)
	{
		m_comboBox->addItem(*it);
	}

	m_comboBox->setCurrentIndex(-1);
	m_comboBox->lineEdit()->setPlaceholderText(placeHolder);
}

void CWidgetLineEditMultiple::setContent(const QString& value)
{
	if (m_comboBox != nullptr)
	{
		delete m_comboBox;
		m_comboBox = nullptr;
	}

	if (m_lineEdit == nullptr)
	{
		m_lineEdit = new QLineEdit(this);
        m_layout->addWidget(m_lineEdit);

		connect(m_lineEdit, SIGNAL(textEdited(const QString&)), this, SIGNAL(textChanged(const QString&)));
	}

	m_lineEdit->setText(value);
}

QString CWidgetLineEditMultiple::text() const
{
	if (m_comboBox != nullptr)
		return m_comboBox->currentText();
	if (m_lineEdit != nullptr)
		return m_lineEdit->text();

	return QString();
}


void CWidgetLineEditMultiple::setText(const QString& text)
{
    if (m_comboBox != nullptr)
        return m_comboBox->setEditText(text);
	if (m_lineEdit != nullptr)
		return m_lineEdit->setText(text);
}


QString CWidgetLineEditMultiple::placeholderText() const
{
	if (m_comboBox != nullptr)
        return m_comboBox->lineEdit()->placeholderText();
	if (m_lineEdit != nullptr)
		return m_lineEdit->placeholderText();

    return QString();
}

    
void CWidgetLineEditMultiple::setPlaceholderText(const QString& placeHolder)
{
	if (m_comboBox != nullptr)
        return m_comboBox->lineEdit()->setPlaceholderText(placeHolder);
	if (m_lineEdit != nullptr)
		return m_lineEdit->setPlaceholderText(placeHolder);
}

/*
QSize CWidgetLineEditMultiple::minimumSizeHint() const
{
	if (m_comboBox != nullptr)
		return m_comboBox->minimumSizeHint();
	if (m_lineEdit != nullptr)
		return m_lineEdit->minimumSizeHint();

	return QSize();
}


QSize CWidgetLineEditMultiple::sizeHint() const
{
	if (m_comboBox != nullptr)
		return m_comboBox->sizeHint();
	if (m_lineEdit != nullptr)
		return m_lineEdit->sizeHint();

	return QSize();
}


void CWidgetLineEditMultiple::setVisible(bool visible)
{
	if (m_comboBox != nullptr)
		return m_comboBox->setVisible(visible);
	if (m_lineEdit != nullptr)
		return m_lineEdit->setVisible(visible);
}
*/

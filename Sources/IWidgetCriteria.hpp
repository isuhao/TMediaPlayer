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

#ifndef FILE_I_WIDGET_CRITERIA
#define FILE_I_WIDGET_CRITERIA

#include <QWidget>
#include "ICriteria.hpp"


class IWidgetCriteria : public QWidget
{
    Q_OBJECT

public:

    explicit IWidgetCriteria(CApplication * application, QWidget * parent = NULL);
    virtual ~IWidgetCriteria();

    virtual ICriteria * getCriteria(void) = 0;
    
protected:

    ICriteria::TType m_type;
    ICriteria::TCondition m_condition;
    CApplication * m_application;
};

#endif // FILE_I_WIDGET_CRITERIA

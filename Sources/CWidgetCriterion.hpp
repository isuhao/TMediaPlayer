/*
Copyright (C) 2012-2013 Teddy Michel

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

#ifndef FILE_C_WIDGET_CRITERION_HPP_
#define FILE_C_WIDGET_CRITERION_HPP_

#include "IWidgetCriterion.hpp"
#include "ui_WidgetCriterion.h"


/**
 * Widget permettant d'éditer un critère d'une liste dynamique.
 */

class CWidgetCriterion : public IWidgetCriterion
{
    Q_OBJECT

    friend class CCriterion;

public:

    explicit CWidgetCriterion(CMainWindow * mainWindow, QWidget * parent = nullptr);
    virtual ~CWidgetCriterion();

    virtual ICriterion * getCriterion();

protected slots:

    void changeType(int num);
    void changeConditionBoolean(int num);
    void changeConditionString(int num);
    void changeConditionNumber(int num);
    void changeConditionTime(int num);
    void changeConditionDate(int num);

private:

    Ui::WidgetCriterion * m_uiWidget;
};

#endif // FILE_C_WIDGET_CRITERION_HPP_

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

#ifndef FILE_C_WIDGET_MULTI_CRITERION
#define FILE_C_WIDGET_MULTI_CRITERION

#include "IWidgetCriterion.hpp"
#include "CMultiCriteria.hpp" // Si on ne l'inclut pas, l'intellisense bug...
#include "ui_WidgetMultiCriteria.h"
#include <QList>
#include <QMap>


class CMultiCriteria;
class QPushButton;


/**
 * Widget permettant d'éditer un sous-critère d'une liste dynamique.
 */

class CWidgetMultiCriteria : public IWidgetCriterion
{
    Q_OBJECT

    friend class CMultiCriteria;

public:

    explicit CWidgetMultiCriteria(CMainWindow * application, QWidget * parent = nullptr);
    virtual ~CWidgetMultiCriteria();

    virtual ICriterion * getCriterion();

protected slots:

    void setMultiCriteriaType(int type);
    void addCriterion();
    void addMultiCriteria();
    void removeCriterion(int row);
    void removeCriterionFromButton();

private:

    void addCriterion(IWidgetCriterion * criteriaWidget);

    Ui::WidgetMultiCriteria * m_uiWidget;
    QList<IWidgetCriterion *> m_children;
    QMap<QPushButton *, int> m_btnRemove;
};

#endif // FILE_C_WIDGET_MULTI_CRITERION

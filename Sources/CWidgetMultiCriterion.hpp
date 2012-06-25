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

#ifndef FILE_C_WIDGET_MULTI_CRITERION
#define FILE_C_WIDGET_MULTI_CRITERION

#include "IWidgetCriteria.hpp"
#include "CMultiCriterion.hpp" // Si on ne l'inclut pas, l'intellisense bug...
#include "ui_WidgetMultiCriterion.h"
#include <QList>
#include <QMap>


class CMultiCriterion;
class QPushButton;


/**
 * Widget permettant d'éditer un sous-critère d'une liste dynamique.
 */

class CWidgetMultiCriterion : public IWidgetCriteria
{
    Q_OBJECT

    friend class CMultiCriterion;

public:

    explicit CWidgetMultiCriterion(CApplication * application, QWidget * parent = NULL);
    virtual ~CWidgetMultiCriterion();

    virtual ICriteria * getCriteria(void);

protected slots:

    void setMultiCriterionType(int type);
    void addCriteria(void);
    void addMultiCriterion(void);
    void removeCriteria(int row);
    void removeCriteriaFromButton(void);

private:

    void addCriteria(IWidgetCriteria * criteriaWidget);

    Ui::WidgetMultiCriterion * m_uiWidget;
    QList<IWidgetCriteria *> m_children;
    QMap<QPushButton *, int> m_btnRemove;
};

#endif // FILE_C_WIDGET_MULTI_CRITERION

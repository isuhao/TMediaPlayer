/*
Copyright (C) 2012-2014 Teddy Michel

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
#include "ICriterion.hpp"


/**
 * Widget représentant un critère multiple.
 */

class IWidgetCriterion : public QWidget
{
    Q_OBJECT

public:

    explicit IWidgetCriterion(CMainWindow * mainWindow, QWidget * parent = nullptr);
    virtual ~IWidgetCriterion();

    virtual ICriterion * getCriterion() = 0;

protected:

    ICriterion::TType m_type;           ///< Type de critère.
    ICriterion::TCondition m_condition; ///< Condition de recherche.
    CMainWindow * m_mainWindow;         ///< Pointeur sur la fenêtre principale de l'application.
};

#endif // FILE_I_WIDGET_CRITERIA

/*
Copyright (C) 2012-2016 Teddy Michel

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

#ifndef FILE_C_DIALOG_EFFECTS
#define FILE_C_DIALOG_EFFECTS

#include <QDialog>
#include "ui_DialogEffects.h"


class CMainWindow;


/**
 * Boite de dialogue des effets sonores.
 */

class CDialogEffects : public QDialog
{
    Q_OBJECT

public:

    explicit CDialogEffects(CMainWindow * mainWindow);
    virtual ~CDialogEffects();

protected slots:

    void enableEcho(bool enable);
    void echoDelayChanged(int delay);
    void enableFilter(bool enable);
    void minFreqChanged(int frequency);
    void maxFreqChanged(int frequency);

private:

    Ui::DialogEffects * m_uiWidget; ///< Pointeur sur le widget de la boite de dialogue.
    CMainWindow * m_mainWindow;     ///< Pointeur sur la fenêtre principale de l'application.
};

#endif // FILE_C_DIALOG_EFFECTS

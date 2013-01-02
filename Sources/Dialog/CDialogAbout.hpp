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

#ifndef FILE_C_DIALOG_ABOUT
#define FILE_C_DIALOG_ABOUT

#include <QDialog>
#include "ui_DialogAbout.h"


class CApplication;


/**
 * Boite de dialogue "À propos".
 */

class CDialogAbout : public QDialog
{
    Q_OBJECT

public:

    explicit CDialogAbout(CApplication * application);
    virtual ~CDialogAbout();

private:

    Ui::DialogAbout * m_uiWidget; ///< Pointeur sur le widget de la boite de dialogue.
    CApplication * m_application; ///< Pointeur sur la classe principale de l'application.
};

#endif // FILE_C_DIALOG_ABOUT

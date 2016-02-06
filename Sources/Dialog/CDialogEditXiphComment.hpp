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

#ifndef FILE_C_DIALOG_EDIT_XIPHCOMMENT
#define FILE_C_DIALOG_EDIT_XIPHCOMMENT

#include <QDialog>
#include "ui_DialogEditMetadataXiph.h"


class CMainWindow;


/**
 * Boite de dialogue pour créer ou modifier un tag XiphComment.
 */

class CDialogEditXiphComment : public QDialog
{
    Q_OBJECT

public:

    CDialogEditXiphComment(CMainWindow * mainWindow, const QString& tag = QString(), const QString& value = QString());
    virtual ~CDialogEditXiphComment();

    QString getTag() const;
    QString getValue() const;

public slots:

    virtual void accept();

private:

    Ui::DialogEditXiphComment * m_uiWidget;
    CMainWindow * m_mainWindow; ///< Pointeur sur la fenêtre principale de l'application.
};

#endif // FILE_C_DIALOG_EDIT_XIPHCOMMENT

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

#ifndef FILE_C_DIALOG_PREFERENCES
#define FILE_C_DIALOG_PREFERENCES

#include <QDialog>
#include "ui_DialogPreferences.h"


class CMainWindow;
class QSettings;
class QNetworkReply;


class CDialogPreferences : public QDialog
{
    Q_OBJECT

public:

    CDialogPreferences(CMainWindow * mainWindow, QSettings * settings);
    virtual ~CDialogPreferences();

protected slots:

    void save();
    void onDriverChange(const QString& name);
    void addFolder();
    void editSelectedFolder();
    void removeSelectedFolder();

private:

    Ui::DialogPreferences * m_uiWidget; ///< Widget utilisé par la boite de dialogue.
    CMainWindow * m_mainWindow;         ///< Pointeur sur la fenêtre principale l'application.
    QSettings * m_settings;
};

#endif // FILE_C_DIALOG_PREFERENCES

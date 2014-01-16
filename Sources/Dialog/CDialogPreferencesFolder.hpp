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

#ifndef FILE_C_DIALOG_PREFERENCES_FOLDER
#define FILE_C_DIALOG_PREFERENCES_FOLDER

#include <QDialog>
#include "ui_DialogPreferencesFolder.h"


class CMainWindow;
class CLibraryFolder;


class CDialogPreferencesFolder : public QDialog
{
    Q_OBJECT

public:

    CDialogPreferencesFolder(CMainWindow * mainWindow, QWidget * parent, CLibraryFolder * folder);
    virtual ~CDialogPreferencesFolder();

protected slots:

    void chooseFolder();
    void save();

private:

    Ui::DialogPreferencesFolder * m_uiWidget; ///< Widget utilisé par la boite de dialogue.
    CMainWindow * m_mainWindow;               ///< Pointeur sur la fenêtre principale de l'application.
    CLibraryFolder * m_folder;                ///< Pointeur sur le dossier à modifier.
    bool m_needDeleteFolder;
};

#endif // FILE_C_DIALOG_PREFERENCES_FOLDER

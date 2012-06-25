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

#ifndef FILE_C_DIALOG_EDIT_FOLDER
#define FILE_C_DIALOG_EDIT_FOLDER

#include <QDialog>
#include "ui_DialogEditFolder.h"


class CApplication;
class CFolder;


class CDialogEditFolder : public QDialog
{
    Q_OBJECT

public:

    CDialogEditFolder(CFolder * folder, CApplication * application, CFolder * folderParent = NULL);
    virtual ~CDialogEditFolder();

protected slots:

    void save(void);

private:
    
    Ui::DialogEditFolder * m_uiWidget;
    CFolder * m_folder;
    CApplication * m_application;
    CFolder * m_folderParent;
};

#endif // FILE_C_DIALOG_EDIT_FOLDER

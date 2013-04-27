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

#ifndef FILE_C_DIALOG_EDIT_DYNAMIC_LIST
#define FILE_C_DIALOG_EDIT_DYNAMIC_LIST

#include <QDialog>
#include "ui_DialogEditDynamicPlayList.h"


class CDynamicList;
class CMainWindow;
class CWidgetMultiCriteria;
class CFolder;


/**
 * Boite de dialogue pour créer ou modifier une liste de lecture dynamique.
 */

class CDialogEditDynamicList : public QDialog
{
    Q_OBJECT

public:

    CDialogEditDynamicList(CDynamicList * playList, CMainWindow * mainWindow, CFolder * folder);
    virtual ~CDialogEditDynamicList();

public slots:

    void resizeWindow();

protected slots:

    void save();

private:
    
    Ui::DialogEditDynamicPlayList * m_uiWidget;
    CWidgetMultiCriteria * m_widgetCriteria;
    CDynamicList * m_playList;
    CMainWindow * m_mainWindow;
    CFolder * m_folder;
};

#endif // FILE_C_DIALOG_EDIT_DYNAMIC_LIST

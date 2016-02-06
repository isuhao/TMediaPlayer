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

#ifndef FILE_C_DIALOG_CDROM_DRIVE_INFOS
#define FILE_C_DIALOG_CDROM_DRIVE_INFOS

#include <QDialog>
#include "ui_DialogCDRomDriveInfos.h"


class CMainWindow;
class CCDRomDrive;


/**
 * Boite de dialogue d'informations sur un lecteur de CD-ROM.
 */

class CDialogCDRomDriveInfos : public QDialog
{
    Q_OBJECT

public:

    CDialogCDRomDriveInfos(CCDRomDrive * cdRomDrive, CMainWindow * mainWindow);
    virtual ~CDialogCDRomDriveInfos();

private:

    Ui::DialogCDRomDriveInfos * m_uiWidget; ///< Pointeur sur le widget de la boite de dialogue.
    CMainWindow * m_mainWindow;             ///< Pointeur sur la classe principale de l'application.
    CCDRomDrive * m_cdRomDrive;             ///< Pointeur sur le lecteur de CD-ROM.
};

#endif // FILE_C_DIALOG_CDROM_DRIVE_INFOS

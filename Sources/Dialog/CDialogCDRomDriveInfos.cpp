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

#include "CDialogCDRomDriveInfos.hpp"
#include "../CCDRomDrive.hpp"
#include "../CMainWindow.hpp"
#include <QPushButton>


/**
 * Constructeur de la boite de dialogue.
 *
 * \param cdRomDrive Pointeur sur le lecteur de CD-ROM.
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 */

CDialogCDRomDriveInfos::CDialogCDRomDriveInfos(CCDRomDrive * cdRomDrive, CMainWindow * mainWindow) :
QDialog      (mainWindow),
m_uiWidget   (new Ui::DialogCDRomDriveInfos()),
m_mainWindow (mainWindow),
m_cdRomDrive (cdRomDrive)
{
    Q_CHECK_PTR(m_mainWindow);
    Q_CHECK_PTR(cdRomDrive);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    m_uiWidget->valueDriveName->setText(cdRomDrive->getDriveName());
    m_uiWidget->valueSCSIName->setText(cdRomDrive->getSCSIName());
    m_uiWidget->valueDeviceName->setText(cdRomDrive->getDeviceName());

    m_uiWidget->valueDiscId->setText(QString::number(cdRomDrive->getDiscId(), 16));
    m_uiWidget->valueMusicBrainzId->setText(cdRomDrive->getMusicBrainzDiscId());

    // Connexions des signaux des boutons
    QPushButton * btnOK = m_uiWidget->buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Destructeur de la boite de dialogue.
 */

CDialogCDRomDriveInfos::~CDialogCDRomDriveInfos()
{
    delete m_uiWidget;
}

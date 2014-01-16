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

#include "CDialogAbout.hpp"
#include "../CMainWindow.hpp"
#include "../CMediaManager.hpp"
#include <QPushButton>


/**
 * Constructeur de la boite de dialogue.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 */

CDialogAbout::CDialogAbout(CMainWindow * mainWindow) :
QDialog      (mainWindow),
m_uiWidget   (new Ui::DialogAbout()),
m_mainWindow (mainWindow)
{
    Q_CHECK_PTR(m_mainWindow);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    // Versions
    const QString appVersion    = CMediaManager::getAppVersion();
    const QString appDate       = CMediaManager::getAppDate();
    const QString FMODVersion   = "FMOD Ex 4.44.03";
    const QString FMODCopyright = "FMOD Ex SoundSystem Copyright © 2005-2012 Firelight Technologies Pty, Ltd.";
    const QString musicBrainz   = tr("MusicBrainzId computation:\n%1\n%2").arg("Copyright (C) 2000 Robert Kaye")
                                                                          .arg("Copyright (C) 2007 Lukas Lalinsky");
    const QString tagLibVersion = "TagLib 1.7.2 (license LGPL 2.1)";

    m_uiWidget->textVersion->setText(QString("TMediaPlayer %1").arg(appVersion));

    m_uiWidget->textInfos->setText(QString("TMediaPlayer %1 (%2)\n\n%3\n%4\n\n%5\n\n%6\n")
                                    .arg(appVersion)
                                    .arg(appDate)
                                    .arg(FMODVersion)
                                    .arg(FMODCopyright)
                                    .arg(tagLibVersion)
                                    .arg(musicBrainz));

    // Licence
    m_uiWidget->textLicense->setPlainText("Copyright (C) 2012-2014 Teddy Michel\n"
                                          "\n"
                                          "TMediaPlayer is free software: you can redistribute it and/or modify\n"
                                          "it under the terms of the GNU General Public License as published by\n"
                                          "the Free Software Foundation, either version 3 of the License, or\n"
                                          "(at your option) any later version.\n"
                                          "\n"
                                          "TMediaPlayer is distributed in the hope that it will be useful,\n"
                                          "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                                          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
                                          "GNU General Public License for more details.\n"
                                          "\n"
                                          "You should have received a copy of the GNU General Public License\n"
                                          "along with TMediaPlayer. If not, see <http://www.gnu.org/licenses/>.");

    // Connexions des signaux des boutons
    QPushButton * btnOK = m_uiWidget->buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Destructeur de la boite de dialogue.
 */

CDialogAbout::~CDialogAbout()
{
    delete m_uiWidget;
}

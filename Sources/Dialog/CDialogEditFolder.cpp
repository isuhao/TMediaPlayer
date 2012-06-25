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

#include "CDialogEditFolder.hpp"
#include "CApplication.hpp"
#include "CFolder.hpp"
#include <QMessageBox>
#include <QPushButton>


/**
 * Constructeur de la boite de dialogue.
 *
 * \param folder       Pointeur sur le dossier à modifier, ou NULL pour créer un dossier.
 * \param application  Pointeur sur l'application.
 * \param folderParent Pointeur sur le dossier parent.
 */

CDialogEditFolder::CDialogEditFolder(CFolder * folder, CApplication * application, CFolder * folderParent) :
    QDialog        (application),
    m_uiWidget     (new Ui::DialogEditFolder()),
    m_folder       (folder),
    m_application  (application),
    m_folderParent (folderParent)
{
    Q_CHECK_PTR(application);
    Q_CHECK_PTR(folderParent);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    if (!m_folder)
    {
        m_folder = new CFolder(m_application);
    }

    m_uiWidget->editName->setText(m_folder->getName());


    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Détruit le widget.
 */

CDialogEditFolder::~CDialogEditFolder()
{
    delete m_uiWidget;
}


void CDialogEditFolder::save(void)
{
    QString name = m_uiWidget->editName->text();

    if (name.isEmpty())
    {
        QMessageBox::warning(this, QString(), tr("You need to choose a name for the folder."));
        return;
    }

    m_folder->setName(name);
    m_folder->setFolder(m_folderParent);

    if (!m_folder->updateDatabase())
    {
        return;
    }

    m_application->addFolder(m_folder);

    close();
}

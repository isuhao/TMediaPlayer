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

#include "CDialogEditXiphComment.hpp"
#include "../CMainWindow.hpp"

#include <QPushButton>
#include <QMessageBox>


/**
 * Construit la boite de dialogue.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 */

CDialogEditXiphComment::CDialogEditXiphComment(CMainWindow * mainWindow, const QString& tag, const QString& value) :
QDialog      (mainWindow),
m_uiWidget   (new Ui::DialogEditXiphComment()),
m_mainWindow (mainWindow)
{
    Q_CHECK_PTR(m_mainWindow);

    m_uiWidget->setupUi(this);

    QPushButton * btnOK = m_uiWidget->buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    connect(btnOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    // Tags prédéfinis
    QStringList tags;
    
    tags.append(tag);
    tags.append("TITLE");
    tags.append("SUBTITLE");
    tags.append("GROUPING");
    tags.append("ARTIST");
    tags.append("ALBUM");
    tags.append("ALBUMARTIST");
    tags.append("COMPOSER");
    tags.append("TITLESORT");
    tags.append("ARTISTSORT");
    tags.append("ALBUMSORT");
    tags.append("ALBUMARTISTSORT");
    tags.append("COMPOSERSORT");
    tags.append("DATE");
    tags.append("TRACKNUMBER");
    tags.append("TRACKTOTAL");
    tags.append("DISCNUMBER");
    tags.append("DISCTOTAL");
    tags.append("GENRE");
    tags.append("COMMENT");
    tags.append("TEMPO");
    tags.append("LYRICS");
    tags.append("LANGUAGE");
    tags.append("LYRICIST");

    tags.removeDuplicates();
    tags.sort();
    m_uiWidget->editTag->addItems(tags);
    
    m_uiWidget->editTag->setCurrentIndex(m_uiWidget->editTag->findText(tag));
    m_uiWidget->editValue->setPlainText(value);
}


/**
 * Détruit la boite de dialogue.
 */

CDialogEditXiphComment::~CDialogEditXiphComment()
{
    delete m_uiWidget;
}


void CDialogEditXiphComment::accept()
{
    if (getTag().isEmpty())
    {
        QMessageBox dialog(QMessageBox::Warning, QString(), tr("You must specify a name for the tag."), QMessageBox::NoButton, this);
        dialog.addButton(tr("OK"), QMessageBox::AcceptRole);
        dialog.exec();
        return;
    }

    if (getValue().isEmpty())
    {
        QMessageBox dialog(QMessageBox::Warning, QString(), tr("You must specify a value for the tag."), QMessageBox::NoButton, this);
        dialog.addButton(tr("OK"), QMessageBox::AcceptRole);
        dialog.exec();
        return;
    }

    QDialog::accept();
}


QString CDialogEditXiphComment::getTag() const
{
    return m_uiWidget->editTag->currentText().trimmed();
}


QString CDialogEditXiphComment::getValue() const
{
    return m_uiWidget->editValue->toPlainText().trimmed();
}

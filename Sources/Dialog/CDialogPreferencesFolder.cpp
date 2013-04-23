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

#include "CDialogPreferencesFolder.hpp"
#include "../CMainWindow.hpp"
#include "../CLibraryFolder.hpp"

#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>


/**
 * Construit la boite de dialogue des préférences.
 *
 * \param application Pointeur sur la classe principale de l'application.
 * \param parent      Pointeur sur le widget parent.
 * \param folder      Pointeur sur le dossier à modifier.
 */

CDialogPreferencesFolder::CDialogPreferencesFolder(CMainWindow * application, QWidget * parent, CLibraryFolder * folder) :
QDialog            (parent),
m_uiWidget         (new Ui::DialogPreferencesFolder()),
m_application      (application),
m_folder           (folder),
m_needDeleteFolder (false)
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);
    
    if (!m_folder)
    {
        m_folder = new CLibraryFolder(m_application);
        m_needDeleteFolder = true;

        connect(m_uiWidget->btnChooseFolder, SIGNAL(clicked()), this, SLOT(chooseFolder()));
    }
    else
    {
        m_uiWidget->editFolder->setReadOnly(true);
        m_uiWidget->btnChooseFolder->setEnabled(false);
    }

    m_uiWidget->editFolder->setText(m_folder->pathName);

    m_uiWidget->keepOrganized->setChecked(m_folder->keepOrganized);
    m_uiWidget->editOrgFormat->setText(m_folder->format);

    m_uiWidget->editOrgTitleDefault->setText(m_folder->titleDefault);
    m_uiWidget->editOrgArtistDefault->setText(m_folder->artistDefault);
    m_uiWidget->editOrgAlbumDefault->setText(m_folder->albumDefault);
    m_uiWidget->editOrgYearDefault->setText(m_folder->yearDefault);
    m_uiWidget->editOrgTrackDefault->setText(m_folder->trackDefault);
    m_uiWidget->editOrgDiscDefault->setText(m_folder->discDefault);
    m_uiWidget->editOrgGenreDefault->setText(m_folder->genreDefault);

    m_uiWidget->editOrgTitleEmpty->setText(m_folder->titleEmpty);
    m_uiWidget->editOrgArtistEmpty->setText(m_folder->artistEmpty);
    m_uiWidget->editOrgAlbumEmpty->setText(m_folder->albumEmpty);
    m_uiWidget->editOrgYearEmpty->setText(m_folder->yearEmpty);
    m_uiWidget->editOrgTrackEmpty->setText(m_folder->trackEmpty);
    m_uiWidget->editOrgDiscEmpty->setText(m_folder->discEmpty);
    m_uiWidget->editOrgGenreEmpty->setText(m_folder->genreEmpty);

    m_uiWidget->editOrgCompilation->setText(m_folder->compilationName);

    // Connexions des signaux des boutons
    QPushButton * btnOK = m_uiWidget->buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::DestructiveRole);

    connect(btnOK, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Détruit la boite de dialogue.
 */

CDialogPreferencesFolder::~CDialogPreferencesFolder()
{
    if (m_needDeleteFolder)
        delete m_folder;

    delete m_uiWidget;
}


void CDialogPreferencesFolder::chooseFolder()
{
    QString pathName = QFileDialog::getExistingDirectory(this);

    if (pathName.isEmpty())
        return;

    pathName.replace('\\', '/');

    m_uiWidget->editFolder->setText(pathName);
}


/**
 * Enregistre les modifications.
 */

void CDialogPreferencesFolder::save()
{
    const QString pathName = m_uiWidget->editFolder->text().trimmed();

    if (pathName.isEmpty())
    {
        QMessageBox::warning(this, QString(), tr("You need to select a valid folder."));
        return;
    }

    CLibraryFolder * folder = m_application->getLibraryFolder(m_application->getLibraryFolderId(pathName));

    if (folder != m_folder)
    {
        QMessageBox::warning(this, QString(), tr("This folder is already in the library."));
        return;
    }

    m_folder->pathName = m_uiWidget->editFolder->text();

    m_folder->keepOrganized = m_uiWidget->keepOrganized->isChecked();
    m_folder->format = m_uiWidget->editOrgFormat->text();

    m_folder->titleDefault    = m_uiWidget->editOrgTitleDefault ->text();
    m_folder->artistDefault   = m_uiWidget->editOrgArtistDefault->text();
    m_folder->albumDefault    = m_uiWidget->editOrgAlbumDefault ->text();
    m_folder->yearDefault     = m_uiWidget->editOrgYearDefault  ->text();
    m_folder->trackDefault    = m_uiWidget->editOrgTrackDefault ->text();
    m_folder->discDefault     = m_uiWidget->editOrgDiscDefault  ->text();
    m_folder->genreDefault    = m_uiWidget->editOrgGenreDefault ->text();

    m_folder->titleEmpty      = m_uiWidget->editOrgTitleEmpty   ->text();
    m_folder->artistEmpty     = m_uiWidget->editOrgArtistEmpty  ->text();
    m_folder->albumEmpty      = m_uiWidget->editOrgAlbumEmpty   ->text();
    m_folder->yearEmpty       = m_uiWidget->editOrgYearEmpty    ->text();
    m_folder->trackEmpty      = m_uiWidget->editOrgTrackEmpty   ->text();
    m_folder->discEmpty       = m_uiWidget->editOrgDiscEmpty    ->text();
    m_folder->genreEmpty      = m_uiWidget->editOrgGenreEmpty   ->text();

    m_folder->compilationName = m_uiWidget->editOrgCompilation  ->text();

    // Mise à jour de la base de données
    QSqlQuery query(m_application->getDataBase());
    const QString str = m_folder->convertFormatItemsToString();

    // Ajout du dossier
    if (m_folder->id < 0)
    {
        query.prepare("INSERT INTO libpath ("
                        "path_location,"
                        "path_keep_organized,"
                        "path_format,"
                        "path_format_items"
                      ") VALUES (?, ?, ?, ?)");

        query.bindValue(0, m_folder->pathName);
        query.bindValue(1, (m_folder->keepOrganized ? 1 : 0));
        query.bindValue(2, m_folder->format);
        query.bindValue(3, m_folder->convertFormatItemsToString());

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return;
        }

        if (m_application->getDataBase().driverName() == "QPSQL")
        {
            query.prepare("SELECT currval('libpath_seq')");

            if (!query.exec())
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return;
            }

            if (query.next())
            {
                m_folder->id = query.value(0).toInt();
            }
            else
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return;
            }
        }
        else
        {
            m_folder->id = query.lastInsertId().toInt();
        }

        m_application->addLibraryFolder(m_folder);
        m_needDeleteFolder = false;
    }
    // Modification du dossier
    else
    {
        query.prepare("UPDATE libpath SET "
                        "path_location       = ?,"
                        "path_keep_organized = ?,"
                        "path_format         = ?,"
                        "path_format_items   = ? "
                      "WHERE path_id = ?");

        query.bindValue(0, m_folder->pathName);
        query.bindValue(1, (m_folder->keepOrganized ? 1 : 0));
        query.bindValue(2, m_folder->format);
        query.bindValue(3, m_folder->convertFormatItemsToString());
        query.bindValue(4, m_folder->id);

        if (!query.exec())
        {
            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return;
        }
    }

    close();
}

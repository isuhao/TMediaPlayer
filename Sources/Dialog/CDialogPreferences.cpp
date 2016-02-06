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

#include "CDialogPreferences.hpp"
#include "CDialogPreferencesFolder.hpp"
#include "../CMainWindow.hpp"
#include "../CMediaManager.hpp"
#include "../CLibraryFolder.hpp"
#include "../Language.hpp"

#include <QSettings>
#include <QDesktopServices>
#include <QDir>


/**
 * Construit la boite de dialogue des préférences.
 *
 * \todo Gérer toutes les préferences.
 */

CDialogPreferences::CDialogPreferences(CMainWindow * mainWindow, QSettings * settings) :
QDialog      (mainWindow),
m_uiWidget   (new Ui::DialogPreferences()),
m_mainWindow (mainWindow),
m_settings   (settings)
{
    Q_CHECK_PTR(m_mainWindow);
    Q_CHECK_PTR(settings);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    // Récupération des paramètres
    m_uiWidget->editShowButtonStop->setChecked(m_settings->value("Preferences/ShowButtonStop", true).toBool());
    m_uiWidget->editEditSongAutoSave->setChecked(m_settings->value("Preferences/EditSongAutoSave", false).toBool());
    m_uiWidget->editShowRemainingTime->setChecked(m_settings->value("Preferences/ShowRemainingTime", false).toBool());
    m_uiWidget->editRowHeight->setValue(m_settings->value("Preferences/RowHeight", 19).toInt());

    // Langues
    QDir langDirectory("Lang/");
    QStringList langFiles = langDirectory.entryList(QStringList() << "TMediaPlayer_??.qm");

    m_uiWidget->editLanguage->addItem(tr("System language"));

    for (QStringList::ConstIterator langFile = langFiles.begin(); langFile != langFiles.end(); ++langFile)
    {
        const QString langISO2 = langFile->mid(13, 2);
        m_uiWidget->editLanguage->addItem(getLanguageName(getLanguageForISO2Code(langISO2)), langISO2);
    }

    QString currentLanguage = m_settings->value("Preferences/Language", QString()).toString();

    int langIndex = 0;

    if (!currentLanguage.isEmpty())
        langIndex = m_uiWidget->editLanguage->findData(currentLanguage);

    if (langIndex < 0)
        langIndex = 0;

    m_uiWidget->editLanguage->setCurrentIndex(langIndex);

    // Base de données
    QString driver = m_settings->value("Database/Type", QString("QSQLITE")).toString();
    connect(m_uiWidget->editDBDriver, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onDriverChange(const QString&)));

    if (driver == "QSQLITE2")
    {
        m_uiWidget->editDBDriver->setCurrentIndex(m_uiWidget->editDBDriver->findText(tr("SQLite version 2")));
        onDriverChange(tr("SQLite version 2"));
    }
    else if (driver == "QPSQL")
    {
        m_uiWidget->editDBDriver->setCurrentIndex(m_uiWidget->editDBDriver->findText(tr("PostgreSQL")));
        onDriverChange(tr("PostgreSQL"));
    }
    else if (driver == "QMYSQL")
    {
        m_uiWidget->editDBDriver->setCurrentIndex(m_uiWidget->editDBDriver->findText(tr("MySQL")));
        onDriverChange(tr("MySQL"));
    }
    else if (driver == "QODBC")
    {
        m_uiWidget->editDBDriver->setCurrentIndex(m_uiWidget->editDBDriver->findText(tr("ODBC")));
        onDriverChange(tr("ODBC"));
    }
    else
    {
        m_uiWidget->editDBDriver->setCurrentIndex(m_uiWidget->editDBDriver->findText(tr("SQLite version 3 or above")));
        onDriverChange(tr("SQLite version 3 or above"));
    }

    m_uiWidget->editDBHost->setText(m_settings->value("Database/Host", QString("localhost")).toString());
    m_uiWidget->editDBPort->setValue(m_settings->value("Database/Port", 0).toInt());
    m_uiWidget->editDBUserName->setText(m_settings->value("Database/UserName", QString("root")).toString());
    m_uiWidget->editDBPassword->setText(m_settings->value("Database/Password", QString("")).toString());
    m_uiWidget->editDBDatabase->setText(m_settings->value("Database/Base", m_mainWindow->getMediaManager()->getApplicationPath() + "library.sqlite").toString());
    m_uiWidget->groupUseLastFm->setChecked(m_settings->value("LastFm/EnableScrobble", false).toBool());
    m_uiWidget->editLastFmDelayBeforeNotification->setValue(m_settings->value("LastFm/DelayBeforeNotification", 5000).toInt() / 1000);
    m_uiWidget->editLastFmPercentageBeforeScrobbling->setValue(m_settings->value("LastFm/PercentageBeforeScrobbling", 60).toInt());

    // Liste des répertoires surveillés
    QList<CLibraryFolder *> folders = m_mainWindow->getMediaManager()->getLibraryFolders();

    for (QList<CLibraryFolder *>::ConstIterator it = folders.begin(); it != folders.end(); ++it)
    {
        QListWidgetItem * item = new QListWidgetItem((*it)->pathName);
        item->setData(Qt::UserRole, (*it)->id);
        m_uiWidget->listFolders->addItem(item);
    }

    connect(m_uiWidget->btnAddFolder, SIGNAL(clicked()), this, SLOT(addFolder()));
    connect(m_uiWidget->btnEditFolder, SIGNAL(clicked()), this, SLOT(editSelectedFolder()));
    connect(m_uiWidget->btnRemoveFolder, SIGNAL(clicked()), this, SLOT(removeSelectedFolder()));

    // Connexions des signaux des boutons
    QPushButton * btnOK = m_uiWidget->buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    connect(btnOK, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    connect(m_uiWidget->btnLastFm, SIGNAL(clicked()), m_mainWindow, SLOT(connectToLastFm()));
}


/**
 * Détruit la boite de dialogue.
 */

CDialogPreferences::~CDialogPreferences()
{
    delete m_uiWidget;
}


/**
 * Enregistre les préférences et ferme la boite de dialogue.
 *
 * \todo Gérer toutes les préferences.
 */

void CDialogPreferences::save()
{
    m_mainWindow->setRowHeight(m_uiWidget->editRowHeight->value());
    m_mainWindow->showButtonStop(m_uiWidget->editShowButtonStop->isChecked());
    m_settings->setValue("Preferences/EditSongAutoSave", m_uiWidget->editEditSongAutoSave->isChecked());
    m_mainWindow->showRemainingTime(m_uiWidget->editShowRemainingTime->isChecked());

    m_settings->setValue("Preferences/Language", m_uiWidget->editLanguage->itemData(m_uiWidget->editLanguage->currentIndex()));

    // Database
    QString driver = m_uiWidget->editDBDriver->currentText();

    if (driver == tr("SQLite version 2"))
        m_settings->setValue("Database/Type", "QSQLITE2");
    else if (driver == tr("PostgreSQL"))
        m_settings->setValue("Database/Type", "QPSQL");
    else if (driver == tr("MySQL"))
        m_settings->setValue("Database/Type", "QMYSQL");
    else if (driver == tr("ODBC"))
        m_settings->setValue("Database/Type", "QODBC");
    else
        m_settings->setValue("Database/Type", "QSQLITE");

    m_settings->setValue("Database/Host", m_uiWidget->editDBHost->text());
    m_settings->setValue("Database/Port", m_uiWidget->editDBPort->value());
    m_settings->setValue("Database/Base", m_uiWidget->editDBDatabase->text());
    m_settings->setValue("Database/UserName", m_uiWidget->editDBUserName->text());
    m_settings->setValue("Database/Password", m_uiWidget->editDBPassword->text());

    // Last.fm
    m_mainWindow->enableScrobbling(m_uiWidget->groupUseLastFm->isChecked());
    m_mainWindow->setDelayBeforeNotification(m_uiWidget->editLastFmDelayBeforeNotification->value() * 1000);
    m_mainWindow->setPercentageBeforeScrobbling(m_uiWidget->editLastFmPercentageBeforeScrobbling->value());

    close();
}


/**
 * Méthode appellée lorsque le driver de base de données est modifié.
 *
 * \param name Nom du nouveau driver.
 */

void CDialogPreferences::onDriverChange(const QString& name)
{
    if (name == tr("SQLite version 2") || name == tr("SQLite version 3 or above"))
    {
        m_uiWidget->editDBHost->setEnabled(false);
        m_uiWidget->editDBPort->setEnabled(false);
        m_uiWidget->editDBUserName->setEnabled(false);
        m_uiWidget->editDBPassword->setEnabled(false);
    }
    else
    {
        m_uiWidget->editDBHost->setEnabled(true);
        m_uiWidget->editDBPort->setEnabled(true);
        m_uiWidget->editDBUserName->setEnabled(true);
        m_uiWidget->editDBPassword->setEnabled(true);
    }
}


/**
 * Ajoute un répertoire à la liste des répertoires surveillés.
 */

void CDialogPreferences::addFolder()
{
    CDialogPreferencesFolder * dialog = new CDialogPreferencesFolder(m_mainWindow, this, nullptr);
    dialog->show();
}


void CDialogPreferences::editSelectedFolder()
{
    QListWidgetItem * item = m_uiWidget->listFolders->currentItem();

    if (!item)
        return;

    const int folderId = item->data(Qt::UserRole).toInt();
    CLibraryFolder * folder = m_mainWindow->getMediaManager()->getLibraryFolder(folderId);

    if (!folder)
        return;

    CDialogPreferencesFolder * dialog = new CDialogPreferencesFolder(m_mainWindow, this, folder);
    dialog->show();
}


/**
 * Supprime le répertoire sélectionné de la liste des répertoires surveillés.
 */

void CDialogPreferences::removeSelectedFolder()
{
    QListWidgetItem * item = m_uiWidget->listFolders->currentItem();

    if (!item)
        return;

    const int folderId = item->data(Qt::UserRole).toInt();

    // Suppression du répertoire
    m_mainWindow->getMediaManager()->removeLibraryFolder(m_mainWindow->getMediaManager()->getLibraryFolder(folderId));

    // Modification de la vue
    m_uiWidget->listFolders->removeItemWidget(item);
}

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

#include "CDialogPreferences.hpp"
#include "CApplication.hpp"
#include <QSettings>


/**
 * Construit la boite de dialogue des préférences.
 *
 * \todo Gérer toutes les préferences.
 */

CDialogPreferences::CDialogPreferences(CApplication * application, QSettings * settings) :
    QDialog       (application),
    m_uiWidget    (new Ui::DialogPreferences()),
    m_application (application),
    m_settings    (settings)
{
    Q_CHECK_PTR(application);
    Q_CHECK_PTR(settings);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    // Récupération des paramètres
    m_uiWidget->editRowHeight->setValue(m_settings->value("Preferences/RowHeight", 19).toInt());
    m_uiWidget->editShowButtonStop->setChecked(m_settings->value("Preferences/ShowButtonStop", true).toBool());

    m_uiWidget->groupUseLastFm->setChecked(m_settings->value("LastFm/EnableScrobble", false).toBool());
    m_uiWidget->editLastFmDelayBeforeNotification->setValue(m_settings->value("LastFm/DelayBeforeNotification", 5000).toInt() / 1000);
    m_uiWidget->editLastFmPercentageBeforeScrobbling->setValue(m_settings->value("LastFm/PercentageBeforeScrobbling", 60).toInt());

    // Connexions des signaux des boutons
    QPushButton * btnOK = m_uiWidget->buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    connect(btnOK, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    connect(m_uiWidget->btnLastFm, SIGNAL(clicked()), m_application, SLOT(connectToLastFm()));
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

void CDialogPreferences::save(void)
{
    m_application->setRowHeight(m_uiWidget->editRowHeight->value());
    m_application->showButtonStop(m_uiWidget->editShowButtonStop->isChecked());

    // Last.fm
    m_application->enableScrobbling(m_uiWidget->groupUseLastFm->isChecked());
    m_application->setDelayBeforeNotification(m_uiWidget->editLastFmDelayBeforeNotification->value() * 1000);
    m_application->setPercentageBeforeScrobbling(m_uiWidget->editLastFmPercentageBeforeScrobbling->value());

    close();
}

/*
Copyright (C) 2012-2015 Teddy Michel

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

#include "CDialogEffects.hpp"
#include "../CMainWindow.hpp"
#include "../CMediaManager.hpp"

#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>


/**
 * Constructeur de la boite de dialogue.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 */

CDialogEffects::CDialogEffects(CMainWindow * mainWindow) :
QDialog      (mainWindow),
m_uiWidget   (new Ui::DialogEffects()),
m_mainWindow (mainWindow)
{
    Q_CHECK_PTR(m_mainWindow);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    // Récupération des paramètres actuels
    int echoDelay = m_mainWindow->getMediaManager()->getEchoDelay();

    if (echoDelay > 0)
    {
        m_uiWidget->groupEcho->setChecked(true);
        m_uiWidget->editEchoDelay->setValue(echoDelay);
    }

    int minFreq = m_mainWindow->getMediaManager()->getMinFilter();
    int maxFreq = m_mainWindow->getMediaManager()->getMaxFilter();

    if (minFreq > 0 || maxFreq > 0)
    {
        m_uiWidget->groupFilter->setChecked(true);
        m_uiWidget->editMinFreq->setValue(minFreq);
        m_uiWidget->editMaxFreq->setValue(maxFreq);
    }

    connect(m_uiWidget->groupEcho, SIGNAL(clicked(bool)), this, SLOT(enableEcho(bool)));
    connect(m_uiWidget->groupFilter, SIGNAL(clicked(bool)), this, SLOT(enableFilter(bool)));

    connect(m_uiWidget->editEchoDelay, SIGNAL(valueChanged(int)), this, SLOT(echoDelayChanged(int)));
    connect(m_uiWidget->editMinFreq, SIGNAL(valueChanged(int)), this, SLOT(minFreqChanged(int)));
    connect(m_uiWidget->editMaxFreq, SIGNAL(valueChanged(int)), this, SLOT(maxFreqChanged(int)));

    // Connexions des signaux des boutons
    QPushButton * btnOK = m_uiWidget->buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Destructeur de la boite de dialogue.
 */

CDialogEffects::~CDialogEffects()
{
    delete m_uiWidget;
}


void CDialogEffects::enableEcho(bool enable)
{
    if (enable)
    {
        m_mainWindow->getMediaManager()->setEchoDelay(m_uiWidget->editEchoDelay->value());
    }
    else
    {
        m_mainWindow->getMediaManager()->setEchoDelay(0);
    }
}


void CDialogEffects::echoDelayChanged(int delay)
{
    m_mainWindow->getMediaManager()->setEchoDelay(delay);
}


void CDialogEffects::enableFilter(bool enable)
{
    if (enable)
    {
        m_mainWindow->getMediaManager()->setMinFilter(m_uiWidget->editMinFreq->value());
        m_mainWindow->getMediaManager()->setMaxFilter(m_uiWidget->editMaxFreq->value());
    }
    else
    {
        m_mainWindow->getMediaManager()->setMinFilter(0);
        m_mainWindow->getMediaManager()->setMaxFilter(0);
    }
}


void CDialogEffects::minFreqChanged(int frequency)
{
    m_mainWindow->getMediaManager()->setMinFilter(frequency);
}


void CDialogEffects::maxFreqChanged(int frequency)
{
    m_mainWindow->getMediaManager()->setMaxFilter(frequency);
}

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

#include "CDialogEqualizer.hpp"
#include "../CApplication.hpp"
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>


/**
 * Constructeur de la boite de dialogue.
 *
 * \param application Pointeur sur la classe principale de l'application.
 */

CDialogEqualizer::CDialogEqualizer(CApplication * application) :
    QDialog       (application),
    m_uiWidget    (new Ui::DialogEqualizer()),
    m_application (application)
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    m_uiWidget->enableEqualizer->setChecked(m_application->isEqualizerEnabled());
    connect(m_uiWidget->enableEqualizer, SIGNAL(toggled(bool)), m_application, SLOT(setEqualizerEnabled(bool)));

    m_uiWidget->slider0->setValue(convertGainToSliderValue(m_application->getEqualizerGain(CApplication::EqFreq32 )));
    m_uiWidget->slider1->setValue(convertGainToSliderValue(m_application->getEqualizerGain(CApplication::EqFreq64 )));
    m_uiWidget->slider2->setValue(convertGainToSliderValue(m_application->getEqualizerGain(CApplication::EqFreq125)));
    m_uiWidget->slider3->setValue(convertGainToSliderValue(m_application->getEqualizerGain(CApplication::EqFreq250)));
    m_uiWidget->slider4->setValue(convertGainToSliderValue(m_application->getEqualizerGain(CApplication::EqFreq500)));
    m_uiWidget->slider5->setValue(convertGainToSliderValue(m_application->getEqualizerGain(CApplication::EqFreq1K )));
    m_uiWidget->slider6->setValue(convertGainToSliderValue(m_application->getEqualizerGain(CApplication::EqFreq2K )));
    m_uiWidget->slider7->setValue(convertGainToSliderValue(m_application->getEqualizerGain(CApplication::EqFreq4K )));
    m_uiWidget->slider8->setValue(convertGainToSliderValue(m_application->getEqualizerGain(CApplication::EqFreq8K )));
    m_uiWidget->slider9->setValue(convertGainToSliderValue(m_application->getEqualizerGain(CApplication::EqFreq16K)));

    connect(m_uiWidget->slider0, SIGNAL(valueChanged(int)), this, SLOT(onSlider0Change(int)));
    connect(m_uiWidget->slider1, SIGNAL(valueChanged(int)), this, SLOT(onSlider1Change(int)));
    connect(m_uiWidget->slider2, SIGNAL(valueChanged(int)), this, SLOT(onSlider2Change(int)));
    connect(m_uiWidget->slider3, SIGNAL(valueChanged(int)), this, SLOT(onSlider3Change(int)));
    connect(m_uiWidget->slider4, SIGNAL(valueChanged(int)), this, SLOT(onSlider4Change(int)));
    connect(m_uiWidget->slider5, SIGNAL(valueChanged(int)), this, SLOT(onSlider5Change(int)));
    connect(m_uiWidget->slider6, SIGNAL(valueChanged(int)), this, SLOT(onSlider6Change(int)));
    connect(m_uiWidget->slider7, SIGNAL(valueChanged(int)), this, SLOT(onSlider7Change(int)));
    connect(m_uiWidget->slider8, SIGNAL(valueChanged(int)), this, SLOT(onSlider8Change(int)));
    connect(m_uiWidget->slider9, SIGNAL(valueChanged(int)), this, SLOT(onSlider9Change(int)));

    // Connexions des signaux des boutons
    QPushButton * btnOK = m_uiWidget->buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    QPushButton * btnReset = m_uiWidget->buttonBox->addButton(tr("Reset"), QDialogButtonBox::ResetRole);

    connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnReset, SIGNAL(clicked()), this, SLOT(reset()));

    // Liste des préréglages
    m_uiWidget->listPreset->insertItem(0, tr("New preset..."));
    m_uiWidget->listPreset->insertItem(1, tr("Manual"));
    m_uiWidget->listPreset->insertSeparator(2);

    resetPresetList();

    connect(m_uiWidget->listPreset, SIGNAL(currentIndexChanged(int)), this, SLOT(onListPresetChange(int)));
    connect(m_uiWidget->btnRenamePreset, SIGNAL(clicked()), this, SLOT(renameCurrentPreset()));
    connect(m_uiWidget->btnDeletePreset, SIGNAL(clicked()), this, SLOT(deleteCurrentPreset()));
}


/**
 * Destructeur de la boite de dialogue.
 */

CDialogEqualizer::~CDialogEqualizer()
{
    delete m_uiWidget;
}


/**
 * Réinitialise les valeurs de l'égaliseur.
 */

void CDialogEqualizer::reset()
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->resetEqualizer();

    m_uiWidget->slider0->setValue(0);
    m_uiWidget->slider1->setValue(0);
    m_uiWidget->slider2->setValue(0);
    m_uiWidget->slider3->setValue(0);
    m_uiWidget->slider4->setValue(0);
    m_uiWidget->slider5->setValue(0);
    m_uiWidget->slider6->setValue(0);
    m_uiWidget->slider7->setValue(0);
    m_uiWidget->slider8->setValue(0);
    m_uiWidget->slider9->setValue(0);
}


/**
 * Convertit un gain en une valeur utilisable par un slider.
 *
 * \param gain Valeur du gain (entre 0.05 et 3.00).
 * \return Valeur utilisable par un slider (entre -300 et 300).
 */

int CDialogEqualizer::convertGainToSliderValue(double gain) const
{
    gain = qBound(1/3.0, gain, 3.0);

    if (gain > 1.0f)
    {
        return (gain - 1.0) * 150;
    }
    else if (gain < 1.0f)
    {
        return (gain - 1.0) * 450;
    }
    else
    {
        return 0;
    }
}


double CDialogEqualizer::convertSliderValueToGain(int value) const
{
    if (value > 0)
    {
        return (static_cast<double>(value) / 150.0) + 1.0;
    }
    else if (value < 0)
    {
        return (static_cast<double>(value) / 450.0) + 1.0;
    }
    else
    {
        return 1.0;
    }
}


/**
 * Réinitialise la liste des préréglages.
 */

void CDialogEqualizer::resetPresetList()
{
    m_uiWidget->listPreset->setCurrentIndex(1);

    // Suppression des préréglages
    while (m_uiWidget->listPreset->count() > 3)
    {
        m_uiWidget->listPreset->removeItem(3);
    }

    // Remplissage de la liste des préréglages
    CApplication::TEqualizerPreset currentEqualizer = m_application->getCurrentEqualizerPreset();
    QList<CApplication::TEqualizerPreset> equalizers = m_application->getEqualizerPresetList();

    int index = 3;

    for (QList<CApplication::TEqualizerPreset>::const_iterator it = equalizers.begin(); it != equalizers.end(); ++it, ++index)
    {
        m_uiWidget->listPreset->addItem(it->name, it->id);

        if (it->id == currentEqualizer.id)
        {
            m_uiWidget->listPreset->setCurrentIndex(index);
        }
    }
}


void CDialogEqualizer::onSlider0Change(int value)
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->setEqualizerGain(CApplication::EqFreq32, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider1Change(int value)
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->setEqualizerGain(CApplication::EqFreq64, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider2Change(int value)
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->setEqualizerGain(CApplication::EqFreq125, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider3Change(int value)
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->setEqualizerGain(CApplication::EqFreq250, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider4Change(int value)
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->setEqualizerGain(CApplication::EqFreq500, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider5Change(int value)
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->setEqualizerGain(CApplication::EqFreq1K, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider6Change(int value)
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->setEqualizerGain(CApplication::EqFreq2K, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider7Change(int value)
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->setEqualizerGain(CApplication::EqFreq4K, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider8Change(int value)
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->setEqualizerGain(CApplication::EqFreq8K, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider9Change(int value)
{
    m_uiWidget->listPreset->setCurrentIndex(1);
    m_application->setEqualizerGain(CApplication::EqFreq16K, convertSliderValueToGain(value));
}


void CDialogEqualizer::onListPresetChange(int index)
{
    // Nouveau préréglage
    if (index == 0)
    {
        m_uiWidget->listPreset->setCurrentIndex(1);
        QInputDialog * dialog = new QInputDialog(this);

        dialog->setInputMode(QInputDialog::TextInput);
        dialog->setOkButtonText(tr("OK"));
        dialog->setCancelButtonText(tr("Cancel"));
        dialog->setLabelText(tr("Name of the preset:"));
        connect(dialog, SIGNAL(textValueSelected(const QString&)), this, SLOT(onPresetSave(const QString&)));

        dialog->show();
        return;
    }
    // Mode manuel
    else if (index == 1)
    {
        m_uiWidget->btnRenamePreset->setEnabled(false);
        m_uiWidget->btnDeletePreset->setEnabled(false);

        return;
    }
    // Affichage d'un préréglage
    else if (index > 2)
    {
        m_uiWidget->btnRenamePreset->setEnabled(true);
        m_uiWidget->btnDeletePreset->setEnabled(true);

        int presetId = m_uiWidget->listPreset->itemData(index).toInt();
        CApplication::TEqualizerPreset eq = m_application->getEqualizerPresetFromId(presetId);

        m_uiWidget->slider0->setValue(convertGainToSliderValue(eq.value[0]));
        m_uiWidget->slider1->setValue(convertGainToSliderValue(eq.value[1]));
        m_uiWidget->slider2->setValue(convertGainToSliderValue(eq.value[2]));
        m_uiWidget->slider3->setValue(convertGainToSliderValue(eq.value[3]));
        m_uiWidget->slider4->setValue(convertGainToSliderValue(eq.value[4]));
        m_uiWidget->slider5->setValue(convertGainToSliderValue(eq.value[5]));
        m_uiWidget->slider6->setValue(convertGainToSliderValue(eq.value[6]));
        m_uiWidget->slider7->setValue(convertGainToSliderValue(eq.value[7]));
        m_uiWidget->slider8->setValue(convertGainToSliderValue(eq.value[8]));
        m_uiWidget->slider9->setValue(convertGainToSliderValue(eq.value[9]));

        m_application->setCurrentEqualizerPreset(eq);

        m_uiWidget->listPreset->setCurrentIndex(index);
        return;
    }
    else
    {
        m_uiWidget->listPreset->setCurrentIndex(1);
        return;
    }
}


void CDialogEqualizer::onPresetSave(const QString& name)
{
    CApplication::TEqualizerPreset eq;
    eq.name = name.trimmed();

    if (eq.name.isEmpty())
    {
        QMessageBox::warning(this, QString(), tr("You need to choose a name for the preset."));
        return;
    }

    eq.id   = m_application->getEqualizerPresetIdFromName(eq.name);

    eq.value[0] = convertSliderValueToGain(m_uiWidget->slider0->value());
    eq.value[1] = convertSliderValueToGain(m_uiWidget->slider1->value());
    eq.value[2] = convertSliderValueToGain(m_uiWidget->slider2->value());
    eq.value[3] = convertSliderValueToGain(m_uiWidget->slider3->value());
    eq.value[4] = convertSliderValueToGain(m_uiWidget->slider4->value());
    eq.value[5] = convertSliderValueToGain(m_uiWidget->slider5->value());
    eq.value[6] = convertSliderValueToGain(m_uiWidget->slider6->value());
    eq.value[7] = convertSliderValueToGain(m_uiWidget->slider7->value());
    eq.value[8] = convertSliderValueToGain(m_uiWidget->slider8->value());
    eq.value[9] = convertSliderValueToGain(m_uiWidget->slider9->value());

    // Nom déjà utilisé
    if (eq.id > 0)
    {
        QMessageBox::StandardButton res = QMessageBox::question(this, QString(), tr("The preset \"%1\" already exists. Do you want to replace it?").arg(eq.name), QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::No)
            return;
    }

    m_application->saveEqualizerPreset(eq);
    resetPresetList();
}


void CDialogEqualizer::onPresetRename(const QString& name)
{
    int presetId = m_uiWidget->listPreset->itemData(m_uiWidget->listPreset->currentIndex()).toInt();
    CApplication::TEqualizerPreset eq = m_application->getEqualizerPresetFromId(presetId);
    eq.name = name.trimmed();

    if (eq.name.isEmpty())
    {
        QMessageBox::warning(this, QString(), tr("You need to choose a name for the preset."));
        return;
    }

    int presetIdNewName = m_application->getEqualizerPresetIdFromName(eq.name);

    // Nom déjà utilisé
    if (presetIdNewName > 0 && presetIdNewName != presetId)
    {
        QMessageBox::StandardButton res = QMessageBox::question(this, QString(), tr("The preset \"%1\" already exists. Do you want to replace it?").arg(eq.name), QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::No)
            return;

        m_application->deleteEqualizerPreset(m_application->getEqualizerPresetFromId(presetIdNewName));
    }

    eq.value[0] = convertSliderValueToGain(m_uiWidget->slider0->value());
    eq.value[1] = convertSliderValueToGain(m_uiWidget->slider1->value());
    eq.value[2] = convertSliderValueToGain(m_uiWidget->slider2->value());
    eq.value[3] = convertSliderValueToGain(m_uiWidget->slider3->value());
    eq.value[4] = convertSliderValueToGain(m_uiWidget->slider4->value());
    eq.value[5] = convertSliderValueToGain(m_uiWidget->slider5->value());
    eq.value[6] = convertSliderValueToGain(m_uiWidget->slider6->value());
    eq.value[7] = convertSliderValueToGain(m_uiWidget->slider7->value());
    eq.value[8] = convertSliderValueToGain(m_uiWidget->slider8->value());
    eq.value[9] = convertSliderValueToGain(m_uiWidget->slider9->value());

    m_application->saveEqualizerPreset(eq);
    resetPresetList();
}


/**
 * Slot pour renommer un préréglage.
 * Affiche une boite de dialogue.
 */

void CDialogEqualizer::renameCurrentPreset()
{
    QInputDialog * dialog = new QInputDialog(this);

    dialog->setInputMode(QInputDialog::TextInput);
    dialog->setOkButtonText(tr("OK"));
    dialog->setCancelButtonText(tr("Cancel"));
    dialog->setLabelText(tr("Name of the preset:"));
    connect(dialog, SIGNAL(textValueSelected(const QString&)), this, SLOT(onPresetRename(const QString&)));

    dialog->show();
}


/**
 * Slot pour supprimer un préréglage.
 * Affiche une boite de dialogue de confirmation.
 */

void CDialogEqualizer::deleteCurrentPreset()
{
    if (m_uiWidget->listPreset->currentIndex() <= 2)
        return;
    
    int presetId = m_uiWidget->listPreset->itemData(m_uiWidget->listPreset->currentIndex()).toInt();
    CApplication::TEqualizerPreset eq = m_application->getEqualizerPresetFromId(presetId);

    // Boite de dialogue de confirmation
    QMessageBox::StandardButton res = QMessageBox::question(this, QString(), tr("Are you sure you want to delete the equalizer preset \"%1\"?").arg(eq.name), QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::No)
        return;

    // Suppression du préréglage
    m_application->deleteEqualizerPreset(eq);
    resetPresetList();
}

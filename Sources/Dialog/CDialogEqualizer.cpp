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
#include "CApplication.hpp"
#include <QPushButton>


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
}


CDialogEqualizer::~CDialogEqualizer()
{
    delete m_uiWidget;
}


void CDialogEqualizer::reset(void)
{
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


void CDialogEqualizer::onSlider0Change(int value)
{
    m_application->setEqualizerGain(CApplication::EqFreq32, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider1Change(int value)
{
    m_application->setEqualizerGain(CApplication::EqFreq64, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider2Change(int value)
{
    m_application->setEqualizerGain(CApplication::EqFreq125, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider3Change(int value)
{
    m_application->setEqualizerGain(CApplication::EqFreq250, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider4Change(int value)
{
    m_application->setEqualizerGain(CApplication::EqFreq500, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider5Change(int value)
{
    m_application->setEqualizerGain(CApplication::EqFreq1K, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider6Change(int value)
{
    m_application->setEqualizerGain(CApplication::EqFreq2K, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider7Change(int value)
{
    m_application->setEqualizerGain(CApplication::EqFreq4K, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider8Change(int value)
{
    m_application->setEqualizerGain(CApplication::EqFreq8K, convertSliderValueToGain(value));
}

void CDialogEqualizer::onSlider9Change(int value)
{
    m_application->setEqualizerGain(CApplication::EqFreq16K, convertSliderValueToGain(value));
}

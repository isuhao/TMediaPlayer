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

#ifndef FILE_C_DIALOG_EQUALIZER
#define FILE_C_DIALOG_EQUALIZER

#include <QDialog>
#include "ui_DialogEqualizer.h"


class CApplication;


class CDialogEqualizer : public QDialog
{
    Q_OBJECT

public:

    CDialogEqualizer(CApplication * application);
    ~CDialogEqualizer();

public slots:

    void reset(void);

protected:

    int convertGainToSliderValue(double gain) const;
    double convertSliderValueToGain(int value) const;

protected slots:

    void onSlider0Change(int value);
    void onSlider1Change(int value);
    void onSlider2Change(int value);
    void onSlider3Change(int value);
    void onSlider4Change(int value);
    void onSlider5Change(int value);
    void onSlider6Change(int value);
    void onSlider7Change(int value);
    void onSlider8Change(int value);
    void onSlider9Change(int value);

private:

    Ui::DialogEqualizer * m_uiWidget;
    CApplication * m_application;
};

#endif // FILE_C_DIALOG_EQUALIZER

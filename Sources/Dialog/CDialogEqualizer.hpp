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

#ifndef FILE_C_DIALOG_EQUALIZER
#define FILE_C_DIALOG_EQUALIZER

#include <QDialog>
#include "ui_DialogEqualizer.h"


class CMainWindow;


/**
 * Boite de dialogue de l'égaliseur.
 * Permet de modifier les valeurs de gain de l'égaliseur graphiquement,
 * de sélectionner un préréglage, et d'ajouter ou modifier un préréglage.
 */

class CDialogEqualizer : public QDialog
{
    Q_OBJECT

public:

    explicit CDialogEqualizer(CMainWindow * mainWindow);
    virtual ~CDialogEqualizer();

public slots:

    void reset();

protected:

    int convertGainToSliderValue(double gain) const;
    double convertSliderValueToGain(int value) const;
    void resetPresetList();

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

    void onListPresetChange(int index);
    void onPresetSave(const QString& name);
    void onPresetRename(const QString& name);
    void renameCurrentPreset();
    void deleteCurrentPreset();

private:

    Ui::DialogEqualizer * m_uiWidget; ///< Pointeur sur le widget de la boite de dialogue.
    CMainWindow * m_mainWindow;       ///< Pointeur sur la fenêtre principale de l'application.
};

#endif // FILE_C_DIALOG_EQUALIZER

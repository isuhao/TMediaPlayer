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

#ifndef FILE_C_SPECIAL_SPIN_BOX
#define FILE_C_SPECIAL_SPIN_BOX

#include <QSpinBox>


/**
 * Spin box permettant d'afficher un placeholder pour une valeur spéciale.
 * Les valeurs valides sont supérieures à 0. La valeur nulle est une valeur spéciale.
 */

class CSpecialSpinBox : public QSpinBox
{
    Q_OBJECT

public:

    explicit CSpecialSpinBox(QWidget * parent = nullptr);
    void setPlaceholderText(const QString& text);
    void setSpecialValue(int value);

protected:

    virtual QString textFromValue(int value) const;

protected slots:

    void onValueChange(int value);

private:

    int m_spacialValue;
    int m_savedMax;
};

#endif // FILE_C_SPECIAL_SPIN_BOX

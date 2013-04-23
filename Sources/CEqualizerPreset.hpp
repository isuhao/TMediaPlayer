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

#ifndef FILE_C_EQUALIZER_PRESET
#define FILE_C_EQUALIZER_PRESET

#include <QObject>
#include <QString>
#include <QList>


class CMainWindow;


/// Class contenant un préréglage d'égaliseur.
class CEqualizerPreset : public QObject
{
    Q_OBJECT

public:

    /// Liste des fréquences prédéfinies.
    enum TFrequency
    {
        Frequency32  = 0, ///< 32 Hz.
        Frequency64  = 1, ///< 64 Hz.
        Frequency125 = 2, ///< 125 Hz.
        Frequency250 = 3, ///< 250 Hz.
        Frequency500 = 4, ///< 500 Hz.
        Frequency1K  = 5, ///< 1 KHz.
        Frequency2K  = 6, ///< 2 kHz.
        Frequency4K  = 7, ///< 4 kHz.
        Frequency8K  = 8, ///< 8 kHz.
        Frequency16K = 9  ///< 16 kHz.
    };

    explicit CEqualizerPreset(CMainWindow * application);

    inline int getId() const;
    inline QString getName() const;
    inline double getValue(int frequency) const;

    void setName(const QString& name);
    void setValue(int frequency, double value);

    bool operator==(const CEqualizerPreset& other) const;
    inline bool operator!=(const CEqualizerPreset& other) const;

    void updateDataBase();
    void removeFromDataBase();

    static QList<CEqualizerPreset *> loadFromDatabase(CMainWindow * application);

private:

    CMainWindow * m_application;
    int m_id;           ///< Identifiant du préréglage en base de données.
    QString m_name;     ///< Nom du préréglage.
    double m_value[10]; ///< Valeurs de gain de l'égaliseur.
};


inline int CEqualizerPreset::getId() const
{
    return m_id;
}


inline QString CEqualizerPreset::getName() const
{
    return m_name;
}


inline double CEqualizerPreset::getValue(int frequency) const
{
    if (frequency < 0 || frequency >= 10)
        return 1.0f;

    return m_value[frequency];
}


inline bool CEqualizerPreset::operator!=(const CEqualizerPreset& other) const
{
    return !operator==(other);
}

#endif // FILE_C_EQUALIZER_PRESET

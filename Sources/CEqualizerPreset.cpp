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

#include "CEqualizerPreset.hpp"
#include "CMediaManager.hpp"

#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>


/**
 * Constructeur par défaut.
 *
 * \param mediaManager Pointeurs sur la classe principale de l'application.
 */

CEqualizerPreset::CEqualizerPreset(CMediaManager * mediaManager) :
QObject        (mediaManager),
m_mediaManager (mediaManager),
m_id           (-1)
{
    Q_CHECK_PTR(m_mediaManager);

    for (std::size_t f = 0; f < 10; ++f)
    {
        m_value[f] = 1.0f;
    }
}


void CEqualizerPreset::setName(const QString& name)
{
    m_name = name;
}


void CEqualizerPreset::setValue(int frequency, double value)
{
    if (frequency < 0 || frequency >= 10)
        return;

    m_value[frequency] = value;
}


bool CEqualizerPreset::operator==(const CEqualizerPreset& other) const
{
    return (m_id       == other.m_id       &&
            m_name     == other.m_name     &&
            m_value[0] == other.m_value[0] &&
            m_value[1] == other.m_value[1] &&
            m_value[2] == other.m_value[2] &&
            m_value[3] == other.m_value[3] &&
            m_value[4] == other.m_value[4] &&
            m_value[5] == other.m_value[5] &&
            m_value[6] == other.m_value[6] &&
            m_value[7] == other.m_value[7] &&
            m_value[8] == other.m_value[8] &&
            m_value[9] == other.m_value[9]);
}


void CEqualizerPreset::updateDataBase()
{
    if (m_id < 0)
    {
        QSqlQuery query(m_mediaManager->getDataBase());

        query.prepare("INSERT INTO equalizer("
                          "equalizer_name, "
                          "equalizer_val0, "
                          "equalizer_val1, "
                          "equalizer_val2, "
                          "equalizer_val3, "
                          "equalizer_val4, "
                          "equalizer_val5, "
                          "equalizer_val6, "
                          "equalizer_val7, "
                          "equalizer_val8, "
                          "equalizer_val9"
                      ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

        query.bindValue( 0, m_name);
        query.bindValue( 1, m_value[0]);
        query.bindValue( 2, m_value[1]);
        query.bindValue( 3, m_value[2]);
        query.bindValue( 4, m_value[3]);
        query.bindValue( 5, m_value[4]);
        query.bindValue( 6, m_value[5]);
        query.bindValue( 7, m_value[6]);
        query.bindValue( 8, m_value[7]);
        query.bindValue( 9, m_value[8]);
        query.bindValue(10, m_value[9]);

        if (!query.exec())
        {
            m_mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return;
        }

        if (m_mediaManager->getDataBase().driverName() == "QPSQL")
        {
            query.prepare("SELECT currval('equalizer_seq')");

            if (!query.exec())
            {
                m_mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return;
            }

            if (query.next())
            {
                m_id = query.value(0).toInt();
            }
            else
            {
                m_mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return;
            }
        }
        else
        {
            m_id = query.lastInsertId().toInt();
        }
    }
    else if (m_id > 0)
    {
        QSqlQuery query(m_mediaManager->getDataBase());

        query.prepare("UPDATE equalizer SET "
                        "equalizer_name = ?,"
                        "equalizer_val0 = ?,"
                        "equalizer_val1 = ?,"
                        "equalizer_val2 = ?,"
                        "equalizer_val3 = ?,"
                        "equalizer_val4 = ?,"
                        "equalizer_val5 = ?,"
                        "equalizer_val6 = ?,"
                        "equalizer_val7 = ?,"
                        "equalizer_val8 = ?,"
                        "equalizer_val9 = ? "
                      "WHERE equalizer_id = ?");

        query.bindValue( 0, m_name);
        query.bindValue( 1, m_value[0]);
        query.bindValue( 2, m_value[1]);
        query.bindValue( 3, m_value[2]);
        query.bindValue( 4, m_value[3]);
        query.bindValue( 5, m_value[4]);
        query.bindValue( 6, m_value[5]);
        query.bindValue( 7, m_value[6]);
        query.bindValue( 8, m_value[7]);
        query.bindValue( 9, m_value[8]);
        query.bindValue(10, m_value[9]);
        query.bindValue(11, m_id);

        if (!query.exec())
        {
            m_mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }
}


void CEqualizerPreset::removeFromDataBase()
{
    if (m_id <= 0)
        return;

    QSqlQuery query(m_mediaManager->getDataBase());

    query.prepare("DELETE FROM equalizer WHERE equalizer_id = ?");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    m_id = 0;
}


QList<CEqualizerPreset *> CEqualizerPreset::loadFromDatabase(CMediaManager * mediaManager)
{
    Q_CHECK_PTR(mediaManager);

    QList<CEqualizerPreset *> presets;

    QSqlQuery query(mediaManager->getDataBase());

    if (!query.exec("SELECT "
                        "equalizer_id,"
                        "equalizer_name,"
                        "equalizer_val0,"
                        "equalizer_val1,"
                        "equalizer_val2,"
                        "equalizer_val3,"
                        "equalizer_val4,"
                        "equalizer_val5,"
                        "equalizer_val6,"
                        "equalizer_val7,"
                        "equalizer_val8,"
                        "equalizer_val9 "
                    "FROM equalizer ORDER BY equalizer_name"))
    {
        mediaManager->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return presets;
    }

    while (query.next())
    {
        CEqualizerPreset * preset = new CEqualizerPreset(mediaManager);
        preset->m_id   = query.value(0).toInt();
        preset->m_name = query.value(1).toString();

        for (int f = 0; f < 10; ++f)
        {
            preset->m_value[f] = query.value(2 + f).toDouble();
        }

        presets.append(preset);
    }

    return presets;
}

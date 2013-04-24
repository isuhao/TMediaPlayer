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

#ifndef FILE_C_MEDIA_MANAGER_HPP_
#define FILE_C_MEDIA_MANAGER_HPP_

#include <QObject>
#include <QString>
#include <QMap>
#include <QTranslator>


class QFile;
class QSettings;

namespace FMOD
{
    class System;
}


class CMediaManager : public QObject
{
public:

    explicit CMediaManager(QObject * parent = nullptr);
    ~CMediaManager();

    bool initSoundSystem();
    
    QFile * getLogFile(const QString& logName);
    void logError(const QString& message, const char * function, const char * file, int line);

    inline QString getApplicationPath() const;
    inline QSettings * getSettings() const;
    inline FMOD::System * getSoundSystem() const;

    static QString getAppVersion();
    static QString getAppDate();

private:
    
    QString m_applicationPath;        ///< Répertoire contenant l'application.
    QTranslator m_translator;
    QSettings * m_settings;           ///< Paramètres de l'application.
    FMOD::System * m_soundSystem;     ///< Système de son de FMOD.
    QMap<QString, QFile *> m_logList; ///< Liste des fichiers de log ouverts.
};


inline QString CMediaManager::getApplicationPath() const
{
    return m_applicationPath;
}


inline QSettings * CMediaManager::getSettings() const
{
    return m_settings;
}


inline FMOD::System * CMediaManager::getSoundSystem() const
{
    return m_soundSystem;
}

#endif // FILE_C_MEDIA_MANAGER_HPP_

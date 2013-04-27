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
#include <QDateTime>
#include <QMap>
#include <QTranslator>
#include <QSqlDatabase>


class CLibraryFolder;
class QFile;
class QSettings;

namespace FMOD
{
    class System;
}


class CMediaManager : public QObject
{
    Q_OBJECT

public:

    explicit CMediaManager(QObject * parent = nullptr);
    ~CMediaManager();

    bool initSoundSystem();
    bool loadDatabase();
    

    // Log
    QFile * getLogFile(const QString& logName);
    void logError(const QString& message, const char * function, const char * file, int line);
    void logDatabaseError(const QString& msg, const QString& query, const QString& fileName, int line);


    // Notifications
    struct TNotification
    {
        QString message; ///< Texte de la notification.
        QDateTime date;  ///< Date de l'envoi.

        inline TNotification(const QString& m, const QDateTime& d) : message(m), date(d) { }
    };

    inline QList<TNotification> getNotifications() const;
    void notifyInformation(const QString& message);


    // Dossiers de la médiathèque
    inline QList<CLibraryFolder *> getLibraryFolders() const;
    CLibraryFolder * getLibraryFolder(int folderId) const;
    int getLibraryFolderId(const QString& fileName) const;
    void addLibraryFolder(CLibraryFolder * folder);
    void removeLibraryFolder(CLibraryFolder * folder);


    inline QString getApplicationPath() const;

    inline QSqlDatabase getDataBase() const;
    inline QSettings * getSettings() const;
    inline FMOD::System * getSoundSystem() const;

    static QString getAppVersion();
    static QString getAppDate();

signals:

    void informationNotified(const QString& message);

private:

    void createDatabaseSQLite();
    void createDatabaseMySQL();
    void createDatabasePostgreSQL();
    
    QString m_applicationPath;                ///< Répertoire contenant l'application.
    QSqlDatabase m_dataBase;                  ///< Base de données.
    QTranslator m_translator;                 ///< Objet utilisé pour la traduction de l'application.
    QSettings * m_settings;                   ///< Paramètres de l'application.
    FMOD::System * m_soundSystem;             ///< Système de son de FMOD.
    QMap<QString, QFile *> m_logList;         ///< Liste des fichiers de log ouverts.
    QList<TNotification> m_infosNotified;     ///< Liste des notifications.
    QList<CLibraryFolder *> m_libraryFolders; ///< Liste des répertoires de la médiathèque.
};


inline QList<CMediaManager::TNotification> CMediaManager::getNotifications() const
{
    return m_infosNotified;
}


inline QList<CLibraryFolder *> CMediaManager::getLibraryFolders() const
{
    return m_libraryFolders;
}


inline QString CMediaManager::getApplicationPath() const
{
    return m_applicationPath;
}


inline QSqlDatabase CMediaManager::getDataBase() const
{
    return m_dataBase;
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

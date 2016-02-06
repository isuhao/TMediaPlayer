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

#ifndef FILE_C_MEDIA_MANAGER_HPP_
#define FILE_C_MEDIA_MANAGER_HPP_

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QList>
#include <QTranslator>
#include <QSqlDatabase>
#include "CEqualizerPreset.hpp"


class CLibraryFolder;
class CSong;
class QFile;
class QSettings;

namespace FMOD
{
    class System;
    class DSP;
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


    // Égaliseur
    void setEqualizerGain(CEqualizerPreset::TFrequency frequency, double gain);
    double getEqualizerGain(CEqualizerPreset::TFrequency frequency) const;
    void resetEqualizer();
    bool isEqualizerEnabled() const;
    inline QList<CEqualizerPreset *> getEqualizerPresets() const;
    void addEqualizerPreset(CEqualizerPreset * preset);
    void deleteEqualizerPreset(CEqualizerPreset * preset);
    CEqualizerPreset * getEqualizerPresetFromId(int id) const;
    CEqualizerPreset * getEqualizerPresetFromName(const QString& name) const;
    inline CEqualizerPreset * getCurrentEqualizerPreset() const;
    void setCurrentEqualizerPreset(CEqualizerPreset * equalizer);

    // Effets
    void setEchoDelay(int delay);
    int getEchoDelay() const;
    void setMinFilter(int frequency);
    int getMinFilter() const;
    void setMaxFilter(int frequency);
    int getMaxFilter() const;


    int getArtistId(const QString& name, const QString& nameSort);
    int getAlbumId(const QString& title, const QString& titleSort);
    int getGenreId(const QString& name);
    QStringList getGenreList();

    inline bool isMute() const;
    inline int getVolume() const;
    inline bool isShuffle() const;

    inline QString getApplicationPath() const;

    inline QSqlDatabase getDataBase() const;
    inline QSettings * getSettings() const;
    inline FMOD::System * getSoundSystem() const;

    static QString getAppVersion();
    static QString getAppDate();

public slots:

    void setMute(bool mute);
    void setVolume(int volume);
    void setShuffle(bool shuffle);
    void setEqualizerEnabled(bool enabled = true);

    void onSongModified();

signals:

    void informationNotified(const QString& message);
    void songModified(CSong * song); ///< Signal émis lorsque les informations d'un morceau sont modifiées.

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
    bool m_isMute;                            ///< Indique si le son est coupé.
    int m_volume;                             ///< Volume sonore (entre 0 et 100).
    bool m_isShuffle;                         ///< Indique si la lecture aléatoire est activée.

    // Égaliseur
    double m_equalizerGains[10];                  ///< Gains de l'égaliseur.
    FMOD::DSP * m_dsp[10];                        ///< Gains de l'égaliseur pour FMOD.
    QList<CEqualizerPreset *> m_equalizerPresets; ///< Liste des préréglages d'égaliseur.
    CEqualizerPreset * m_currentEqualizerPreset;  ///< Préréglage de l'égaliseur actuel.

    // Effets sonores
    FMOD::DSP * m_dspEcho;     ///< Effet d'écho.
    int m_echoDelay;
    FMOD::DSP * m_dspLowPass;  ///< Filtre passe-bas.
    int m_freqLowPass;
    FMOD::DSP * m_dspHighPass; ///< Filtre passe-haut.
    int m_freqHighPass;
};


inline QList<CMediaManager::TNotification> CMediaManager::getNotifications() const
{
    return m_infosNotified;
}


inline QList<CLibraryFolder *> CMediaManager::getLibraryFolders() const
{
    return m_libraryFolders;
}


inline QList<CEqualizerPreset *> CMediaManager::getEqualizerPresets() const
{
    return m_equalizerPresets;
}


inline CEqualizerPreset * CMediaManager::getCurrentEqualizerPreset() const
{
    return m_currentEqualizerPreset;
}


/**
 * Indique si le son est coupé.
 *
 * \return Booléen.
 */

inline bool CMediaManager::isMute() const
{
    return m_isMute;
}


/**
 * Donne le volume sonore.
 *
 * \return Volume sonore, entre 0 et 100.
 */

inline int CMediaManager::getVolume() const
{
    return m_volume;
}


/**
 * Indique si la lecture aléatoire est active.
 *
 * \return Booléen.
 */

inline bool CMediaManager::isShuffle() const
{
    return m_isShuffle;
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

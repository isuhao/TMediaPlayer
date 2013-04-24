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

#include "CMediaManager.hpp"

// Qt
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QDir>
#include <QSettings>
#include <QDesktopServices>
#include <QCoreApplication>

// FMOD
#include <fmod/fmod.hpp>

#include <QtDebug>


const QString appVersion = "1.0.46";     ///< Numéro de version de l'application.
const QString appDate    = "20/04/2013"; ///< Date de sortie de cette version.


/**
 * Retourne le numéro de version de l'application.
 *
 * \return Numéro de version.
 */

QString CMediaManager::getAppVersion()
{
    return appVersion;
}


/**
 * Retourne la date de sortie de cette version de l'application.
 *
 * \return Date de sortie.
 */

QString CMediaManager::getAppDate()
{
    return appDate;
}


CMediaManager::CMediaManager(QObject * parent) :
QObject       (parent),
m_settings    (nullptr),
m_soundSystem (nullptr)
{
#if QT_VERSION >= 0x050000
    m_applicationPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator();
#else
    m_applicationPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QDir::separator();
#endif

    // Création du répertoire si nécessaire
    QDir(m_applicationPath).mkpath(".");

    // Chargement des paramètres de l'application
    m_settings = new QSettings(this);

    // Internationnalisation
    QString lang = m_settings->value("Preferences/Language", QLocale::system().name()).toString();

    if (lang.isEmpty() || !m_translator.load(QString("Lang/TMediaPlayer_") + lang))
        m_translator.load(QString("Lang/TMediaPlayer_") + QLocale::system().name());

    qApp->installTranslator(&m_translator);
}


CMediaManager::~CMediaManager()
{
    m_soundSystem->release();
}


/**
 * Initialise FMOD.
 *
 * \return Booléen indiquant le succès ou l'échec du chargement.
 */

bool CMediaManager::initSoundSystem()
{
    bool ret = true;
    FMOD_RESULT res;

    res = FMOD::System_Create(&m_soundSystem);
    if (res != FMOD_OK)
        return false;

    unsigned int version;
    res = m_soundSystem->getVersion(&version);
    if (res != FMOD_OK)
        return false;

    if (version < FMOD_VERSION)
    {
        logError(tr("This program requires FMOD %1 or superior.").arg(FMOD_VERSION), __FUNCTION__, __FILE__, __LINE__);
        return false;
    }

    int numDrivers;
    res = m_soundSystem->getNumDrivers(&numDrivers);
    if (res != FMOD_OK)
        return false;

    if (numDrivers == 0)
    {
        res = m_soundSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        if (res != FMOD_OK)
            return false;
    }
    else
    {
        FMOD_CAPS caps;
        FMOD_SPEAKERMODE speakermode;
        res = m_soundSystem->getDriverCaps(0, &caps, nullptr, &speakermode);
        if (res != FMOD_OK)
            return false;

        // Set the user selected speaker mode
        res = m_soundSystem->setSpeakerMode(speakermode);
        if (res != FMOD_OK)
            return false;

        if (caps & FMOD_CAPS_HARDWARE_EMULATED)
        {
            res = m_soundSystem->setDSPBufferSize(1024, 10);
            if (res != FMOD_OK)
                return false;
        }

        char name[256] = "";
        res = m_soundSystem->getDriverInfo(0, name, 256, 0);
        if (res != FMOD_OK)
            return false;

        if (strstr(name, "SigmaTel"))
        {
            res = m_soundSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
            if (res != FMOD_OK)
                return false;
        }
    }

    res = m_soundSystem->init(100, FMOD_INIT_NORMAL, 0);
    if (res == FMOD_ERR_OUTPUT_CREATEBUFFER)
    {
        res = m_soundSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        if (res != FMOD_OK)
            return false;
        res = m_soundSystem->init(2, FMOD_INIT_NORMAL, 0);
    }

    return (res == FMOD_OK);
}


/**
 * Retourne le pointeur sur un fichier de log.
 *
 * \param logName Nom du fichier de log.
 * \return Pointeur sur le fichier ouvert en écriture.
 */

QFile * CMediaManager::getLogFile(const QString& logName)
{
    QString fileName = logName + QDateTime::currentDateTime().toString("-yyyy-MM-dd");

    if (!m_logList.contains(fileName))
    {
        QString logFileName = m_applicationPath + fileName + ".log";
        QFile * logFile = new QFile(logFileName, this);

        if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append))
        {
            logError(tr("can't open the log file \"%1\"").arg(logFileName), __FUNCTION__, __FILE__, __LINE__);
            return nullptr;
        }

        m_logList[fileName] = logFile;
    }

    return m_logList.value(fileName);
}


/**
 * Gestion des messages d'erreur.
 *
 * \param message  Message d'erreur.
 * \param function Nom de la fonction où l'erreur est survenue.
 * \param file     Nom du fichier source contenant la fonction.
 * \param line     Ligne dans le fichier source.
 */

void CMediaManager::logError(const QString& message, const char * function, const char * file, int line)
{
    static QFile * logFile = nullptr;
    static bool fileOpened = false;

    // L'ouverture du fichier n'est tentée qu'une seule fois pour éviter des appels récursifs infinis entre getLogFile et logError.
    if (!fileOpened || !logFile)
    {
        logFile = getLogFile("errors");
        fileOpened = true;
    }

    QString txt = QObject::tr("%2 (%3 line %4): %1").arg(message).arg(function).arg(file).arg(line);

    QTextStream stream(logFile);
    stream << txt << "\n";

#ifdef QT_DEBUG
    qWarning() << txt;
#endif
}

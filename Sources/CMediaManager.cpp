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
#include "CLibraryFolder.hpp"
#include "CSong.hpp"

// Qt
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QDir>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QDesktopServices>
#include <QCoreApplication>

// FMOD
#include <fmod/fmod.hpp>

#include <QtDebug>


const QString appVersion = "1.0.50";     ///< Numéro de version de l'application.
const QString appDate    = "21/10/2013"; ///< Date de sortie de cette version.


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


/**
 * Construit le gestionnaire de médias.
 *
 * \param parent Pointeur sur l'objet parent.
 */

CMediaManager::CMediaManager(QObject * parent) :
QObject       (parent),
m_settings    (nullptr),
m_soundSystem (nullptr),
m_isMute      (false),
m_volume      (50)
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


/**
 * Détruit le gestionnaire de médias et le système de son de FMOD, et ferme la connexion à la base de données.
 */

CMediaManager::~CMediaManager()
{
    // Enregistrement des paramètres
    m_settings->setValue("Preferences/Volume", m_volume);

    for (QList<CLibraryFolder *>::ConstIterator folder = m_libraryFolders.begin(); folder != m_libraryFolders.end(); ++folder)
    {
        delete *folder;
    }

    m_libraryFolders.clear();

    m_dataBase.close();
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
            res = m_soundSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
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

    // Paramètres de lecture
    setVolume(m_settings->value("Preferences/Volume", 50).toInt());

    return (res == FMOD_OK);
}


/**
 * Charge la base de données de la médiathèque.
 *
 * \return True si le chargement s'est bien passé, false sinon.
 */

bool CMediaManager::loadDatabase()
{
    // Chargement de la base de données
    QString dbType = m_settings->value("Database/Type", QString("QSQLITE")).toString();
    m_settings->setValue("Database/Type", dbType);
    m_dataBase = QSqlDatabase::addDatabase(dbType, "library");

    QString dbHostName = m_settings->value("Database/Host", QString("localhost")).toString();
    int dbPort = m_settings->value("Database/Port", 0).toInt();

#if QT_VERSION >= 0x050000
    QString dbBaseName = m_settings->value("Database/Base", QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "library.sqlite").toString();
#else
    QString dbBaseName = m_settings->value("Database/Base", QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QDir::separator() + "library.sqlite").toString();
#endif

    QString dbUserName = m_settings->value("Database/UserName", QString("root")).toString();
    QString dbPassword = m_settings->value("Database/Password", QString("")).toString();

    m_dataBase.setHostName(dbHostName);
    m_dataBase.setPort(dbPort);
    m_dataBase.setDatabaseName(dbBaseName);
    m_dataBase.setUserName(dbUserName);
    m_dataBase.setPassword(dbPassword);

    m_settings->setValue("Database/Host", dbHostName);
    m_settings->setValue("Database/Port", dbPort);
    m_settings->setValue("Database/Base", dbBaseName);
    m_settings->setValue("Database/UserName", dbUserName);
    m_settings->setValue("Database/Password", dbPassword);

    if (!m_dataBase.open())
    {
        logError(tr("Failed to load database: %1.").arg(m_dataBase.lastError().text()), __FUNCTION__, __FILE__, __LINE__);
        return false;
    }

    QSqlQuery query(m_dataBase);

    // Création des relations
    if (m_dataBase.driverName() == "QSQLITE")
    {
        createDatabaseSQLite();
    }
    else if (m_dataBase.driverName() == "QMYSQL")
    {
        createDatabaseMySQL();
    }
    else if (m_dataBase.driverName() == "QPSQL")
    {
        createDatabasePostgreSQL();
    }

    // Création des vues
    QStringList tables = m_dataBase.tables(QSql::Views);

    if (!tables.contains("albums"))
    {
        if (!query.exec("CREATE VIEW albums AS SELECT DISTINCT(album_title) FROM album NATURAL JOIN song WHERE song_id IS NOT NULL"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("artists"))
    {
        if (!query.exec("CREATE VIEW artists AS SELECT DISTINCT(artist_name) FROM artist NATURAL JOIN song WHERE song_id IS NOT NULL"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("genres"))
    {
        if (!query.exec("CREATE VIEW genres AS SELECT DISTINCT(genre_name) FROM genre NATURAL JOIN song WHERE song_id IS NOT NULL"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    // Liste des répertoires
    if (!query.exec("SELECT path_id, path_location, path_keep_organized, path_format, path_format_items FROM libpath"))
    {
        logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        while (query.next())
        {
            CLibraryFolder * libraryFolder = new CLibraryFolder(this);

            libraryFolder->id            = query.value(0).toInt();
            libraryFolder->pathName      = query.value(1).toString();
            libraryFolder->keepOrganized = query.value(2).toBool();
            libraryFolder->format        = query.value(3).toString();

            libraryFolder->convertStringToFormatItems(query.value(4).toString());

            m_libraryFolders.append(libraryFolder);
        }
    }

    // Préréglages d'égaliseur
    m_equalizerPresets = CEqualizerPreset::loadFromDatabase(this);

    // Égaliseur
    const float eqFrequencies[10] = {32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};

    for (int i = 0; i < 10; ++i)
    {
        m_equalizerGains[i] = qBound(0.05f, m_settings->value(QString("Equalizer/Gain_%1").arg(i), 1.0f).toFloat(), 3.0f);
        FMOD_RESULT res;

        res = m_soundSystem->createDSPByType(FMOD_DSP_TYPE_PARAMEQ, &m_dsp[i]);

        if (res != FMOD_OK)
            logError(tr("createDSPByType #%1").arg(i), __FUNCTION__, __FILE__, __LINE__);

        res = m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_CENTER, eqFrequencies[i]);

        if (res != FMOD_OK)
            logError(tr("dsp->setParameter(FMOD_DSP_PARAMEQ_CENTER) #%1").arg(i), __FUNCTION__, __FILE__, __LINE__);

        res = m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_BANDWIDTH, 1.0);

        if (res != FMOD_OK)
            logError(tr("dsp->setParameter(FMOD_DSP_PARAMEQ_BANDWIDTH) #%1").arg(i), __FUNCTION__, __FILE__, __LINE__);

        res = m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_GAIN, m_equalizerGains[i]);

        if (res != FMOD_OK)
            logError(tr("dsp->setParameter(FMOD_DSP_PARAMEQ_GAIN) #%1").arg(i), __FUNCTION__, __FILE__, __LINE__);

        res = m_soundSystem->addDSP(m_dsp[i], nullptr);

        if (res != FMOD_OK)
            logError(tr("addDSP #%1").arg(i), __FUNCTION__, __FILE__, __LINE__);
    }

    setEqualizerEnabled(m_settings->value("Equalizer/Enabled", false).toBool());

    QString presetName = m_settings->value(QString("Equalizer/PresetName"), QString()).toString();
    CEqualizerPreset * preset = getEqualizerPresetFromName(presetName);

    if (preset)
    {
        bool currentEqualizerPresetDefined = true;

        for (int f = 0; f < 10; ++f)
        {
            if (std::abs(preset->getValue(f) - m_equalizerGains[f]) < std::numeric_limits<double>::epsilon())
                currentEqualizerPresetDefined = false;
        }

        if (currentEqualizerPresetDefined)
        {
            m_currentEqualizerPreset = preset;
        }
    }

    return true;
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

    QString txt = tr("%2 (%3 line %4): %1").arg(message).arg(function).arg(file).arg(line);

    QTextStream stream(logFile);
    stream << txt << "\n";

#ifdef QT_DEBUG
    qWarning() << txt;
#endif
}


void CMediaManager::logDatabaseError(const QString& msg, const QString& query, const QString& fileName, int line)
{
    logError(msg + "\n" + tr("Query: ") + query, "", fileName.toUtf8().data(), line);
}


/**
 * Affiche un message dans la barre d'état.
 * Le message est affiché pendant 5 secondes.
 *
 * \param message Message à afficher.
 */

void CMediaManager::notifyInformation(const QString& message)
{
    m_infosNotified << TNotification(message, QDateTime::currentDateTime());
    emit informationNotified(message);
}


CLibraryFolder * CMediaManager::getLibraryFolder(int folderId) const
{
    for (QList<CLibraryFolder *>::ConstIterator it = m_libraryFolders.begin(); it != m_libraryFolders.end(); ++it)
    {
        if ((*it)->id == folderId)
            return *it;
    }

    return nullptr;
}


int CMediaManager::getLibraryFolderId(const QString& fileName) const
{
    for (QList<CLibraryFolder *>::ConstIterator it = m_libraryFolders.begin(); it != m_libraryFolders.end(); ++it)
    {
        if (fileName.startsWith((*it)->pathName))
            return (*it)->id;
    }

    return -1;
}


void CMediaManager::addLibraryFolder(CLibraryFolder * folder)
{
    if (!folder || m_libraryFolders.contains(folder))
        return;

    m_libraryFolders.append(folder);
}


/**
 * Supprime un répertoire de la médiathèque.
 *
 * \param folder Pointeur sur le répertoire à supprimer.
 */

void CMediaManager::removeLibraryFolder(CLibraryFolder * folder)
{
    if (!folder)
        return;

    m_libraryFolders.removeAll(folder);

    if (folder->id > 0)
    {
        QSqlQuery query(m_dataBase);
        query.prepare("DELETE FROM libpath WHERE path_id = ?");
        query.bindValue(0, folder->id);

        if (!query.exec())
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    delete folder;
}


/**
 * Modifie le gain de l'égaliseur pour une bande de fréquence.
 *
 * \param frequency Bande de fréquence.
 * \param gain      Valeur du gain (entre 0.05 et 3).
 */

void CMediaManager::setEqualizerGain(CEqualizerPreset::TFrequency frequency, double gain)
{
    m_equalizerGains[frequency] = qBound(0.05, gain, 3.0);
    m_settings->setValue(QString("Equalizer/Gain_%1").arg(frequency), m_equalizerGains[frequency]);
    FMOD_RESULT res = m_dsp[frequency]->setParameter(FMOD_DSP_PARAMEQ_GAIN, m_equalizerGains[frequency]);

    if (res != FMOD_OK)
        logError(tr("dsp->setParameter(FMOD_DSP_PARAMEQ_GAIN) #%1").arg(frequency), __FUNCTION__, __FILE__, __LINE__);
}


/**
 * Récupère le gain de l'égaliseur pour une bande de fréquence.
 *
 * \param frequency Bande de fréquence.
 * \return Valeur du gain (entre 0.05 et 3).
 */

double CMediaManager::getEqualizerGain(CEqualizerPreset::TFrequency frequency) const
{
    return m_equalizerGains[frequency];
}


/**
 * Réinitialise les gains de l'égaliseur.
 * Tous les gains sont définis à 1.
 */

void CMediaManager::resetEqualizer()
{
    setEqualizerGain(CEqualizerPreset::Frequency32 , 1.0f);
    setEqualizerGain(CEqualizerPreset::Frequency64 , 1.0f);
    setEqualizerGain(CEqualizerPreset::Frequency125, 1.0f);
    setEqualizerGain(CEqualizerPreset::Frequency250, 1.0f);
    setEqualizerGain(CEqualizerPreset::Frequency500, 1.0f);
    setEqualizerGain(CEqualizerPreset::Frequency1K , 1.0f);
    setEqualizerGain(CEqualizerPreset::Frequency2K , 1.0f);
    setEqualizerGain(CEqualizerPreset::Frequency4K , 1.0f);
    setEqualizerGain(CEqualizerPreset::Frequency8K , 1.0f);
    setEqualizerGain(CEqualizerPreset::Frequency16K, 1.0f);
}


/**
 * Active ou désactive l'égaliseur.
 *
 * \param enabled Booléen.
 */

void CMediaManager::setEqualizerEnabled(bool enabled)
{
    FMOD_RESULT res;

    m_settings->setValue(QString("Equalizer/Enabled"), enabled);

    for (int i = 0; i < 10; ++i)
    {
        res = m_dsp[i]->setParameter(FMOD_DSP_PARAMEQ_GAIN, (enabled ? m_equalizerGains[i] : 1.0));

        if (res != FMOD_OK)
            logError(tr("dsp->setParameter(FMOD_DSP_PARAMEQ_GAIN) #%1").arg(i), __FUNCTION__, __FILE__, __LINE__);
    }
}


/**
 * Indique si l'égaliseur est activé.
 *
 * \return Booléen.
 */

bool CMediaManager::isEqualizerEnabled() const
{
    return m_settings->value(QString("Equalizer/Enabled"), false).toBool();
}


void CMediaManager::addEqualizerPreset(CEqualizerPreset * preset)
{
    if (!preset)
        return;

    if (m_equalizerPresets.contains(preset))
        return;

    m_equalizerPresets.append(preset);
}


void CMediaManager::deleteEqualizerPreset(CEqualizerPreset * preset)
{
    if (!preset)
        return;

    if (m_currentEqualizerPreset == preset)
    {
        m_currentEqualizerPreset = nullptr;
        m_settings->setValue("Equalizer/PresetName", QString());
    }

    preset->removeFromDataBase();
    delete preset;

    m_equalizerPresets.removeOne(preset);
}


/**
 * Retourne le préréglage d'égaliseur correspondant à un identifiant.
 *
 * \param id Identifiant du préréglage.
 * \return Pointeur sur le préréglage, ou nullptr.
 */

CEqualizerPreset * CMediaManager::getEqualizerPresetFromId(int id) const
{
    for (QList<CEqualizerPreset *>::ConstIterator it = m_equalizerPresets.begin(); it != m_equalizerPresets.end(); ++it)
    {
        if ((*it)->getId() == id)
            return *it;
    }

    return nullptr;
}


/**
 * Retourne le préréglage d'égaliseur correspondant à un nom.
 *
 * \param name Nom du préréglage.
 * \return Pointeur sur le préréglage, ou nullptr.
 */

CEqualizerPreset * CMediaManager::getEqualizerPresetFromName(const QString& name) const
{
    for (QList<CEqualizerPreset *>::ConstIterator it = m_equalizerPresets.begin(); it != m_equalizerPresets.end(); ++it)
    {
        if ((*it)->getName() == name)
            return *it;
    }

    return nullptr;
}


void CMediaManager::setCurrentEqualizerPreset(CEqualizerPreset * equalizer)
{
    if (!equalizer)
    {
        m_settings->setValue("Equalizer/PresetName", QString());
        return;
    }

    m_currentEqualizerPreset = equalizer;

    setEqualizerGain(CEqualizerPreset::Frequency32 , equalizer->getValue(0));
    setEqualizerGain(CEqualizerPreset::Frequency64 , equalizer->getValue(1));
    setEqualizerGain(CEqualizerPreset::Frequency125, equalizer->getValue(2));
    setEqualizerGain(CEqualizerPreset::Frequency250, equalizer->getValue(3));
    setEqualizerGain(CEqualizerPreset::Frequency500, equalizer->getValue(4));
    setEqualizerGain(CEqualizerPreset::Frequency1K , equalizer->getValue(5));
    setEqualizerGain(CEqualizerPreset::Frequency2K , equalizer->getValue(6));
    setEqualizerGain(CEqualizerPreset::Frequency4K , equalizer->getValue(7));
    setEqualizerGain(CEqualizerPreset::Frequency8K , equalizer->getValue(8));
    setEqualizerGain(CEqualizerPreset::Frequency16K, equalizer->getValue(9));

    m_settings->setValue("Equalizer/PresetName", equalizer->getName());
}


/**
 * Récupère l'identifiant d'un artiste en base de données.
 *
 * \param name     Nom de l'artiste.
 * \param nameSort Nom de l'artiste pour le tri.
 * \return Identifiant de l'artiste, ou -1 en cas d'erreur.
 */

int CMediaManager::getArtistId(const QString& name, const QString& nameSort)
{
    Q_ASSERT(!name.isNull());
    Q_ASSERT(!nameSort.isNull());

    QSqlQuery query(m_dataBase);
    query.prepare("SELECT artist_id FROM artist WHERE artist_name = ? AND artist_name_sort = ?");
    query.bindValue(0, name);
    query.bindValue(1, nameSort);

    if (!query.exec())
    {
        logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (query.next())
    {
        return query.value(0).toInt();
    }

    query.prepare("INSERT INTO artist (artist_name, artist_name_sort) VALUES (?, ?)");
    query.bindValue(0, name);
    query.bindValue(1, nameSort);

    if (!query.exec())
    {
        logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (m_dataBase.driverName() == "QPSQL")
    {
        query.prepare("SELECT currval('artist_seq')");

        if (!query.exec())
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }

        if (query.next())
        {
            return query.value(0).toInt();
        }
        else
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }
    }
    else
    {
        return query.lastInsertId().toInt();
    }
}


/**
 * Récupère l'identifiant d'un album en base de données.
 *
 * \param title     Titre de l'album.
 * \param titleSort Titre de l'album pour le tri.
 * \return Identifiant de l'album, ou -1 en cas d'erreur.
 */

int CMediaManager::getAlbumId(const QString& title, const QString& titleSort)
{
    Q_ASSERT(!title.isNull());
    Q_ASSERT(!titleSort.isNull());

    QSqlQuery query(m_dataBase);
    query.prepare("SELECT album_id FROM album WHERE album_title = ? AND album_title_sort = ?");
    query.bindValue(0, title);
    query.bindValue(1, titleSort);

    if (!query.exec())
    {
        logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (query.next())
    {
        return query.value(0).toInt();
    }

    query.prepare("INSERT INTO album (album_title, album_title_sort) VALUES (?, ?)");
    query.bindValue(0, title);
    query.bindValue(1, titleSort);

    if (!query.exec())
    {
        logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (m_dataBase.driverName() == "QPSQL")
    {
        query.prepare("SELECT currval('album_seq')");

        if (!query.exec())
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }

        if (query.next())
        {
            return query.value(0).toInt();
        }
        else
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }
    }
    else
    {
        return query.lastInsertId().toInt();
    }
}


/**
 * Récupère l'identifiant d'un genre en base de données.
 *
 * \param name Nom du genre.
 * \return Identifiant du genre, ou -1 en cas d'erreur.
 */

int CMediaManager::getGenreId(const QString& name)
{
    Q_ASSERT(!name.isNull());

    QSqlQuery query(m_dataBase);
    query.prepare("SELECT genre_id FROM genre WHERE genre_name = ?");
    query.bindValue(0, name);

    if (!query.exec())
    {
        logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (query.next())
    {
        return query.value(0).toInt();
    }

    query.prepare("INSERT INTO genre (genre_name) VALUES (?)");
    query.bindValue(0, name);

    if (!query.exec())
    {
        logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (m_dataBase.driverName() == "QPSQL")
    {
        query.prepare("SELECT currval('genre_seq')");

        if (!query.exec())
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }

        if (query.next())
        {
            return query.value(0).toInt();
        }
        else
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
            return -1;
        }
    }
    else
    {
        return query.lastInsertId().toInt();
    }
}


/**
 * Retourne la liste des genres classée par nom.
 *
 * \todo Compléter la liste des genres prédéfinis.
 *
 * \return Liste des genres, qui contient l'ensemble des genres utilisés
 *         par les morceaux, en plus de certains genres prédéfinis.
 */

QStringList CMediaManager::getGenreList()
{
    QStringList genres;

    // Genres prédéfinis
    genres.append("Blues");
    genres.append("Classical");
    genres.append("Country");
    genres.append("Funk");
    genres.append("Hard Rock");
    genres.append("Heavy Metal");
    genres.append("Jazz");
    genres.append("Punk");
    genres.append("Rap");
    genres.append("Reggae");
    genres.append("Rock");

    // Liste des genres utilisés
    QSqlQuery query(m_dataBase);

    if (query.exec("SELECT genre_name FROM genres"))
    {
        while (query.next())
        {
            genres.append(query.value(0).toString());
        }
    }
    else
    {
        logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }

    genres.removeDuplicates();
    genres.sort();
    return genres;
}


/**
 * Active ou désactive le son.
 *
 * \param mute True pour couper le son, false pour le remettre.
 */

void CMediaManager::setMute(bool mute)
{
    if (mute != m_isMute)
    {
        m_isMute = mute;
/*
        if (m_currentSongItem)
        {
            m_currentSongItem->getSong()->setMute(m_isMute);
        }
*/
    }
}


/**
 * Modifie le volume.
 *
 * \param volume Volume du son (entre 0 et 100).
 */

void CMediaManager::setVolume(int volume)
{
    volume = qBound(0, volume, 100);

    if (volume != m_volume)
    {
        m_volume = volume;
/*
        if (m_currentSongItem)
        {
            m_currentSongItem->getSong()->setVolume(volume);
        }
*/
    }
}


/**
 * Méthode appelée lorsqu'un morceau est modifié.
 * Le signal songModified est émis.
 */

void CMediaManager::onSongModified()
{
    CSong * song = qobject_cast<CSong *>(sender());

    if (song)
    {
        emit songModified(song);
    }
}


/**
 * Crée la structure de la base de données pour SQLite.
 */

void CMediaManager::createDatabaseSQLite()
{
    QSqlQuery query(m_dataBase);
    QStringList tables = m_dataBase.tables(QSql::Tables);

    if (!tables.contains("folder"))
    {
        if (!query.exec("CREATE TABLE folder ("
                            "folder_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "folder_name VARCHAR(512) NOT NULL,"
                            "folder_parent INTEGER NOT NULL,"
                            "folder_position INTEGER NOT NULL,"
                            "folder_expanded INTEGER NOT NULL"
                            //",UNIQUE (folder_parent, folder_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO folder VALUES (0, '', 0, 1, 1)"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("playlist"))
    {
        if (!query.exec("CREATE TABLE playlist ("
                            "playlist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "playlist_name VARCHAR(512) NOT NULL,"
                            "folder_id INTEGER NOT NULL,"
                            "list_position INTEGER NOT NULL,"
                            "list_columns VARCHAR(512) NOT NULL"
                            //",UNIQUE (folder_id, list_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO playlist (playlist_id, playlist_name, folder_id, list_position, list_columns) "
                        "VALUES (0, 'Library', 0, -1, '0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("dynamic_list"))
    {
        if (!query.exec("CREATE TABLE dynamic_list ("
                            "dynamic_list_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "criteria_id INTEGER NOT NULL,"
                            "playlist_id INTEGER NOT NULL,"
                            "auto_update INTEGER NOT NULL,"
                            "only_checked INTEGER NOT NULL,"
                            "UNIQUE (playlist_id)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("criteria"))
    {
        if (!query.exec("CREATE TABLE criteria ("
                            "criteria_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "dynamic_list_id INTEGER NOT NULL,"
                            "criteria_parent INTEGER NOT NULL,"
                            "criteria_position INTEGER NOT NULL,"
                            "criteria_type INTEGER NOT NULL,"
                            "criteria_condition INTEGER NOT NULL,"
                            "criteria_value1 VARCHAR(512),"
                            "criteria_value2 VARCHAR(512),"
                            "UNIQUE (dynamic_list_id, criteria_parent, criteria_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("static_list"))
    {
        if (!query.exec("CREATE TABLE static_list ("
                            "static_list_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "playlist_id INTEGER NOT NULL,"
                            "UNIQUE (playlist_id)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("static_list_song"))
    {
        if (!query.exec("CREATE TABLE static_list_song ("
                            "static_list_id INTEGER NOT NULL,"
                            "song_id INTEGER NOT NULL,"
                            "song_position INTEGER NOT NULL,"
                            "UNIQUE (static_list_id, song_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("song"))
    {
        if (!query.exec("CREATE TABLE song ("
                            "song_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "song_filename VARCHAR(512) NOT NULL UNIQUE,"
                            "song_filesize INTEGER NOT NULL,"
                            "song_bitrate INTEGER NOT NULL,"
                            "song_sample_rate INTEGER NOT NULL,"
                            "song_format INTEGER NOT NULL,"
                            "song_channels INTEGER NOT NULL,"
                            "song_duration INTEGER NOT NULL,"
                            "song_creation DATETIME NOT NULL,"
                            "song_modification DATETIME NOT NULL,"
                            "song_enabled INTEGER NOT NULL,"
                            "song_title VARCHAR(512) NOT NULL,"
                            "song_title_sort VARCHAR(512) NOT NULL,"
                            "song_subtitle VARCHAR(512) NOT NULL,"
                            "song_grouping VARCHAR(512) NOT NULL,"
                            "artist_id INTEGER NOT NULL,"
                            "album_id INTEGER NOT NULL,"
                            "album_artist_id INTEGER NOT NULL,"
                            "song_composer VARCHAR(512) NOT NULL,"
                            "song_composer_sort VARCHAR(512) NOT NULL,"
                            "song_year INTEGER NOT NULL,"
                            "song_track_number INTEGER NOT NULL,"
                            "song_track_count INTEGER NOT NULL,"
                            "song_disc_number INTEGER NOT NULL,"
                            "song_disc_count INTEGER NOT NULL,"
                            "genre_id INTEGER NOT NULL,"
                            "song_rating INTEGER NOT NULL,"
                            "song_comments TEXT NOT NULL,"
                            "song_bpm INTEGER NOT NULL,"
                            "song_lyrics TEXT NOT NULL,"
                            "song_language VARCHAR(2) NOT NULL,"
                            "song_lyricist VARCHAR(512) NOT NULL,"
                            "song_compilation INTEGER NOT NULL,"
                            "song_skip_shuffle INTEGER NOT NULL,"
                            "song_play_count INTEGER NOT NULL,"
                            "song_play_time TIMESTAMP,"
                            "song_play_time_utc TIMESTAMP,"
                            "song_track_gain FLOAT,"
                            "song_track_peak FLOAT,"
                            "song_album_gain FLOAT,"
                            "song_album_peak FLOAT"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("album"))
    {
        if (!query.exec("CREATE TABLE album ("
                            "album_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "album_title VARCHAR(512) NOT NULL,"
                            "album_title_sort VARCHAR(512),"
                            "UNIQUE (album_title, album_title_sort)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO album (album_id, album_title, album_title_sort) VALUES (0, '', '')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("artist"))
    {
        if (!query.exec("CREATE TABLE artist ("
                            "artist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "artist_name VARCHAR(512) NOT NULL,"
                            "artist_name_sort VARCHAR(512),"
                            "UNIQUE (artist_name, artist_name_sort)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO artist (artist_id, artist_name, artist_name_sort) VALUES (0, '', '')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("genre"))
    {
        if (!query.exec("CREATE TABLE genre ("
                            "genre_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "genre_name VARCHAR(512) NOT NULL UNIQUE"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO genre (genre_id, genre_name) VALUES (0, '')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("play"))
    {
        if (!query.exec("CREATE TABLE play ("
                            "play_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "song_id INTEGER NOT NULL,"
                            "play_time TIMESTAMP,"
                            "play_time_utc TIMESTAMP"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("libpath"))
    {
        if (!query.exec("CREATE TABLE libpath ("
                            "path_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "path_location VARCHAR(512) NOT NULL UNIQUE,"
                            "path_keep_organized INTEGER NOT NULL,"
                            "path_format VARCHAR(512) NOT NULL,"
                            "path_format_items TEXT"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("equalizer"))
    {
        if (!query.exec("CREATE TABLE equalizer ("
                            "equalizer_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                            "equalizer_name VARCHAR(512) NOT NULL UNIQUE,"
                            "equalizer_val0 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val1 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val2 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val3 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val4 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val5 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val6 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val7 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val8 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val9 FLOAT NOT NULL DEFAULT 1.0"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }
}


/**
 * Crée la structure de la base de données pour MySQL.
 */

void CMediaManager::createDatabaseMySQL()
{
    QSqlQuery query(m_dataBase);
    QStringList tables = m_dataBase.tables(QSql::Tables);

    if (!tables.contains("folder"))
    {
        if (!query.exec("CREATE TABLE folder ("
                            "folder_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "folder_name VARCHAR(512) NOT NULL,"
                            "folder_parent INTEGER NOT NULL,"
                            "folder_position INTEGER NOT NULL,"
                            "folder_expanded INTEGER NOT NULL"
                            //",UNIQUE (folder_parent, folder_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO folder VALUES (0, '', 0, 1, 1)"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("playlist"))
    {
        if (!query.exec("CREATE TABLE playlist ("
                            "playlist_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "playlist_name VARCHAR(512) NOT NULL,"
                            "folder_id INTEGER NOT NULL,"
                            "list_position INTEGER NOT NULL,"
                            "list_columns VARCHAR(512) NOT NULL"
                            //",UNIQUE (folder_id, list_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO playlist (playlist_id, playlist_name, folder_id, list_position, list_columns) "
                        "VALUES (0, 'Library', 0, -1, '0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("dynamic_list"))
    {
        if (!query.exec("CREATE TABLE dynamic_list ("
                            "dynamic_list_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "criteria_id INTEGER NOT NULL,"
                            "playlist_id INTEGER NOT NULL,"
                            "auto_update INTEGER NOT NULL,"
                            "only_checked INTEGER NOT NULL,"
                            "UNIQUE (playlist_id)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("criteria"))
    {
        if (!query.exec("CREATE TABLE criteria ("
                            "criteria_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "dynamic_list_id INTEGER NOT NULL,"
                            "criteria_parent INTEGER NOT NULL,"
                            "criteria_position INTEGER NOT NULL,"
                            "criteria_type INTEGER NOT NULL,"
                            "criteria_condition INTEGER NOT NULL,"
                            "criteria_value1 VARCHAR(512),"
                            "criteria_value2 VARCHAR(512),"
                            "UNIQUE (dynamic_list_id, criteria_parent, criteria_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("static_list"))
    {
        if (!query.exec("CREATE TABLE static_list ("
                            "static_list_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "playlist_id INTEGER NOT NULL,"
                            "UNIQUE (playlist_id)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("static_list_song"))
    {
        if (!query.exec("CREATE TABLE static_list_song ("
                            "static_list_id INTEGER NOT NULL,"
                            "song_id INTEGER NOT NULL,"
                            "song_position INTEGER NOT NULL,"
                            "UNIQUE (static_list_id, song_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("song"))
    {
        if (!query.exec("CREATE TABLE song ("
                            "song_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "song_filename VARCHAR(512) NOT NULL UNIQUE,"
                            "song_filesize INTEGER NOT NULL,"
                            "song_bitrate INTEGER NOT NULL,"
                            "song_sample_rate INTEGER NOT NULL,"
                            "song_format INTEGER NOT NULL,"
                            "song_channels INTEGER NOT NULL,"
                            "song_duration INTEGER NOT NULL,"
                            "song_creation DATETIME NOT NULL,"
                            "song_modification DATETIME NOT NULL,"
                            "song_enabled INTEGER NOT NULL,"
                            "song_title VARCHAR(512) NOT NULL,"
                            "song_title_sort VARCHAR(512) NOT NULL,"
                            "song_subtitle VARCHAR(512) NOT NULL,"
                            "song_grouping VARCHAR(512) NOT NULL,"
                            "artist_id INTEGER NOT NULL,"
                            "album_id INTEGER NOT NULL,"
                            "album_artist_id INTEGER NOT NULL,"
                            "song_composer VARCHAR(512) NOT NULL,"
                            "song_composer_sort VARCHAR(512) NOT NULL,"
                            "song_year INTEGER NOT NULL,"
                            "song_track_number INTEGER NOT NULL,"
                            "song_track_count INTEGER NOT NULL,"
                            "song_disc_number INTEGER NOT NULL,"
                            "song_disc_count INTEGER NOT NULL,"
                            "genre_id INTEGER NOT NULL,"
                            "song_rating INTEGER NOT NULL,"
                            "song_comments TEXT NOT NULL,"
                            "song_bpm INTEGER NOT NULL,"
                            "song_lyrics TEXT NOT NULL,"
                            "song_language VARCHAR(2) NOT NULL,"
                            "song_lyricist VARCHAR(512) NOT NULL,"
                            "song_compilation INTEGER NOT NULL,"
                            "song_skip_shuffle INTEGER NOT NULL,"
                            "song_play_count INTEGER NOT NULL,"
                            "song_play_time TIMESTAMP,"
                            "song_play_time_utc TIMESTAMP,"
                            "song_track_gain FLOAT,"
                            "song_track_peak FLOAT,"
                            "song_album_gain FLOAT,"
                            "song_album_peak FLOAT"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("album"))
    {
        if (!query.exec("CREATE TABLE album ("
                            "album_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "album_title VARCHAR(512) NOT NULL,"
                            "album_title_sort VARCHAR(512),"
                            "UNIQUE (album_title, album_title_sort)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO album (album_id, album_title, album_title_sort) VALUES (0, '', '')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("artist"))
    {
        if (!query.exec("CREATE TABLE artist ("
                            "artist_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "artist_name VARCHAR(512) NOT NULL,"
                            "artist_name_sort VARCHAR(512),"
                            "UNIQUE (artist_name, artist_name_sort)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO artist (artist_id, artist_name, artist_name_sort) VALUES (0, '', '')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("genre"))
    {
        if (!query.exec("CREATE TABLE genre ("
                            "genre_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "genre_name VARCHAR(512) NOT NULL UNIQUE"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO genre (genre_id, genre_name) VALUES (0, '')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("play"))
    {
        if (!query.exec("CREATE TABLE play ("
                            "play_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "song_id INTEGER NOT NULL,"
                            "play_time TIMESTAMP,"
                            "play_time_utc TIMESTAMP"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("libpath"))
    {
        if (!query.exec("CREATE TABLE libpath ("
                            "path_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "path_location VARCHAR(512) NOT NULL UNIQUE,"
                            "path_keep_organized INTEGER NOT NULL,"
                            "path_format VARCHAR(512) NOT NULL,"
                            "path_format_items TEXT"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("equalizer"))
    {
        if (!query.exec("CREATE TABLE equalizer ("
                            "equalizer_id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                            "equalizer_name VARCHAR(512) NOT NULL UNIQUE,"
                            "equalizer_val0 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val1 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val2 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val3 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val4 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val5 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val6 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val7 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val8 FLOAT NOT NULL DEFAULT 1.0,"
                            "equalizer_val9 FLOAT NOT NULL DEFAULT 1.0"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }
}


/**
 * Crée la structure de la base de données pour PostgreSQL.
 */

void CMediaManager::createDatabasePostgreSQL()
{
    QSqlQuery query(m_dataBase);
    QStringList tables = m_dataBase.tables(QSql::Tables);

    if (!tables.contains("folder"))
    {
        if (!query.exec("CREATE TABLE folder ("
                            "folder_id SERIAL PRIMARY KEY,"
                            "folder_name VARCHAR(512) NOT NULL,"
                            "folder_parent INTEGER NOT NULL,"
                            "folder_position INTEGER NOT NULL,"
                            "folder_expanded INTEGER NOT NULL"
                            //",UNIQUE (folder_parent, folder_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE folder_folder_id_seq RENAME TO folder_seq"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO folder VALUES (0, '', 0, 1, 1)"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("playlist"))
    {
        if (!query.exec("CREATE TABLE playlist ("
                            "playlist_id SERIAL PRIMARY KEY,"
                            "playlist_name VARCHAR(512) NOT NULL,"
                            "folder_id INTEGER NOT NULL,"
                            "list_position INTEGER NOT NULL,"
                            "list_columns VARCHAR(512) NOT NULL"
                            //",UNIQUE (folder_id, list_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO playlist (playlist_id, playlist_name, folder_id, list_position, list_columns)"
                        "VALUES (0, 'Library', 0, -1, '0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("dynamic_list"))
    {
        if (!query.exec("CREATE TABLE dynamic_list ("
                            "dynamic_list_id SERIAL PRIMARY KEY,"
                            "criteria_id INTEGER NOT NULL,"
                            "playlist_id INTEGER NOT NULL,"
                            "auto_update INTEGER NOT NULL,"
                            "only_checked INTEGER NOT NULL,"
                            "UNIQUE (playlist_id)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE dynamic_list_dynamic_list_id_seq RENAME TO dynamic_list_seq"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("criteria"))
    {
        if (!query.exec("CREATE TABLE criteria ("
                            "criteria_id SERIAL PRIMARY KEY,"
                            "dynamic_list_id INTEGER NOT NULL,"
                            "criteria_parent INTEGER NOT NULL,"
                            "criteria_position INTEGER NOT NULL,"
                            "criteria_type INTEGER NOT NULL,"
                            "criteria_condition INTEGER NOT NULL,"
                            "criteria_value1 VARCHAR(512),"
                            "criteria_value2 VARCHAR(512),"
                            "UNIQUE (dynamic_list_id, criteria_parent, criteria_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE criteria_criteria_id_seq RENAME TO criteria_seq"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("static_list"))
    {
        if (!query.exec("CREATE TABLE static_list ("
                            "static_list_id SERIAL PRIMARY KEY,"
                            "playlist_id INTEGER NOT NULL,"
                            "UNIQUE (playlist_id)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE static_list_static_list_id_seq RENAME TO static_list_seq"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("static_list_song"))
    {
        if (!query.exec("CREATE TABLE static_list_song ("
                            "static_list_id INTEGER NOT NULL,"
                            "song_id INTEGER NOT NULL,"
                            "song_position INTEGER NOT NULL,"
                            "UNIQUE (static_list_id, song_position)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("song"))
    {
        if (!query.exec("CREATE TABLE song ("
                            "song_id SERIAL PRIMARY KEY,"
                            "song_filename VARCHAR(512) NOT NULL UNIQUE,"
                            "song_filesize INTEGER NOT NULL,"
                            "song_bitrate INTEGER NOT NULL,"
                            "song_sample_rate INTEGER NOT NULL,"
                            "song_format INTEGER NOT NULL,"
                            "song_channels INTEGER NOT NULL,"
                            "song_duration INTEGER NOT NULL,"
                            "song_creation TIMESTAMP NOT NULL,"
                            "song_modification TIMESTAMP NOT NULL,"
                            "song_enabled INTEGER NOT NULL,"
                            "song_title VARCHAR(512) NOT NULL,"
                            "song_title_sort VARCHAR(512) NOT NULL,"
                            "song_subtitle VARCHAR(512) NOT NULL,"
                            "song_grouping VARCHAR(512) NOT NULL,"
                            "artist_id INTEGER NOT NULL,"
                            "album_id INTEGER NOT NULL,"
                            "album_artist_id INTEGER NOT NULL,"
                            "song_composer VARCHAR(512) NOT NULL,"
                            "song_composer_sort VARCHAR(512) NOT NULL,"
                            "song_year INTEGER NOT NULL,"
                            "song_track_number INTEGER NOT NULL,"
                            "song_track_count INTEGER NOT NULL,"
                            "song_disc_number INTEGER NOT NULL,"
                            "song_disc_count INTEGER NOT NULL,"
                            "genre_id INTEGER NOT NULL,"
                            "song_rating INTEGER NOT NULL,"
                            "song_comments TEXT NOT NULL,"
                            "song_bpm INTEGER NOT NULL,"
                            "song_lyrics TEXT NOT NULL,"
                            "song_language CHAR(2) NOT NULL,"
                            "song_lyricist VARCHAR(512) NOT NULL,"
                            "song_compilation INTEGER NOT NULL,"
                            "song_skip_shuffle INTEGER NOT NULL,"
                            "song_play_count INTEGER NOT NULL,"
                            "song_play_time TIMESTAMP,"
                            "song_play_time_utc TIMESTAMP,"
                            "song_track_gain FLOAT NOT NULL,"
                            "song_track_peak FLOAT NOT NULL,"
                            "song_album_gain FLOAT NOT NULL,"
                            "song_album_peak FLOAT NOT NULL"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("album"))
    {
        if (!query.exec("CREATE TABLE album ("
                            "album_id SERIAL PRIMARY KEY,"
                            "album_title VARCHAR(512) NOT NULL,"
                            "album_title_sort VARCHAR(512),"
                            "UNIQUE (album_title, album_title_sort)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE album_album_id_seq RENAME TO album_seq"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO album (album_id, album_title, album_title_sort) VALUES (0, '', '')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("artist"))
    {
        if (!query.exec("CREATE TABLE artist ("
                            "artist_id SERIAL PRIMARY KEY,"
                            "artist_name character varying(512) NOT NULL,"
                            "artist_name_sort character varying(512),"
                            "UNIQUE (artist_name, artist_name_sort)"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE artist_artist_id_seq RENAME TO artist_seq"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO artist (artist_id, artist_name, artist_name_sort) VALUES (0, '', '')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("genre"))
    {
        if (!query.exec("CREATE TABLE genre ("
                            "genre_id SERIAL PRIMARY KEY,"
                            "genre_name VARCHAR(512) NOT NULL UNIQUE"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE genre_genre_id_seq RENAME TO genre_seq"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        if (!query.exec("INSERT INTO genre (genre_id, genre_name) VALUES (0, '')"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("play"))
    {
        if (!query.exec("CREATE TABLE play ("
                            "play_id SERIAL PRIMARY KEY,"
                            "song_id INTEGER NOT NULL,"
                            "play_time TIMESTAMP,"
                            "play_time_utc TIMESTAMP"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("libpath"))
    {
        if (!query.exec("CREATE TABLE libpath ("
                            "path_id SERIAL PRIMARY KEY,"
                            "path_location VARCHAR(512) NOT NULL UNIQUE,"
                            "path_keep_organized INTEGER NOT NULL,"
                            "path_format VARCHAR(512) NOT NULL,"
                            "path_format_items TEXT"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE libpath_path_id_seq RENAME TO libpath_seq"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }

    if (!tables.contains("equalizer"))
    {
        if (!query.exec("CREATE TABLE equalizer ("
                            "equalizer_id SERIAL PRIMARY KEY,"
                            "equalizer_name character varying(512) NOT NULL,"
                            "equalizer_val0 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val1 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val2 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val3 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val4 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val5 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val6 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val7 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val8 double precision NOT NULL DEFAULT 1.0,"
                            "equalizer_val9 double precision NOT NULL DEFAULT 1.0"
                        ")"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        // Renommage de la séquence
        if (!query.exec("ALTER TABLE equalizer_equalizer_id_seq RENAME TO equalizer_seq"))
        {
            logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }
    }
}

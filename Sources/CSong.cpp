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

#include "CSong.hpp"
#include "CApplication.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QDir>

// TagLib
#include <fileref.h>
#include <tag.h>
#include <flacfile.h>
#include <mpegfile.h>
#include <vorbisfile.h>
#include <tmap.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <apetag.h>
#include <xiphcomment.h>
#include <commentsframe.h>
#include <unsynchronizedlyricsframe.h>
#include <textidentificationframe.h>

// Debug
#include <QtDebug>


CSong::TSongInfos::TSongInfos(void) :
    isEnabled       (true),
    title           (""),
    subTitle        (""),
    grouping        (""),
    artistName      (""),
    albumTitle      (""),
    albumArtist     (""),
    composer        (""),
    titleSort       (""),
    artistNameSort  (""),
    albumTitleSort  (""),
    albumArtistSort (""),
    composerSort    (""),
    year            (0),
    trackNumber     (0),
    trackCount      (0),
    discNumber      (0),
    discCount       (0),
    genre           (""),
    rating          (0),
    comments        (""),
    bpm             (0),
    lyrics          (""),
    language        (LangUnknown),
    lyricist        (""),
    compilation     (false),
    skipShuffle     (false),
    trackGain       (std::numeric_limits<float>::infinity()),
    trackPeak       (std::numeric_limits<float>::infinity()),
    albumGain       (std::numeric_limits<float>::infinity()),
    albumPeak       (std::numeric_limits<float>::infinity())
{

}


CSong::TSongProperties::TSongProperties(void) :
    fileName    (QString()),
    fileSize    (0),
    bitRate     (0),
    sampleRate  (0),
    format      (CSong::FormatUnknown),
    numChannels (0),
    duration    (0)
{

};


/**
 * Crée un nouveau morceau invalide.
 *
 * \todo Supprimer ou rendre privée ?
 */

CSong::CSong(CApplication * application) :
    QObject         (application),
    m_application   (application),
    m_sound         (NULL),
    m_channel       (NULL),
    m_isModified    (false),
    m_needWriteTags (false),
    m_id            (-1),
    m_fileStatus    (true)
{
    Q_CHECK_PTR(application);
}


/**
 * Crée un nouveau morceau en le chargeant depuis un fichier.
 * Si le fichier est présent en base de données, les données du morceau sont
 * chargées depuis le base, sinon, les données sont lues depuis les métadonnées
 * du morceau.
 *
 * \param fileName Nom du fichier contenant le son son à charger.
 */

CSong::CSong(const QString& fileName, CApplication * application) :
    QObject         (application),
    m_application   (application),
    m_sound         (NULL),
    m_channel       (NULL),
    m_isModified    (false),
    m_needWriteTags (false),
    m_id            (-1),
    m_fileStatus    (true)
{
    Q_CHECK_PTR(application);

    m_properties.fileName = fileName;
    m_id = getId(application, fileName);

    if (m_id >= 0)
        loadFromDatabase();
    else
        loadTags();
}


/**
 * Arrête la lecture du morceau et libère les ressources.
 */

CSong::~CSong()
{
    stop();

    if (m_sound)
    {
        m_sound->release();
        m_sound = NULL;
    }

    m_channel = NULL;

    // Mise à jour du fichier
    if (m_needWriteTags)
        writeTags();

    // Mise à jour de la base de données
    updateDatabase();
}


/**
 * Charge les informations du morceau à partir la base de données.
 * Si le morceau n'existe pas en base de données, rien n'est modifié.
 */

void CSong::loadFromDatabase(void)
{
    if (m_id <= 0)
        return;

    QSqlQuery query(m_application->getDataBase());

    query.prepare("SELECT "
                      "song_filename,"
                      "song_filesize,"
                      "song_bitrate,"
                      "song_sample_rate,"
                      "song_format,"
                      "song_channels, "
                      "song_duration,"
                      "song_creation,"
                      "song_modification,"
                      "song_enabled,"
                      "song_title,"
                      "song_title_sort,"
                      "song_artist.artist_name,"
                      "song_artist.artist_name_sort,"
                      "album_title,"
                      "album_title_sort,"
                      "album_subtitle,"
                      "album_grouping,"
                      "album_artist.artist_name,"
                      "album_artist.artist_name_sort,"
                      "song_composer,"
                      "song_composer_sort,"
                      "song_year,"
                      "song_track_number,"
                      "song_track_count,"
                      "song_disc_number,"
                      "song_disc_count,"
                      "genre_name,"
                      "song_rating,"
                      "song_comments,"
                      "song_lyrics,"
                      "song_language,"
                      "song_lyricist,"
                      "song_compilation,"
                      "song_skip_shuffle,"
                      "song_track_gain,"
                      "song_track_peak,"
                      "song_album_gain,"
                      "song_album_peak"
                  " FROM song"
                  " NATURAL JOIN artist AS song_artist"
                  " NATURAL JOIN album"
                  " NATURAL JOIN genre"
                  " LEFT JOIN artist AS album_artist ON album_artist.artist_id = album_artist_id"
                  " WHERE id = ?");

    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    if (!query.next())
    {
        m_application->logError(tr("invalid identifier (%1)").arg(m_id), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    int numValue = 0;

    m_propertiesDB.fileName    = query.value(numValue++).toString();
    m_propertiesDB.fileSize    = query.value(numValue++).toLongLong();
    m_propertiesDB.bitRate     = query.value(numValue++).toInt();
    m_propertiesDB.sampleRate  = query.value(numValue++).toInt();
    m_propertiesDB.format      = getFormatFromInteger(query.value(numValue++).toInt());
    m_propertiesDB.numChannels = query.value(numValue++).toInt();
    m_propertiesDB.duration    = query.value(numValue++).toInt();

    m_properties = m_propertiesDB;

    m_creation               = query.value(numValue++).toDateTime();
    m_modification           = query.value(numValue++).toDateTime();

    m_infosDB.isEnabled        = query.value(numValue++).toBool();
    m_infosDB.title            = query.value(numValue++).toString();
    m_infosDB.titleSort        = query.value(numValue++).toString();
    m_infosDB.subTitle         = query.value(numValue++).toString();
    m_infosDB.grouping         = query.value(numValue++).toString();
    m_infosDB.artistName       = query.value(numValue++).toString();
    m_infosDB.artistNameSort   = query.value(numValue++).toString();
    m_infosDB.albumTitle       = query.value(numValue++).toString();
    m_infosDB.albumTitleSort   = query.value(numValue++).toString();
    m_infosDB.albumArtist      = query.value(numValue++).toString();
    m_infosDB.albumArtistSort  = query.value(numValue++).toString();
    m_infosDB.composer         = query.value(numValue++).toString();
    m_infosDB.composerSort     = query.value(numValue++).toString();
    m_infosDB.year             = query.value(numValue++).toInt();
    m_infosDB.trackNumber      = query.value(numValue++).toInt();
    m_infosDB.trackCount       = query.value(numValue++).toInt();
    m_infosDB.discNumber       = query.value(numValue++).toInt();
    m_infosDB.discCount        = query.value(numValue++).toInt();
    m_infosDB.genre            = query.value(numValue++).toString();
    m_infosDB.rating           = query.value(numValue++).toInt();
    m_infosDB.comments         = query.value(numValue++).toString();
    m_infosDB.lyrics           = query.value(numValue++).toString();
    m_infosDB.language         = getLanguageForISO2Code(query.value(numValue++).toString());
    m_infosDB.lyricist         = query.value(numValue++).toString();
    m_infosDB.compilation      = query.value(numValue++).toBool();
    m_infosDB.skipShuffle      = query.value(numValue++).toBool();

    // Replay Gain
    QVariant v;

    v = query.value(numValue++);
    m_infosDB.trackGain = (v.isNull() ? std::numeric_limits<float>::infinity() : v.toFloat());
    v = query.value(numValue++);
    m_infosDB.trackPeak = (v.isNull() ? std::numeric_limits<float>::infinity() : v.toFloat());
    v = query.value(numValue++);
    m_infosDB.albumGain = (v.isNull() ? std::numeric_limits<float>::infinity() : v.toFloat());
    v = query.value(numValue++);
    m_infosDB.albumPeak = (v.isNull() ? std::numeric_limits<float>::infinity() : v.toFloat());

    m_infos = m_infosDB;

    // Lectures
    query.prepare("SELECT play_time, play_time_utc FROM play WHERE song_id = ? ORDER BY play_time_utc ASC");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    while (query.next())
    {
        TSongPlay playTime;
        playTime.time = query.value(0).toDateTime();
        playTime.timeUTC = query.value(1).toDateTime();
        playTime.timeUTC.setTimeSpec(Qt::UTC);

        if (playTime.time.isNull())
            m_plays.append(playTime);
        else
            m_plays.prepend(playTime);
    }
}


/**
 * Charge les informations du morceau à partir de ses métadonnées.
 *
 * \param readProperties Indique si on doit lire les propriétés du fichier.
 * \return Booléen valant true si les tags ont pu être lus, false sinon.
 */

bool CSong::loadTags(bool readProperties)
{
    switch (m_properties.format)
    {
        default:
            m_application->logError(tr("Unknown format"), __FUNCTION__, __FILE__, __LINE__);
            return false;

        case CSong::FormatMP3:
        {

#ifdef Q_OS_WIN32
            std::wstring fileNameWString = m_properties.fileName.toStdWString();
            TagLib::MPEG::File file(fileNameWString.c_str(), readProperties);
#else
            TagLib::MPEG::File file(qPrintable(m_properties.fileName), readProperties);
#endif

            if (!file.isValid())
            {
                m_fileStatus = false;
                m_application->logError(tr("can't read the MP3 file \"%1\"").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);
                return false;
            }

            m_fileStatus = true;
            setFileSize(file.length());

            // Propriétés du morceau
            if (readProperties)
            {
                if (file.audioProperties())
                {
                    //TODO: Récupérer la version de MPEG : file.audioProperties()->version();
                    //TODO: Récupérer le mode stéréo : file.audioProperties()->channelMode();
                    setBitRate(file.audioProperties()->bitrate());
                    setSampleRate(file.audioProperties()->sampleRate());
                    setNumChannels(file.audioProperties()->channels());
                }
                else
                {
                    m_application->logError(tr("impossible de récupérer les propriétés du fichier %1").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);
                }
            }

            // Réinitialisation de Replay Gain
            setTrackGain(std::numeric_limits<float>::infinity());
            setTrackPeak(std::numeric_limits<float>::infinity());
            setAlbumGain(std::numeric_limits<float>::infinity());
            setAlbumPeak(std::numeric_limits<float>::infinity());

            loadTags(file.APETag(false));
            loadTags(file.ID3v1Tag(true));
            loadTags(file.ID3v2Tag(true));

            break;
        }

        case CSong::FormatOGG:
        {

#ifdef Q_OS_WIN32
            std::wstring fileNameWString = m_properties.fileName.toStdWString();
            TagLib::Ogg::Vorbis::File file(fileNameWString.c_str(), readProperties);
#else
            TagLib::Ogg::Vorbis::File file(qPrintable(m_properties.fileName), readProperties);
#endif

            if (!file.isValid())
            {
                m_fileStatus = false;
                m_application->logError(tr("can't read the Ogg file \"%1\"").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);
                return false;
            }

            m_fileStatus = true;
            setFileSize(file.length());

            // Propriétés du morceau
            if (readProperties)
            {
                if (file.audioProperties())
                {
                    setBitRate(file.audioProperties()->bitrate());
                    setSampleRate(file.audioProperties()->sampleRate());
                    setNumChannels(file.audioProperties()->channels());
                }
                else
                {
                    m_application->logError(tr("can't get properties of the file \"%1\"").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);
                }
            }

            // Réinitialisation de Replay Gain
            setTrackGain(std::numeric_limits<float>::infinity());
            setTrackPeak(std::numeric_limits<float>::infinity());
            setAlbumGain(std::numeric_limits<float>::infinity());
            setAlbumPeak(std::numeric_limits<float>::infinity());

            loadTags(file.tag());

            break;
        }

        case CSong::FormatFLAC:
        {

#ifdef Q_OS_WIN32
            std::wstring fileNameWString = m_properties.fileName.toStdWString();
            TagLib::FLAC::File file(fileNameWString.c_str(), readProperties);
#else
            TagLib::FLAC::File file(qPrintable(m_properties.fileName), readProperties);
#endif

            if (!file.isValid())
            {
                m_fileStatus = false;
                m_application->logError(QString("impossible de lire le fichier FLAC \"%1\"").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);
                return false;
            }

            m_fileStatus = true;
            setFileSize(file.length());

            // Propriétés du morceau
            if (readProperties)
            {
                if (file.audioProperties())
                {
                    setBitRate(file.audioProperties()->bitrate());
                    setSampleRate(file.audioProperties()->sampleRate());
                    setNumChannels(file.audioProperties()->channels());
                }
                else
                {
                    m_application->logError(QString("impossible de récupérer les propriétés du fichier \"%1\"").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);
                }
            }

            // Réinitialisation de Replay Gain
            setTrackGain(std::numeric_limits<float>::infinity());
            setTrackPeak(std::numeric_limits<float>::infinity());
            setAlbumGain(std::numeric_limits<float>::infinity());
            setAlbumPeak(std::numeric_limits<float>::infinity());

            loadTags(file.ID3v1Tag(true));
            loadTags(file.ID3v2Tag(true));
            loadTags(file.xiphComment(true));

            break;
        }
    }

    m_isModified = true;
    return true;
}


/**
 * Met à jour les métadonnées du fichier.
 * Pour le format MP3, les tags ID3v1 et ID3v2 sont écrits, et les tags APE enlevés.
 * Pour le format FLAC, les tags xiphComment sont écrits.
 * Seuls les tags gérés par l'application sont mis à jour, les autres sont conservés.
 *
 * \todo Vérifier que la taille est bien mise à jour.
 *
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::writeTags(void)
{
    switch (m_properties.format)
    {
        default:
            m_application->logError(tr("unknown format"), __FUNCTION__, __FILE__, __LINE__);
            return false;

        case CSong::FormatMP3:
        {

#ifdef Q_OS_WIN32
            std::wstring fileNameWString = m_properties.fileName.toStdWString();
            TagLib::MPEG::File file(fileNameWString.c_str(), false);
#else
            TagLib::MPEG::File file(qPrintable(m_properties.fileName), false);
#endif

            if (!file.isValid())
            {
                m_fileStatus = false;
                m_application->logError(tr("file \"%1\" can't be opened").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);
                m_needWriteTags = true;
                return false;
            }

            m_fileStatus = true;

            if (file.readOnly())
            {
                m_application->logError(tr("the file \"%1\" is open in read-only").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);
                m_needWriteTags = true;
                return false;
            }

            QFile * logFile = m_application->getLogFile("metadata");

            //writeTags(file.APETag(false), m_infos, logFile, m_fileName);
            writeTags(file.ID3v1Tag(true), m_infos, logFile, m_properties.fileName);
            writeTags(file.ID3v2Tag(true), m_infos, logFile, m_properties.fileName);

            file.save(TagLib::MPEG::File::ID3v1 | TagLib::MPEG::File::ID3v2, true);
            setFileSize(file.length());

            break;
        }

        case CSong::FormatOGG:
        {

#ifdef Q_OS_WIN32
            std::wstring fileNameWString = m_properties.fileName.toStdWString();
            TagLib::Ogg::Vorbis::File file(fileNameWString.c_str(), false);
#else
            TagLib::Ogg::Vorbis::File file(qPrintable(m_properties.fileName), false);
#endif

            if (!file.isValid())
            {
                m_fileStatus = false;
                m_application->logError(tr("can't read the Ogg file \"%1\"").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);
                m_needWriteTags = true;
                return false;
            }

            m_fileStatus = true;

            if (file.readOnly())
            {
                m_application->logError(tr("the file \"%1\" is open in read-only"), __FUNCTION__, __FILE__, __LINE__);
                m_needWriteTags = true;
                return false;
            }

            writeTags(file.tag(), m_infos, m_application->getLogFile("metadata"), m_properties.fileName);

            file.save();
            setFileSize(file.length());

            break;
        }

        case CSong::FormatFLAC:
        {

#ifdef Q_OS_WIN32
            std::wstring fileNameWString = m_properties.fileName.toStdWString();
            TagLib::FLAC::File file(fileNameWString.c_str(), false);
#else
            TagLib::FLAC::File file(qPrintable(m_properties.fileName), false);
#endif

            if (!file.isValid())
            {
                m_fileStatus = false;
                m_application->logError(tr("can't read the FLAC file \"%1\"").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);
                m_needWriteTags = true;
                return false;
            }

            m_fileStatus = true;

            if (file.readOnly())
            {
                m_application->logError(tr("the file \"%1\" is open in read-only"), __FUNCTION__, __FILE__, __LINE__);
                m_needWriteTags = true;
                return false;
            }

            QFile * logFile = m_application->getLogFile("metadata");

            writeTags(file.ID3v1Tag(false), m_infos, logFile, m_properties.fileName);
            writeTags(file.ID3v2Tag(false), m_infos, logFile, m_properties.fileName);
            writeTags(file.xiphComment(true), m_infos, logFile, m_properties.fileName);

            file.save();
            setFileSize(file.length());

            break;
        }
    }

    updateFileInfos();
    //emit songModified();

    m_needWriteTags = false;
    return true;
}


/**
 * Déplace un fichier à partir de ses informations.
 * Cette méthode doit être appellée quand on ajoute un fichier, ou qu'on modifie l'une des informations
 * utilisées (titre, artiste, album, numéro de piste, de disque, ou année).
 *
 * \todo Supprimer les répertoires vides après déplacement des fichiers.
 *
 * \return Booléen indiquant si le déplacement a eu lieu.
 */

bool CSong::moveFile(void)
{
    // Recherche du répertoire de la médiathèque
    QString folder = m_application->getLibraryFolderFromFileName(m_properties.fileName);

    if (folder.isEmpty())
    {
        qDebug() << "CSong::moveFile: pas dans un répertoire de la médiathèque";
        return false;
    }

    QString title;
    QString artistName;
    QString albumTitle;
    QString year;
    QString trackNumber;
    QString discNumber;

    if (m_infos.title.isEmpty())
        title = m_application->getSettings()->value("Folders/TitleEmpty", tr("Unknown title")).toString();
    else
        title = m_application->getSettings()->value("Folders/TitleDefault", "%1").toString().arg(m_infos.title).left(40);

    title.replace('\\', '_');
    title.replace('/', '_');
    title.replace(':', '_');
    title.replace('*', '_');
    title.replace('?', '_');
    title.replace('"', '_');
    title.replace('<', '_');
    title.replace('>', '_');
    title.replace('|', '_');

    if (m_infos.artistName.isEmpty())
        artistName = m_application->getSettings()->value("Folders/ArtistEmpty", tr("Unknown artist")).toString();
    else
        artistName = m_application->getSettings()->value("Folders/ArtistDefault", "%1").toString().arg(m_infos.artistName).left(40);

    artistName.replace('\\', '_');
    artistName.replace('/', '_');
    artistName.replace(':', '_');
    artistName.replace('*', '_');
    artistName.replace('?', '_');
    artistName.replace('"', '_');
    artistName.replace('<', '_');
    artistName.replace('>', '_');
    artistName.replace('|', '_');

    if (m_infos.albumTitle.isEmpty())
        albumTitle = m_application->getSettings()->value("Folders/AlbumEmpty", tr("Unknown album")).toString();
    else
        albumTitle = m_application->getSettings()->value("Folders/AlbumDefault", "%1").toString().arg(m_infos.albumTitle).left(40);

    albumTitle.replace('\\', '_');
    albumTitle.replace('/', '_');
    albumTitle.replace(':', '_');
    albumTitle.replace('*', '_');
    albumTitle.replace('?', '_');
    albumTitle.replace('"', '_');
    albumTitle.replace('<', '_');
    albumTitle.replace('>', '_');
    albumTitle.replace('|', '_');

    if (m_infos.year == 0)
        year = m_application->getSettings()->value("Folders/YearEmpty", tr("")).toString();
    else
        year = m_application->getSettings()->value("Folders/YearDefault", " (%1)").toString().arg(m_infos.year);

    year.replace('\\', '_');
    year.replace('/', '_');
    year.replace(':', '_');
    year.replace('*', '_');
    year.replace('?', '_');
    year.replace('"', '_');
    year.replace('<', '_');
    year.replace('>', '_');
    year.replace('|', '_');

    if (m_infos.trackNumber <= 0)
        trackNumber = m_application->getSettings()->value("Folders/TrackEmpty", tr("")).toString();
    else if (m_infos.trackNumber < 10)
        trackNumber = m_application->getSettings()->value("Folders/TrackDefault", "%1 ").toString().arg(QString("0%1").arg(m_infos.trackNumber));
    else
        trackNumber = m_application->getSettings()->value("Folders/TrackDefault", "%1 ").toString().arg(m_infos.trackNumber);

    trackNumber.replace('\\', '_');
    trackNumber.replace('/', '_');
    trackNumber.replace(':', '_');
    trackNumber.replace('*', '_');
    trackNumber.replace('?', '_');
    trackNumber.replace('"', '_');
    trackNumber.replace('<', '_');
    trackNumber.replace('>', '_');
    trackNumber.replace('|', '_');

    if (m_infos.discNumber == 0)
        discNumber = m_application->getSettings()->value("Folders/DiscEmpty", tr("")).toString();
    else
        discNumber = m_application->getSettings()->value("Folders/DiscDefault", "%1-").toString().arg(m_infos.discNumber);

    discNumber.replace('\\', '_');
    discNumber.replace('/', '_');
    discNumber.replace(':', '_');
    discNumber.replace('*', '_');
    discNumber.replace('?', '_');
    discNumber.replace('"', '_');
    discNumber.replace('<', '_');
    discNumber.replace('>', '_');
    discNumber.replace('|', '_');

    QString pathName = m_application->getSettings()->value("Folders/Format", "%2/%3%4/%6%5%1").toString();
    pathName.replace("%1", title);
    pathName.replace("%2", artistName);
    pathName.replace("%3", albumTitle);
    pathName.replace("%4", year);
    pathName.replace("%5", trackNumber);
    pathName.replace("%6", discNumber);

    pathName = folder + '/' + pathName;

    qDebug() << "CSong::moveFile: " << pathName;

    QString extension;

    // Recherche de l'extension
    switch (m_properties.format)
    {
        default: break;
        case FormatMP3:  extension = ".mp3";  break;
        case FormatOGG:  extension = ".ogg";  break;
        case FormatFLAC: extension = ".flac"; break;
    }

    // Comparaison entre l'ancien nom et le nouveau nom
    if (m_properties.fileName == pathName + extension)
    {
        qDebug() << "CSong::moveFile: nom identique";
        return false;
    }

    // Ajout d'un suffixe si nécessaire
    if (QFileInfo(pathName + extension).exists())
    {
        int num = 1;

        while (QFileInfo(pathName + ' ' + QString::number(num) + extension).exists())
        {
            ++num;
        }

        pathName += ' ' + QString::number(num);
    }

    pathName += extension;

    // Déplacement du fichier
    QFile file(m_properties.fileName);

    // Création des répertoires si nécessaire
    QDir dir;
    dir.mkpath(QFileInfo(pathName).path());

    if (!file.rename(pathName))
    {
        qDebug() << "CSong::moveFile: can't rename";
        return false;
    }

    // Mise-à-jour de la base de données
    QSqlQuery query(m_application->getDataBase());
    query.prepare("UPDATE song SET song_filename = ? WHERE song_id = ?");
    query.bindValue(0, pathName);
    query.bindValue(1, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return false;
    }

    m_properties.fileName = pathName;
    m_propertiesDB.fileName = pathName;

    return true;
}


/**
 * Récupère l'image illustrant le morceau.
 *
 * \todo Implémentation.
 */

QImage CSong::getCoverImage(void) const
{
    QString fileName;

    QFileInfo fileInfo(m_properties.fileName);
    QString pathName = fileInfo.path().replace('\\', '/') + '/';

    if (QFileInfo(pathName + "cover.jpg").exists())
        fileName = pathName + "cover.jpg";
    else if (QFileInfo(pathName + "cover.jpeg").exists())
        fileName = pathName + "cover.jpeg";
    else if (QFileInfo(pathName + "cover.png").exists())
        fileName = pathName + "cover.png";

    else if (QFileInfo(pathName + "folder.jpg").exists())
        fileName = pathName + "folder.jpg";
    else if (QFileInfo(pathName + "folder.jpeg").exists())
        fileName = pathName + "folder.jpeg";
    else if (QFileInfo(pathName + "folder.png").exists())
        fileName = pathName + "folder.png";

    if (fileName.isEmpty())
    {
        return QImage();
    }

    return QImage(fileName).scaled(120, 120, Qt::KeepAspectRatio);
}


/**
 * Indique si le morceau correspond à un filtre de recherche.
 * La recherche est effectuée sur toutes les chaines de caractère.
 *
 * \param filter Filtre de recherche.
 * \return Booléen.
 */

bool CSong::matchFilter(const QString& filter) const
{
    return (m_properties.fileName   .contains(filter, Qt::CaseInsensitive) ||
            m_infos.title           .contains(filter, Qt::CaseInsensitive) ||
            m_infos.artistName      .contains(filter, Qt::CaseInsensitive) ||
            m_infos.albumTitle      .contains(filter, Qt::CaseInsensitive) ||
            m_infos.albumArtist     .contains(filter, Qt::CaseInsensitive) ||
            m_infos.genre           .contains(filter, Qt::CaseInsensitive) ||
            m_infos.comments        .contains(filter, Qt::CaseInsensitive) ||
            m_infos.lyrics          .contains(filter, Qt::CaseInsensitive) ||
            m_infos.composer        .contains(filter, Qt::CaseInsensitive) ||
            m_infos.titleSort       .contains(filter, Qt::CaseInsensitive) ||
            m_infos.artistNameSort  .contains(filter, Qt::CaseInsensitive) ||
            m_infos.albumTitleSort  .contains(filter, Qt::CaseInsensitive) ||
            m_infos.albumArtistSort .contains(filter, Qt::CaseInsensitive) ||
            m_infos.composerSort    .contains(filter, Qt::CaseInsensitive) ||
            m_infos.lyricist        .contains(filter, Qt::CaseInsensitive) ||
            m_infos.grouping        .contains(filter, Qt::CaseInsensitive) ||
            m_infos.subTitle        .contains(filter, Qt::CaseInsensitive));
}


/**
 * Retourne la position de lecture du morceau.
 *
 * \return Position de lecture, ou -1 si le morceau n'est pas en cours de lecture.
 */

int CSong::getPosition(void) const
{
    if (m_channel)
    {
        unsigned int pos;
        FMOD_RESULT res = m_channel->getPosition(&pos, FMOD_TIMEUNIT_MS);
        return (res == FMOD_OK ? pos : -1);
    }

    return -1;
}


/**
 * Indique si la lecture du morceau est terminée.
 *
 * \return Booléen.
 */

bool CSong::isEnded(void) const
{
    if (m_channel)
    {
        bool isPlaying;
        FMOD_RESULT res = m_channel->isPlaying(&isPlaying);
        return (res == FMOD_OK && !isPlaying);
    }

    return false;
}


/**
 * Cherche l'identifiant d'un fichier en base de données.
 *
 * \param fileName Nom du fichier, doit être un chemin absolu simplifié.
 * \return Identifiant du fichier, ou -1 s'il n'existe pas.
 */

int CSong::getId(CApplication * application, const QString& fileName)
{
    QSqlQuery query(application->getDataBase());
    query.prepare("SELECT song_id FROM song WHERE song_filename = ?");
    query.bindValue(0, fileName);

    if (!query.exec())
    {
        application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (query.next())
    {
        return query.value(0).toInt();
    }

#ifdef Q_OS_WIN32

    query.prepare("SELECT song_id FROM song WHERE song_filename ILIKE ?");
    query.bindValue(0, fileName);

    if (!query.exec())
    {
        application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return -1;
    }

    if (query.next())
    {
        int songId = query.value(0).toInt();

        // Mise-à-jour du nom de fichier sous Windows (insensible à la casse)
        query.prepare("UPDATE song SET song_filename = ? WHERE song_id = ?");
        query.bindValue(0, fileName);
        query.bindValue(1, songId);

        if (!query.exec())
        {
            application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        }

        return songId;
    }

#endif

    return -1;
}


/**
 * Crée un morceau à partir d'un fichier.
 * Si le fichier est déjà présent en base de données, la méthode retourne NULL.
 *
 * \param fileName Fichier à lire.
 * \return Pointeur sur le son crée, ou NULL en cas d'erreur.
 */

CSong * CSong::loadFromFile(CApplication * application, const QString& fileName)
{
    if (getId(application, fileName) >= 0)
    {
        application->logError(tr("le fichier %1 est déjà dans la médiathèque").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
        return NULL;
    }

    FMOD_RESULT res;
    FMOD::Sound * sound;

    // Chargement du son
    res = application->getSoundSystem()->createStream(qPrintable(fileName), FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, NULL, &sound);

    if (res != FMOD_OK || !sound)
    {
        application->logError(tr("erreur lors du chargement du fichier %1 avec FMOD").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
        return NULL;
    }

    // Création du morceau
    CSong * song = new CSong(application);

    song->m_isModified = true;
    //song->m_sound = sound;
    song->m_properties.fileName = fileName;

    // Recherche de la durée du morceau
    FMOD_SOUND_TYPE type;

    res = sound->getLength(reinterpret_cast<unsigned int *>(&(song->m_properties.duration)), FMOD_TIMEUNIT_MS);
    if (res != FMOD_OK)
    {
        application->logError(tr("can't compute song duration for file \"%1\"").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
        song->m_properties.duration = 0;
    }

    // Recherche du format du morceau
    res = sound->getFormat(&type, NULL, NULL, NULL);

    if (res != FMOD_OK)
    {
        application->logError(tr("impossible de déterminer le format du morceau %1").arg(fileName), __FUNCTION__, __FILE__, __LINE__);
    }
    else
    {
        switch (type)
        {
            default:
                application->logError(tr("unknown format"), __FUNCTION__, __FILE__, __LINE__);
                delete song;
                return NULL;

            case FMOD_SOUND_TYPE_MPEG:
                song->m_properties.format = CSong::FormatMP3;
                break;

            case FMOD_SOUND_TYPE_OGGVORBIS:
                song->m_properties.format = CSong::FormatOGG;
                break;

            case FMOD_SOUND_TYPE_FLAC:
                song->m_properties.format = CSong::FormatFLAC;
                break;
        }
    }

    sound->release();

    // Chargement des métadonnées
    if (!song->loadTags(true))
    {
        delete song;
        return NULL;
    }

    song->updateDatabase();
    //song->moveFile();

    connect(song, SIGNAL(songModified()), application, SLOT(onSongModified()));

    return song;
}


/**
 * Charge tous les morceaux depuis la base de données.
 *
 * \return Liste des morceaux.
 */

QList<CSong *> CSong::loadAllSongsFromDatabase(CApplication * application)
{
    QList<CSong *> songList;

    QSqlQuery query(application->getDataBase());

    // Remarque : on peut éventuellement différer le chargement des lectures de chaque morceau
    QSqlQuery query2(application->getDataBase());
    query2.prepare("SELECT play_time, play_time_utc FROM play WHERE song_id = ? ORDER BY play_time_utc ASC");

    // Liste des morceaux
    if (!query.exec("SELECT "
                        "song_id,"
                        "song_filename,"
                        "song_filesize,"
                        "song_bitrate,"
                        "song_sample_rate,"
                        "song_format,"
                        "song_channels,"
                        "song_duration,"
                        "song_creation,"
                        "song_modification,"
                        "song_enabled,"
                        "song_title,"
                        "song_title_sort,"
                        "song_subtitle,"
                        "song_grouping,"
                        "song_artist.artist_name,"
                        "song_artist.artist_name_sort,"
                        "album_title,"
                        "album_title_sort, "
                        "album_artist.artist_name,"
                        "album_artist.artist_name_sort,"
                        "song_composer,"
                        "song_composer_sort,"
                        "song_year,"
                        "song_track_number,"
                        "song_track_count,"
                        "song_disc_number,"
                        "song_disc_count,"
                        "genre_name,"
                        "song_rating,"
                        "song_comments,"
                        "song_bpm,"
                        "song_lyrics,"
                        "song_language,"
                        "song_lyricist,"
                        "song_compilation,"
                        "song_skip_shuffle,"
                        "song_track_gain,"
                        "song_track_peak,"
                        "song_album_gain,"
                        "song_album_peak"
                    " FROM song"
                    " NATURAL JOIN artist AS song_artist"
                    " NATURAL JOIN album"
                    " NATURAL JOIN genre"
                    " LEFT JOIN artist AS album_artist ON album_artist.artist_id = album_artist_id"))
    {
        application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return songList;
    }

    while (query.next())
    {
        CSong * song = new CSong(application);

        int numValue = 0;

        song->m_id                       = query.value(numValue++).toInt();
        song->m_propertiesDB.fileName    = query.value(numValue++).toString();
        song->m_propertiesDB.fileSize    = query.value(numValue++).toLongLong();
        song->m_propertiesDB.bitRate     = query.value(numValue++).toInt();
        song->m_propertiesDB.sampleRate  = query.value(numValue++).toInt();
        song->m_propertiesDB.format      = CSong::getFormatFromInteger(query.value(numValue++).toInt());
        song->m_propertiesDB.numChannels = query.value(numValue++).toInt();
        song->m_propertiesDB.duration    = query.value(numValue++).toInt();
        song->m_creation                 = query.value(numValue++).toDateTime();
        song->m_modification             = query.value(numValue++).toDateTime();

        song->m_infosDB.isEnabled        = query.value(numValue++).toBool();
        song->m_infosDB.title            = query.value(numValue++).toString();
        song->m_infosDB.titleSort        = query.value(numValue++).toString();
        song->m_infosDB.subTitle         = query.value(numValue++).toString();
        song->m_infosDB.grouping         = query.value(numValue++).toString();
        song->m_infosDB.artistName       = query.value(numValue++).toString();
        song->m_infosDB.artistNameSort   = query.value(numValue++).toString();
        song->m_infosDB.albumTitle       = query.value(numValue++).toString();
        song->m_infosDB.albumTitleSort   = query.value(numValue++).toString();
        song->m_infosDB.albumArtist      = query.value(numValue++).toString();
        song->m_infosDB.albumArtistSort  = query.value(numValue++).toString();
        song->m_infosDB.composer         = query.value(numValue++).toString();
        song->m_infosDB.composerSort     = query.value(numValue++).toString();
        song->m_infosDB.year             = query.value(numValue++).toInt();
        song->m_infosDB.trackNumber      = query.value(numValue++).toInt();
        song->m_infosDB.trackCount       = query.value(numValue++).toInt();
        song->m_infosDB.discNumber       = query.value(numValue++).toInt();
        song->m_infosDB.discCount        = query.value(numValue++).toInt();
        song->m_infosDB.genre            = query.value(numValue++).toString();
        song->m_infosDB.rating           = query.value(numValue++).toInt();
        song->m_infosDB.comments         = query.value(numValue++).toString();
        song->m_infosDB.bpm              = query.value(numValue++).toInt();
        song->m_infosDB.lyrics           = query.value(numValue++).toString();
        song->m_infosDB.language         = getLanguageForISO2Code(query.value(numValue++).toString());
        song->m_infosDB.lyricist         = query.value(numValue++).toString();
        song->m_infosDB.compilation      = query.value(numValue++).toBool();
        song->m_infosDB.skipShuffle      = query.value(numValue++).toBool();

        // Replay Gain
        QVariant v;

        v = query.value(numValue++);
        song->m_infosDB.trackGain = (v.isNull() ? std::numeric_limits<float>::infinity() : v.toFloat());
        v = query.value(numValue++);
        song->m_infosDB.trackPeak = (v.isNull() ? std::numeric_limits<float>::infinity() : v.toFloat());
        v = query.value(numValue++);
        song->m_infosDB.albumGain = (v.isNull() ? std::numeric_limits<float>::infinity() : v.toFloat());
        v = query.value(numValue++);
        song->m_infosDB.albumPeak = (v.isNull() ? std::numeric_limits<float>::infinity() : v.toFloat());

        // Lectures
        query2.bindValue(0, song->m_id);

        if (!query2.exec())
        {
            application->showDatabaseError(query2.lastError().text(), query2.lastQuery(), __FILE__, __LINE__);
        }
        else
        {
            while (query2.next())
            {
                TSongPlay playTime;
                playTime.time = query2.value(0).toDateTime();
                playTime.timeUTC = query2.value(1).toDateTime();
                playTime.timeUTC.setTimeSpec(Qt::UTC);

                if (playTime.time.isNull())
                    song->m_plays.append(playTime);
                else
                    song->m_plays.prepend(playTime);
            }
        }

        song->m_properties = song->m_propertiesDB;
        song->m_infos = song->m_infosDB;

        connect(song, SIGNAL(songModified()), application, SLOT(onSongModified()));
        songList.append(song);
    }

    return songList;
}


/**
 * Convertit un nombre d'octets en chaine de caractères plus compréhensible.
 *
 * \param fileSize Nombre d'octets.
 * \return Chaine de caractère utilisant les préfixes binaires (Kio, Mio, Gio, pour
 *         respectivement 1024, 1024*1024, et 1024*1024*1024 octets).
 */

QString CSong::getFileSize(int fileSize)
{
    Q_ASSERT(fileSize >= 0);

    // Plus de 1 Gio
    if (fileSize >= 1024 * 1024 * 1024)
    {
        float fileSizeDisplay = static_cast<float>(static_cast<int>(static_cast<float>(10 * fileSize) / (1024*1024*1024))) / 10;
        return tr("%1 Gio").arg(fileSizeDisplay);
    }
    // Plus de 1 Mio
    else if (fileSize >= 1024 * 1024)
    {
        float fileSizeDisplay = static_cast<float>(static_cast<int>(static_cast<float>(10 * fileSize) / (1024*1024))) / 10;
        return tr("%1 Mio").arg(fileSizeDisplay);
    }
    // Moins de 1 Kio
    else if (fileSize >= 1024)
    {
        float fileSizeDisplay = static_cast<float>(static_cast<int>(static_cast<float>(10 * fileSize) / 1024)) / 10;
        return tr("%1 Kio").arg(fileSizeDisplay);
    }
    // Moins de 1 Kio
    else
    {
        return tr("%n byte(s)", "", fileSize);
    }
}


void CSong::setEnabled(bool enabled)
{
    if (m_infos.isEnabled != enabled)
    {
        m_infos.isEnabled = enabled;
        m_isModified = true;
    }
}


/**
 * Modifie le titre du morceau.
 *
 * \param title Nouveau titre du morceau.
 */

void CSong::setTitle(const QString& title)
{
    if (m_infos.title != title)
    {
        m_infos.title = title;
        m_isModified = true;
    }
}


/**
 * Modifie le sous-titre du morceau.
 *
 * \param subTitle Nouveau sous-titre du morceau.
 */

void CSong::setSubTitle(const QString& subTitle)
{
    if (m_infos.subTitle != subTitle)
    {
        m_infos.subTitle = subTitle;
        m_isModified = true;
    }
}


/**
 * Modifie le regroupement du morceau.
 *
 * \param grouping Nouveau regroupement du morceau.
 */

void CSong::setGrouping(const QString& grouping)
{
    if (m_infos.grouping != grouping)
    {
        m_infos.grouping = grouping;
        m_isModified = true;
    }
}


/**
 * Modifie le nom de l'artiste du morceau.
 *
 * \param artistName Nom de l'artiste du morceau.
 */

void CSong::setArtistName(const QString& artistName)
{
    if (m_infos.artistName != artistName)
    {
        m_infos.artistName = artistName;
        m_isModified = true;
    }
}


/**
 * Modifie le titre de l'album du morceau.
 *
 * \param albumTitle Nouveau titre de l'album.
 */

void CSong::setAlbumTitle(const QString& albumTitle)
{
    if (m_infos.albumTitle != albumTitle)
    {
        m_infos.albumTitle = albumTitle;
        m_isModified = true;
    }
}


/**
 * Modifie l'artiste de l'album du morceau.
 *
 * \param albumArtist Artiste de l'album.
 */

void CSong::setAlbumArtist(const QString& albumArtist)
{
    if (m_infos.albumArtist != albumArtist)
    {
        m_infos.albumArtist = albumArtist;
        m_isModified = true;
    }
}


/**
 * Modifie le compositeur du morceau.
 *
 * \param composer Nouveau compositeur.
 */

void CSong::setComposer(const QString& composer)
{
    if (m_infos.composer != composer)
    {
        m_infos.composer = composer;
        m_isModified = true;
    }
}


/**
 * Modifie le titre pour le tri du morceau.
 *
 * \param title Nouveau titre pour le tri.
 */

void CSong::setTitleSort(const QString& title)
{
    if (m_infos.titleSort != title)
    {
        m_infos.titleSort = title;
        m_isModified = true;
    }
}


/**
 * Modifie le nom de l'artiste pour le tri du morceau.
 *
 * \param artistName Nouveau nom de l'artiste pour le tri.
 */

void CSong::setArtistNameSort(const QString& artistName)
{
    if (m_infos.artistNameSort != artistName)
    {
        m_infos.artistNameSort = artistName;
        m_isModified = true;
    }
}


/**
 * Modifie le titre de l'album pour le tri du morceau.
 *
 * \param albumTitle Nouveau titre de l'album pour le tri.
 */

void CSong::setAlbumTitleSort(const QString& albumTitle)
{
    if (m_infos.albumTitleSort != albumTitle)
    {
        m_infos.albumTitleSort = albumTitle;
        m_isModified = true;
    }
}


/**
 * Modifie le nom de l'artiste de l'album pour le tri.
 *
 * \param albumArtist Nom de l'artiste de l'album pour le tri.
 */

void CSong::setAlbumArtistSort(const QString& albumArtist)
{
    if (m_infos.albumArtistSort != albumArtist)
    {
        m_infos.albumArtistSort = albumArtist;
        m_isModified = true;
    }
}


/**
 * Modifie le compositeur pour le tri du morceau.
 *
 * \param composer Nouveau compositeur pour le tri.
 */

void CSong::setComposerSort(const QString& composer)
{
    if (m_infos.composerSort != composer)
    {
        m_infos.composerSort = composer;
        m_isModified = true;
    }
}


/**
 * Modifie l'année de sortie du morceau ou de l'album.
 *
 * \param year Nouvelle année de sortie.
 */

void CSong::setYear(int year)
{
    Q_ASSERT(year >= 0);

    if (m_infos.year != year)
    {
        m_infos.year = year;
        m_isModified = true;
    }
}


/**
 * Modifie le numéro de piste du morceau.
 *
 * \param trackNumber Numéro de piste.
 */

void CSong::setTrackNumber(int trackNumber)
{
    Q_ASSERT(trackNumber >= 0);

    if (m_infos.trackNumber != trackNumber)
    {
        m_infos.trackNumber = trackNumber;
        m_isModified = true;
    }
}


/**
 * Modifie le nombre de pistes de l'album.
 *
 * \param trackCount Nombre de pistes de l'album.
 */

void CSong::setTrackCount(int trackCount)
{
    Q_ASSERT(trackCount >= 0);

    if (m_infos.trackCount != trackCount)
    {
        m_infos.trackCount = trackCount;
        m_isModified = true;
    }
}


/**
 * Modifie le numéro de disque du morceau.
 *
 * \param discNumber Numéro de disque.
 */

void CSong::setDiscNumber(int discNumber)
{
    Q_ASSERT(discNumber >= 0);

    if (m_infos.discNumber != discNumber)
    {
        m_infos.discNumber = discNumber;
        m_isModified = true;
    }
}


/**
 * Modifie le nombre de disques de l'album.
 *
 * \param discCount Nombre de disques de l'album.
 */

void CSong::setDiscCount(int discCount)
{
    Q_ASSERT(discCount >= 0);

    if (m_infos.discCount != discCount)
    {
        m_infos.discCount = discCount;
        m_isModified = true;
    }
}


/**
 * Modifie le genre du morceau.
 *
 * \param genre Nouveau genre du morceau.
 */

void CSong::setGenre(const QString& genre)
{
    if (m_infos.genre != genre)
    {
        m_infos.genre = genre;
        m_isModified = true;
    }
}


/**
 * Modifie la note du morceau.
 *
 * \param rating Nouvelle note du morceau, entre 0 et 5.
 */

void CSong::setRating(int rating)
{
    Q_ASSERT(rating >= 0 && rating <= 5);

    if (m_infos.rating != rating)
    {
        m_infos.rating = rating;
        m_isModified = true;
    }
}


/**
 * Modifie les commentaires du morceau.
 *
 * \param comments Nouveaux commentaires.
 */

void CSong::setComments(const QString& comments)
{
    if (m_infos.comments != comments)
    {
        m_infos.comments = comments;
        m_isModified = true;
    }
}


/**
 * Modifie le nombre de battements par minute du morceau.
 *
 * \param bpm Battements par minute.
 */

void CSong::setBPM(int bpm)
{
    Q_ASSERT(bpm >= 0);

    if (m_infos.bpm != bpm)
    {
        m_infos.bpm = bpm;
        m_isModified = true;
    }
}


/**
 * Modifie les paroles du morceau.
 *
 * \param lyrics Nouvelles paroles.
 */

void CSong::setLyrics(const QString& lyrics)
{
    if (m_infos.lyrics != lyrics)
    {
        m_infos.lyrics = lyrics;
        m_isModified = true;
    }
}


/**
 * Modifie la langue du morceau.
 *
 * \param language Nouvelle langue du morceau.
 */

void CSong::setLanguage(CSong::TLanguage language)
{
    if (m_infos.language != language)
    {
        m_infos.language = language;
        m_isModified = true;
    }
}


/**
 * Modifie l'auteur des paroles du morceau.
 *
 * \param lyricist Auteur des paroles.
 */

void CSong::setLyricist(const QString& lyricist)
{
    if (m_infos.lyricist != lyricist)
    {
        m_infos.lyricist = lyricist;
        m_isModified = true;
    }
}


void CSong::setCompilation(bool compilation)
{
    if (m_infos.compilation != compilation)
    {
        m_infos.compilation = compilation;
        m_isModified = true;
    }
}


void CSong::setSkipShuffle(bool skipShuffle)
{
    if (m_infos.skipShuffle != skipShuffle)
    {
        m_infos.skipShuffle = skipShuffle;
        m_isModified = true;
    }
}


void CSong::setTrackGain(float gain)
{
    if (m_infos.trackGain != gain)
    {
        m_infos.trackGain = gain;
        m_isModified = true;
    }
}


void CSong::setTrackPeak(float peak)
{
    if (m_infos.trackPeak != peak)
    {
        m_infos.trackPeak = peak;
        m_isModified = true;
    }
}


void CSong::setAlbumGain(float gain)
{
    if (m_infos.albumGain != gain)
    {
        m_infos.albumGain = gain;
        m_isModified = true;
    }
}


void CSong::setAlbumPeak(float peak)
{
    if (m_infos.albumPeak != peak)
    {
        m_infos.albumPeak = peak;
        m_isModified = true;
    }
}


/**
 * Démarre la lecture du morceau.
 */

void CSong::startPlay(void)
{
    Q_CHECK_PTR(m_sound);

    // Rechargement des métadonnées
    loadTags(true);
    updateDatabase();

    FMOD_RESULT res;
    res = m_application->getSoundSystem()->playSound(FMOD_CHANNEL_FREE, m_sound, true, &m_channel);

    if (res == FMOD_OK && m_channel)
    {
        res = m_channel->setMute(m_application->isMute());
        Q_ASSERT(res == FMOD_OK);

        res = m_channel->setVolume(static_cast<float>(m_application->getVolume()) / 100.0f);
        Q_ASSERT(res == FMOD_OK);

        res = m_channel->setPaused(false);
        Q_ASSERT(res == FMOD_OK);

        m_fileStatus = true;
    }
    else
    {
        m_fileStatus = false;
        m_sound = NULL;
        m_channel = NULL;
    }
}


void CSong::setPosition(int position)
{
    Q_ASSERT(position >= 0);

    if (m_sound && m_channel)
    {
        m_channel->setPosition(position, FMOD_TIMEUNIT_MS);
    }
}


/**
 * Modifie le volume sonore si le morceau est en cours de lecture.
 *
 * \param volume Volume sonore, entre 0 et 100.
 */

void CSong::setVolume(int volume)
{
    Q_ASSERT(volume >= 0 && volume <= 100);

    if (m_sound && m_channel)
    {
        m_channel->setVolume(static_cast<float>(volume) / 100);
    }
}


/**
 * Coupe ou remet le son si le morceau est en cours de lecture.
 *
 * \param mute Booléen valant true si le son doit être coupé, false sinon.
 */

void CSong::setMute(bool mute)
{
    if (m_sound && m_channel)
    {
        m_channel->setMute(mute);
    }
}


/**
 * Charge le son en mémoire.
 */

bool CSong::loadSound(void)
{
    // Déjà initialisé
    if (m_sound && m_channel)
        return true;

    FMOD_RESULT res;

    // Chargement du son
    //res = m_application->getSoundSystem()->createStream(qPrintable(m_properties.fileName), FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, NULL, &m_sound);
    res = m_application->getSoundSystem()->createStream(reinterpret_cast<const char *>(m_properties.fileName.utf16()), FMOD_UNICODE | FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, NULL, &m_sound);

    if (res == FMOD_OK && m_sound)
    {
        // Durée du son
        unsigned int length;
        res = m_sound->getLength(&length, FMOD_TIMEUNIT_MS);

        if (res == FMOD_OK)
        {
            // Mise à jour de la durée du morceau
            if (m_properties.duration != static_cast<int>(length))
            {
                m_application->logError(tr("duration of song \"%1\" has to be updated"), __FUNCTION__, __FILE__, __LINE__);
                m_properties.duration = length;

                QSqlQuery query(m_application->getDataBase());

                query.prepare("UPDATE song SET song_duration = ? WHERE song_id = ?");

                query.bindValue(0, m_properties.duration);
                query.bindValue(1, m_id);

                if (!query.exec())
                {
                    m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                }
            }

            m_fileStatus = true;
            return true;
        }
        else
        {
            m_fileStatus = false;
        }
    }
    else
    {
        m_fileStatus = false;
    }

    m_application->logError(tr("file \"%1\" can't be opened with FMOD").arg(m_properties.fileName), __FUNCTION__, __FILE__, __LINE__);

    m_sound = NULL;
    m_channel = NULL;
    return false;
}


/**
 * Démarre ou reprend la lecture du morceau.
 */

void CSong::play(void)
{
    if (!m_sound)
        return;

    FMOD_RESULT res;

    if (m_channel)
    {
        bool paused;
        res = m_channel->getPaused(&paused);

        if (res == FMOD_OK && paused)
        {
            res = m_channel->setPaused(false);
            Q_ASSERT(res == FMOD_OK);
            return;
        }
    }

    startPlay();
}


/**
 * Met la lecture du morceau en pause.
 */

void CSong::pause(void)
{
    if (m_sound && m_channel)
    {
        FMOD_RESULT res = m_channel->setPaused(true);
        Q_ASSERT(res == FMOD_OK);
    }
}


/**
 * Arrête la lecture du morceau.
 */

void CSong::stop(void)
{
    if (m_sound && m_channel)
    {
        m_channel->stop();

        // Fermeture du fichier
        m_sound->release();
        m_sound = NULL;
        m_channel = NULL;

        // Écriture des métadonnées
        if (m_needWriteTags)
        {
            writeTags();
        }
    }
}


/**
 * Met à jour la base de données si les informations du morceau ont été modifiées.
 */

void CSong::updateDatabase(void)
{
    if (m_isModified && (m_properties != m_propertiesDB || m_infos != m_infosDB))
    {
        // Déplacement du fichier
        if (m_infos.title       != m_infosDB.title      ||
            m_infos.artistName  != m_infosDB.artistName ||
            m_infos.albumTitle  != m_infosDB.albumTitle ||
            m_infos.year        != m_infosDB.year       ||
            m_infos.trackNumber != m_infosDB.trackNumber||
            m_infos.discNumber  != m_infosDB.discNumber )
        {
            moveFile();
        }

        m_modification = QDateTime::currentDateTime();

        QSqlQuery query(m_application->getDataBase());

        int artistId = m_application->getArtistId(m_infos.artistName, m_infos.artistNameSort);
        if (artistId < 0) artistId = 0;
        int albumId = m_application->getAlbumId(m_infos.albumTitle, m_infos.albumTitleSort);
        if (albumId < 0) albumId = 0;
        int albumArtistId = m_application->getArtistId(m_infos.albumArtist, m_infos.albumArtistSort);
        if (albumArtistId < 0) albumArtistId = 0;
        int genreId = m_application->getGenreId(m_infos.genre);
        if (genreId < 0) genreId = 0;

        // Insertion
        if (m_id <= 0)
        {
            m_creation = m_modification;

            query.prepare("INSERT INTO song ("
                              "song_filename,"
                              "song_filesize,"
                              "song_bitrate,"
                              "song_sample_rate,"
                              "song_format,"
                              "song_channels,"
                              "song_duration,"
                              "song_creation,"
                              "song_modification,"
                              "song_enabled,"
                              "song_title,"
                              "song_title_sort,"
                              "song_subtitle,"
                              "song_grouping,"
                              "artist_id,"
                              "album_id,"
                              "album_artist_id,"
                              "song_composer,"
                              "song_composer_sort,"
                              "song_year,"
                              "song_track_number,"
                              "song_track_count,"
                              "song_disc_number,"
                              "song_disc_count,"
                              "genre_id,"
                              "song_rating,"
                              "song_comments,"
                              "song_bpm,"
                              "song_lyrics,"
                              "song_language,"
                              "song_lyricist,"
                              "song_compilation,"
                              "song_skip_shuffle,"
                              "song_play_count,"
                              "song_play_time,"
                              "song_play_time_utc,"
                              "song_track_gain,"
                              "song_track_peak,"
                              "song_album_gain,"
                              "song_album_peak"
                          ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

            int numValue = 0;

            query.bindValue(numValue++, m_properties.fileName);
            query.bindValue(numValue++, m_properties.fileSize);
            query.bindValue(numValue++, m_properties.bitRate);
            query.bindValue(numValue++, m_properties.sampleRate);
            query.bindValue(numValue++, m_properties.format);
            query.bindValue(numValue++, m_properties.numChannels);
            query.bindValue(numValue++, m_properties.duration);
            query.bindValue(numValue++, m_creation);
            query.bindValue(numValue++, m_modification);
            query.bindValue(numValue++, m_infos.isEnabled ? 1 : 0);
            query.bindValue(numValue++, m_infos.title);
            query.bindValue(numValue++, m_infos.titleSort);
            query.bindValue(numValue++, m_infos.subTitle);
            query.bindValue(numValue++, m_infos.grouping);
            query.bindValue(numValue++, artistId);
            query.bindValue(numValue++, albumId);
            query.bindValue(numValue++, albumArtistId);
            query.bindValue(numValue++, m_infos.composer);
            query.bindValue(numValue++, m_infos.composerSort);
            query.bindValue(numValue++, m_infos.year);
            query.bindValue(numValue++, m_infos.trackNumber);
            query.bindValue(numValue++, m_infos.trackCount);
            query.bindValue(numValue++, m_infos.discNumber);
            query.bindValue(numValue++, m_infos.discCount);
            query.bindValue(numValue++, genreId);
            query.bindValue(numValue++, m_infos.rating);
            query.bindValue(numValue++, m_infos.comments);
            query.bindValue(numValue++, m_infos.bpm);
            query.bindValue(numValue++, m_infos.lyrics);
            query.bindValue(numValue++, getISO2CodeForLanguage(m_infos.language));
            query.bindValue(numValue++, m_infos.lyricist);
            query.bindValue(numValue++, m_infos.compilation ? 1 : 0);
            query.bindValue(numValue++, m_infos.skipShuffle ? 1 : 0);
            query.bindValue(numValue++, 0);  // Play count
            query.bindValue(numValue++, QDateTime()); // Last play time
            query.bindValue(numValue++, QDateTime()); // Last play time UTC
            query.bindValue(numValue++, (m_infos.trackGain == std::numeric_limits<float>::infinity() ? QVariant::Invalid : m_infos.trackGain));
            query.bindValue(numValue++, (m_infos.trackPeak == std::numeric_limits<float>::infinity() ? QVariant::Invalid : m_infos.trackPeak));
            query.bindValue(numValue++, (m_infos.albumGain == std::numeric_limits<float>::infinity() ? QVariant::Invalid : m_infos.albumGain));
            query.bindValue(numValue++, (m_infos.albumPeak == std::numeric_limits<float>::infinity() ? QVariant::Invalid : m_infos.albumPeak));

            if (!query.exec())
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return;
            }

            m_propertiesDB = m_properties;
            m_infosDB = m_infos;

            if (m_application->getDataBase().driverName() == "QPSQL")
            {
                query.prepare("SELECT currval('song_song_id_seq')");

                if (!query.exec())
                {
                    m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                    return;
                }

                if (query.next())
                {
                    m_id = query.value(0).toInt();
                }
                else
                {
                    m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                    return;
                }
            }
            else
            {
                m_id = query.lastInsertId().toInt();
            }
        }
        // Mise à jour
        else
        {
            query.prepare("UPDATE song SET "
                              "song_filesize      = ?,"
                              "song_bitrate       = ?,"
                              "song_sample_rate   = ?,"
                              "song_format        = ?,"
                              "song_channels      = ?,"
                              "song_duration      = ?,"
                              "song_modification  = ?,"
                              "song_enabled       = ?,"
                              "song_title         = ?,"
                              "song_title_sort    = ?,"
                              "song_subtitle      = ?,"
                              "song_grouping      = ?,"
                              "artist_id          = ?,"
                              "album_id           = ?,"
                              "album_artist_id    = ?,"
                              "song_composer      = ?,"
                              "song_composer_sort = ?,"
                              "song_year          = ?,"
                              "song_track_number  = ?,"
                              "song_track_count   = ?,"
                              "song_disc_number   = ?,"
                              "song_disc_count    = ?,"
                              "genre_id           = ?,"
                              "song_rating        = ?,"
                              "song_comments      = ?,"
                              "song_bpm           = ?,"
                              "song_lyrics        = ?,"
                              "song_language      = ?,"
                              "song_lyricist      = ?,"
                              "song_compilation   = ?,"
                              "song_skip_shuffle  = ?,"
                              "song_track_gain    = ?,"
                              "song_track_peak    = ?,"
                              "song_album_gain    = ?,"
                              "song_album_peak    = ? "
                          "WHERE song_id = ?");

            int numValue = 0;

            query.bindValue(numValue++, m_properties.fileSize);
            query.bindValue(numValue++, m_properties.bitRate);
            query.bindValue(numValue++, m_properties.sampleRate);
            query.bindValue(numValue++, m_properties.format);
            query.bindValue(numValue++, m_properties.numChannels);
            query.bindValue(numValue++, m_properties.duration);
            query.bindValue(numValue++, m_modification);
            query.bindValue(numValue++, m_infos.isEnabled ? 1 : 0);
            query.bindValue(numValue++, m_infos.title);
            query.bindValue(numValue++, m_infos.titleSort);
            query.bindValue(numValue++, m_infos.subTitle);
            query.bindValue(numValue++, m_infos.grouping);
            query.bindValue(numValue++, artistId);
            query.bindValue(numValue++, albumId);
            query.bindValue(numValue++, albumArtistId);
            query.bindValue(numValue++, m_infos.composer);
            query.bindValue(numValue++, m_infos.composerSort);
            query.bindValue(numValue++, m_infos.year);
            query.bindValue(numValue++, m_infos.trackNumber);
            query.bindValue(numValue++, m_infos.trackCount);
            query.bindValue(numValue++, m_infos.discNumber);
            query.bindValue(numValue++, m_infos.discCount);
            query.bindValue(numValue++, genreId);
            query.bindValue(numValue++, m_infos.rating);
            query.bindValue(numValue++, m_infos.comments);
            query.bindValue(numValue++, m_infos.bpm);
            query.bindValue(numValue++, m_infos.lyrics);
            query.bindValue(numValue++, getISO2CodeForLanguage(m_infos.language));
            query.bindValue(numValue++, m_infos.lyricist);
            query.bindValue(numValue++, m_infos.compilation ? 1 : 0);
            query.bindValue(numValue++, m_infos.skipShuffle ? 1 : 0);
            query.bindValue(numValue++, m_infos.trackGain == std::numeric_limits<float>::infinity() ? QVariant(QVariant::Double) : m_infos.trackGain);
            query.bindValue(numValue++, m_infos.trackPeak == std::numeric_limits<float>::infinity() ? QVariant(QVariant::Double) : m_infos.trackPeak);
            query.bindValue(numValue++, m_infos.albumGain == std::numeric_limits<float>::infinity() ? QVariant(QVariant::Double) : m_infos.albumGain);
            query.bindValue(numValue++, m_infos.albumPeak == std::numeric_limits<float>::infinity() ? QVariant(QVariant::Double) : m_infos.albumPeak);
            query.bindValue(numValue++, m_id);

            if (!query.exec())
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return;
            }

            m_propertiesDB = m_properties;
            m_infosDB = m_infos;
        }

        m_isModified = false;
        emit songModified();
    }
}


/**
 * Met à jour le nom et la taille du fichier en base de données.
 */

void CSong::updateFileInfos(void)
{
    if (m_id <= 0)
        return;

    QSqlQuery query(m_application->getDataBase());
    query.prepare("UPDATE song SET song_filename = ?, song_filesize = ? WHERE song_id = ?");
    query.bindValue(0, m_properties.fileName);
    query.bindValue(1, m_properties.fileSize);
    query.bindValue(2, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }
}


void CSong::setFileSize(qlonglong fileSize)
{
    if (m_properties.fileSize != fileSize)
    {
        m_properties.fileSize = fileSize;
        m_isModified = true;
    }
}


void CSong::setBitRate(int bitRate)
{
    if (m_properties.bitRate != bitRate)
    {
        m_properties.bitRate = bitRate;
        m_isModified = true;
    }
}


void CSong::setSampleRate(int sampleRate)
{
    if (m_properties.sampleRate != sampleRate)
    {
        m_properties.sampleRate = sampleRate;
        m_isModified = true;
    }
}


void CSong::setNumChannels(int numChannels)
{
    if (m_properties.numChannels != numChannels)
    {
        m_properties.numChannels = numChannels;
        m_isModified = true;
    }
}


/**
 * Méthode appelée quand la lecture du morceau est terminée.
 */

void CSong::emitPlayEnd(void)
{
    const QDateTime currentTime = QDateTime::currentDateTime();
    const QDateTime currentTimeUTC = QDateTime::currentDateTimeUtc();

    QSqlQuery query(m_application->getDataBase());
    query.prepare("UPDATE song SET "
                      "song_play_count    = song_play_count + 1,"
                      "song_play_time     = ?,"
                      "song_play_time_utc = ? "
                  "WHERE song_id = ?");

    query.bindValue(0, currentTime);
    query.bindValue(1, currentTimeUTC);
    query.bindValue(2, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    query.prepare("INSERT INTO play (song_id, play_time, play_time_utc) VALUES (?, ?, ?)");
    query.bindValue(0, m_id);
    query.bindValue(1, currentTime);
    query.bindValue(2, currentTimeUTC);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        TSongPlay playTime;
        playTime.time = currentTime;
        playTime.timeUTC = currentTimeUTC;
        playTime.timeUTC.setTimeSpec(Qt::UTC);
        m_plays.prepend(playTime);
    }

    emit playEnd();
}


/**
 * Lit les informations d'un morceau depuis des tags ID3 version 1.
 *
 * \param tags Métadonnées.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::loadTags(TagLib::ID3v1::Tag * tags)
{
    if (!tags)
        return false;

    // Log
    QFile * logFile = m_application->getLogFile("metadata");
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   " << tr("Chargement des tags ID3v1") << '\n';
    stream << "----------------------------------------\n";
    stream << tr("File:") << ' ' << m_properties.fileName << '\n';
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString(tr("dd/MM/yyyy HH:mm:ss")) << '\n';

    TagLib::String str;

    // Titre
    str = tags->title();
    if (!str.isNull())
        setTitle(QString::fromUtf8(str.toCString(true)));

    // Artiste
    str = tags->artist();
    if (!str.isNull())
        setArtistName(QString::fromUtf8(str.toCString(true)));

    // Album
    str = tags->album();
    if (!str.isNull())
        setAlbumTitle(QString::fromUtf8(str.toCString(true)));

    // Année
    setYear(tags->year());

    // Numéro de piste
    setTrackNumber(tags->track());

    // Genre
    str = tags->genre();
    if (!str.isNull())
        setGenre(QString::fromUtf8(str.toCString(true)));

    // Commentaires
    str = tags->comment();
    if (!str.isNull())
        setComments(QString::fromUtf8(str.toCString(true)));

    return true;
}


/**
 * Lit les informations d'un morceau depuis des tags ID3 version 2.
 *
 * \param tags Métadonnées.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::loadTags(TagLib::ID3v2::Tag * tags)
{
    if (!tags)
        return false;

    // Log
    QFile * logFile = m_application->getLogFile("metadata");
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   " << tr("Chargement des tags ID3v2") << '\n';
    stream << "----------------------------------------\n";
    stream << tr("File:") << ' ' << m_properties.fileName << '\n';
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString(tr("dd/MM/yyyy HH:mm:ss")) << '\n';
    stream << "----------------------------------------\n";

    TagLib::ID3v2::FrameListMap tagMap = tags->frameListMap();

    for (TagLib::ID3v2::FrameListMap::ConstIterator it = tagMap.begin(); it != tagMap.end(); ++it)
    {
        QString tagKey = QByteArray(it->first.data(), it->first.size());

        for (TagLib::ID3v2::FrameList::ConstIterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            QString tagValue = QString::fromUtf8((*it2)->toString().toCString(true)).replace('\r', ' ').replace('\n', ' ');

            stream.setFieldWidth(4);
            stream.setFieldAlignment(QTextStream::AlignLeft);

            if (tagValue.size() > 100)
                stream << tagKey << reset << ": " << tagValue.left(97) << "...\n";
            else
                stream << tagKey << reset << ": " << tagValue << '\n';
        }
    }

    stream << "----------------------------------------\n";

    TagLib::ID3v2::FrameList tagList;

    // Titre
    tagList = tags->frameList("TIT2");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TIT2") << '\n';
        setTitle(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Sous-titre
    tagList = tags->frameList("TIT3");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TIT3") << '\n';
        setSubTitle(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Artiste
    tagList = tags->frameList("TPE1");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TPE1") << '\n';
        setArtistName(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Album
    tagList = tags->frameList("TALB");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TALB") << '\n';
        setAlbumTitle(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Artiste de l'album
    tagList = tags->frameList("TPE2");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TPE2") << '\n';
        setAlbumArtist(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Compositeur
    tagList = tags->frameList("TCOM");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TCOM") << '\n';
        setComposer(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Titre pour le tri
    tagList = tags->frameList("TSOT");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TSOT") << '\n';
        setTitleSort(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Artiste pour le tri
    tagList = tags->frameList("TSOP");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TSOP") << '\n';
        setArtistNameSort(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Album pour le tri
    tagList = tags->frameList("TSOA");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TSOA") << '\n';
        setAlbumTitleSort(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Artiste de l'album pour le tri
    tagList = tags->frameList("TSO2");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TSO2") << '\n';
        setAlbumArtistSort(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Année
    tagList = tags->frameList("TDRC");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TDRC") << '\n';
        TagLib::String str = tagList.front()->toString();

        if (!str.isNull() && str.size() >= 4)
        {
            bool ok;
            int year = str.substr(0, 4).toInt(&ok);

            if (!ok)
            {
                stream << tr("Error: invalid tag '%1'").arg("TDRC") << '\n';
                year = 0; // est-ce utile ?
            }

            setYear(year);
        }
    }

    // Regroupement
    tagList = tags->frameList("TIT1");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TIT1") << '\n';
        setGrouping(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Numéro de piste
    tagList = tags->frameList("TRCK");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TRCK") << '\n';

        QString str = QString::fromUtf8(tagList.front()->toString().toCString(true));

        if (str.contains('/'))
        {
            QStringList strSplit = str.split('/');

            if (strSplit.size() == 2)
            {
                bool ok;
                setTrackNumber(strSplit[0].toInt(&ok));
                if (!ok)
                    stream << tr("Error: invalid tag '%1'").arg("TRCK") << '\n';
                setTrackCount(strSplit[1].toInt(&ok));
                if (!ok)
                    stream << tr("Error: invalid tag '%1'").arg("TRCK") << '\n';
            }
            else
            {
                stream << tr("Error: invalid tag '%1'").arg("TRCK") << '\n';
            }
        }
        else
        {
            bool ok;
            setTrackNumber(str.toInt(&ok));
            if (!ok)
                stream << tr("Error: invalid tag '%1'").arg("TRCK") << '\n';
        }
    }

    // Numéro de disque
    tagList = tags->frameList("TPOS");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TPOS") << '\n';
        QString str = QString::fromUtf8(tagList.front()->toString().toCString(true));

        if (str.contains('/'))
        {
            QStringList strSplit = str.split('/');

            if (strSplit.size() == 2)
            {
                bool ok;
                setDiscNumber(strSplit[0].toInt(&ok));
                if (!ok)
                    stream << tr("Error: invalid tag '%1'").arg("TPOS") << '\n';
                setDiscCount(strSplit[1].toInt(&ok));
                if (!ok)
                    stream << tr("Error: invalid tag '%1'").arg("TPOS") << '\n';
            }
            else
            {
                stream << tr("Error: invalid tag '%1'").arg("TPOS") << '\n';
            }
        }
        else
        {
            bool ok;
            setDiscNumber(str.toInt(&ok));
            if (!ok)
                stream << tr("Error: invalid tag '%1'").arg("TPOS") << '\n';
        }
    }

    // Genre
    tagList = tags->frameList("TCON");
    if (!tagList.isEmpty())
    {
        setGenre(QString::fromUtf8(tags->genre().toCString(true)));
    }

    // Commentaires
    tagList = tags->frameList("COMM");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("COMM") << '\n';

        TagLib::ID3v2::CommentsFrame * frame = dynamic_cast<TagLib::ID3v2::CommentsFrame *>(tagList.front());

        if (frame)
        {
            setComments(QString::fromUtf8(frame->text().toCString(true)));
        }
    }

    // BPM
    tagList = tags->frameList("TBPM");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TBPM") << '\n';
        QString str = QString::fromUtf8(tagList.front()->toString().toCString(true));

        bool ok;
        setBPM(str.toInt(&ok));
        if (!ok)
            stream << tr("Error: invalid tag '%1'").arg("TBPM") << '\n';
    }

    // Paroles
    tagList = tags->frameList("USLT");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("USLT") << '\n';

        TagLib::ID3v2::UnsynchronizedLyricsFrame * frame = dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame *>(tagList.front());

        if (frame)
        {
            setLyrics(QString::fromUtf8(frame->text().toCString(true)));
            TagLib::ByteVector lng = frame->language();

            if (lng.size() != 3)
            {
                stream << tr("Error: language of tag 'USLT' invalid") << '\n';
            }
            else
            {
                setLanguage(getLanguageForISO3Code(QByteArray(lng.data(), 3)));
            }
        }
    }

    // Langue
    tagList = tags->frameList("TLAN");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TLAN") << '\n';

        QString str = QString::fromUtf8(tagList.front()->toString().toCString(false));

        if (str.size() != 3)
        {
            stream << tr("Error: invalid tag '%1'").arg("TLAN") << '\n';
        }
        else
        {
            TLanguage lng = getLanguageForISO3Code(qPrintable(str));

            if (m_infos.language != LangUnknown && lng != m_infos.language)
            {
                stream << tr("Error: language of lyrics and language of song are differents") << '\n';
            }

            setLanguage(lng);
        }
    }

    // Parolier
    tagList = tags->frameList("TEXT");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << tr("Error: several tags '%1'").arg("TEXT") << '\n';
        setLyricist(QString::fromUtf8(tagList.front()->toString().toCString(true)));
    }

    // Replay Gain
    tagList = tags->frameList("TXXX");

    if (!tagList.isEmpty())
    {
        for (TagLib::ID3v2::FrameList::ConstIterator it = tagList.begin(); it != tagList.end(); ++it)
        {
            TagLib::ID3v2::UserTextIdentificationFrame * frame = dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame *>(*it);

            if (!frame)
            {
                stream << tr("Error: invalid tag '%1'").arg("TXXX") << '\n';
                continue;
            }

            TagLib::StringList fl = frame->fieldList();

            if (fl.size() != 2)
            {
                stream << tr("Error: tag %1 with several fields").arg("TXXX") << '\n';
                continue;
            }

            QString val = QString::fromUtf8(fl[1].toCString(true));
            QString frameName = QString::fromUtf8(frame->description().toCString(true));

            if (frameName.compare("REPLAYGAIN_TRACK_GAIN", Qt::CaseInsensitive) == 0)
            {
                if (val.endsWith("dB", Qt::CaseInsensitive))
                {
                    val.chop(2);
                    bool ok;
                    setTrackGain(val.toFloat(&ok));

                    if (!ok)
                    {
                        stream << tr("Error: invalid tag '%1'").arg("TXXX [REPLAYGAIN_TRACK_GAIN]") << '\n';
                        setTrackGain(std::numeric_limits<float>::infinity());
                    }
                }
                else
                {
                    stream << tr("Error: invalid tag '%1'").arg("TXXX [REPLAYGAIN_TRACK_GAIN]") << '\n';
                }
            }
            else if (frameName.compare("REPLAYGAIN_TRACK_PEAK", Qt::CaseInsensitive) == 0)
            {
                bool ok;
                setTrackPeak(val.toFloat(&ok));

                if (!ok)
                {
                    stream << tr("Error: invalid tag '%1'").arg("TXXX [REPLAYGAIN_TRACK_PEAK]") << '\n';
                    setTrackPeak(std::numeric_limits<float>::infinity());
                }
            }
            else if (frameName.compare("REPLAYGAIN_ALBUM_GAIN", Qt::CaseInsensitive) == 0)
            {
                if (val.endsWith("dB", Qt::CaseInsensitive))
                {
                    val.chop(2);
                    bool ok;
                    setAlbumGain(val.toFloat(&ok));

                    if (!ok)
                    {
                        stream << tr("Error: invalid tag '%1'").arg("TXXX [REPLAYGAIN_ALBUM_GAIN]") << '\n';
                        setAlbumGain(std::numeric_limits<float>::infinity());
                    }
                }
                else
                {
                    stream << tr("Error: invalid tag '%1'").arg("TXXX [REPLAYGAIN_ALBUM_GAIN]") << '\n';
                }
            }
            else if (frameName.compare("REPLAYGAIN_ALBUM_PEAK", Qt::CaseInsensitive) == 0)
            {
                bool ok;
                setAlbumPeak(val.toFloat(&ok));

                if (!ok)
                {
                    stream << tr("Error: invalid tag '%1'").arg("TXXX [REPLAYGAIN_ALBUM_PEAK]") << '\n';
                    setAlbumPeak(std::numeric_limits<float>::infinity());
                }
            }
        }
    }

    return true;
}


/**
 * Lit les informations d'un morceau depuis des tags APE.
 *
 * \param tags Métadonnées.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::loadTags(TagLib::APE::Tag * tags)
{
    if (!tags)
        return false;

    TSongInfos& infos = m_infos;

    const TagLib::APE::ItemListMap tagMap = tags->itemListMap();

    // Log
    QFile * logFile = m_application->getLogFile("metadata");
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   " << tr("Chargement des tags APE") << '\n';
    stream << "----------------------------------------\n";
    stream << tr("File:") << ' ' << m_properties.fileName << '\n';
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString(tr("dd/MM/yyyy HH:mm:ss")) << '\n';
    stream << "----------------------------------------\n";

    for (TagLib::APE::ItemListMap::ConstIterator it = tagMap.begin(); it != tagMap.end(); ++it)
    {
        const QString tagKey = it->first.toCString();
        const TagLib::StringList tagValues = it->second.toStringList();

        for (TagLib::StringList::ConstIterator it2 = tagValues.begin(); it2 != tagValues.end(); ++it2)
        {
            QString tagValue = QString::fromUtf8(it2->toCString(true)).replace('\r', ' ').replace('\n', ' ');

            stream.setFieldWidth(10);
            stream.setFieldAlignment(QTextStream::AlignLeft);

            if (tagValue.size() > 100)
                stream << tagKey << reset << ": " << tagValue.left(97) << "...\n";
            else
                stream << tagKey << reset << ": " << tagValue << '\n';
        }
    }

    stream << "----------------------------------------\n";

    // Titre
    if (!tagMap["TITLE"].isEmpty())
    {
        setTitle(QString::fromUtf8(tagMap["TITLE"].toString().toCString(true)));
    }

    // Sous-titre
    if (!tagMap["SUBTITLE"].isEmpty())
    {
        setSubTitle(QString::fromUtf8(tagMap["SUBTITLE"].toString().toCString(true)));
    }

    // Artiste
    if (!tagMap["ARTIST"].isEmpty())
    {
        infos.artistName = QString::fromUtf8(tagMap["ARTIST"].toString().toCString(true));
    }

    // Album
    if (!tagMap["ALBUM"].isEmpty())
    {
        infos.albumTitle = QString::fromUtf8(tagMap["ALBUM"].toString().toCString(true));
    }

    // Compositeur
    if (!tagMap["COMPOSER"].isEmpty())
    {
        infos.composer = QString::fromUtf8(tagMap["COMPOSER"].toString().toCString(true));
    }

    // Genre
    if (!tagMap["GENRE"].isEmpty())
    {
        infos.genre = QString::fromUtf8(tagMap["GENRE"].toString().toCString(true));
    }

    // Année
    if (!tagMap["YEAR"].isEmpty())
    {
        infos.year = tagMap["YEAR"].toString().toInt();
    }
    else if (!tagMap["RECORDDATE"].isEmpty())
    {
        // TODO: tester...
        infos.year = tagMap["RECORDDATE"].toString().toInt();
    }

    // Commentaires
    if (!tagMap["COMMENT"].isEmpty())
    {
        infos.comments = QString::fromUtf8(tagMap["COMMENT"].toString().toCString(true));
    }

    // Numéro de piste
    if (!tagMap["TRACK"].isEmpty())
    {
        QString str = QString::fromUtf8(tagMap["TRACK"].toString().toCString(true));

        if (str.contains('/'))
        {
            QStringList strSplit = str.split('/');

            if (strSplit.size() == 2)
            {
                bool ok;
                infos.trackNumber = strSplit[0].toInt(&ok);
                if (!ok)
                    stream << tr("Error: invalid tag '%1'").arg("TRACK") << '\n';
                infos.trackCount = strSplit[1].toInt(&ok);
                if (!ok)
                    stream << tr("Error: invalid tag '%1'").arg("TRACK") << '\n';
            }
            else
            {
                stream << tr("Error: invalid tag '%1'").arg("TRACK") << '\n';
            }
        }
        else
        {
            bool ok;
            infos.trackNumber = str.toInt(&ok);
            if (!ok)
                stream << tr("Error: invalid tag '%1'").arg("TRACK") << '\n';
        }
    }

    // Numéro de disque
    if (!tagMap["MEDIA"].isEmpty())
    {
        QString str = QString::fromUtf8(tagMap["MEDIA"].toString().toCString(true));

        if (str.contains('/'))
        {
            QStringList strSplit = str.split('/');

            if (strSplit.size() == 2)
            {
                bool ok;
                infos.discNumber = strSplit[0].toInt(&ok);
                if (!ok)
                    stream << tr("Error: invalid tag '%1'").arg("MEDIA") << '\n';
                infos.discCount = strSplit[1].toInt(&ok);
                if (!ok)
                    stream << tr("Error: invalid tag '%1'").arg("MEDIA") << '\n';
            }
            else
            {
                stream << tr("Error: invalid tag '%1'").arg("MEDIA") << '\n';
            }
        }
        else
        {
            bool ok;
            infos.discNumber = str.toInt(&ok);
            if (!ok)
                stream << tr("Error: invalid tag '%1'").arg("MEDIA") << '\n';
        }
    }

    // Langue
    if (!tagMap["LANGUAGE"].isEmpty())
    {
        stream << tr("Error: tag 'LANGUAGE' non géré") << '\n';
        qDebug() << "CSong::loadTags() : APE : langue = " << QString::fromUtf8(tagMap["LANGUAGE"].toString().toCString(true));
        //infos.language = QString::fromUtf8(tagMap["LANGUAGE"].toString().toCString(true));
    }

    return false;
}


/**
 * Lit les informations d'un morceau depuis des tags Xiph Comment.
 *
 * \param tags Métadonnées.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::loadTags(TagLib::Ogg::XiphComment * tags)
{
    if (!tags)
        return false;

    const TagLib::Ogg::FieldListMap tagMap = tags->fieldListMap();

    // Log
    QFile * logFile = m_application->getLogFile("metadata");
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   " << tr("Loading XiphComment tags") << '\n';
    stream << "----------------------------------------\n";
    stream << tr("File:") << ' ' << m_properties.fileName << '\n';
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString(tr("dd/MM/yyyy HH:mm:ss")) << '\n';
    stream << "----------------------------------------\n";

    for (TagLib::Ogg::FieldListMap::ConstIterator it = tagMap.begin(); it != tagMap.end(); ++it)
    {
        QString tagKey = QString::fromUtf8(it->first.toCString(true));

        for (TagLib::StringList::ConstIterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            QString tagValue = QString::fromUtf8(it2->toCString(true)).replace('\r', ' ').replace('\n', ' ');

            stream.setFieldWidth(12);
            stream.setFieldAlignment(QTextStream::AlignLeft);

            if (tagValue.size() > 100)
                stream << tagKey << reset << ": " << tagValue.left(97) << "...\n";
            else
                stream << tagKey << reset << ": " << tagValue << '\n';
        }
    }

    stream << "----------------------------------------\n";

    // Titre
    if (!tagMap["TITLE"].isEmpty())
    {
        setTitle(QString::fromUtf8(tagMap["TITLE"].toString().toCString(true)));
    }

    // Sous-titre
    if (!tagMap["SUBTITLE"].isEmpty())
    {
        setSubTitle(QString::fromUtf8(tagMap["SUBTITLE"].toString().toCString(true)));
    }

    // Regroupement
    if (!tagMap["GROUPING"].isEmpty())
    {
        setGrouping(QString::fromUtf8(tagMap["GROUPING"].toString().toCString(true)));
    }

    // Album
    if (!tagMap["ALBUM"].isEmpty())
    {
        setAlbumTitle(QString::fromUtf8(tagMap["ALBUM"].toString().toCString(true)));
    }

    // Album pour le tri
    if (!tagMap["ALBUMSORT"].isEmpty())
    {
        setAlbumTitleSort(QString::fromUtf8(tagMap["ALBUMSORT"].toString().toCString(true)));
    }

    // Artiste
    if (!tagMap["ARTIST"].isEmpty())
    {
        setArtistName(QString::fromUtf8(tagMap["ARTIST"].toString().toCString(true)));
    }

    // Artiste pour le tri
    if (!tagMap["ARTISTSORT"].isEmpty())
    {
        setArtistNameSort(QString::fromUtf8(tagMap["ARTISTSORT"].toString().toCString(true)));
    }

    // Artiste de l'album
    if (!tagMap["ALBUMARTIST"].isEmpty())
    {
        setAlbumArtist(QString::fromUtf8(tagMap["ALBUMARTIST"].toString().toCString(true)));
    }

    // Artiste de l'album pour le tri
    if (!tagMap["ALBUMARTISTSORT"].isEmpty())
    {
        setAlbumArtistSort(QString::fromUtf8(tagMap["ALBUMARTISTSORT"].toString().toCString(true)));
    }

    // Genre
    if (!tagMap["GENRE"].isEmpty())
    {
        setGenre(QString::fromUtf8(tagMap["GENRE"].toString().toCString(true)));
    }

    // Année
    if (!tagMap["DATE"].isEmpty())
    {
        QString year = QString::fromUtf8(tagMap["DATE"].toString().toCString(true));

        if (year.size() != 4)
        {
            stream << tr("Error: invalid tag '%1'").arg("DATE") << '\n';
        }
        else
        {
            bool ok;
            setYear(year.toInt(&ok));
            if (!ok)
                stream << tr("Error: invalid tag '%1'").arg("DATE") << '\n';
        }
    }

    // Numéro de piste
    if (!tagMap["TRACKNUMBER"].isEmpty())
    {
        QString trackNumber = QString::fromUtf8(tagMap["TRACKNUMBER"].toString().toCString(true));
        bool ok;
        setTrackNumber(trackNumber.toInt(&ok));

        if (!ok || m_infos.trackNumber <= 0)
        {
            stream << tr("Error: invalid tag '%1'").arg("TRACKNUMBER") << '\n';
            setTrackNumber(0); // est-ce utile ?
        }
    }

    // Nombre de pistes
    if (!tagMap["TRACKTOTAL"].isEmpty())
    {
        QString trackCount = QString::fromUtf8(tagMap["TRACKTOTAL"].toString().toCString(true));
        bool ok;
        setTrackCount(trackCount.toInt(&ok));

        if (!ok || m_infos.trackCount <= 0)
        {
            stream << tr("Error: invalid tag '%1'").arg("TRACKTOTAL") << '\n';
            setTrackCount(0); // est-ce utile ?
        }
    }

    if (!tagMap["TOTALTRACKS"].isEmpty())
    {
        if (!tagMap["TRACKTOTAL"].isEmpty())
        {
            stream << tr("Error: les tags TRACKTOTAL et TOTALTRACKS sont présents tous les deux") << '\n';
        }

        QString trackCount = QString::fromUtf8(tagMap["TOTALTRACKS"].toString().toCString(true));
        bool ok;
        setTrackCount(trackCount.toInt(&ok));

        if (!ok || m_infos.trackCount <= 0)
        {
            stream << tr("Error: invalid tag '%1'").arg("TOTALTRACKS") << '\n';
            setTrackCount(0); // est-ce utile ?
        }
    }

    // BPM
    if (!tagMap["TEMPO"].isEmpty())
    {
        QString bpmStr = QString::fromUtf8(tagMap["TEMPO"].toString().toCString(true));
        bool ok;
        int bpm = bpmStr.toInt(&ok);

        if (!ok || bpm <= 0)
        {
            stream << tr("Error: invalid tag '%1'").arg("TEMPO") << '\n';
            bpm = 0; // est-ce utile ?
        }

        setBPM(bpm);
    }

    if (!tagMap["BPM"].isEmpty())
    {
        if (!tagMap["TEMPO"].isEmpty())
        {
            stream << tr("Error: les tags TEMPO et BPM sont présents tous les deux") << '\n';
        }

        QString bpmStr = QString::fromUtf8(tagMap["BPM"].toString().toCString(true));
        bool ok;
        int bpm = bpmStr.toInt(&ok);

        if (!ok || bpm <= 0)
        {
            stream << tr("Error: invalid tag '%1'").arg("BPM") << '\n';
            bpm = 0; // est-ce utile ?
        }

        setBPM(bpm);
    }

    // Paroles
    if (!tagMap["LYRICS"].isEmpty())
    {
        setLyrics(QString::fromUtf8(tagMap["LYRICS"].toString().toCString(true)));
    }

    // Parolier
    if (!tagMap["LYRICIST"].isEmpty())
    {
        setLyricist(QString::fromUtf8(tagMap["LYRICIST"].toString().toCString(true)));
    }

    // Langue
    if (!tagMap["LANGUAGE"].isEmpty())
    {
        QString str = QString::fromUtf8(tagMap["LANGUAGE"].toString().toCString(true));

        if (str.size() != 3)
        {
            stream << tr("Error: invalid tag '%1'").arg("LANGUAGE") << '\n';
        }
        else
        {
            TLanguage lng = getLanguageForISO3Code(qPrintable(str));
            setLanguage(lng);
        }
    }

    // Compositeur
    if (!tagMap["COMPOSER"].isEmpty())
    {
        setComposer(QString::fromUtf8(tagMap["COMPOSER"].toString().toCString(true)));
    }

    // Compositeur pour le tri
    if (!tagMap["COMPOSERSORT"].isEmpty())
    {
        setComposerSort(QString::fromUtf8(tagMap["COMPOSERSORT"].toString().toCString(true)));
    }

    // Numéro de disque
    if (!tagMap["DISCNUMBER"].isEmpty())
    {
        QString discNumberStr = QString::fromUtf8(tagMap["DISCNUMBER"].toString().toCString(true));
        bool ok;
        int discNumber = discNumberStr.toInt(&ok);

        if (!ok || m_infos.discNumber <= 0)
        {
            stream << tr("Error: invalid tag '%1'").arg("DISCNUMBER") << '\n';
            discNumber = 0; // est-ce utile ?
        }

        setDiscNumber(discNumber);
    }

    // Nombre de disques
    if (!tagMap["DISCTOTAL"].isEmpty())
    {
        QString discCountStr = QString::fromUtf8(tagMap["DISCTOTAL"].toString().toCString(true));
        bool ok;
        int discCount = discCountStr.toInt(&ok);

        if (!ok || m_infos.discCount <= 0)
        {
            stream << tr("Error: invalid tag '%1'").arg("DISCTOTAL") << '\n';
            discCount = 0; // est-ce utile ?
        }

        setDiscCount(discCount);
    }

    // Commentaires
    if (!tagMap["COMMENT"].isEmpty())
    {
        setComments(QString::fromUtf8(tagMap["COMMENT"].toString().toCString(true)));
    }

    // Replay Gain
    if (!tagMap["REPLAYGAIN_TRACK_GAIN"].isEmpty())
    {
        QString val = QString::fromUtf8(tagMap["REPLAYGAIN_TRACK_GAIN"].toString().toCString(true));

        if (val.endsWith("dB", Qt::CaseInsensitive))
        {
            val.chop(2);
            bool ok;
            setTrackGain(val.toFloat(&ok));

            if (!ok)
            {
                stream << tr("Erreur : tag REPLAYGAIN_TRACK_GAIN incorrect") << '\n';
                setTrackGain(std::numeric_limits<float>::infinity());
            }
        }
        else
        {
            stream << tr("Erreur : tag REPLAYGAIN_TRACK_GAIN incorrect") << '\n';
        }
    }

    if (!tagMap["REPLAYGAIN_TRACK_PEAK"].isEmpty())
    {
        bool ok;
        setTrackPeak(QString::fromUtf8(tagMap["REPLAYGAIN_TRACK_PEAK"].toString().toCString(true)).toFloat(&ok));

        if (!ok)
        {
            stream << tr("Erreur : tag REPLAYGAIN_TRACK_PEAK incorrect") << '\n';
            setTrackPeak(std::numeric_limits<float>::infinity());
        }
    }

    if (!tagMap["REPLAYGAIN_ALBUM_GAIN"].isEmpty())
    {
        QString val = QString::fromUtf8(tagMap["REPLAYGAIN_ALBUM_GAIN"].toString().toCString(true));

        if (val.endsWith("dB", Qt::CaseInsensitive))
        {
            val.chop(2);
            bool ok;
            setAlbumGain(val.toFloat(&ok));

            if (!ok)
            {
                stream << tr("Erreur : tag REPLAYGAIN_ALBUM_GAIN incorrect") << '\n';
                setAlbumGain(std::numeric_limits<float>::infinity());
            }
        }
        else
        {
            stream << tr("Erreur : tag REPLAYGAIN_ALBUM_GAIN incorrect") << '\n';
        }
    }

    if (!tagMap["REPLAYGAIN_ALBUM_PEAK"].isEmpty())
    {
        bool ok;
        setAlbumPeak(QString::fromUtf8(tagMap["REPLAYGAIN_ALBUM_PEAK"].toString().toCString(true)).toFloat(&ok));

        if (!ok)
        {
            stream << tr("Erreur : tag REPLAYGAIN_ALBUM_PEAK incorrect") << '\n';
            setAlbumPeak(std::numeric_limits<float>::infinity());
        }
    }

    return false;
}


/**
 * Écrit les informations d'un morceau dans les tags ID3 version 1.
 *
 * \param tags     Métadonnées à modifier.
 * \param infos    Informations à écrire.
 * \param logFile  Fichier de log.
 * \param fileName Nom du fichier contenant le morceau.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::writeTags(TagLib::ID3v1::Tag * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName)
{
    if (!tags)
        return false;

    // Log
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   " << tr("Enregistrement des tags ID3v1") << '\n';
    stream << "----------------------------------------\n";
    stream << tr("File:") << ' ' << fileName << '\n';
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString(tr("dd/MM/yyyy HH:mm:ss")) << '\n';

    tags->setTitle(TagLib::String(infos.title.toUtf8().constData(), TagLib::String::UTF8));
    tags->setArtist(TagLib::String(infos.artistName.toUtf8().constData(), TagLib::String::UTF8));
    tags->setAlbum(TagLib::String(infos.albumTitle.toUtf8().constData(), TagLib::String::UTF8));
    tags->setComment(TagLib::String(infos.comments.toUtf8().constData(), TagLib::String::UTF8));
    tags->setGenre(TagLib::String(infos.genre.toUtf8().constData(), TagLib::String::UTF8));
    tags->setYear(infos.year);
    tags->setTrack(infos.trackNumber);

    return true;
}


/**
 * Écrit les informations d'un morceau dans les tags ID3 version 2.
 *
 * \todo Écrire les valeurs de Replay Gain.
 *
 * \param tags     Métadonnées à modifier.
 * \param infos    Informations à écrire.
 * \param logFile  Fichier de log.
 * \param fileName Nom du fichier contenant le morceau.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::writeTags(TagLib::ID3v2::Tag * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName)
{
    if (!tags)
        return false;

    // Log
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   " << tr("Enregistrement des tags ID3v2") << '\n';
    stream << "----------------------------------------\n";
    stream << tr("File:") << ' ' << fileName << '\n';
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString(tr("dd/MM/yyyy HH:mm:ss")) << '\n';
    stream << "----------------------------------------\n";

    // Titre
    {
        tags->removeFrames("TIT2");
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TIT2", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.title.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Sous-titre
    tags->removeFrames("TIT3");
    if (!infos.subTitle.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TIT3", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.subTitle.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Regroupement
    tags->removeFrames("TIT1");
    if (!infos.grouping.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TIT1", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.grouping.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Artiste
    {
        tags->removeFrames("TPE1");
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TPE1", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.artistName.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Album
    tags->removeFrames("TALB");
    if (!infos.albumTitle.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TALB", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.albumTitle.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Artiste de l'album
    tags->removeFrames("TPE2");
    if (!infos.albumArtist.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TPE2", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.albumArtist.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Compositeur
    tags->removeFrames("TCOM");
    if (!infos.composer.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TCOM", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.composer.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Titre pour le tri
    tags->removeFrames("TSOT");
    if (!infos.titleSort.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TSOT", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.titleSort.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Artiste pour le tri
    tags->removeFrames("TSOP");
    if (!infos.artistNameSort.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TSOP", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.artistNameSort.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Album pour le tri
    tags->removeFrames("TSOA");
    if (!infos.albumTitleSort.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TSOA", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.albumTitleSort.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Artiste de l'album pour le tri
    tags->removeFrames("TSO2");
    if (!infos.albumArtistSort.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TSO2", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.albumArtistSort.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Compositeur pour le tri
    stream << tr("Aucun tag pour enregistrer le compositeur pour le tri") << '\n';

    // Année
    tags->removeFrames("TDRC");
    if (infos.year > 0)
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TDRC", TagLib::String::UTF8);
        frame->setText(TagLib::String::number(infos.year));
        tags->addFrame(frame);
    }

    // Numéro de piste
    tags->removeFrames("TRCK");
    if (infos.trackNumber > 0)
    {
        if (infos.trackCount >= infos.trackNumber)
        {
            TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TRCK", TagLib::String::UTF8);
            frame->setText(TagLib::String::number(infos.trackNumber) + '/' + TagLib::String::number(infos.trackCount));
            tags->addFrame(frame);
        }
        else
        {
            TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TRCK", TagLib::String::UTF8);
            frame->setText(TagLib::String::number(infos.trackNumber));
            tags->addFrame(frame);
        }
    }

    // Numéro de disque
    tags->removeFrames("TPOS");
    if (infos.discNumber > 0)
    {
        if (infos.discCount >= infos.discNumber)
        {
            TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TPOS", TagLib::String::UTF8);
            frame->setText(TagLib::String::number(infos.discNumber) + "/" + TagLib::String::number(infos.discCount));
            tags->addFrame(frame);
        }
        else
        {
            TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TPOS", TagLib::String::UTF8);
            frame->setText(TagLib::String::number(infos.discNumber));
            tags->addFrame(frame);
        }
    }

    // Genre
    tags->removeFrames("TCON");
    if (!infos.genre.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TCON", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.genre.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // Commentaires
    tags->removeFrames("COMM"); // <= Garder les commentaires multiples ?
    if (!infos.comments.isEmpty())
    {
        TagLib::ID3v2::CommentsFrame * frame = new TagLib::ID3v2::CommentsFrame(TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.comments.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

    // BPM
    tags->removeFrames("TBPM");
    if (infos.bpm > 0)
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::UTF8);
        frame->setText(TagLib::String::number(infos.bpm));
        tags->addFrame(frame);
    }

    // Paroles
    tags->removeFrames("USLT");
    if (!infos.lyrics.isEmpty())
    {
        TagLib::ID3v2::UnsynchronizedLyricsFrame * frame = new TagLib::ID3v2::UnsynchronizedLyricsFrame(TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.lyrics.toUtf8().constData(), TagLib::String::UTF8));
        frame->setLanguage(CSong::getISO3CodeForLanguage(infos.language).toLatin1().constData());
        tags->addFrame(frame);
    }

    // Langue
    tags->removeFrames("TLAN");
    if (infos.language != CSong::LangUnknown)
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TLAN", TagLib::String::UTF8);
        frame->setText(CSong::getISO3CodeForLanguage(infos.language).toLatin1().constData());
        tags->addFrame(frame);
    }

    // Parolier
    tags->removeFrames("TEXT");
    if (!infos.lyricist.isEmpty())
    {
        TagLib::ID3v2::TextIdentificationFrame * frame = new TagLib::ID3v2::TextIdentificationFrame("TEXT", TagLib::String::UTF8);
        frame->setText(TagLib::String(infos.lyricist.toUtf8().constData(), TagLib::String::UTF8));
        tags->addFrame(frame);
    }

/*
Track replay gain 	    REPLAYGAIN_TRACK_GAIN 	[-]a.bb dB
Peak track amplitude 	REPLAYGAIN_TRACK_PEAK 	c.dddddd
Album replay gain 	    REPLAYGAIN_ALBUM_GAIN 	[-]a.bb dB
Peak album amplitude 	REPLAYGAIN_ALBUM_PEAK 	c.dddddd
*/

    return true;
}


/**
 * Écrit les informations d'un morceau dans les tags APE.
 *
 * \todo Écrire les valeurs de Replay Gain.
 *
 * \param tags     Métadonnées à modifier.
 * \param infos    Informations à écrire.
 * \param logFile  Fichier de log.
 * \param fileName Nom du fichier contenant le morceau.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::writeTags(TagLib::APE::Tag * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName)
{
    if (!tags)
        return false;

    // Log
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   " << tr("Enregistrement des tags APE") << '\n';
    stream << "----------------------------------------\n";
    stream << tr("File:") << ' ' << fileName << '\n';
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString(tr("dd/MM/yyyy HH:mm:ss")) << '\n';
    stream << "----------------------------------------\n";

    // Titre
    tags->addValue("TITLE", TagLib::String(infos.title.toUtf8().constData(), TagLib::String::UTF8));

    // Sous-titre
    tags->addValue("SUBTITLE", TagLib::String(infos.subTitle.toUtf8().constData(), TagLib::String::UTF8));

    // Regroupement
    stream << tr("Aucun tag pour enregistrer le regroupement") << '\n';

    // Artiste
    tags->addValue("ARTIST", TagLib::String(infos.artistName.toUtf8().constData(), TagLib::String::UTF8));

    // Album
    tags->addValue("ALBUM", TagLib::String(infos.albumTitle.toUtf8().constData(), TagLib::String::UTF8));

    // Artiste de l'album
    stream << tr("Aucun tag pour enregistrer l'artiste de l'album") << '\n';

    // Compositeur
    tags->addValue("COMPOSER", TagLib::String(infos.composer.toUtf8().constData(), TagLib::String::UTF8));

    // Titre pour le tri
    stream << tr("Aucun tag pour enregistrer le titre pour le tri") << '\n';

    // Artiste pour le tri
    stream << tr("Aucun tag pour enregistrer l'artiste pour le tri") << '\n';

    // Album pour le tri
    stream << tr("Aucun tag pour enregistrer l'album pour le tri") << '\n';

    // Artiste de l'album pour le tri
    stream << tr("Aucun tag pour enregistrer l'artiste pour le tri") << '\n';

    // Compositeur pour le tri
    stream << tr("Aucun tag pour enregistrer le compositeur pour le tri") << '\n';

    // Année
    if (infos.year > 0)
        tags->addValue("YEAR", TagLib::String::number(infos.year));
    else
        tags->removeItem("DATE");

    // Numéro de piste
    if (infos.trackNumber > 0)
    {
        if (infos.trackCount >= infos.trackNumber)
        {
            tags->addValue("TRACK", TagLib::String::number(infos.trackNumber) + "/" + TagLib::String::number(infos.trackCount));
        }
        else
        {
            tags->addValue("TRACK", TagLib::String::number(infos.trackNumber));
        }
    }
    else
    {
        tags->removeItem("TRACK");
    }

    // Numéro de disque
    if (infos.discNumber > 0)
    {
        if (infos.discCount >= infos.discNumber)
        {
            tags->addValue("MEDIA", TagLib::String::number(infos.discNumber) + "/" + TagLib::String::number(infos.discCount));
        }
        else
        {
            tags->addValue("MEDIA", TagLib::String::number(infos.discNumber));
        }
    }
    else
    {
        tags->removeItem("MEDIA");
    }

    // Genre
    tags->addValue("GENRE", TagLib::String(infos.genre.toUtf8().constData(), TagLib::String::UTF8));

    // Commentaires
    tags->addValue("COMMENT", TagLib::String(infos.comments.toUtf8().constData(), TagLib::String::UTF8));

    // BPM
    stream << tr("Aucun tag pour enregistrer le nombre de battements par minute") << '\n';

    // Paroles
    stream << tr("Aucun tag pour enregistrer les paroles") << '\n';

    // Langue
    tags->addValue("LANGUAGE", CSong::getISO3CodeForLanguage(infos.language).toUtf8().constData());

    // Parolier
    stream << tr("Aucun tag pour enregistrer le parolier") << '\n';

    return true;
}


/**
 * Écrit les informations d'un morceau dans les tags Xiph Comment.
 *
 * \param tags     Métadonnées à modifier.
 * \param infos    Informations à écrire.
 * \param logFile  Fichier de log.
 * \param fileName Nom du fichier contenant le morceau.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::writeTags(TagLib::Ogg::XiphComment * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName)
{
    if (!tags)
        return false;

    // Log
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   " << tr("Enregistrement des tags Xiph Comment") << '\n';
    stream << "----------------------------------------\n";
    stream << tr("File:") << ' ' << fileName << '\n';
    stream << tr("Date:") << ' ' << QDateTime::currentDateTime().toString(tr("dd/MM/yyyy HH:mm:ss")) << '\n';
    stream << "----------------------------------------\n";

    // Titre
    tags->addField("TITLE", TagLib::String(infos.title.toUtf8().constData(), TagLib::String::UTF8));

    // Sous-titre
    if (infos.subTitle.isEmpty())
        tags->removeField("SUBTITLE");
    else
        tags->addField("SUBTITLE", TagLib::String(infos.subTitle.toUtf8().constData(), TagLib::String::UTF8));

    // Regroupement
    if (infos.grouping.isEmpty())
        tags->removeField("GROUPING");
    else
        tags->addField("GROUPING", TagLib::String(infos.grouping.toUtf8().constData(), TagLib::String::UTF8));

    // Artiste
    if (infos.artistName.isEmpty())
        tags->removeField("ARTIST");
    else
        tags->addField("ARTIST", TagLib::String(infos.artistName.toUtf8().constData(), TagLib::String::UTF8));

    // Album
    if (infos.albumTitle.isEmpty())
        tags->removeField("ALBUM");
    else
        tags->addField("ALBUM", TagLib::String(infos.albumTitle.toUtf8().constData(), TagLib::String::UTF8));

    // Artiste de l'album
    if (infos.albumArtist.isEmpty())
        tags->removeField("ALBUMARTIST");
    else
        tags->addField("ALBUMARTIST", TagLib::String(infos.albumArtist.toUtf8().constData(), TagLib::String::UTF8));

    // Compositeur
    if (infos.composer.isEmpty())
        tags->removeField("COMPOSER");
    else
        tags->addField("COMPOSER", TagLib::String(infos.composer.toUtf8().constData(), TagLib::String::UTF8));

    // Titre pour le tri
    if (infos.titleSort.isEmpty())
        tags->removeField("TITLESORT");
    else
        tags->addField("TITLESORT", TagLib::String(infos.titleSort.toUtf8().constData(), TagLib::String::UTF8));

    // Artiste pour le tri
    if (infos.artistNameSort.isEmpty())
        tags->removeField("ARTISTSORT");
    else
        tags->addField("ARTISTSORT", TagLib::String(infos.artistNameSort.toUtf8().constData(), TagLib::String::UTF8));

    // Album pour le tri
    if (infos.albumTitleSort.isEmpty())
        tags->removeField("ALBUMSORT");
    else
        tags->addField("ALBUMSORT", TagLib::String(infos.albumTitleSort.toUtf8().constData(), TagLib::String::UTF8));

    // Artiste de l'album pour le tri
    if (infos.albumArtistSort.isEmpty())
        tags->removeField("ALBUMARTISTSORT");
    else
        tags->addField("ALBUMARTISTSORT", TagLib::String(infos.albumArtistSort.toUtf8().constData(), TagLib::String::UTF8));

    // Compositeur pour le tri
    if (infos.composerSort.isEmpty())
        tags->removeField("COMPOSERSORT");
    else
        tags->addField("COMPOSERSORT", TagLib::String(infos.composerSort.toUtf8().constData(), TagLib::String::UTF8));

    // Année
    if (infos.year > 0)
        tags->addField("DATE", TagLib::String::number(infos.year));
    else
        tags->removeField("DATE");

    // Numéro de piste
    if (infos.trackNumber > 0)
        tags->addField("TRACKNUMBER", TagLib::String::number(infos.trackNumber));
    else
        tags->removeField("TRACKNUMBER");

    // Nombre de pistes
    if (infos.trackCount > 0)
        tags->addField("TRACKTOTAL", TagLib::String::number(infos.trackCount));
    else
        tags->removeField("TRACKTOTAL");

    // Numéro de disque
    if (infos.discNumber > 0)
        tags->addField("DISCNUMBER", TagLib::String::number(infos.discNumber));
    else
        tags->removeField("DISCNUMBER");

    // Nombre de disques
    if (infos.discCount > 0)
        tags->addField("DISCTOTAL", TagLib::String::number(infos.discCount));
    else
        tags->removeField("DISCTOTAL");

    // Genre
    if (infos.genre.isEmpty())
        tags->removeField("GENRE");
    else
        tags->addField("GENRE", TagLib::String(infos.genre.toUtf8().constData(), TagLib::String::UTF8));

    // Commentaires
    if (infos.comments.isEmpty())
        tags->removeField("COMMENT");
    else
        tags->addField("COMMENT", TagLib::String(infos.comments.toUtf8().constData(), TagLib::String::UTF8));

    // BPM
    if (infos.bpm > 0)
        tags->addField("TEMPO", TagLib::String::number(infos.bpm));
    else
        tags->removeField("TEMPO");

    // Paroles
    if (infos.lyrics.isEmpty())
        tags->removeField("LYRICS");
    else
        tags->addField("LYRICS", TagLib::String(infos.lyrics.toUtf8().constData(), TagLib::String::UTF8));

    // Langue
    if (infos.language == CSong::LangUnknown)
        tags->removeField("LANGUAGE");
    else
        tags->addField("LANGUAGE", CSong::getISO3CodeForLanguage(infos.language).toLatin1().constData());

    // Parolier
    if (infos.lyricist.isEmpty())
        tags->removeField("LYRICIST");
    else
        tags->addField("LYRICIST", TagLib::String(infos.lyricist.toUtf8().constData(), TagLib::String::UTF8));

/*
    // Replay Gain
    tags->addField("REPLAYGAIN_ALBUM_GAIN", QString("%1 db").arg(infos.albumGain).toUtf8().constData());
    tags->addField("REPLAYGAIN_ALBUM_PEAK", QString::number(infos.albumPeak).toUtf8().constData());
    tags->addField("REPLAYGAIN_TRACK_GAIN", QString("%1 dB").arg(infos.trackGain).toUtf8().constData());
    tags->addField("REPLAYGAIN_TRACK_PEAK", QString::number(infos.trackPeak).toUtf8().constData());
*/

    return true;
}

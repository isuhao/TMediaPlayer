
#include "CSong.hpp"
#include "CApplication.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFile>

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
    language        (LangUnknown)
{

}


/**
 * Crée un nouveau morceau invalide.
 *
 * \todo Supprimer ou rendre privée ?
 */

CSong::CSong(CApplication * application) :
    QObject             (application),
    m_application       (application),
    m_sound             (NULL),
    m_channel           (NULL),
    m_multiModification (false),
    m_id                (-1),
    m_fileName          (""),
    m_fileSize          (0),
    m_bitRate           (0),
    m_sampleRate        (0),
    m_format            (FormatUnknown),
    m_numChannels       (0),
    m_duration          (0),
/*
    m_isEnabled         (true),
    m_title             (""),
    m_subTitle          (""),
    m_grouping          (""),
    m_artistName        (""),
    m_albumTitle        (""),
    m_albumArtist       (""),
    m_composer          (""),
    m_titleSort         (""),
    m_artistNameSort    (""),
    m_albumTitleSort    (""),
    m_albumArtistSort   (""),
    m_composerSort      (""),
    m_year              (0),
    m_trackNumber       (0),
    m_trackCount        (0),
    m_discNumber        (0),
    m_discCount         (0),
    m_genre             (""),
    m_rating            (0),
    m_comments          (""),
    m_bpm               (0),
    m_lyrics            (""),
    m_language          (LangUnknown),
*/
    m_isModified        (false)
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
    QObject             (application),
    m_application       (application),
    m_sound             (NULL),
    m_channel           (NULL),
    m_multiModification (false),
    m_id                (-1),
    m_fileName          (fileName),
    m_fileSize          (0),
    m_bitRate           (0),
    m_sampleRate        (0),
    m_format            (FormatUnknown),
    m_numChannels       (0),
    m_duration          (0),
/*
    m_isEnabled         (true),
    m_title             (""),
    m_subTitle          (""),
    m_grouping          (""),
    m_artistName        (""),
    m_albumTitle        (""),
    m_albumArtist       (""),
    m_composer          (""),
    m_titleSort         (""),
    m_artistNameSort    (""),
    m_albumTitleSort    (""),
    m_albumArtistSort   (""),
    m_composerSort      (""),
    m_year              (0),
    m_trackNumber       (0),
    m_trackCount        (0),
    m_discNumber        (0),
    m_discCount         (0),
    m_genre             (""),
    m_rating            (0),
    m_comments          (""),
    m_bpm               (0),
    m_lyrics            (""),
    m_language          (LangUnknown),
*/
    m_isModified        (false)
{
    Q_CHECK_PTR(application);

    m_id = getId(application, fileName);

    if (m_id >= 0)
    {
        loadFromDatabase();
    }
    else
    {
        loadTags();
    }
}


/**
 * Arrête la lecture du morceau et libère les ressources.
 */

CSong::~CSong()
{
    //qDebug() << "CSong::~CSong()";
    stop();

    if (m_sound)
    {
        m_sound->release();
    }

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
    {
        return;
    }

    QSqlQuery query(m_application->getDataBase());

    query.prepare("SELECT"
                      " song_filename,"
                      " song_filesize,"
                      " song_bitrate,"
                      " song_sample_rate,"
                      " song_format,"
                      " song_channels, "
                      " song_duration,"
                      " song_creation,"
                      " song_modification,"
                      " song_enabled,"
                      " song_title,"
                      " song_title_sort,"
                      " song_artist.artist_name,"
                      " song_artist.artist_name_sort,"
                      " album_title,"
                      " album_title_sort,"
                      " album_subtitle,"
                      " album_grouping,"
                      " album_artist.artist_name,"
                      " album_artist.artist_name_sort,"
                      " song_composer,"
                      " song_composer_sort,"
                      " song_year,"
                      " song_track_number,"
                      " song_track_count,"
                      " song_disc_number,"
                      " song_disc_count,"
                      " genre_name,"
                      " song_rating,"
                      " song_comments,"
                      " song_lyrics,"
                      " song_language"
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
        qWarning() << "CSong::loadFromDatabase() : l'identifiant " << m_id << " est invalide";
        return;
    }

    int numValue = 0;

    m_fileName        = query.value(numValue++).toString();
    m_fileSize        = query.value(numValue++).toInt();
    m_bitRate         = query.value(numValue++).toInt();
    m_sampleRate      = query.value(numValue++).toInt();
    m_format          = getFormatFromInteger(query.value(numValue++).toInt());
    m_numChannels     = query.value(numValue++).toInt();
    m_duration        = query.value(numValue++).toInt();
    m_creation        = query.value(numValue++).toDateTime();
    m_modification    = query.value(numValue++).toDateTime();

    m_infos.isEnabled       = query.value(numValue++).toBool();
    m_infos.title           = query.value(numValue++).toString();
    m_infos.titleSort       = query.value(numValue++).toString();
    m_infos.subTitle        = query.value(numValue++).toString();
    m_infos.grouping        = query.value(numValue++).toString();
    m_infos.artistName      = query.value(numValue++).toString();
    m_infos.artistNameSort  = query.value(numValue++).toString();
    m_infos.albumTitle      = query.value(numValue++).toString();
    m_infos.albumTitleSort  = query.value(numValue++).toString();
    m_infos.albumArtist     = query.value(numValue++).toString();
    m_infos.albumArtistSort = query.value(numValue++).toString();
    m_infos.composer        = query.value(numValue++).toString();
    m_infos.composerSort    = query.value(numValue++).toString();
    m_infos.year            = query.value(numValue++).toInt();
    m_infos.trackNumber     = query.value(numValue++).toInt();
    m_infos.trackCount      = query.value(numValue++).toInt();
    m_infos.discNumber      = query.value(numValue++).toInt();
    m_infos.discCount       = query.value(numValue++).toInt();
    m_infos.genre           = query.value(numValue++).toString();
    m_infos.rating          = query.value(numValue++).toInt();
    m_infos.comments        = query.value(numValue++).toString();
    m_infos.lyrics          = query.value(numValue++).toString();
    m_infos.language        = getLanguageForISO2Code(query.value(numValue++).toString());
/*
    m_isEnabled       = query.value(numValue++).toBool();
    m_title           = query.value(numValue++).toString();
    m_titleSort       = query.value(numValue++).toString();
    m_subTitle        = query.value(numValue++).toString();
    m_grouping        = query.value(numValue++).toString();
    m_artistName      = query.value(numValue++).toString();
    m_artistNameSort  = query.value(numValue++).toString();
    m_albumTitle      = query.value(numValue++).toString();
    m_albumTitleSort  = query.value(numValue++).toString();
    m_albumArtist     = query.value(numValue++).toString();
    m_albumArtistSort = query.value(numValue++).toString();
    m_composer        = query.value(numValue++).toString();
    m_composerSort    = query.value(numValue++).toString();
    m_year            = query.value(numValue++).toInt();
    m_trackNumber     = query.value(numValue++).toInt();
    m_trackCount      = query.value(numValue++).toInt();
    m_discNumber      = query.value(numValue++).toInt();
    m_discCount       = query.value(numValue++).toInt();
    m_genre           = query.value(numValue++).toString();
    m_rating          = query.value(numValue++).toInt();
    m_comments        = query.value(numValue++).toString();
    m_lyrics          = query.value(numValue++).toString();
    m_language        = getLanguageForISO2Code(query.value(numValue++).toString());
*/
    // Lectures
    query.prepare("SELECT play_time FROM play WHERE song_id = ? ORDER BY play_time DESC");
    query.bindValue(0, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    while (query.next())
    {
        m_plays.append(query.value(0).toDateTime());
    }

    //TODO: trier les dates de lecture
}


/**
 * Charge les informations du morceau à partir de ses métadonnées.
 *
 * \todo Implémentation.
 * \todo Retourner un booléen ?
 * \todo Retourner une map ?
 *
 * \param readProperties Indique si on doit lire les propriétés du fichier.
 * \return Booléen valant true si les tags ont pu être lus, false sinon.
 */

bool CSong::loadTags(bool readProperties)
{
    switch (m_format)
    {
        default:
            qWarning() << "CSong::loadTags() : format non géré";
            return false;

        case CSong::FormatMP3:
        {
            TagLib::MPEG::File file(qPrintable(m_fileName), readProperties);

            if (!file.isValid())
            {
                qWarning() << "CSong::loadTags() : impossible de lire le fichier MP3 " << m_fileName;
                return false;
            }

            m_fileSize = file.length();
            
            // Propriétés du morceau
            if (readProperties)
            {
                if (file.audioProperties())
                {
                    //TODO: Récupérer la version de MPEG : file.audioProperties()->version();
                    //TODO: Récupérer le mode stéréo : file.audioProperties()->channelMode();
                    m_bitRate     = file.audioProperties()->bitrate();
                    m_sampleRate  = file.audioProperties()->sampleRate();
                    m_numChannels = file.audioProperties()->channels();
                }
                else
                {
                    qWarning() << "CSong::loadTags() : impossible de récupérer les propriétés du fichier " << m_fileName;
                }
            }

            loadTags(file.APETag(false), m_infos, m_application->getLogMetadata(), m_fileName);
            loadTags(file.ID3v1Tag(true), m_infos, m_application->getLogMetadata(), m_fileName);
            loadTags(file.ID3v2Tag(true), m_infos, m_application->getLogMetadata(), m_fileName);

            break;
        }

        case CSong::FormatOGG:
        {
            TagLib::Ogg::Vorbis::File file(qPrintable(m_fileName), readProperties);

            if (!file.isValid())
            {
                qWarning() << "CSong::loadTags() : impossible de lire le fichier Ogg " << m_fileName;
                return false;
            }

            m_fileSize = file.length();
            
            // Propriétés du morceau
            if (readProperties)
            {
                if (file.audioProperties())
                {
                    m_bitRate     = file.audioProperties()->bitrate();
                    m_sampleRate  = file.audioProperties()->sampleRate();
                    m_numChannels = file.audioProperties()->channels();
                }
                else
                {
                    qWarning() << "CSong::loadTags() : impossible de récupérer les propriétés du fichier " << m_fileName;
                }
            }

            loadTags(file.tag(), m_infos, m_application->getLogMetadata(), m_fileName);

            break;
        }

        case CSong::FormatFLAC:
        {
            TagLib::FLAC::File file(qPrintable(m_fileName), readProperties);

            if (!file.isValid())
            {
                qWarning() << "CSong::loadTags() : impossible de lire le fichier FLAC " << m_fileName;
                return false;
            }

            m_fileSize = file.length();
            
            // Propriétés du morceau
            if (readProperties)
            {
                if (file.audioProperties())
                {
                    m_bitRate     = file.audioProperties()->bitrate();
                    m_sampleRate  = file.audioProperties()->sampleRate();
                    m_numChannels = file.audioProperties()->channels();
                }
                else
                {
                    qWarning() << "CSong::loadTags() : impossible de récupérer les propriétés du fichier " << m_fileName;
                }
            }

            loadTags(file.ID3v1Tag(true), m_infos, m_application->getLogMetadata(), m_fileName);
            loadTags(file.ID3v2Tag(true), m_infos, m_application->getLogMetadata(), m_fileName);
            loadTags(file.xiphComment(true), m_infos, m_application->getLogMetadata(), m_fileName);

            break;
        }
    }

    return true;
}


/**
 * Met à jour les métadonnées du fichier.
 * Pour le format MP3, les tags ID3v1 et ID3v2 sont écrits, et les tags APE enlevés.
 * Pour le format FLAC, les tags xiphComment, ID3v1 et ID3v2 sont écrits.
 * Seuls les tags gérés par l'application sont mis à jour, les autres sont conservés.
 *
 * \todo Implémentation.
 */

void CSong::writeTags(void) const
{
    qDebug() << "CSong::writeTags()";

    return; // Read-only !

    //...
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
 * Cherche l'identifiant d'un fichier  en base de données.
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

    return -1;
}


/**
 * Crée un morceau à partir d'un fichier.
 * Si le fichier est déjà présent en base de données, la méthode retourne NULL.
 *
 * \todo Implémentation.
 *
 * \param fileName Fichier à lire.
 * \return Pointeur sur le son crée, ou NULL en cas d'erreur.
 */

CSong * CSong::loadFromFile(CApplication * application, const QString& fileName)
{
    if (getId(application, fileName) >= 0)
    {
        qWarning() << "CSong::loadFromFile() : le fichier " << fileName << " est déjà dans la médiathèque";
        return NULL;
    }

    FMOD_RESULT res;
    FMOD::Sound * sound;

    // Chargement du son
    res = application->getSoundSystem()->createStream(qPrintable(fileName), FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, NULL, &sound);

    if (res != FMOD_OK || !sound)
    {
        qWarning() << "CSong::loadFromFile() : erreur lors du chargement du son avec FMOD";
        return NULL;
    }

    // Création du morceau
    CSong * song = new CSong(application);

    song->m_isModified = true;
    song->m_sound      = sound;
    song->m_fileName   = fileName;

    // Recherche de la durée du morceau
    FMOD_SOUND_TYPE type;

    res = sound->getLength(reinterpret_cast<unsigned int *>(&(song->m_duration)), FMOD_TIMEUNIT_MS);
    if (res != FMOD_OK)
    {
        qWarning() << "CSong::loadFromFile() : impossible de calculer la durée du morceau";
        song->m_duration = 0;
    }

    // Recherche du format du morceau
    res = sound->getFormat(&type, NULL, NULL, NULL);

    if (res != FMOD_OK)
    {
        qWarning() << "CSong::loadFromFile() : impossible de déterminer le format du morceau";
    }
    else
    {
        switch (type)
        {
            default:
                qWarning() << "CSong::loadFromFile() : format inconnu";
                delete song;
                return NULL;

            case FMOD_SOUND_TYPE_MPEG:
                song->m_format = CSong::FormatMP3;
                break;

            case FMOD_SOUND_TYPE_OGGVORBIS:
                song->m_format = CSong::FormatOGG;
                break;

            case FMOD_SOUND_TYPE_FLAC:
                song->m_format = CSong::FormatFLAC;
                break;
        }
    }

    //sound->release();
    
    // Chargement des métadonnées
    if (!song->loadTags(true))
    {
        delete song;
        return NULL;
    }

    song->updateDatabase();
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

    // Liste des morceaux
    if (!query.exec("SELECT"
                        " song_id,"
                        " song_filename,"
                        " song_filesize,"
                        " song_bitrate,"
                        " song_sample_rate,"
                        " song_format,"
                        " song_channels,"
                        " song_duration,"
                        " song_creation,"
                        " song_modification,"
                        " song_enabled,"
                        " song_title,"
                        " song_title_sort,"
                        " song_subtitle,"
                        " song_grouping,"
                        " song_artist.artist_name,"
                        " song_artist.artist_name_sort,"
                        " album_title,"
                        " album_title_sort, "
                        " album_artist.artist_name,"
                        " album_artist.artist_name_sort,"
                        " song_composer,"
                        " song_composer_sort,"
                        " song_year,"
                        " song_track_number,"
                        " song_track_count,"
                        " song_disc_number,"
                        " song_disc_count,"
                        " genre_name,"
                        " song_rating,"
                        " song_comments,"
                        " song_bpm,"
                        " song_lyrics,"
                        " song_language"
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

        song->m_id              = query.value(numValue++).toInt();
        song->m_fileName        = query.value(numValue++).toString();
        song->m_fileSize        = query.value(numValue++).toInt();
        song->m_bitRate         = query.value(numValue++).toInt();
        song->m_sampleRate      = query.value(numValue++).toInt();
        song->m_format          = CSong::getFormatFromInteger(query.value(numValue++).toInt());
        song->m_numChannels     = query.value(numValue++).toInt();
        song->m_duration        = query.value(numValue++).toInt();
        song->m_creation        = query.value(numValue++).toDateTime();
        song->m_modification    = query.value(numValue++).toDateTime();
        
        song->m_infos.isEnabled       = query.value(numValue++).toBool();
        song->m_infos.title           = query.value(numValue++).toString();
        song->m_infos.titleSort       = query.value(numValue++).toString();
        song->m_infos.subTitle        = query.value(numValue++).toString();
        song->m_infos.grouping        = query.value(numValue++).toString();
        song->m_infos.artistName      = query.value(numValue++).toString();
        song->m_infos.artistNameSort  = query.value(numValue++).toString();
        song->m_infos.albumTitle      = query.value(numValue++).toString();
        song->m_infos.albumTitleSort  = query.value(numValue++).toString();
        song->m_infos.albumArtist     = query.value(numValue++).toString();
        song->m_infos.albumArtistSort = query.value(numValue++).toString();
        song->m_infos.composer        = query.value(numValue++).toString();
        song->m_infos.composerSort    = query.value(numValue++).toString();
        song->m_infos.year            = query.value(numValue++).toInt();
        song->m_infos.trackNumber     = query.value(numValue++).toInt();
        song->m_infos.trackCount      = query.value(numValue++).toInt();
        song->m_infos.discNumber      = query.value(numValue++).toInt();
        song->m_infos.discCount       = query.value(numValue++).toInt();
        song->m_infos.genre           = query.value(numValue++).toString();
        song->m_infos.rating          = query.value(numValue++).toInt();
        song->m_infos.comments        = query.value(numValue++).toString();
        song->m_infos.bpm             = query.value(numValue++).toInt();
        song->m_infos.lyrics          = query.value(numValue++).toString();
        song->m_infos.language        = getLanguageForISO2Code(query.value(numValue++).toString());

        // Lectures
        QSqlQuery query2(application->getDataBase());

        query2.prepare("SELECT play_time FROM play WHERE song_id = ? ORDER BY play_time DESC");
        query2.bindValue(0, song->m_id);

        if (!query2.exec())
        {
            application->showDatabaseError(query2.lastError().text(), query2.lastQuery(), __FILE__, __LINE__);
        }
        else
        {
            while (query2.next())
            {
                song->m_plays.append(query2.value(0).toDateTime());
            }
        }

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

    if (fileSize >= 1024)
    {
        if (fileSize >= 1024 * 1024)
        {
            if (fileSize >= 1024 * 1024 * 1024)
            {
                // Plus de 1 Gio, inutile d'aller plus loin...
                float fileSizeDisplay = static_cast<float>(static_cast<int>(static_cast<float>(10 * fileSize) / (1024*1024*1024))) / 10;
                return tr("%1 Gio").arg(fileSizeDisplay);
            }

            // Moins de 1 Gio
            float fileSizeDisplay = static_cast<float>(static_cast<int>(static_cast<float>(10 * fileSize) / (1024*1024))) / 10;
            return tr("%1 Mio").arg(fileSizeDisplay);
        }

        // Moins de 1 Mio
        float fileSizeDisplay = static_cast<float>(static_cast<int>(static_cast<float>(10 * fileSize) / 1024)) / 10;
        return tr("%1 Kio").arg(fileSizeDisplay);
    }

    // Moins de 1 Kio
    return tr("%n byte(s)", "", fileSize);
}


void CSong::setEnabled(bool enabled)
{
    if (m_infos.isEnabled != enabled)
    {
        m_infos.isEnabled = enabled;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setGrouping(const QString& grouping)
{
    if (m_infos.grouping != grouping)
    {
        m_infos.grouping = grouping;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setTitleSort(const QString& title)
{
    if (m_infos.titleSort != title)
    {
        m_infos.titleSort = title;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setArtistNameSort(const QString& artistName)
{
    if (m_infos.artistNameSort != artistName)
    {
        m_infos.artistNameSort = artistName;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setAlbumTitleSort(const QString& albumTitle)
{
    if (m_infos.albumTitleSort != albumTitle)
    {
        m_infos.albumTitleSort = albumTitle;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setAlbumArtistSort(const QString& albumArtist)
{
    if (m_infos.albumArtistSort != albumArtist)
    {
        m_infos.albumArtistSort = albumArtist;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setComposerSort(const QString& composer)
{
    if (m_infos.composerSort != composer)
    {
        m_infos.composerSort = composer;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setYear(int year)
{
    Q_ASSERT(year >= 0);

    if (m_infos.year != year)
    {
        m_infos.year = year;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
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
        if (!m_multiModification) emit songModified();
    }
}


void CSong::startPlay(void)
{
    Q_CHECK_PTR(m_sound);

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
    }
    else
    {
        m_sound = NULL;
        m_channel = NULL;
        return;
    }
}


void CSong::startMultiModification(void)
{
    m_multiModification = true;
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
    {
        return true;
    }

    FMOD_RESULT res;

    // Chargement du son
    res = m_application->getSoundSystem()->createStream(qPrintable(m_fileName), FMOD_LOOP_OFF | FMOD_HARDWARE | FMOD_2D, NULL, &m_sound);

    if (res == FMOD_OK && m_sound)
    {
        // Durée du son
        unsigned int length;
        res = m_sound->getLength(&length, FMOD_TIMEUNIT_MS);

        if (res == FMOD_OK)
        {
            // Mise à jour de la durée du morceau
            if (m_duration != length)
            {
                qWarning() << "CSong::loadSound() : La durée du morceau doit être mise à jour.";
                m_duration = length;

                QSqlQuery query(m_application->getDataBase());

                query.prepare("UPDATE song SET song_duration = ? WHERE song_id = ?");

                query.bindValue(0, m_duration);
                query.bindValue(1, m_id);

                if (!query.exec())
                {
                    m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                }
            }

            return true;
        }
    }

    qWarning() << "CSong::loadSound() : Échec du chargement du morceau " << m_fileName;

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
    {
        return;
    }

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
    }
}


/**
 * Met à jour la base de données si les informations du morceau ont été modifiées.
 */

void CSong::updateDatabase(void)
{
    if (m_isModified)
    {
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
                              "song_filename, "
                              "song_filesize, "
                              "song_bitrate, "
                              "song_sample_rate, "
                              "song_format, "
                              "song_channels, "
                              "song_duration, "
                              "song_creation, "
                              "song_modification, "
                              "song_enabled, "
                              "song_title, "
                              "song_title_sort, "
                              "song_subtitle, "
                              "song_grouping, "
                              "artist_id, "
                              "album_id, "
                              "album_artist_id, "
                              "song_composer, "
                              "song_composer_sort, "
                              "song_year, "
                              "song_track_number, "
                              "song_track_count, "
                              "song_disc_number, "
                              "song_disc_count, "
                              "genre_id, "
                              "song_rating, "
                              "song_comments, "
                              "song_bpm, "
                              "song_lyrics, "
                              "song_language, "
                              "song_play_count, "
                              "song_play_time"
                          ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

            int numValue = 0;

            query.bindValue(numValue++, m_fileName);
            query.bindValue(numValue++, m_fileSize);
            query.bindValue(numValue++, m_bitRate);
            query.bindValue(numValue++, m_sampleRate);
            query.bindValue(numValue++, m_format);
            query.bindValue(numValue++, m_numChannels);
            query.bindValue(numValue++, m_duration);
            query.bindValue(numValue++, m_creation);
            query.bindValue(numValue++, m_modification);
            query.bindValue(numValue++, (m_infos.isEnabled ? 1 : 0));
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
            query.bindValue(numValue++, 0);  // Play count
            query.bindValue(numValue++, ""); // Last play time

            if (!query.exec())
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return;
            }

            m_id = query.lastInsertId().toInt();
        }
        // Mise à jour
        else
        {
            query.prepare("UPDATE song SET "
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
                              "song_language      = ? "
                          "WHERE song_id = ?");

            int numValue = 0;

            query.bindValue(numValue++, m_modification);
            query.bindValue(numValue++, (m_infos.isEnabled ? 1 : 0));
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
            query.bindValue(numValue++, m_id);

            if (!query.exec())
            {
                m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                return;
            }
        }

        m_isModified = false;

        if (m_multiModification)
        {
            m_multiModification = false;
            emit songModified();
        }
    }
}


/**
 * Méthode appelée quand la lecture du morceau est terminée.
 */

void CSong::emitPlayEnd(void)
{
    const QDateTime currentTime = QDateTime::currentDateTime();

    QSqlQuery query(m_application->getDataBase());
    query.prepare("UPDATE song SET "
                      "song_play_count = song_play_count + 1,"
                      "song_play_time  = ? "
                  "WHERE song_id = ?");

    query.bindValue(0, currentTime);
    query.bindValue(1, m_id);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    query.prepare("INSERT INTO play (song_id, play_time) VALUES (?, ?)");
    query.bindValue(0, m_id);
    query.bindValue(1, currentTime);

    if (!query.exec())
    {
        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
        return;
    }

    m_plays.append(currentTime);

    emit playEnd();
}


/**
 * Lit les informations d'un morceau depuis des tags ID3 version 1.
 *
 * \param tags     Métadonnées.
 * \param infos    Structure à remplir.
 * \param fileName Nom du fichier contenant ces métadonnées.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::loadTags(TagLib::ID3v1::Tag * tags, TSongInfos& infos, QFile * logFile, const QString& fileName)
{
    if (!tags)
        return false;

    //infos = TSongInfos();

    TagLib::String str;

    // Titre
    str = tags->title();
    if (!str.isNull())
        infos.title = QString::fromUtf8(str.toCString(true));

    // Artiste
    str = tags->artist();
    if (!str.isNull())
        infos.artistName = QString::fromUtf8(str.toCString(true));

    // Album
    str = tags->album();
    if (!str.isNull())
        infos.albumTitle = QString::fromUtf8(str.toCString(true));

    // Année
    infos.year = tags->year();
    
    // Numéro de piste
    infos.trackNumber = tags->track();
    
    // Genre
    str = tags->genre();
    if (!str.isNull())
        infos.genre = QString::fromUtf8(str.toCString(true));

    // Commentaires
    str = tags->comment();
    if (!str.isNull())
        infos.comments = QString::fromUtf8(str.toCString(true));

    return true;
}


/**
 * Lit les informations d'un morceau depuis des tags ID3 version 2.
 *
 * \param tags     Métadonnées.
 * \param infos    Structure à remplir.
 * \param fileName Nom du fichier contenant ces métadonnées.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::loadTags(TagLib::ID3v2::Tag * tags, TSongInfos& infos, QFile * logFile, const QString& fileName)
{
    if (!tags)
        return false;

    //infos = TSongInfos();
    
    // Log
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   Tags ID3v2\n";
    stream << "----------------------------------------\n";
    stream << " Fichier : " << fileName << '\n';
    stream << " Date    : " << QDateTime::currentDateTime().toString() << '\n';
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
            stream << "Erreur : plusieurs tags TIT2";
        infos.title = QString::fromUtf8(tagList.front()->toString().toCString(true));
    }

    // Sous-titre
    tagList = tags->frameList("TIT3");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TIT3";
        infos.subTitle = QString::fromUtf8(tagList.front()->toString().toCString(true));
    }

    // Artiste
    tagList = tags->frameList("TPE1");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TPE1";
        infos.artistName = QString::fromUtf8(tagList.front()->toString().toCString(true));
    }

    // Album
    tagList = tags->frameList("TALB");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TALB";
        infos.albumTitle = QString::fromUtf8(tagList.front()->toString().toCString(true));
    }

    // Artiste de l'album
    tagList = tags->frameList("TPE2");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TPE2";
        infos.albumArtist = QString::fromUtf8(tagList.front()->toString().toCString(true));
    }

    // Compositeur
    tagList = tags->frameList("TCOM");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TCOM";
        infos.composer = QString::fromUtf8(tagList.front()->toString().toCString(true));
    }

    // Titre pour le tri
    tagList = tags->frameList("TSOT");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TSOT";
        infos.titleSort = QString::fromUtf8(tagList.front()->toString().toCString(true));
    }

    // Artiste pour le tri
    tagList = tags->frameList("TSOP");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TSOP";
        infos.artistNameSort = QString::fromUtf8(tagList.front()->toString().toCString(true));
    }

    // Album pour le tri
    tagList = tags->frameList("TSOA");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TSOA";
        infos.albumTitleSort = QString::fromUtf8(tagList.front()->toString().toCString(true));
    }

    // Année
    tagList = tags->frameList("TDRC");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TDRC";
        TagLib::String str = tagList.front()->toString();

        if (!str.isNull() && str.size() >= 4)
        {
            bool ok;
            infos.year = str.substr(0, 4).toInt(&ok);
            if (!ok)
            {
                stream << "Erreur : tag TDRC invalide";
                infos.year = 0;
            }
        }
    }

    // Regroupement
    tagList = tags->frameList("TIT1");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TIT1";
        infos.grouping = QString::fromUtf8(tagList.front()->toString().toCString(true));
    }

    // Numéro de piste
    tagList = tags->frameList("TRCK");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TRCK";

        QString str = QString::fromUtf8(tagList.front()->toString().toCString(true));

        if (str.contains('/'))
        {
            QStringList strSplit = str.split('/');

            if (strSplit.size() == 2)
            {
                bool ok;
                infos.trackNumber = strSplit[0].toInt(&ok);
                if (!ok)
                    stream << "Erreur : tag TRCK invalide";
                infos.trackCount = strSplit[1].toInt(&ok);
                if (!ok)
                    stream << "Erreur : tag TRCK invalide";
            }
            else
            {
                stream << "Erreur : tag TRCK invalide";
            }
        }
        else
        {
            bool ok;
            infos.trackNumber = str.toInt(&ok);
            if (!ok)
                stream << "Erreur : tag TRCK invalide";
        }
    }

    // Numéro de disque
    tagList = tags->frameList("TPOS");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TPOS";
        QString str = QString::fromUtf8(tagList.front()->toString().toCString(true));

        if (str.contains('/'))
        {
            QStringList strSplit = str.split('/');

            if (strSplit.size() == 2)
            {
                bool ok;
                infos.discNumber = strSplit[0].toInt(&ok);
                if (!ok)
                    stream << "Erreur : tag TPOS invalide";
                infos.discCount = strSplit[1].toInt(&ok);
                if (!ok)
                    stream << "Erreur : tag TPOS invalide";
            }
            else
            {
                stream << "Erreur : tag TPOS invalide";
            }
        }
        else
        {
            bool ok;
            infos.discNumber = str.toInt(&ok);
            if (!ok)
                stream << "Erreur : tag TPOS invalide";
        }
    }

    // Genre
    tagList = tags->frameList("TCON");
    if (!tagList.isEmpty())
    {
        infos.genre = QString::fromUtf8(tags->genre().toCString(true));
    }

    // Commentaires
    tagList = tags->frameList("COMM");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags COMM";

        TagLib::ID3v2::CommentsFrame * frame = dynamic_cast<TagLib::ID3v2::CommentsFrame *>(tagList.front());

        if (frame)
        {
            infos.comments = QString::fromUtf8(frame->text().toCString(true));
        }
    }

    // BPM
    tagList = tags->frameList("TBPM");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TBPM";
        QString str = QString::fromUtf8(tagList.front()->toString().toCString(true));

        bool ok;
        infos.bpm = str.toInt(&ok);
        if (!ok)
            stream << "Erreur : tag TBPM invalide";
    }

    // Paroles
    tagList = tags->frameList("USLT");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags USLT";

        TagLib::ID3v2::UnsynchronizedLyricsFrame * frame = dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame *>(tagList.front());

        if (frame)
        {
            infos.lyrics = QString::fromUtf8(frame->text().toCString(true));
            TagLib::ByteVector lng = frame->language();
            
            if (lng.size() != 3)
            {
                stream << "Erreur : langue du tag USLT invalide";
            }
            else
            {
                infos.language = getLanguageForISO3Code(QByteArray(lng.data(), 3));
            }
        }
    }

    // Langue
    tagList = tags->frameList("TLAN");
    if (!tagList.isEmpty())
    {
        if (tagList.size() > 1)
            stream << "Erreur : plusieurs tags TLAN";

        QString str = QString::fromUtf8(tagList.front()->toString().toCString(false));

        if (str.size() != 3)
        {
            stream << "Erreur : tag TLAN invalide";
        }
        else
        {
            TLanguage lng = getLanguageForISO3Code(qPrintable(str));

            if (infos.language != LangUnknown && lng != infos.language)
            {
                stream << "Erreur : la langue des paroles et différente de la langue du morceau";
            }

            infos.language = lng;
        }
    }

    return true;
}


/**
 * Lit les informations d'un morceau depuis des tags APE.
 *
 * \param tags     Métadonnées.
 * \param infos    Structure à remplir.
 * \param fileName Nom du fichier contenant ces métadonnées.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::loadTags(TagLib::APE::Tag * tags, TSongInfos& infos, QFile * logFile, const QString& fileName)
{
    if (!tags)
        return false;

    //infos = TSongInfos();

    const TagLib::APE::ItemListMap tagMap = tags->itemListMap();

    // Log
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   Tags APE\n";
    stream << "----------------------------------------\n";
    stream << " Fichier : " << fileName << '\n';
    stream << " Date    : " << QDateTime::currentDateTime().toString() << '\n';
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
        infos.title = QString::fromUtf8(tagMap["TITLE"].toString().toCString(true));
    }

    // Sous-titre
    if (!tagMap["SUBTITLE"].isEmpty())
    {
        infos.subTitle = QString::fromUtf8(tagMap["SUBTITLE"].toString().toCString(true));
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
                    stream << "Erreur : tag TRACK invalide";
                infos.trackCount = strSplit[1].toInt(&ok);
                if (!ok)
                    stream << "Erreur : tag TRACK invalide";
            }
            else
            {
                stream << "Erreur : tag TRACK invalide";
            }
        }
        else
        {
            bool ok;
            infos.trackNumber = str.toInt(&ok);
            if (!ok)
                stream << "Erreur : tag TRACK invalide";
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
                    stream << "Erreur : tag MEDIA invalide";
                infos.discCount = strSplit[1].toInt(&ok);
                if (!ok)
                    stream << "Erreur : tag MEDIA invalide";
            }
            else
            {
                stream << "Erreur : tag MEDIA invalide";
            }
        }
        else
        {
            bool ok;
            infos.discNumber = str.toInt(&ok);
            if (!ok)
                stream << "Erreur : tag MEDIA invalide";
        }
    }

    // Langue
    if (!tagMap["LANGUAGE"].isEmpty())
    {
        qDebug() << "CSong::loadTags() : APE : langue = " << QString::fromUtf8(tagMap["LANGUAGE"].toString().toCString(true));
        //infos.language = QString::fromUtf8(tagMap["LANGUAGE"].toString().toCString(true));
    }

    return false;
}


/**
 * Lit les informations d'un morceau depuis des tags Xiph Comment.
 *
 * \param tags     Métadonnées.
 * \param infos    Structure à remplir.
 * \param fileName Nom du fichier contenant ces métadonnées.
 * \return Booléen indiquant le succès de l'opération.
 */

bool CSong::loadTags(TagLib::Ogg::XiphComment * tags, TSongInfos& infos, QFile * logFile, const QString& fileName)
{
    if (!tags)
        return false;

    //infos = TSongInfos();

    const TagLib::Ogg::FieldListMap tagMap = tags->fieldListMap();

    // Log
    QTextStream stream(logFile);
    stream << "========================================\n";
    stream << "   Tags XiphComment\n";
    stream << "----------------------------------------\n";
    stream << " Fichier : " << fileName << '\n';
    stream << " Date    : " << QDateTime::currentDateTime().toString() << '\n';
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
        infos.title = QString::fromUtf8(tagMap["TITLE"].toString().toCString(true));
    }

    // Album
    if (!tagMap["ALBUM"].isEmpty())
    {
        infos.albumTitle = QString::fromUtf8(tagMap["ALBUM"].toString().toCString(true));
    }

    // Artiste
    if (!tagMap["ARTIST"].isEmpty())
    {
        infos.artistName = QString::fromUtf8(tagMap["ARTIST"].toString().toCString(true));
    }

    // Artiste pour le tri
    if (!tagMap["ARTISTSORT"].isEmpty())
    {
        infos.artistNameSort = QString::fromUtf8(tagMap["ARTISTSORT"].toString().toCString(true));
    }

    // Artiste de l'album
    if (!tagMap["ALBUMARTIST"].isEmpty())
    {
        infos.albumArtist = QString::fromUtf8(tagMap["ALBUMARTIST"].toString().toCString(true));
    }

    // Genre
    if (!tagMap["GENRE"].isEmpty())
    {
        infos.genre = QString::fromUtf8(tagMap["GENRE"].toString().toCString(true));
    }

    // Année
    if (!tagMap["DATE"].isEmpty())
    {
        QString year = QString::fromUtf8(tagMap["DATE"].toString().toCString(true));

        if (year.size() != 4)
        {
            stream << "Erreur : tag DATE invalide";
        }
        else
        {
            bool ok;
            infos.year = year.toInt(&ok);
            if (!ok)
                stream << "Erreur : tag DATE invalide";
        }
    }

    // Numéro de piste
    if (!tagMap["TRACKNUMBER"].isEmpty())
    {
        QString trackNumber = QString::fromUtf8(tagMap["TRACKNUMBER"].toString().toCString(true));
        bool ok;
        infos.trackNumber = trackNumber.toInt(&ok);

        if (!ok || infos.trackNumber <= 0)
        {
            stream << "Erreur : tag TRACKNUMBER invalide";
            infos.trackNumber = 0;
        }
    }

    // Nombre de pistes
    if (!tagMap["TRACKTOTAL"].isEmpty())
    {
        QString trackCount = QString::fromUtf8(tagMap["TRACKTOTAL"].toString().toCString(true));
        bool ok;
        infos.trackCount = trackCount.toInt(&ok);

        if (!ok || infos.trackCount <= 0)
        {
            stream << "Erreur : tag TRACKTOTAL invalide";
            infos.trackCount = 0;
        }
    }

    if (!tagMap["TOTALTRACKS"].isEmpty())
    {
        if (!tagMap["TRACKTOTAL"].isEmpty())
        {
            stream << "Erreur : les tags TRACKTOTAL et TOTALTRACKS sont présents tous les deux";
        }

        QString trackCount = QString::fromUtf8(tagMap["TOTALTRACKS"].toString().toCString(true));
        bool ok;
        infos.trackCount = trackCount.toInt(&ok);

        if (!ok || infos.trackCount <= 0)
        {
            stream << "Erreur : tag TOTALTRACKS invalide";
            infos.trackCount = 0;
        }
    }

    // BPM
    if (!tagMap["TEMPO"].isEmpty())
    {
        QString bpm = QString::fromUtf8(tagMap["TEMPO"].toString().toCString(true));
        bool ok;
        infos.bpm = bpm.toInt(&ok);

        if (!ok || infos.bpm <= 0)
        {
            stream << "Erreur : tag TEMPO invalide";
            infos.bpm = 0;
        }
    }

    if (!tagMap["BPM"].isEmpty())
    {
        if (!tagMap["TEMPO"].isEmpty())
        {
            stream << "Erreur : les tags TEMPO et BPM sont présents tous les deux";
        }

        QString bpm = QString::fromUtf8(tagMap["BPM"].toString().toCString(true));
        bool ok;
        infos.bpm = bpm.toInt(&ok);

        if (!ok || infos.bpm <= 0)
        {
            stream << "Erreur : tag BPM invalide";
            infos.bpm = 0;
        }
    }

    // Paroles
    if (!tagMap["LYRICS"].isEmpty())
    {
        infos.lyrics = QString::fromUtf8(tagMap["LYRICS"].toString().toCString(true));
    }

    // Autres tags
/*
    Autres tags :
    - "ALBUM ARTIST"
    - "ENCODER"
    - "ENSEMBLE"
*/

    return false;
}

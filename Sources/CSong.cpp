
#include "CSong.hpp"
#include "CApplication.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

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
    m_trackTotal        (0),
    m_discNumber        (0),
    m_discTotal         (0),
    m_genre             (""),
    m_rating            (0),
    m_comments          (""),
    m_bpm               (0),
    m_lyrics            (""),
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
    m_trackTotal        (0),
    m_discNumber        (0),
    m_discTotal         (0),
    m_genre             (""),
    m_rating            (0),
    m_comments          (""),
    m_bpm               (0),
    m_lyrics            (""),
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
    qDebug() << "CSong::~CSong()";
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
                      " song_track_total,"
                      " song_disc_number,"
                      " song_disc_total,"
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
    m_trackTotal      = query.value(numValue++).toInt();
    m_discNumber      = query.value(numValue++).toInt();
    m_discTotal       = query.value(numValue++).toInt();
    m_genre           = query.value(numValue++).toString();
    m_rating          = query.value(numValue++).toInt();
    m_comments        = query.value(numValue++).toString();
    m_lyrics          = query.value(numValue++).toString();
    m_language        = getLanguageForISO2Code(query.value(numValue++).toString());

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
 */

void CSong::loadTags(void)
{
    switch (m_format)
    {
        case CSong::FormatMP3:
        {
            TagLib::MPEG::File file(qPrintable(m_fileName), false);

            if (!file.isValid())
            {
                qWarning() << "CSong::loadTags() : impossible de lire le fichier MP3 " << m_fileName;
                return;
            }

            //...

            break;
        }

        case CSong::FormatOGG:
        {
            TagLib::Ogg::Vorbis::File file(qPrintable(m_fileName), false);

            if (!file.isValid())
            {
                qWarning() << "CSong::loadTags() : impossible de lire le fichier Ogg " << m_fileName;
                return;
            }

            //...

            break;
        }

        case CSong::FormatFLAC:
        {
            TagLib::FLAC::File file(qPrintable(m_fileName), false);

            if (!file.isValid())
            {
                qWarning() << "CSong::loadTags() : impossible de lire le fichier FLAC " << m_fileName;
                return;
            }

            //...

            break;
        }
    }
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
                song->m_format = CSong::FormatUnknown;
                break;

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
    switch (song->m_format)
    {
        case CSong::FormatMP3:
        {
            TagLib::MPEG::File file(qPrintable(fileName));

            if (!file.isValid())
            {
                qWarning() << "CSong::loadFromFile() : impossible de lire le fichier MP3 " << fileName;
                delete song;
                return NULL;
            }

            song->m_fileSize = file.length();

            // Propriétés du morceau
            //TODO: Récupérer la version de MPEG : file.audioProperties()->version();
            //TODO: Récupérer le mode stéréo : file.audioProperties()->channelMode();
            song->m_bitRate     = file.audioProperties()->bitrate();
            song->m_sampleRate  = file.audioProperties()->sampleRate();
            song->m_numChannels = file.audioProperties()->channels();

            TagLib::ID3v2::Tag * tagID3v2 = file.ID3v2Tag(true);
            TagLib::ID3v1::Tag * tagID3v1 = file.ID3v1Tag(true);
            TagLib::APE::Tag * tagAPE = file.APETag(false);

            TagLib::ID3v2::FrameList tagID3v2List;
/*
            // DEBUG
            TagLib::ID3v2::FrameListMap tagMap = tagID3v2->frameListMap();
            for (TagLib::ID3v2::FrameListMap::ConstIterator it = tagMap.begin(); it != tagMap.end(); ++it)
            {
                QString tagKey = QByteArray(it->first.data(), it->first.size());

                for (TagLib::ID3v2::FrameList::ConstIterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    qDebug() << tagKey << ":" << (*it2)->toString().toCString(true);
                }
            }
*/
            // Titre
            tagID3v2List = tagID3v2->frameList("TIT2");
            if (tagID3v2List.isEmpty())
            {
                TagLib::String str;
                if (tagID3v1)
                    str = tagID3v1->title();
                if (str.isNull() && tagAPE)
                    str = tagAPE->title();
                song->m_title = QString::fromUtf8(str.toCString(true));
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TIT2";
                song->m_title = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));
            }

            // Sous-titre
            tagID3v2List = tagID3v2->frameList("TIT3");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TIT3";
                song->m_subTitle = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));
            }

            // Artiste
            tagID3v2List = tagID3v2->frameList("TPE1");
            if (tagID3v2List.isEmpty())
            {
                TagLib::String str;
                if (tagID3v1)
                    str = tagID3v1->artist();
                if (str.isNull() && tagAPE)
                    str = tagAPE->artist();
                song->m_artistName = QString::fromUtf8(str.toCString(true));
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TPE1";
                song->m_artistName = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));
            }

            // Album
            tagID3v2List = tagID3v2->frameList("TALB");
            if (tagID3v2List.isEmpty())
            {
                TagLib::String str;
                if (tagID3v1)
                    str = tagID3v1->album();
                if (str.isNull() && tagAPE)
                    str = tagAPE->album();
                song->m_albumTitle = QString::fromUtf8(str.toCString(true));
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TALB";
                song->m_albumTitle = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));
            }

            // Artiste de l'album
            tagID3v2List = tagID3v2->frameList("TPE2");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TPE2";
                song->m_albumArtist = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));
            }

            // Compositeur
            tagID3v2List = tagID3v2->frameList("TCOM");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TCOM";
                song->m_composer = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));
            }

            // Titre pour le tri
            tagID3v2List = tagID3v2->frameList("TSOT");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TSOT";
                song->m_titleSort = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));
            }

            // Artiste pour le tri
            tagID3v2List = tagID3v2->frameList("TSOP");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TSOP";
                song->m_artistNameSort = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));
            }

            // Album pour le tri
            tagID3v2List = tagID3v2->frameList("TSOA");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TSOA";
                song->m_albumTitleSort = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));
            }

            // Année
            tagID3v2List = tagID3v2->frameList("TDRC");
            if (tagID3v2List.isEmpty())
            {
                song->m_year = tagID3v1->year();

                if (song->m_year == 0)
                {
                    //TODO: APE
                }
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TDRC";
                TagLib::String str = tagID3v2List.front()->toString();

                if (!str.isNull() && str.size() >= 4)
                {
                    bool ok;
                    song->m_year = str.substr(0, 4).toInt(&ok);
                    if (!ok)
                    {
                        qWarning() << "CSong::loadFromFile() : ID3v2 : tag TDRC invalide";
                        song->m_year = 0;
                    }
                }
            }

            // Regroupement
            tagID3v2List = tagID3v2->frameList("TIT1");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TIT1";
                song->m_grouping = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));
            }

            // Numéro de piste
            tagID3v2List = tagID3v2->frameList("TRCK");
            if (tagID3v2List.isEmpty())
            {
                song->m_trackNumber = tagID3v1->track();

                if (song->m_trackNumber == 0)
                {
                    //TODO: APE
                }
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TRCK";

                QString str = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));

                if (str.contains('/'))
                {
                    QStringList strSplit = str.split('/');

                    if (strSplit.size() == 2)
                    {
                        bool ok;
                        song->m_trackNumber = strSplit[0].toInt(&ok);
                        if (!ok) qWarning() << "CSong::loadFromFile() : ID3v2 : tag TRCK invalide";
                        song->m_trackTotal = strSplit[1].toInt(&ok);
                        if (!ok) qWarning() << "CSong::loadFromFile() : ID3v2 : tag TRCK invalide";
                    }
                    else
                    {
                        qWarning() << "CSong::loadFromFile() : ID3v2 : tag TRCK invalide";
                    }
                }
                else
                {
                    bool ok;
                    song->m_trackNumber = str.toInt(&ok);
                    if (!ok) qWarning() << "CSong::loadFromFile() : ID3v2 : tag TRCK invalide";
                }
            }

            // Numéro de disque
            tagID3v2List = tagID3v2->frameList("TPOS");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TPOS";
                QString str = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));

                if (str.contains('/'))
                {
                    QStringList strSplit = str.split('/');

                    if (strSplit.size() == 2)
                    {
                        bool ok;
                        song->m_discNumber = strSplit[0].toInt(&ok);
                        if (!ok) qWarning() << "CSong::loadFromFile() : ID3v2 : tag TPOS invalide";
                        song->m_discTotal = strSplit[1].toInt(&ok);
                        if (!ok) qWarning() << "CSong::loadFromFile() : ID3v2 : tag TPOS invalide";
                    }
                    else
                    {
                        qWarning() << "CSong::loadFromFile() : ID3v2 : tag TPOS invalide";
                    }
                }
                else
                {
                    bool ok;
                    song->m_discNumber = str.toInt(&ok);
                    if (!ok) qWarning() << "CSong::loadFromFile() : ID3v2 : tag TPOS invalide";
                }
            }

            // Genre
            tagID3v2List = tagID3v2->frameList("TCON");
            if (tagID3v2List.isEmpty())
            {
                TagLib::String str = tagID3v1->genre();

                if (str.isNull())
                {
                    //TODO: APE
                }
                else
                    song->m_genre = QString::fromUtf8(str.toCString(true));
            }
            else
            {
                song->m_genre = QString::fromUtf8(tagID3v2->genre().toCString(true));
            }

            // Commentaires
            tagID3v2List = tagID3v2->frameList("COMM");
            if (tagID3v2List.isEmpty())
            {
                TagLib::String str = tagID3v1->comment();

                if (str.isNull())
                {
                    //TODO: APE
                }
                else
                    song->m_comments = QString::fromUtf8(str.toCString(true));
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags COMM";

                TagLib::ID3v2::CommentsFrame * frame = dynamic_cast<TagLib::ID3v2::CommentsFrame *>(tagID3v2List.front());

                if (frame)
                {
                    song->m_comments = QString::fromUtf8(frame->text().toCString(true));
                }
            }

            // BPM
            tagID3v2List = tagID3v2->frameList("TBPM");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TBPM";
                QString str = QString::fromUtf8(tagID3v2List.front()->toString().toCString(true));

                bool ok;
                song->m_bpm = str.toInt(&ok);
                if (!ok) qWarning() << "CSong::loadFromFile() : ID3v2 : tag TBPM invalide";
            }

            // Paroles
            tagID3v2List = tagID3v2->frameList("USLT");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags USLT";

                TagLib::ID3v2::UnsynchronizedLyricsFrame * frame = dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame *>(tagID3v2List.front());

                if (frame)
                {
                    song->m_lyrics = QString::fromUtf8(frame->text().toCString(true));
                    TagLib::ByteVector lng = frame->language();
                    Q_ASSERT(lng.size() == 3);
                    song->m_language = getLanguageForISO3Code(QByteArray(lng.data(), 3));
                }
            }

            // Langue
            tagID3v2List = tagID3v2->frameList("TLAN");
            if (tagID3v2List.isEmpty())
            {
                //TODO: APE
            }
            else
            {
                if (tagID3v2List.size() > 1)
                    qWarning() << "CSong::loadFromFile() : ID3v2 : plusieurs tags TLAN";

                QString str = QString::fromUtf8(tagID3v2List.front()->toString().toCString(false));

                if (str.size() != 3)
                {
                    qWarning() << "CSong::loadFromFile() : ID3v2 : tag TLAN invalide";
                }
                else
                {
                    song->m_language = getLanguageForISO3Code(qPrintable(str));
                }
            }

            break;
        }

        case CSong::FormatOGG:
        {
            TagLib::Ogg::Vorbis::File file(qPrintable(fileName));

            if (!file.isValid())
            {
                qWarning() << "CSong::loadFromFile() : impossible de lire le fichier Ogg " << fileName;
                delete song;
                return NULL;
            }

            //...

            break;
        }

        case CSong::FormatFLAC:
        {
            TagLib::FLAC::File file(qPrintable(fileName));

            if (!file.isValid())
            {
                qWarning() << "CSong::loadFromFile() : impossible de lire le fichier FLAC " << fileName;
                delete song;
                return NULL;
            }

            //...

            break;
        }
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
                        " song_track_total,"
                        " song_disc_number,"
                        " song_disc_total,"
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
        
        song->m_isEnabled       = query.value(numValue++).toBool();
        song->m_title           = query.value(numValue++).toString();
        song->m_titleSort       = query.value(numValue++).toString();
        song->m_subTitle        = query.value(numValue++).toString();
        song->m_grouping        = query.value(numValue++).toString();
        song->m_artistName      = query.value(numValue++).toString();
        song->m_artistNameSort  = query.value(numValue++).toString();
        song->m_albumTitle      = query.value(numValue++).toString();
        song->m_albumTitleSort  = query.value(numValue++).toString();
        song->m_albumArtist     = query.value(numValue++).toString();
        song->m_albumArtistSort = query.value(numValue++).toString();
        song->m_composer        = query.value(numValue++).toString();
        song->m_composerSort    = query.value(numValue++).toString();
        song->m_year            = query.value(numValue++).toInt();
        song->m_trackNumber     = query.value(numValue++).toInt();
        song->m_trackTotal      = query.value(numValue++).toInt();
        song->m_discNumber      = query.value(numValue++).toInt();
        song->m_discTotal       = query.value(numValue++).toInt();
        song->m_genre           = query.value(numValue++).toString();
        song->m_rating          = query.value(numValue++).toInt();
        song->m_comments        = query.value(numValue++).toString();
        song->m_bpm             = query.value(numValue++).toInt();
        song->m_lyrics          = query.value(numValue++).toString();
        song->m_language        = getLanguageForISO2Code(query.value(numValue++).toString());

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
    if (m_isEnabled != enabled)
    {
        m_isEnabled = enabled;
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
    if (m_title != title)
    {
        m_title = title;
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
    if (m_subTitle != subTitle)
    {
        m_subTitle = subTitle;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setGrouping(const QString& grouping)
{
    if (m_grouping != grouping)
    {
        m_grouping = grouping;
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
    if (m_artistName != artistName)
    {
        m_artistName = artistName;
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
    if (m_albumTitle != albumTitle)
    {
        m_albumTitle = albumTitle;
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
    if (m_albumArtist != albumArtist)
    {
        m_albumArtist = albumArtist;
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
    if (m_composer != composer)
    {
        m_composer = composer;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setTitleSort(const QString& title)
{
    if (m_titleSort != title)
    {
        m_titleSort = title;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setArtistNameSort(const QString& artistName)
{
    if (m_artistNameSort != artistName)
    {
        m_artistNameSort = artistName;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setAlbumTitleSort(const QString& albumTitle)
{
    if (m_albumTitleSort != albumTitle)
    {
        m_albumTitleSort = albumTitle;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setAlbumArtistSort(const QString& albumArtist)
{
    if (m_albumArtistSort != albumArtist)
    {
        m_albumArtistSort = albumArtist;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setComposerSort(const QString& composer)
{
    if (m_composerSort != composer)
    {
        m_composerSort = composer;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setYear(int year)
{
    Q_ASSERT(year >= 0);

    if (m_year != year)
    {
        m_year = year;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setTrackNumber(int trackNumber)
{
    Q_ASSERT(trackNumber >= 0);

    if (m_trackNumber != trackNumber)
    {
        m_trackNumber = trackNumber;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setTrackTotal(int trackNumber)
{
    Q_ASSERT(trackNumber >= 0);

    if (m_trackTotal != trackNumber)
    {
        m_trackTotal = trackNumber;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setDiscNumber(int discNumber)
{
    Q_ASSERT(discNumber >= 0);

    if (m_discNumber != discNumber)
    {
        m_discNumber = discNumber;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setDiscTotal(int discNumber)
{
    Q_ASSERT(discNumber >= 0);

    if (m_discTotal != discNumber)
    {
        m_discTotal = discNumber;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setGenre(const QString& genre)
{
    if (m_genre != genre)
    {
        m_genre = genre;
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

    if (m_rating != rating)
    {
        m_rating = rating;
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
    if (m_comments != comments)
    {
        m_comments = comments;
        m_isModified = true;
        if (!m_multiModification) emit songModified();
    }
}


void CSong::setBPM(int bpm)
{
    if (m_bpm != bpm)
    {
        m_bpm = bpm;
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
    if (m_lyrics != lyrics)
    {
        m_lyrics = lyrics;
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
    if (m_language != language)
    {
        m_language = language;
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

        int artistId = m_application->getArtistId(m_artistName, m_artistNameSort);
        if (artistId < 0) artistId = 0;
        int albumId = m_application->getAlbumId(m_albumTitle, m_albumTitleSort);
        if (albumId < 0) albumId = 0;
        int albumArtistId = m_application->getArtistId(m_albumArtist, m_albumArtistSort);
        if (albumArtistId < 0) albumArtistId = 0;
        int genreId = m_application->getGenreId(m_genre);
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
                              "song_track_total, "
                              "song_disc_number, "
                              "song_disc_total, "
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
            query.bindValue(numValue++, (m_isEnabled ? 1 : 0));
            query.bindValue(numValue++, m_title);
            query.bindValue(numValue++, m_titleSort);
            query.bindValue(numValue++, m_subTitle);
            query.bindValue(numValue++, m_grouping);
            query.bindValue(numValue++, artistId);
            query.bindValue(numValue++, albumId);
            query.bindValue(numValue++, albumArtistId);
            query.bindValue(numValue++, m_composer);
            query.bindValue(numValue++, m_composerSort);
            query.bindValue(numValue++, m_year);
            query.bindValue(numValue++, m_trackNumber);
            query.bindValue(numValue++, m_trackTotal);
            query.bindValue(numValue++, m_discNumber);
            query.bindValue(numValue++, m_discTotal);
            query.bindValue(numValue++, genreId);
            query.bindValue(numValue++, m_rating);
            query.bindValue(numValue++, m_comments);
            query.bindValue(numValue++, m_bpm);
            query.bindValue(numValue++, m_lyrics);
            query.bindValue(numValue++, getISO2CodeForLanguage(m_language));
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
                              "song_track_total   = ?,"
                              "song_disc_number   = ?,"
                              "song_disc_total    = ?,"
                              "genre_id           = ?,"
                              "song_rating        = ?,"
                              "song_comments      = ?,"
                              "song_bpm           = ?,"
                              "song_lyrics        = ?,"
                              "song_language      = ? "
                          "WHERE song_id = ?");

            int numValue = 0;

            query.bindValue(numValue++, m_modification);
            query.bindValue(numValue++, (m_isEnabled ? 1 : 0));
            query.bindValue(numValue++, m_title);
            query.bindValue(numValue++, m_titleSort);
            query.bindValue(numValue++, m_subTitle);
            query.bindValue(numValue++, m_grouping);
            query.bindValue(numValue++, artistId);
            query.bindValue(numValue++, albumId);
            query.bindValue(numValue++, albumArtistId);
            query.bindValue(numValue++, m_composer);
            query.bindValue(numValue++, m_composerSort);
            query.bindValue(numValue++, m_year);
            query.bindValue(numValue++, m_trackNumber);
            query.bindValue(numValue++, m_trackTotal);
            query.bindValue(numValue++, m_discNumber);
            query.bindValue(numValue++, m_discTotal);
            query.bindValue(numValue++, genreId);
            query.bindValue(numValue++, m_rating);
            query.bindValue(numValue++, m_comments);
            query.bindValue(numValue++, m_bpm);
            query.bindValue(numValue++, m_lyrics);
            query.bindValue(numValue++, getISO2CodeForLanguage(m_language));

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

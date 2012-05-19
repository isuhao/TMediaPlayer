
#include "CSong.hpp"
#include "CApplication.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

#include <QtDebug>


CSong::CSong(CApplication * application) :
    QObject           (),
    m_application     (application),
    m_sound           (NULL),
    m_channel         (NULL),
    m_id              (-1),
    m_fileName        (""),
    m_fileSize        (0),
    m_bitRate         (0),
    m_fileType        (TypeUnknown),
    m_numChannels     (0),
    m_duration        (0),
    m_title           (""),
    m_artistName      (""),  
    m_albumTitle      (""),
    m_albumArtist     (""),
    m_composer        (""),
    m_titleSort       (""),
    m_artistNameSort  (""),
    m_albumTitleSort  (""),
    m_albumArtistSort (""),
    m_composerSort    (""),
    m_year            (0),
    m_trackNumber     (0),
    m_trackTotal      (0),
    m_discNumber      (0),
    m_discTotal       (0),
    m_genre           (""),
    m_rating          (0),
    m_comments        (""),
    m_lyrics          (""),
    m_isModified      (false)
{
    Q_CHECK_PTR(application);
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
    }
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
        emit songModified();
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
        emit songModified();
    }
}


void CSong::setAlbumTitle(const QString& albumTitle)
{
    if (m_albumTitle != albumTitle)
    {
        m_albumTitle = albumTitle;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setAlbumArtist(const QString& albumArtist)
{
    if (m_albumArtist != albumArtist)
    {
        m_albumArtist = albumArtist;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setComposer(const QString& composer)
{
    if (m_composer != composer)
    {
        m_composer = composer;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setTitleSort(const QString& title)
{
    if (m_titleSort != title)
    {
        m_titleSort = title;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setArtistNameSort(const QString& artistName)
{
    if (m_artistNameSort != artistName)
    {
        m_artistNameSort = artistName;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setAlbumTitleSort(const QString& albumTitle)
{
    if (m_albumTitleSort != albumTitle)
    {
        m_albumTitleSort = albumTitle;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setAlbumArtistSort(const QString& albumArtist)
{
    if (m_albumArtistSort != albumArtist)
    {
        m_albumArtistSort = albumArtist;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setComposerSort(const QString& composer)
{
    if (m_composer != composer)
    {
        m_composer = composer;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setYear(int year)
{
    Q_ASSERT(year >= 0);

    if (m_year != year)
    {
        m_year = year;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setTrackNumber(int trackNumber)
{
    Q_ASSERT(trackNumber >= 0);

    if (m_trackNumber != trackNumber)
    {
        m_trackNumber = trackNumber;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setTrackTotal(int trackNumber)
{
    Q_ASSERT(trackNumber >= 0);

    if (m_trackTotal != trackNumber)
    {
        m_trackTotal = trackNumber;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setDiscNumber(int discNumber)
{
    Q_ASSERT(discNumber >= 0);

    if (m_discNumber != discNumber)
    {
        m_discNumber = discNumber;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setDiscTotal(int discNumber)
{
    Q_ASSERT(discNumber >= 0);

    if (m_discTotal != discNumber)
    {
        m_discTotal = discNumber;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setGenre(const QString& genre)
{
    if (m_genre != genre)
    {
        m_genre = genre;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setRating(int rating)
{
    Q_ASSERT(rating >=0 && rating <= 5);

    if (m_rating != rating)
    {
        m_rating = rating;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setComments(const QString& comments)
{
    if (m_comments != comments)
    {
        m_comments = comments;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setLyrics(const QString& lyrics)
{
    if (m_lyrics != lyrics)
    {
        m_lyrics = lyrics;
        m_isModified = true;
        emit songModified();
    }
}


void CSong::setLanguage(CSong::TLanguage language)
{
    if (m_language != language)
    {
        m_language = language;
        m_isModified = true;
        emit songModified();
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


void CSong::setPosition(int position)
{
    Q_ASSERT(position >= 0);

    if (m_sound && m_channel)
    {
        m_channel->setPosition(position, FMOD_TIMEUNIT_MS);
    }
}


void CSong::setVolume(int volume)
{
    Q_ASSERT(volume >= 0 || volume <= 100);

    if (m_sound && m_channel)
    {
        m_channel->setVolume(static_cast<float>(volume) / 100);
    }
}


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
                m_duration = length;

                QSqlQuery query(m_application->getDataBase());

                query.prepare("UPDATE song SET song_duration = ? WHERE song_id = ?");

                query.bindValue(0, m_duration);
                query.bindValue(1, m_id);

                if (!query.exec())
                {
                    QString error = query.lastError().text();
                    QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
                }
            }

            return true;
        }
    }

    qWarning() << "Echec du chargement du son " << m_fileName;
    
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
 *
 * \todo Modifier les dates de création et de modification.
 * \todo Modifier les critères de tri.
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
                              "song_format, "
                              "song_channels, "
                              "song_duration, "
                              "song_creation, "
                              "song_modification, "
                              "song_title, "
                              "song_title_sort, "
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
                              "song_lyrics, "
                              "song_language, "
                              "song_play_count, "
                              "song_play_time"
                          ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

            query.bindValue( 0, m_fileName);
            query.bindValue( 1, m_fileSize);
            query.bindValue( 2, m_bitRate);
            query.bindValue( 3, m_fileType);
            query.bindValue( 4, m_numChannels);
            query.bindValue( 5, m_duration);
            query.bindValue( 6, m_creation);
            query.bindValue( 7, m_modification);
            query.bindValue( 8, m_title);
            query.bindValue( 9, m_titleSort);
            query.bindValue(10, artistId);
            query.bindValue(11, albumId);
            query.bindValue(12, albumArtistId);
            query.bindValue(13, m_composer);
            query.bindValue(14, m_composerSort);
            query.bindValue(15, m_year);
            query.bindValue(16, m_trackNumber);
            query.bindValue(17, m_trackTotal);
            query.bindValue(18, m_discNumber);
            query.bindValue(19, m_discTotal);
            query.bindValue(20, genreId);
            query.bindValue(21, m_rating);
            query.bindValue(22, m_comments);
            query.bindValue(23, m_lyrics);

            switch (m_language)
            {
                default:
                case LangUnknown: query.bindValue(24, "00"); break;
                case LangEnglish: query.bindValue(24, "EN"); break;
                case LangFrench:  query.bindValue(24, "FR"); break;
            }

            query.bindValue(25, 0);
            query.bindValue(26, "");

            if (!query.exec())
            {
                QString error = query.lastError().text();
                QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
                return;
            }

            m_id = query.lastInsertId().toInt();
        }
        // Mise à jour
        else
        {
            query.prepare("UPDATE song SET "
                              "song_title         = ?,"
                              "song_title_sort    = ?,"
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
                              "song_lyrics        = ?,"
                              "song_language      = ? "
                          "WHERE song_id = ?");

            query.bindValue( 0, m_title);
            query.bindValue( 1, m_titleSort);
            query.bindValue( 2, artistId);
            query.bindValue( 3, albumId);
            query.bindValue( 4, albumArtistId);
            query.bindValue( 5, m_composer);
            query.bindValue( 6, m_composerSort);
            query.bindValue( 7, m_year);
            query.bindValue( 8, m_trackNumber);
            query.bindValue( 9, m_trackTotal);
            query.bindValue(10, m_discNumber);
            query.bindValue(11, m_discTotal);
            query.bindValue(12, m_genre);
            query.bindValue(13, m_rating);
            query.bindValue(14, m_comments);
            query.bindValue(15, m_lyrics);
            query.bindValue(16, m_language);

            query.bindValue(17, m_id);

            if (!query.exec())
            {
                QString error = query.lastError().text();
                QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
                return;
            }
        }

        m_isModified = false;
    }
}


/**
 * Méthode appelée quand la lecture du morceau est terminée.
 *
 * \todo Incrémenter le compteur de lecture.
 * \todo Ajouter une entrée dans la liste des écoutes.
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
        QString error = query.lastError().text();
        QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
        return;
    }

    query.prepare("INSERT INTO play (song_id, play_time) VALUES (?, ?)");
    query.bindValue(0, m_id);
    query.bindValue(1, currentTime);

    if (!query.exec())
    {
        QString error = query.lastError().text();
        QMessageBox::warning(m_application, QString(), tr("Database error:\n%1").arg(error));
        return;
    }

    emit playEnd();
}

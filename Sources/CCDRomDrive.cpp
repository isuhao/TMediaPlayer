/*
Copyright (C) 2012-2014 Teddy Michel

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

#include "CCDRomDrive.hpp"
#include "CMainWindow.hpp"
#include "CMediaManager.hpp"
#include "CSong.hpp"
#include "Utils.hpp"
#include "MusicBrainz/sha1.h"
#include "MusicBrainz/base64.h"
#include "MusicBrainz/CMusicBrainzLookup.hpp"


/**
 * Constructeur du lecteur de CD-ROM.
 *
 * \param driveName  Nom du lecteur.
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 * \param SCSIName   Nom SCSI du lecteur.
 * \param deviceName Nom du périphérique.
 */

CCDRomDrive::CCDRomDrive(const QString& driveName, CMainWindow * mainWindow, const QString& SCSIName, const QString& deviceName) :
CMediaTableView (mainWindow),
m_driveName     (driveName),
m_SCSIName      (SCSIName),
m_deviceName    (deviceName),
m_discId        (0),
m_sound         (nullptr),
m_disc          (qPrintable(driveName))
{
    for (int track = 0; track < 100; ++track)
    {
        m_songs[track] = nullptr;
    }

    // Glisser-déposer
    setDragEnabled(false);
}


/**
 * Arrête la lecture du CD-ROM et libère les ressources.
 */

CCDRomDrive::~CCDRomDrive()
{
    // Destruction des morceaux
    for (int track = 0; track < 100; ++track)
    {
        delete m_songs[track];
        m_songs[track] = nullptr;
    }

    if (m_sound)
    {
        m_sound->release();
        m_sound = nullptr;
    }
}


/**
 * Indique s'il y a un disque dans le lecteur.
 *
 * \return Booléen.
 */

bool CCDRomDrive::hasCDInDrive()
{
    if (m_sound)
    {
        if (m_disc.hasDiscInDrive())
        {
            return true;
        }
        else
        {
            removeAllSongsFromTable();

            // Destruction des morceaux
            for (int track = 0; track < 100; ++track)
            {
                delete m_songs[track];
                m_songs[track] = nullptr;
            }

            m_sound->release();
            m_sound = nullptr;

            m_discId = 0;
            m_musicBrainzId = QString();

            return false;
        }
    }
    else
    {
        FMOD_RESULT res = m_mainWindow->getMediaManager()->getSoundSystem()->createStream(qPrintable(m_driveName), FMOD_OPENONLY, 0, &m_sound);

        // Pas de disque dans le lecteur
        if (res == FMOD_ERR_CDDA_NODISC)
        {
            return false;
        }

        if (res != FMOD_OK || !m_sound)
        {
            m_mainWindow->getMediaManager()->logError(tr("the CD-ROM drive \"%1\" can't be opened with FMOD").arg(m_driveName), __FUNCTION__, __FILE__, __LINE__);
            m_sound = nullptr;
            return false;
        }

        int numTracks;
        res = m_sound->getNumSubSounds(&numTracks);

        if (res != FMOD_OK || !m_sound)
        {
            m_mainWindow->getMediaManager()->logError(tr("can't get number of track for CD-ROM drive \"%1\"").arg(m_driveName), __FUNCTION__, __FILE__, __LINE__);
            return false;
        }

        // Récupération des informations du CD-ROM
        FMOD_TAG tag;
        m_discId = 0;
        m_musicBrainzId = QString();
        int songDurations[100] = { 0 };

        while (m_sound->getTag(nullptr, -1, &tag) == FMOD_OK)
        {
            if (tag.datatype == FMOD_TAGDATATYPE_CDTOC)
            {
                FMOD_CDTOC * toc = reinterpret_cast<FMOD_CDTOC *>(tag.data);

                // Calcul des durées des pistes
                for (int track = 0; track < toc->numtracks; ++track)
                {
                    songDurations[track] = (toc->min[track] * 60) + toc->sec[track];

                    if (track > 0)
                        songDurations[track] -= (toc->min[track-1] * 60 + toc->sec[track-1]);

                    songDurations[track] *= 1000;
                }

                // Calcul du DiscId
                int	n = 0;

                for (int track = 0; track < toc->numtracks; ++track)
                {
                    int duration = (toc->min[track] * 60) + toc->sec[track];
                    int ret = 0;

                    while (duration > 0)
                    {
                        ret += (duration % 10);
                        duration /= 10;
                    }

                    n += ret;
                }

                int t = ((toc->min[toc->numtracks] * 60) + toc->sec[toc->numtracks]) - ((toc->min[0] * 60) + toc->sec[0]);
                m_discId = ((n % 0xff) << 24 | t << 8 | toc->numtracks);

                // Calcul du DiscId de MusicBrainz
                if (m_disc.readInfos())
                {
                    // Calcul des durées des pistes
                    for (int track = 1; track < 100; ++track)
                    {
                        if (track == m_disc.infos.lastTrack)
                            songDurations[track] = (1000 * (m_disc.infos.trackOffsets[0] - m_disc.infos.trackOffsets[track]) / 75);
                        else if (track == 99)
                            songDurations[track] = 0;
                        else
                            songDurations[track] = (1000 * (m_disc.infos.trackOffsets[track+1] - m_disc.infos.trackOffsets[track]) / 75);
                    }

                    SHA_INFO sha;
                    sha_init(&sha);

                    char tmp[17] = "";
#ifdef Q_OS_WIN32
                    sprintf_s(tmp, "%02X", m_disc.infos.firstTrack);
#else
                    sprintf(tmp, "%02X", m_disc.infos.firstTrack);
#endif
                    sha_update(&sha, reinterpret_cast<unsigned char *>(tmp), strlen(tmp));

#ifdef Q_OS_WIN32
                    sprintf_s(tmp, "%02X", m_disc.infos.lastTrack);
#else
                    sprintf(tmp, "%02X", m_disc.infos.lastTrack);
#endif
                    sha_update(&sha, reinterpret_cast<unsigned char *>(tmp), strlen(tmp));

                    for (int i = 0; i < 100; ++i)
                    {
#ifdef Q_OS_WIN32
                        sprintf_s(tmp, "%08X", m_disc.infos.trackOffsets[i]);
#else
                        sprintf(tmp, "%08X", m_disc.infos.trackOffsets[i]);
#endif
                        sha_update(&sha, reinterpret_cast<unsigned char *>(tmp), strlen(tmp));
                    }

                    unsigned char digest[20] = "";
                    sha_final(digest, &sha);

                    unsigned long size;

                    unsigned char * base64 = rfc822_binary(digest, sizeof(digest), &size);
                    m_musicBrainzId = reinterpret_cast<char *>(base64);
                    free(base64);
                }
            }
        }

        FMOD::Channel * channel = nullptr;
        res = m_mainWindow->getMediaManager()->getSoundSystem()->playSound(FMOD_CHANNEL_FREE, m_sound, true, &channel);

        // Création des morceaux
        if (res == FMOD_OK && channel)
        {
            QList<CSong *> songs;

            for (int track = 0; track < 100; ++track)
            {
                m_songs[track] = nullptr;
            }

            for (int track = 0; track < numTracks && track < 100; ++track)
            {
                //res = channel->setPosition(track, FMOD_TIMEUNIT_SENTENCE_SUBSOUND);
                res = channel->setPosition(track, FMOD_TIMEUNIT_SENTENCE);

                if (res != FMOD_OK)
                {
                    m_mainWindow->getMediaManager()->logError(tr("can't play track #%1 in CD-ROM drive \"%2\"").arg(track + 1).arg(m_driveName), __FUNCTION__, __FILE__, __LINE__);
                    continue;
                }

                CSong * song = new CSong(m_mainWindow->getMediaManager());

                // Recherche de la durée du morceau
                res = m_sound->getLength(reinterpret_cast<unsigned int *>(&(song->m_properties.duration)), FMOD_TIMEUNIT_SENTENCE_MS);

                if (res != FMOD_OK)
                {
                    m_mainWindow->getMediaManager()->logError(tr("can't compute song duration for track #%1 in CD-ROM drive \"%2\"").arg(track + 1).arg(m_driveName), __FUNCTION__, __FILE__, __LINE__);

                    // Utilisation de la table de contenu
                    song->m_properties.duration = songDurations[track+1];
                }

                // La norme définit la durée minimale d'un morceau à 2s
                if (song->m_properties.duration < 2000)
                {
                    delete song;
                    continue;
                }

                // Attributs du morceau
                song->m_sound            = m_sound;
                song->m_channel          = channel;
                song->m_cdRomDrive       = this;
                song->m_cdRomTrackNumber = track;

                song->m_infos.title       = tr("Track %1").arg(track + 1);
                song->m_infos.trackNumber = track + 1;
                song->m_infos.trackCount  = numTracks;

                if (track + 1 < 10)
                    song->m_properties.fileName = QString("Track0%1.cda").arg(track + 1);
                else
                    song->m_properties.fileName = QString("Track%1.cda").arg(track + 1);

                // Recherche du format du morceau
                FMOD_SOUND_TYPE type;
                FMOD_SOUND_FORMAT format;
                res = m_sound->getFormat(&type, &format, &(song->m_properties.numChannels), nullptr);

                if (song->m_properties.numChannels <= 0)
                    song->m_properties.numChannels = 1;

                song->m_properties.sampleRate = 44100;
                song->m_properties.bitRate    = song->m_properties.sampleRate * 16 * song->m_properties.numChannels / 1000;
                song->m_properties.fileSize   = (static_cast<qlonglong>(song->m_properties.duration) * static_cast<qlonglong>(song->m_properties.bitRate)) / 8; // 8 bits par octet

                if (res != FMOD_OK)
                {
                    m_mainWindow->getMediaManager()->logError(tr("can't find the format for track #%1 in CD-ROM drive \"%2\"").arg(track + 1).arg(m_driveName), __FUNCTION__, __FILE__, __LINE__);
                }
                else
                {
                    switch (type)
                    {
                        default:
                            m_mainWindow->getMediaManager()->logError(tr("unknown format"), __FUNCTION__, __FILE__, __LINE__);
                            break;

                        case FMOD_SOUND_TYPE_CDDA:
                            song->m_properties.format = CSong::FormatCDAudio;
                            break;

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

                m_songs[track + 1] = song;
                songs.append(song);
            }

            addSongsToTable(songs);
        }

        CMusicBrainzLookup * lookup = new CMusicBrainzLookup(this, m_mainWindow);
    }

    return (m_sound != nullptr);
}


/**
 * Indique si la liste de morceaux a été modifiée.
 *
 * \return Booléen (toujours false).
 */

bool CCDRomDrive::isModified() const
{
    // Les lecteurs ne sont pas enregistrés en base de données
    return false;
}


/**
 * Éjecte le disque du lecteur.
 */

void CCDRomDrive::ejectDisc()
{
    m_disc.ejectDisc();

    removeAllSongsFromTable();

    // Destruction des morceaux
    for (int track = 0; track < 100; ++track)
    {
        delete m_songs[track];
        m_songs[track] = nullptr;
    }

    if (m_sound)
    {
        m_sound->release();
        m_sound = nullptr;
    }

    m_discId = 0;
    m_musicBrainzId = QString();
}


/**
 * Met à jour la base de données avec les informations de la table.
 * Les lecteurs ne sont pas enregistrés en base de données, donc cette méthode ne fait rien.
 *
 * \return Booléen indiquant le succès de l'opération (toujours false).
 */

bool CCDRomDrive::updateDatabase()
{
    // Les lecteurs ne sont pas enregistrés en base de données
    return false;
}

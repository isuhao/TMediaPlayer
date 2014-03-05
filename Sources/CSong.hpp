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

#ifndef FILE_C_SONG
#define FILE_C_SONG

#include "Language.hpp"
#include <QList>
#include <QDateTime>
#include <QString>
#include <QVariant>
#include <QStringList>
#include <fmod/fmod.hpp>


namespace TagLib
{
    namespace ID3v1 { class Tag; }
    namespace ID3v2 { class Tag; }
    namespace APE   { class Tag; }
    namespace Ogg   { class XiphComment; }
}

class CMediaManager;
class CCDRomDrive;
class QFile;


/**
 * Représente un morceau pouvant être joué.
 */

class CSong : public QObject
{
    Q_OBJECT

    friend class CMainWindow;
    friend class CCDRomDrive;
    friend class CDialogEditSong;
    friend class CDialogEditSongs;

public:

    enum TFormat
    {
        FormatUnknown = 0,  ///< Format inconnu.

        FormatMP3     = 1,  ///< Format MPEG Layer 3.
        FormatOGG     = 2,  ///< Format Ogg Vorbis.
        FormatFLAC    = 3,  ///< Format FLAC.

        FormatCDAudio = 100 ///< CD-ROM audio.
    };

    static inline TFormat getFormatFromInteger(int format);
    static inline QString getFormatName(TFormat format);
    static inline QStringList getFormatList();


    struct TSongPlay
    {
        QDateTime time;
        QDateTime timeUTC;
    };


    explicit CSong(CMediaManager * mediaManager);
    CSong(const QString& fileName, CMediaManager * mediaManager);
    virtual ~CSong();

    void loadFromDatabase();
    bool loadTags(bool readProperties = false);
    bool writeTags();
    bool moveFile();
    QImage getCoverImage() const;
    bool matchFilter(const QString& filter, bool split = true) const;
    //void computeReplayGain();
    void updateDatabase();
    inline bool isInCDRomDrive() const;

    inline int getId() const;
    inline QString getFileName() const;
    inline qlonglong getFileSize() const;
    inline int getBitRate() const;
    inline int getSampleRate() const;
    inline TFormat getFormat() const;
    inline int getNumChannels() const;
    inline int getDuration() const;
    inline bool getFileStatus() const;
    inline QDateTime getCreationDate() const;
    inline QDateTime getModificationDate() const;

    inline bool isEnabled() const;
    inline QString getTitle() const;
    inline QString getSubTitle() const;
    inline QString getGrouping() const;
    inline QString getArtistName() const;
    inline QString getAlbumTitle() const;
    inline QString getAlbumArtist() const;
    inline QString getComposer() const;
    inline QString getTitleSort(bool empty = true) const;
    inline QString getArtistNameSort(bool empty = true) const;
    inline QString getAlbumTitleSort(bool empty = true) const;
    inline QString getAlbumArtistSort(bool empty = true) const;
    inline QString getComposerSort(bool empty = true) const;
    inline int getYear() const;
    inline int getTrackNumber() const;
    inline int getTrackCount() const;
    inline int getDiscNumber() const;
    inline int getDiscCount() const;
    inline QString getGenre() const;
    inline int getRating() const;
    inline QString getComments() const;
    inline int getBPM() const;
    inline QString getLyrics() const;
    inline TLanguage getLanguage() const;
    inline QString getLyricist() const;
    inline bool isCompilation() const;
    inline bool isSkipShuffle() const;

    // Replay Gain
    inline float getTrackGain() const;
    inline float getTrackPeak() const;
    inline float getAlbumGain() const;
    inline float getAlbumPeak() const;

    int getPosition() const;
    bool isEnded() const;

    inline int getNumPlays() const;
    inline QDateTime getLastPlay() const;
    inline QList<TSongPlay> getPlays() const;

    static int getId(CMediaManager * mediaManager, const QString& fileName);
    static CSong * loadFromFile(CMediaManager * mediaManager, const QString& fileName);
    static QList<CSong *> loadAllSongsFromDatabase(CMediaManager * mediaManager);

public slots:

    void setEnabled(bool enabled = true);
    void setTitle(const QString& title);
    void setSubTitle(const QString& subTitle);
    void setGrouping(const QString& grouping);
    void setArtistName(const QString& artistName);
    void setAlbumTitle(const QString& albumTitle);
    void setAlbumArtist(const QString& albumArtist);
    void setComposer(const QString& composer);
    void setTitleSort(const QString& title);
    void setArtistNameSort(const QString& artistName);
    void setAlbumTitleSort(const QString& albumTitle);
    void setAlbumArtistSort(const QString& albumArtist);
    void setComposerSort(const QString& composer);
    void setYear(int year);
    void setTrackNumber(int trackNumber);
    void setTrackCount(int trackCount);
    void setDiscNumber(int discNumber);
    void setDiscCount(int discCount);
    void setGenre(const QString& genre);
    void setRating(int rating);
    void setComments(const QString& comments);
    void setBPM(int bpm);
    void setLyrics(const QString& lyrics);
    void setLanguage(TLanguage language);
    void setLyricist(const QString& lyricist);
    void setCompilation(bool compilation);
    void setSkipShuffle(bool skipShuffle);

    // Replay Gain
    void setTrackGain(float gain);
    void setTrackPeak(float peak);
    void setAlbumGain(float gain);
    void setAlbumPeak(float peak);

protected:

    void startPlay();
    void updateFileInfos();

    void setFileSize(qlonglong fileSize);
    void setBitRate(int bitRate);
    void setSampleRate(int sampleRate);
    void setNumChannels(int numChannels);

protected slots:

    void setPosition(int position);
    void setVolume(int volume);
    void setMute(bool mute);
    bool loadSound();
    void play();
    void pause();
    void stop();
    void emitPlayEnd();

signals:

    void songModified(); ///< Signal émis lorsque les informations du morceau sont modifiées.
    void playEnd();      ///< Signal émis lorsque la lecture se termine.

private:

    /**
     * Contient toutes les propriétés d'un morceau.
     */

    struct TSongProperties
    {
        QString fileName;   ///< Fichier audio.
        qlonglong fileSize; ///< Taille du fichier en octets.
        int bitRate;        ///< Débit binaire.
        int sampleRate;     ///< Fréquence d'échantillonnage.
        TFormat format;     ///< Format de fichier.
        int numChannels;    ///< Nombre de canaux.
        int duration;       ///< Durée du morceau en millisecondes.

        TSongProperties();

        inline bool operator==(const TSongProperties& other)
        {
            return (other.fileName    == fileName    &&
                    other.fileSize    == fileSize    &&
                    other.bitRate     == bitRate     &&
                    other.sampleRate  == sampleRate  &&
                    other.format      == format      &&
                    other.numChannels == numChannels &&
                    other.duration    == duration);
        }

        inline bool operator!=(const TSongProperties& other)
        {
            return !(operator==(other));
        }
    };


    /**
     * Contient toutes les informations modifiables d'un morceau.
     */

    struct TSongInfos
    {
        bool isEnabled;          ///< Indique si le morceau est activé.
        QString title;           ///< Titre du morceau.
        QString subTitle;        ///< Sous-titre du morceau.
        QString grouping;        ///< Regroupement.
        QString artistName;      ///< Artiste du morceau.
        QString albumTitle;      ///< Titre de l'album.
        QString albumArtist;     ///< Artiste de l'album.
        QString composer;        ///< Compositeur.
        QString titleSort;       ///< Titre du morceau pour le tri.
        QString artistNameSort;  ///< Artiste du morceau pour le tri.
        QString albumTitleSort;  ///< Titre de l'album pour le tri.
        QString albumArtistSort; ///< Artiste de l'album pour le tri.
        QString composerSort;    ///< Compositeur pour le tri.
        int year;                ///< Année de sortie de l'album ou du morceau.
        int trackNumber;         ///< Numéro de piste.
        int trackCount;          ///< Nombre de pistes sur l'album.
        int discNumber;          ///< Numéro de disque.
        int discCount;           ///< Nombre de disques de l'album.
        QString genre;           ///< Genre.
        int rating;              ///< Note (entre 0 et 5).
        QString comments;        ///< Commentaires.
        int bpm;                 ///< Battements par minute.
        QString lyrics;          ///< Paroles.
        TLanguage language;      ///< Langue des paroles.
        QString lyricist;        ///< Auteur des paroles.
        bool compilation;        ///< Indique si le morceau fait partie d'une compilation.
        bool skipShuffle;        ///< Indique si on ne doit pas lire le morceau en mode aléatoire.

        // Replay Gain
        float trackGain;
        float trackPeak;
        float albumGain;
        float albumPeak;

        TSongInfos();

        inline bool operator==(const TSongInfos& other)
        {
            return (other.isEnabled       == isEnabled       &&
                    other.title           == title           &&
                    other.subTitle        == subTitle        &&
                    other.grouping        == grouping        &&
                    other.artistName      == artistName      &&
                    other.albumTitle      == albumTitle      &&
                    other.albumArtist     == albumArtist     &&
                    other.composer        == composer        &&
                    other.titleSort       == titleSort       &&
                    other.artistNameSort  == artistNameSort  &&
                    other.albumTitleSort  == albumTitleSort  &&
                    other.albumArtistSort == albumArtistSort &&
                    other.composerSort    == composerSort    &&
                    other.year            == year            &&
                    other.trackNumber     == trackNumber     &&
                    other.trackCount      == trackCount      &&
                    other.discNumber      == discNumber      &&
                    other.discCount       == discCount       &&
                    other.genre           == genre           &&
                    other.rating          == rating          &&
                    other.comments        == comments        &&
                    other.bpm             == bpm             &&
                    other.lyrics          == lyrics          &&
                    other.language        == language        &&
                    other.lyricist        == lyricist        &&
                    other.compilation     == compilation     &&
                    other.skipShuffle     == skipShuffle     &&

                    ((trackGain == std::numeric_limits<float>::infinity() && other.trackGain == trackGain) || qAbs(other.trackGain - trackGain) < 0.001f) &&
                    ((trackPeak == std::numeric_limits<float>::infinity() && other.trackPeak == trackPeak) || qAbs(other.trackPeak - trackPeak) < 0.001f) &&
                    ((albumGain == std::numeric_limits<float>::infinity() && other.albumGain == albumGain) || qAbs(other.albumGain - albumGain) < 0.001f) &&
                    ((albumPeak == std::numeric_limits<float>::infinity() && other.albumPeak == albumPeak) || qAbs(other.albumPeak - albumPeak) < 0.001f));
        }

        inline bool operator!=(const TSongInfos& other)
        {
            return !(operator==(other));
        }
    };


    // Copie interdite
    CSong(const CSong&);
    CSong& operator=(const CSong&);

    // Lecture et écriture des métadonnées
    bool loadTags(TagLib::ID3v1::Tag * tags);
    bool loadTags(TagLib::ID3v2::Tag * tags);
    bool loadTags(TagLib::APE::Tag * tags);
    bool loadTags(TagLib::Ogg::XiphComment * tags);

    static bool writeTags(TagLib::ID3v1::Tag * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName);
    static bool writeTags(TagLib::ID3v2::Tag * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName);
    static bool writeTags(TagLib::APE::Tag * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName);
    static bool writeTags(TagLib::Ogg::XiphComment * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName);


    CMediaManager * m_mediaManager; ///< Pointeur sur la classe principale de l'application.
    FMOD::Sound * m_sound;          ///< Pointeur sur la structure de FMOD.
    FMOD::Channel * m_channel;      ///< Canal audio.
    CCDRomDrive * m_cdRomDrive;     ///< Pointeur sur le lecteur de CD-ROM d'où provient le morceau.
    int m_cdRomTrackNumber;         ///< Numéro de la piste sur le CD-ROM.
    bool m_isModified;              ///< Indique si les données ont été modifiées.
    mutable bool m_needWriteTags;   ///< Indique si les métadonnées doivent être mises à jour.
    bool m_fileStatus;              ///< Indique si le fichier est accessible.

    int m_id;                       ///< Identifiant du morceau en base de données.
    QDateTime m_creation;           ///< Date de création (ajout à la médiathèque).
    QDateTime m_modification;       ///< Date de la dernière modication.

    TSongProperties m_properties;   ///< Propriétés du morceau.
    TSongInfos m_infos;             ///< Informations modifiables.

    TSongProperties m_propertiesDB; ///< Propriétés du morceau en base de données.
    TSongInfos m_infosDB;           ///< Informations modifiables en base de données.

    QList<TSongPlay> m_plays;       ///< Liste des dates de lecture.
};

Q_DECLARE_METATYPE(CSong *)


inline CSong::TFormat CSong::getFormatFromInteger(int format)
{
    switch (format)
    {
        default:
        case   0: return FormatUnknown;
        case   1: return FormatMP3    ;
        case   2: return FormatOGG    ;
        case   3: return FormatFLAC   ;
        case 100: return FormatCDAudio;
    }
}


inline QString CSong::getFormatName(CSong::TFormat format)
{
    switch (format)
    {
        default:
        case FormatUnknown: return tr("Unknown", "Unknown format");
        case FormatMP3    : return tr("MP3"                      );
        case FormatOGG    : return tr("Ogg Vorbis"               );
        case FormatFLAC   : return tr("FLAC"                     );
        case FormatCDAudio: return tr("CD audio"                 );
    }
}


inline QStringList CSong::getFormatList()
{
    QStringList formatList;

  //formatList << getFormatName(FormatUnknown);
    formatList << getFormatName(FormatMP3    );
    formatList << getFormatName(FormatOGG    );
    formatList << getFormatName(FormatFLAC   );
    formatList << getFormatName(FormatCDAudio);

    return formatList;
}


inline bool CSong::isInCDRomDrive() const
{
    return (m_cdRomDrive != nullptr);
}


/**
 * Retourne l'identifiant du morceau en base de données.
 *
 * \return Identifiant du morceau.
 */

inline int CSong::getId() const
{
    return m_id;
}


/**
 * Retourne le nom du fichier contenant le morceau.
 *
 * \return Nom du fichier.
 */

inline QString CSong::getFileName() const
{
    return m_properties.fileName;
}


/**
 * Retourne la taille du fichier contenant le morceau.
 *
 * \return Taille du fichier en octets.
 */

inline qlonglong CSong::getFileSize() const
{
    return m_properties.fileSize;
}


/**
 * Retourne le débit du morceau.
 *
 * \return Débit binaire.
 */

inline int CSong::getBitRate() const
{
    return m_properties.bitRate;
}


/**
 * Retourne la fréquence d'échantillonnage du morceau.
 *
 * \return Fréquence d'échantillonnage.
 */

inline int CSong::getSampleRate() const
{
    return m_properties.sampleRate;
}


/**
 * Retourne le format du morceau.
 *
 * \return Format du morceau.
 */

inline CSong::TFormat CSong::getFormat() const
{
    return m_properties.format;
}


/**
 * Retourne le nombre de canaux du morceau.
 *
 * \return Nombre de canaux.
 */

inline int CSong::getNumChannels() const
{
    return m_properties.numChannels;
}


/**
 * Retourne la durée du morceau.
 *
 * \return Durée du morceau en millisecondes.
 */

inline int CSong::getDuration() const
{
    return m_properties.duration;
}


/**
 * Retourne le status du fichier.
 *
 * \return Booléen valant true si le fichier est accessible, false sinon.
 */

inline bool CSong::getFileStatus() const
{
    return m_fileStatus;
}


/**
 * Retourne la date de création du morceau.
 *
 * \return Date d'ajout du morceau à la médiathèque.
 */

inline QDateTime CSong::getCreationDate() const
{
    return m_creation;
}


/**
 * Retourne la date de modification du morceau.
 *
 * \return Date de modification du morceau.
 */

inline QDateTime CSong::getModificationDate() const
{
    return m_modification;
}


/**
 * Indique si le morceau est coché ou pas.
 *
 * \return Booléen.
 */

inline bool CSong::isEnabled() const
{
    return m_infos.isEnabled;
}


/**
 * Retourne le titre du morceau.
 *
 * \return Titre du morceau.
 */

inline QString CSong::getTitle() const
{
    return m_infos.title;
}


/**
 * Retourne le sous-titre du morceau.
 *
 * \return Sous-titre du morceau.
 */

inline QString CSong::getSubTitle() const
{
    return m_infos.subTitle;
}


/**
 * Retourne le regroupement du morceau.
 *
 * \return Regroupement du morceau.
 */

inline QString CSong::getGrouping() const
{
    return m_infos.grouping;
}


/**
 * Retourne le nom de l'artiste du morceau.
 *
 * \return Nom de l'artiste du morceau.
 */

inline QString CSong::getArtistName() const
{
    return m_infos.artistName;
}


/**
 * Retourne le titre de l'album.
 *
 * \return Titre de l'album.
 */

inline QString CSong::getAlbumTitle() const
{
    return m_infos.albumTitle;
}


/**
 * Retourne le nom de l'artiste de l'album.
 *
 * \return Nom de l'artiste de l'album.
 */

inline QString CSong::getAlbumArtist() const
{
    return m_infos.albumArtist;
}


/**
 * Retourne le compositeur du morceau.
 *
 * \return Compositeur du morceau.
 */

inline QString CSong::getComposer() const
{
    return m_infos.composer;
}


inline QString CSong::getTitleSort(bool empty) const
{
    if (!empty && m_infos.titleSort.isEmpty()) return m_infos.title;
    return m_infos.titleSort;
}


inline QString CSong::getArtistNameSort(bool empty) const
{
    if (!empty && m_infos.artistNameSort.isEmpty()) return m_infos.artistName;
    return m_infos.artistNameSort;
}


inline QString CSong::getAlbumTitleSort(bool empty) const
{
    if (!empty && m_infos.albumTitleSort.isEmpty()) return m_infos.albumTitle;
    return m_infos.albumTitleSort;
}


inline QString CSong::getAlbumArtistSort(bool empty) const
{
    if (!empty && m_infos.albumArtistSort.isEmpty()) return m_infos.albumArtist;
    return m_infos.albumArtistSort;
}


inline QString CSong::getComposerSort(bool empty) const
{
    if (!empty && m_infos.composerSort.isEmpty()) return m_infos.composer;
    return m_infos.composerSort;
}


inline int CSong::getYear() const
{
    return m_infos.year;
}


inline int CSong::getTrackNumber() const
{
    return m_infos.trackNumber;
}


inline int CSong::getTrackCount() const
{
    return m_infos.trackCount;
}


inline int CSong::getDiscNumber() const
{
    return m_infos.discNumber;
}


inline int CSong::getDiscCount() const
{
    return m_infos.discCount;
}


/**
 * Retourne le genre du morceau.
 *
 * \return Genre du morceau.
 */

inline QString CSong::getGenre() const
{
    return m_infos.genre;
}


/**
 * Retourne la note du morceau.
 *
 * \return Note du morceau (entre 0 et 5).
 */

inline int CSong::getRating() const
{
    return m_infos.rating;
}


inline QString CSong::getComments() const
{
    return m_infos.comments;
}


/**
 * Retourne le nombre de battements par minute (BPM) du morceau.
 *
 * \return Nombre de battements par minute.
 */

inline int CSong::getBPM() const
{
    return m_infos.bpm;
}


/**
 * Retourne les paroles du morceau.
 *
 * \return Paroles du morceau.
 */

inline QString CSong::getLyrics() const
{
    return m_infos.lyrics;
}


/**
 * Retourne la langue des paroles du morceau.
 *
 * \return Langue des paroles.
 */

inline TLanguage CSong::getLanguage() const
{
    return m_infos.language;
}


/**
 * Retourne le parolier du morceau.
 *
 * \return Parolier du morceau.
 */

inline QString CSong::getLyricist() const
{
    return m_infos.lyricist;
}


inline bool CSong::isCompilation() const
{
    return m_infos.compilation;
}


inline bool CSong::isSkipShuffle() const
{
    return m_infos.skipShuffle;
}


inline float CSong::getTrackGain() const
{
    return m_infos.trackGain;
}


inline float CSong::getTrackPeak() const
{
    return m_infos.trackPeak;
}


inline float CSong::getAlbumGain() const
{
    return m_infos.albumGain;
}


inline float CSong::getAlbumPeak() const
{
    return m_infos.albumPeak;
}


inline int CSong::getNumPlays() const
{
    return m_plays.size();
}


inline QDateTime CSong::getLastPlay() const
{
    return (m_plays.isEmpty() ? QDateTime() : m_plays.first().timeUTC);
}


inline QList<CSong::TSongPlay> CSong::getPlays() const
{
    return m_plays;
}

#endif // FILE_C_SONG


#ifndef FILE_C_SONG
#define FILE_C_SONG

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

class QFile;


/**
 * Représente un morceau pouvant être joué.
 */

class CSong : public QObject
{
    Q_OBJECT

    friend class CApplication;
    friend class CDialogEditSong;
    friend class CDialogEditSongs;

public:

    enum TFormat
    {
        FormatUnknown = 0, ///< Format inconnu.
        FormatMP3     = 1, ///< Format MPEG Layer 3.
        FormatOGG     = 2, ///< Format Ogg Vorbis.
        FormatFLAC    = 3  ///< Format FLAC.
    };

    static inline TFormat getFormatFromInteger(int format);
    static inline QString getFormatName(TFormat format);
    static inline QStringList getFormatList(void);

    enum TLanguage
    {
        LangUnknown = 0, ///< Langue inconnue.
        LangEnglish = 1, ///< Anglais.
        LangFrench  = 2, ///< Français.
        LangGerman  = 3, ///< Allemand.
        LangItalian = 4  ///< Italien.
    };

    static inline TLanguage getLanguageFromInteger(int language);
    static inline QString getLanguageName(TLanguage language);
    static inline QStringList getLanguageList(void);
    static inline TLanguage getLanguageForISO2Code(const QString& code);
    static inline TLanguage getLanguageForISO3Code(const QString& code);
    static inline QString getISO2CodeForLanguage(TLanguage language);
    static inline QString getISO3CodeForLanguage(TLanguage language);


    explicit CSong(CApplication * application);
    CSong(const QString& fileName, CApplication * application);
    virtual ~CSong();

    void loadFromDatabase(void);
    bool loadTags(bool readProperties = false);
    bool writeTags(void);
    bool moveFile(void);

    inline int getId(void) const;
    inline QString getFileName(void) const;
    inline int getFileSize(void) const;
    inline int getBitRate(void) const;
    inline int getSampleRate(void) const;
    inline TFormat getFormat(void) const;
    inline int getNumChannels(void) const;
    inline int getDuration(void) const;
    inline QDateTime getCreationDate(void) const;
    inline QDateTime getModificationDate(void) const;

    inline bool isEnabled(void) const;
    inline QString getTitle(void) const;
    inline QString getSubTitle(void) const;
    inline QString getGrouping(void) const;
    inline QString getArtistName(void) const;
    inline QString getAlbumTitle(void) const;
    inline QString getAlbumArtist(void) const;
    inline QString getComposer(void) const;
    inline QString getTitleSort(bool empty = true) const;
    inline QString getArtistNameSort(bool empty = true) const;
    inline QString getAlbumTitleSort(bool empty = true) const;
    inline QString getAlbumArtistSort(bool empty = true) const;
    inline QString getComposerSort(bool empty = true) const;
    inline int getYear(void) const;
    inline int getTrackNumber(void) const;
    inline int getTrackCount(void) const;
    inline int getDiscNumber(void) const;
    inline int getDiscCount(void) const;
    inline QString getGenre(void) const;
    inline int getRating(void) const;
    inline QString getComments(void) const;
    inline int getBPM(void) const;
    inline QString getLyrics(void) const;
    inline TLanguage getLanguage(void) const;
    inline QString getLyricist(void) const;
    inline bool isCompilation(void) const;
    inline bool isSkipShuffle(void) const;

    int getPosition(void) const;
    bool isEnded(void) const;

    inline int getNumPlays(void) const;
    inline QDateTime getLastPlay(void) const;

    static int getId(CApplication * application, const QString& fileName);
    static CSong * loadFromFile(CApplication * application, const QString& fileName);
    static QList<CSong *> loadAllSongsFromDatabase(CApplication * application);
    static QString getFileSize(int fileSize);

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

protected:

    void startPlay(void);
    void updateDatabase(void);
    void updateFileInfos(void);

protected slots:

    void setPosition(int position);
    void setVolume(int volume);
    void setMute(bool mute);
    bool loadSound(void);
    void play(void);
    void pause(void);
    void stop(void);
    void emitPlayEnd(void);

signals:

    void songModified(void); ///< Signal émis lorsque les informations du morceau sont modifiées.
    void playEnd(void);      ///< Signal émis lorsque la lecture se termine.

private:

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

        TSongInfos(void);
    };


    // Copie interdite
    CSong(const CSong&);
    CSong& operator=(const CSong&);

    // Lecture et écriture des métadonnées
    static bool loadTags(TagLib::ID3v1::Tag * tags, TSongInfos& infos, QFile * logFile, const QString& fileName);
    static bool loadTags(TagLib::ID3v2::Tag * tags, TSongInfos& infos, QFile * logFile, const QString& fileName);
    static bool loadTags(TagLib::APE::Tag * tags, TSongInfos& infos, QFile * logFile, const QString& fileName);
    static bool loadTags(TagLib::Ogg::XiphComment * tags, TSongInfos& infos, QFile * logFile, const QString& fileName);

    static bool writeTags(TagLib::ID3v1::Tag * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName);
    static bool writeTags(TagLib::ID3v2::Tag * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName);
    static bool writeTags(TagLib::APE::Tag * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName);
    static bool writeTags(TagLib::Ogg::XiphComment * tags, const TSongInfos& infos, QFile * logFile, const QString& fileName);


    CApplication * m_application; ///< Pointeur sur l'application.
    FMOD::Sound * m_sound;        ///< Pointeur sur la structure de FMOD.
    FMOD::Channel * m_channel;    ///< Canal audio.
    bool m_isModified;            ///< Indique si les données ont été modifiées.
    mutable bool m_needWriteTags; ///< Indique si les métadonnées doivent être mises à jour.
    int m_id;                     ///< Identifiant du morceau en base de données.
    QString m_fileName;           ///< Fichier audio.
    int m_fileSize;               ///< Taille du fichier en octets.
    int m_bitRate;                ///< Débit binaire.
    int m_sampleRate;             ///< Fréquence d'échantillonnage.
    TFormat m_format;             ///< Format de fichier.
    int m_numChannels;            ///< Nombre de canaux.
    int m_duration;               ///< Durée du morceau en millisecondes.
    QDateTime m_creation;         ///< Date de création (ajout à la médiathèque).
    QDateTime m_modification;     ///< Date de la dernière modication.
    TSongInfos m_infos;           ///< Informations modifiables
    QList<QDateTime> m_plays;     ///< Liste des dates de lecture.
};

Q_DECLARE_METATYPE(CSong *)


inline CSong::TFormat CSong::getFormatFromInteger(int format)
{
    switch (format)
    {
        default:
        case 0: return FormatUnknown;
        case 1: return FormatMP3    ;
        case 2: return FormatOGG    ;
        case 3: return FormatFLAC   ;
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
    }
}


inline QStringList CSong::getFormatList(void)
{
    QStringList formatList;
    
  //formatList << getFormatName(FormatUnknown);
    formatList << getFormatName(FormatMP3    );
    formatList << getFormatName(FormatOGG    );
    formatList << getFormatName(FormatFLAC   );

    return formatList;
}


inline CSong::TLanguage CSong::getLanguageFromInteger(int language)
{
    switch (language)
    {
        default:
        case 0: return LangUnknown;
        case 1: return LangEnglish;
        case 2: return LangFrench ;
        case 3: return LangGerman ;
        case 4: return LangItalian;
    }
}


/**
 * Retourne le nom correspondant à une langue.
 *
 * \param language Langue.
 * \param Nom de la langue.
 */

inline QString CSong::getLanguageName(CSong::TLanguage language)
{
    switch (language)
    {
        default:
        case LangUnknown: return tr("Unknown", "Unknown language");
        case LangEnglish: return tr("English"                    );
        case LangFrench : return tr("French"                     );
        case LangGerman : return tr("German"                     );
        case LangItalian: return tr("Italian"                    );
    }
}


/**
 * Retourne la liste des langues.
 *
 * \return Liste des langues.
 */

inline QStringList CSong::getLanguageList(void)
{
    QStringList langList;
    
    langList << getLanguageName(LangUnknown);
    langList << getLanguageName(LangEnglish);
    langList << getLanguageName(LangFrench );
    langList << getLanguageName(LangGerman );
    langList << getLanguageName(LangItalian);

    return langList;
}


inline CSong::TLanguage CSong::getLanguageForISO2Code(const QString& code)
{
    if (code.toLower() == "en") return LangEnglish;
    if (code.toLower() == "fr") return LangFrench ;
    if (code.toLower() == "de") return LangGerman ;
    if (code.toLower() == "it") return LangItalian;

    return LangUnknown;
}


inline CSong::TLanguage CSong::getLanguageForISO3Code(const QString& code)
{
    if (code.toLower() == "eng") return LangEnglish;
    if (code.toLower() == "fra") return LangFrench ;
    if (code.toLower() == "deu") return LangGerman ;
    if (code.toLower() == "ita") return LangItalian;

    return LangUnknown;
}


inline QString CSong::getISO2CodeForLanguage(CSong::TLanguage language)
{
    switch (language)
    {
        default:
        case LangUnknown: return "xx";
        case LangEnglish: return "en";
        case LangFrench : return "fr";
        case LangGerman : return "de";
        case LangItalian: return "it";
    }
}


inline QString CSong::getISO3CodeForLanguage(CSong::TLanguage language)
{
    switch (language)
    {
        default:
        case LangUnknown: return "xxx";
        case LangEnglish: return "eng";
        case LangFrench : return "fra";
        case LangGerman : return "deu";
        case LangItalian: return "ita";
    }
}


/**
 * Retourne l'identifiant du morceau en base de données.
 *
 * \return Identifiant du morceau.
 */

inline int CSong::getId(void) const
{
    return m_id;
}


/**
 * Retourne le nom du fichier contenant le morceau.
 *
 * \return Nom du fichier.
 */

inline QString CSong::getFileName(void) const
{
    return m_fileName;
}


/**
 * Retourne la taille du fichier contenant le morceau.
 *
 * \return Taille du fichier en octets.
 */

inline int CSong::getFileSize(void) const
{
    return m_fileSize;
}


/**
 * Retourne le débit du morceau.
 *
 * \return Débit binaire.
 */

inline int CSong::getBitRate(void) const
{
    return m_bitRate;
}


/**
 * Retourne la fréquence d'échantillonnage du morceau.
 *
 * \return Fréquence d'échantillonnage.
 */

inline int CSong::getSampleRate(void) const
{
    return m_sampleRate;
}


/**
 * Retourne le format du morceau.
 *
 * \return Format du morceau.
 */

inline CSong::TFormat CSong::getFormat(void) const
{
    return m_format;
}


/**
 * Retourne le nombre de canaux du morceau.
 *
 * \return Nombre de canaux.
 */

inline int CSong::getNumChannels(void) const
{
    return m_numChannels;
}


/**
 * Retourne la durée du morceau.
 *
 * \return Durée du morceau en millisecondes.
 */

inline int CSong::getDuration(void) const
{
    return m_duration;
}


/**
 * Retourne la date de création du morceau.
 *
 * \return Date d'ajout du morceau à la médiathèque.
 */

inline QDateTime CSong::getCreationDate(void) const
{
    return m_creation;
}


/**
 * Retourne la date de modification du morceau.
 *
 * \return Date de modification du morceau.
 */

inline QDateTime CSong::getModificationDate(void) const
{
    return m_modification;
}


/**
 * Indique si le morceau est coché ou pas.
 *
 * \return Booléen.
 */

inline bool CSong::isEnabled(void) const
{
    return m_infos.isEnabled;
}


/**
 * Retourne le titre du morceau.
 *
 * \return Titre du morceau.
 */

inline QString CSong::getTitle(void) const
{
    return m_infos.title;
}


/**
 * Retourne le sous-titre du morceau.
 *
 * \return Sous-titre du morceau.
 */

inline QString CSong::getSubTitle(void) const
{
    return m_infos.subTitle;
}


inline QString CSong::getGrouping(void) const
{
    return m_infos.grouping;
}


inline QString CSong::getArtistName(void) const
{
    return m_infos.artistName;
}


inline QString CSong::getAlbumTitle(void) const
{
    return m_infos.albumTitle;
}


inline QString CSong::getAlbumArtist(void) const
{
    return m_infos.albumArtist;
}


inline QString CSong::getComposer(void) const
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


inline int CSong::getYear(void) const
{
    return m_infos.year;
}


inline int CSong::getTrackNumber(void) const
{
    return m_infos.trackNumber;
}


inline int CSong::getTrackCount(void) const
{
    return m_infos.trackCount;
}


inline int CSong::getDiscNumber(void) const
{
    return m_infos.discNumber;
}


inline int CSong::getDiscCount(void) const
{
    return m_infos.discCount;
}


inline QString CSong::getGenre(void) const
{
    return m_infos.genre;
}


inline int CSong::getRating(void) const
{
    return m_infos.rating;
}


inline QString CSong::getComments(void) const
{
    return m_infos.comments;
}


inline int CSong::getBPM(void) const
{
    return m_infos.bpm;
}


inline QString CSong::getLyrics(void) const
{
    return m_infos.lyrics;
}


inline CSong::TLanguage CSong::getLanguage(void) const
{
    return m_infos.language;
}


inline QString CSong::getLyricist(void) const
{
    return m_infos.lyricist;
}


inline bool CSong::isCompilation(void) const
{
    return m_infos.compilation;
}


inline bool CSong::isSkipShuffle(void) const
{
    return m_infos.skipShuffle;
}


inline int CSong::getNumPlays(void) const
{
    return m_plays.size();
}


inline QDateTime CSong::getLastPlay(void) const
{
    return (m_plays.isEmpty() ? QDateTime() : m_plays.last());
}

#endif // FILE_C_SONG

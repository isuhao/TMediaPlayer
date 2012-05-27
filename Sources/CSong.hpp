
#ifndef FILE_C_SONG
#define FILE_C_SONG

#include <QList>
#include <QDateTime>
#include <QString>
#include <QVariant>
#include <QStringList>
#include <fmod/fmod.hpp>


/**
 * Représente un morceau pouvant être joué.
 */

class CSong : public QObject
{
    Q_OBJECT

    friend class CApplication;
    friend class CDialogEditSong;

public:

    enum TFormat
    {
        FormatUnknown = 0,
        FormatMP3     = 1,
        FormatOGG     = 2,
        FormatFLAC    = 3
    };

    static inline TFormat getFormatFromInteger(int format);
    static inline QString getFormatName(TFormat format);
    static inline QStringList getFormatList(void);

    enum TLanguage
    {
        LangUnknown = 0,
        LangEnglish = 1,
        LangFrench  = 2,
        LangGerman  = 3,
        LangItalian = 4
    };

    static inline TLanguage getLanguageFromInteger(int language);
    static inline QString getLanguageName(TLanguage language);
    static inline QStringList getLanguageList(void);
    static inline TLanguage getLanguageForISO2Code(const QString& code);
    static inline TLanguage getLanguageForISO3Code(const QString& code);
    static inline QString getISO2CodeForLanguage(TLanguage language);
    static inline QString getISO3CodeForLanguage(TLanguage language);


    explicit CSong(CApplication * application);
    explicit CSong(const QString& fileName, CApplication * application);
    ~CSong();

    void loadFromDatabase(void);
    void loadTags(void);
    void writeTags(void) const;

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
    inline int getTrackTotal(void) const;
    inline int getDiscNumber(void) const;
    inline int getDiscTotal(void) const;
    inline QString getGenre(void) const;
    inline int getRating(void) const;
    inline QString getComments(void) const;
    inline int getBPM(void) const;
    inline QString getLyrics(void) const;
    inline TLanguage getLanguage(void) const;

    int getPosition(void) const;
    bool isEnded(void) const;

    inline int getNumPlays(void) const;
    inline QDateTime getLastPlay(void) const;

    static int getId(CApplication * application, const QString& fileName);
    static CSong * loadFromFile(CApplication * application, const QString& fileName);
    static QList<CSong *> loadAllSongsFromDatabase(CApplication * application);

    static QString getFileSize(int fileSize);

public slots:

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
    void setTrackTotal(int trackNumber);
    void setDiscNumber(int discNumber);
    void setDiscTotal(int discNumber);
    void setGenre(const QString& genre);
    void setRating(int rating);
    void setComments(const QString& comments);
    void setBPM(int bpm);
    void setLyrics(const QString& lyrics);
    void setLanguage(TLanguage language);

protected:

    void startPlay(void);
    void startMultiModification(void);

protected slots:

    void setPosition(int position);
    void setVolume(int volume);
    void setMute(bool mute);
    bool loadSound(void);
    void play(void);
    void pause(void);
    void stop(void);
    void updateDatabase(void);
    void emitPlayEnd(void);

signals:

    void songModified(void); ///< Signal émis lorsque les informations du morceau sont modifiées.
    void playEnd(void);      ///< Signal émis lorsque la lecture se termine.

private:

    // Copie interdite
    CSong(const CSong&);
    CSong& operator=(const CSong&);

    CApplication * m_application;
    FMOD::Sound * m_sound;        ///< Pointeur sur la structure de FMOD.
    FMOD::Channel * m_channel;    ///< Canal audio.
    bool m_multiModification;     ///< Indique que plusieurs modifications vont être effectuées, le
                                  ///< signal songModified() est alors émis dans la méthode updateDatabase.
    bool m_isModified;            ///< Indique si les données ont été modifiées.

    int m_id;                     ///< Identifiant du morceau en base de données.
    QString m_fileName;           ///< Fichier audio.
    int m_fileSize;               ///< Taille du fichier en octets.
    int m_bitRate;                ///< Débit binaire.
    int m_sampleRate;             ///< Taux d'échantillonnage.
    TFormat m_format;             ///< Format de fichier.
    int m_numChannels;            ///< Nombre de canaux.
    int m_duration;               ///< Durée du morceau en millisecondes.
    QDateTime m_creation;         ///< Date de création (ajout à la médiathèque).
    QDateTime m_modification;     ///< Date de la dernière modication.

    // Informations modifiables
    QString m_title;              ///< Titre du morceau.
    QString m_subTitle;           ///< Sous-titre du morceau.
    QString m_grouping;           ///< Regroupement.
    QString m_artistName;         ///< Artiste du morceau.
    QString m_albumTitle;         ///< Titre de l'album.
    QString m_albumArtist;        ///< Artiste de l'album.
    QString m_composer;           ///< Compositeur.
    QString m_titleSort;          ///< Titre du morceau pour le tri.
    QString m_artistNameSort;     ///< Artiste du morceau pour le tri.
    QString m_albumTitleSort;     ///< Titre de l'album pour le tri.
    QString m_albumArtistSort;    ///< Artiste de l'album pour le tri.
    QString m_composerSort;       ///< Compositeur pour le tri.
    int m_year;                   ///< Année de sortie de l'album ou du morceau.
    int m_trackNumber;            ///< Numéro de piste.
    int m_trackTotal;             ///< Nombre de piste sur l'album.
    int m_discNumber;             ///< Numéro de disque.
    int m_discTotal;              ///< Nombre de disque de l'album.
    QString m_genre;              ///< Genre.
    int m_rating;                 ///< Note (entre 0 et 5).
    QString m_comments;           ///< Commentaires.
    int m_bpm;                    ///< Battements par minute.
    QString m_lyrics;             ///< Paroles.
    TLanguage m_language;         ///< Langue des paroles.

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
 * Retourne le taux d'échantillonnage du morceau.
 *
 * \return Taux d'échantillonnage.
 */

inline int CSong::getSampleRate(void) const
{
    return m_sampleRate;
}


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
 * Retourne le titre du morceau.
 *
 * \return Titre du morceau.
 */

inline QString CSong::getTitle(void) const
{
    return m_title;
}


/**
 * Retourne le sous-titre du morceau.
 *
 * \return Sous-titre du morceau.
 */

inline QString CSong::getSubTitle(void) const
{
    return m_subTitle;
}


inline QString CSong::getGrouping(void) const
{
    return m_grouping;
}


inline QString CSong::getArtistName(void) const
{
    return m_artistName;
}


inline QString CSong::getAlbumTitle(void) const
{
    return m_albumTitle;
}


inline QString CSong::getAlbumArtist(void) const
{
    return m_albumArtist;
}


inline QString CSong::getComposer(void) const
{
    return m_composer;
}


inline QString CSong::getTitleSort(bool empty) const
{
    if (!empty && m_titleSort.isEmpty()) return m_title;
    return m_titleSort;
}


inline QString CSong::getArtistNameSort(bool empty) const
{
    if (!empty && m_artistNameSort.isEmpty()) return m_artistName;
    return m_artistNameSort;
}


inline QString CSong::getAlbumTitleSort(bool empty) const
{
    if (!empty && m_albumTitleSort.isEmpty()) return m_albumTitle;
    return m_albumTitleSort;
}


inline QString CSong::getAlbumArtistSort(bool empty) const
{
    if (!empty && m_albumArtistSort.isEmpty()) return m_albumArtist;
    return m_albumArtistSort;
}


inline QString CSong::getComposerSort(bool empty) const
{
    if (!empty && m_composerSort.isEmpty()) return m_composer;
    return m_composerSort;
}


inline int CSong::getYear(void) const
{
    return m_year;
}


inline int CSong::getTrackNumber(void) const
{
    return m_trackNumber;
}


inline int CSong::getTrackTotal(void) const
{
    return m_trackTotal;
}


inline int CSong::getDiscNumber(void) const
{
    return m_discNumber;
}


inline int CSong::getDiscTotal(void) const
{
    return m_discTotal;
}


inline QString CSong::getGenre(void) const
{
    return m_genre;
}


inline int CSong::getRating(void) const
{
    return m_rating;
}


inline QString CSong::getComments(void) const
{
    return m_comments;
}


inline int CSong::getBPM(void) const
{
    return m_bpm;
}


inline QString CSong::getLyrics(void) const
{
    return m_lyrics;
}


inline CSong::TLanguage CSong::getLanguage(void) const
{
    return m_language;
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


#ifndef FILE_I_CRITERIA
#define FILE_I_CRITERIA

#include <QObject>
#include <QList>
#include <QVariant>


class CSong;
class CDynamicPlayList;
class CApplication;
class IWidgetCriteria;


class ICriteria : public QObject
{
    Q_OBJECT

    friend class CDynamicPlayList;
    friend class CWidgetCriteria;
    friend class CMultiCriterion; /// La m�thode ICriteria::setPlayList est innaccessible depuis CMultiCriterion::setPlayList !

public:

    /**
     * Types de crit�res disponibles.
     * La valeur est cod�e sur 2 octets : le premier indique le type, et le suivant le crit�re.
     */

    enum TType
    {
        // Special
        TypeInvalid            = 0x0000,
        TypeUnion              = 0x0001,
        TypeIntersection       = 0x0002,

        // Boolean
        TypeMaskBoolean        = 0x01,
        TypeLanguage           = 0x0101,
        TypePlayList           = 0x0102,
        TypeFormat             = 0x0103,

        // String
        TypeMaskString         = 0x02,
        TypeTitle              = 0x0201,
        TypeArtist             = 0x0202,
        TypeAlbum              = 0x0203,
        TypeAlbumArtist        = 0x0204,
        TypeComposer           = 0x0205,
        TypeGenre              = 0x0206,
        TypeComments           = 0x0207,
        TypeLyrics             = 0x0208,
        TypeFileName           = 0x0209,

        // Number
        TypeMaskNumber         = 0x03,
        TypeYear               = 0x0301,
        TypeTrackNumber        = 0x0302,
        TypeDiscNumber         = 0x0303,
        TypeBitRate            = 0x0304,
        TypeSampleRate         = 0x0305,
        TypePlayCount          = 0x0306,
        TypeChannels           = 0x0307,
        TypeRating             = 0x0308,
        TypeFileSize           = 0x0309,

        // Time
        TypeMaskTime           = 0x04,
        TypeDuration           = 0x0401,

        // Date
        TypeMaskDate           = 0x05,
        TypeLastPlayTime       = 0x0501,
        TypeAdded              = 0x0502,
        TypeModified           = 0x0503
    };

    enum TCondition
    {
        // Special
        CondInvalid            = 0x0000,

        // Boolean
        CondMaskBoolean        = 0x01,
        CondIs                 = 0x0101,
        CondIsNot              = 0x0102,
        // Dupliquer ?

        // String
        CondMaskString         = 0x02,
        CondStringEqual        = 0x0201,
        CondStringNotEqual     = 0x0202,
        CondStringContains     = 0x0203,
        CondStringNotContains  = 0x0204,
        CondStringStartsWith   = 0x0205,
        CondStringEndsWith     = 0x0206,
        CondStringRegex        = 0x0207,

        // Number
        CondMaskNumber         = 0x03,
        CondNumberEqual        = 0x0301,
        CondNumberNotEqual     = 0x0302,
        CondNumberLessThan     = 0x0303,
        CondNumberGreaterThan  = 0x0304,
        CondNumberBetween      = 0x0305,

        // Time
        CondMaskTime           = 0x04,
        CondTimeIs             = 0x0401,
        CondTimeIsNot          = 0x0402,
        CondTimeLessThan       = 0x0403,
        CondTimeGreaterThan    = 0x0404,
        CondTimeBetween        = 0x0405,

        // Date
        CondMaskDate           = 0x05,
        CondDateIs             = 0x0501,
        CondDateIsNot          = 0x0502,
        CondDateBefore         = 0x0503,
        CondDateAfter          = 0x0504,
        CondDateInLast         = 0x0505,
        CondDateNotInLast      = 0x0506,
        CondDateBetween        = 0x0507
    };

    /**
     * Conditions de mise � jour d'une liste dynamique, selon les types de crit�re.
     */

    enum TUpdateCondition
    {
        UpdateNever          = 0x00, ///< Pas besoin de mise-�-jour.
        UpdateOnSongAdded    = 0x01, ///< Mise � jour lorsqu'un morceau est ajout� � la m�diath�que.
        UpdateOnSongRemoved  = 0x02, ///< Mise � jour lorsqu'un morceau est retir� de la m�diath�que.
        UpdateOnSongModified = 0x04, ///< Mise � jour lorsque les informations d'un morceau sont modifi�s.
        UpdateOnSongMoved    = 0x08, ///< Mise � jour lorsque le fichier d'un morceau est d�plac�.
        UpdateOnSongPlayEnd  = 0x10, ///< Mise � jour lorsque la lecture d'un morceau se termine.
        UpdateOnListModified = 0x20  ///< Mise � jour lorsqu'une liste de lecture est modifi�e.
    };

    Q_DECLARE_FLAGS(TUpdateConditions, TUpdateCondition)


    explicit ICriteria(CApplication * application, QObject * parent = NULL);
    virtual ~ICriteria();

    inline int getId(void) const;
    inline int getType(void) const;
    inline int getCondition(void) const;
    inline QVariant getValue1(void) const;
    inline QVariant getValue2(void) const;
    inline CDynamicPlayList * getPlayList(void) const;

    virtual bool matchCriteria(CSong * song) const = 0;
    virtual QList<CSong *> getSongs(const QList<CSong *>& from, const QList<CSong *>& with = QList<CSong *>()) const;
    //virtual QList<int> getValidTypes(void) const;
    //virtual bool isValidType(int type) const;
    //virtual QList<int> getValidConditions(void) const;
    virtual inline TUpdateConditions getUpdateConditions(void) const;

    virtual IWidgetCriteria * getWidget(void) const = 0;

protected:

    int m_type;        ///< Type de crit�re (TType).
    int m_condition;   ///< Condition. (TCondition)
    QVariant m_value1; ///< Valeur 1.
    QVariant m_value2; ///< Valeur 2.
    CApplication * m_application;

    virtual void setPlayList(CDynamicPlayList * playList);
    virtual void insertIntoDatabase(CApplication * application);

private:

    int m_id;                      ///< Identifiant du crit�re en base de donn�es.
    int m_position;                ///< Position du crit�re.
    ICriteria * m_parent;          ///< Pointeur sur le crit�re parent.
    CDynamicPlayList * m_playList; ///< Pointeur sur la liste de lecture dynamique.
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ICriteria::TUpdateConditions)


inline int ICriteria::getId(void) const
{
    return m_id;
}


inline int ICriteria::getType(void) const
{
    return m_type;
}


inline int ICriteria::getCondition(void) const
{
    return m_condition;
}


inline QVariant ICriteria::getValue1(void) const
{
    return m_value1;
}


inline QVariant ICriteria::getValue2(void) const
{
    return m_value2;
}


inline CDynamicPlayList * ICriteria::getPlayList(void) const
{
    return m_playList;
}


inline ICriteria::TUpdateConditions ICriteria::getUpdateConditions(void) const
{
    return UpdateNever;
}

#endif // FILE_I_CRITERIA


#ifndef FILE_CCRITERIA
#define FILE_CCRITERIA

#include <QList>
#include <QString>
#include <QObject>


class CCriteria : public QObject
{
    Q_OBJECT

public:

    /// Type de critères
    enum TCriteriaType
    {
        Title,        /// String, CApplication::songAdded, songModified, songRemoved
        Artist,       /// String, CApplication::songAdded, songModified, songRemoved
        Album,        /// String, CApplication::songAdded, songModified, songRemoved
        AlbumArtist,  /// String, CApplication::songAdded, songModified, songRemoved
        Duration,     /// Number, CApplication::songAdded, songModified, songRemoved
        Year,         /// Number, CApplication::songAdded, songModified, songRemoved
        TrackNumber,  /// Number, CApplication::songAdded, songModified, songRemoved
        AlbumNumber,  /// Number, CApplication::songAdded, songModified, songRemoved
        Genre,        /// String, CApplication::songAdded, songModified, songRemoved
        Type,         /// String, CApplication::songAdded, songRemoved
        Bitrate,      /// Number, CApplication::songAdded, songRemoved
        LastPlayTime, /// Date, CApplication::songPlayEnd, songAdded, songRemoved
        PlayCount,    /// Number, CApplication::songPlayEnd, songAdded, songRemoved
        PlayList      /// Playlist, CPlayList::listModified
    };

    /// Conditions
    enum TCondition
    {
        NumberEqual,      /// 1
        NumberNotEqual,   /// 1
        NumberInf,        /// 1
        NumberInfOrEqual, /// 1
        NumberSup,        /// 1
        NumberSupOrEqual, /// 1
        NumberBetween,    /// 2

        StringEqual,      /// 1
        StringNotEqual,   /// 1
        StringContains,   /// 1
        StringStartsWith, /// 1
        StringEndsWith,   /// 1

        DateIs,           /// 1
        DateIsNot,        /// 1
        DateInf,          /// 1
        DateInfOrEqual,   /// 1
        DateSup,          /// 1
        DateSupOrEqual,   /// 1
        DateBetween,      /// 2
        DateBefore,       /// 1
        DateAfter,        /// 1

        PlayListIs,       /// 1
        PlayListIsNot     /// 1
    };

    CCriteria(void);
    ~CCriteria();

private:

    TCriteriaType m_type;
    TCondition m_condition;
    QString m_value1;
    QString m_value2;
    bool m_union; // => CSubCriteria
    QList<CCriteria *> m_children; // => CSubCriteria
};

#endif // FILE_CCRITERIA

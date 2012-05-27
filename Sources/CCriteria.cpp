
#include "CCriteria.hpp"
#include "CSong.hpp"

#include <QtDebug>


/**
 * Crée le critère.
 *
 * \param parent Pointeur sur l'objet parent.
 */

CCriteria::CCriteria(QObject * parent) :
    ICriteria (parent)
{

}


/**
 * Détruit le critère.
 */

CCriteria::~CCriteria()
{
    qDebug() << "CCriteria::~CCriteria()";
}


/**
 * Détermine si le morceau vérifie le critère.
 *
 * \todo Gérer le type "Liste de lecture".
 * \todo Gérer le type "Langue".
 * \todo Gérer le type "Format".
 * \todo Pouvoir changer l'échelle de temps pour les conditions "dans les derniers N jours".
 *
 * \param song Morceau à tester.
 * \return Booléen.
 */

bool CCriteria::matchCriteria(CSong * song) const
{
    Q_CHECK_PTR(song);

    if ((m_type >> 8) == ICriteria::TypeMaskBoolean)
    {
        //...
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskString)
    {
        QString str;

        switch (m_type)
        {
            default:
                qWarning() << "CCriteria::matchCriteria() : le type de critère n'est pas géré";
                return false;

            case TypeTitle      : str = song->getTitle();       break;
            case TypeArtist     : str = song->getArtistName();  break;
            case TypeAlbum      : str = song->getAlbumTitle();  break;
            case TypeAlbumArtist: str = song->getAlbumArtist(); break;
            case TypeComposer   : str = song->getComposer();    break;
            case TypeGenre      : str = song->getGenre();       break;
            case TypeComments   : str = song->getComments();    break;
            case TypeLyrics     : str = song->getLyrics();      break;
            case TypeFileName   : str = song->getFileName();    break;
        }

        if ((m_condition >> 8) != ICriteria::CondMaskString)
        {
            qWarning() << "CCriteria::matchCriteria() : la condition et le type ne correspondent pas";
            return false;
        }

        switch (m_condition)
        {
            default:
                qWarning() << "CCriteria::matchCriteria() : la condition n'est pas gérée";
                return false;

            case CondStringEqual      : return (str == m_value1.toString());
            case CondStringNotEqual   : return (str != m_value1.toString());
            case CondStringContains   : return (str.contains(m_value1.toString()));
            case CondStringNotContains: return (!str.contains(m_value1.toString()));
            case CondStringStartsWith : return (str.startsWith(m_value1.toString()));
            case CondStringEndsWith   : return (str.endsWith(m_value1.toString()));
        }
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskNumber)
    {
        int num;

        switch (m_type)
        {
            default:
                qWarning() << "CCriteria::matchCriteria() : le type de critère n'est pas géré";
                return false;

            case TypeYear       : num = song->getYear();       break;
            case TypeTrackNumber: num = song->getTrackNumber();break;
            case TypeDiscNumber : num = song->getDiscNumber(); break;
            case TypeBitRate    : num = song->getBitRate();    break;
            case TypeSampleRate : num = song->getSampleRate(); break;
            case TypePlayCount  : num = song->getNumPlays();   break;
            case TypeChannels   : num = song->getNumChannels();break;
            case TypeRating     : num = song->getRating();     break;
            case TypeFileSize   : num = song->getFileSize();   break;
        }

        if ((m_condition >> 8) != ICriteria::CondMaskNumber)
        {
            qWarning() << "CCriteria::matchCriteria() : la condition et le type ne correspondent pas";
            return false;
        }

        switch (m_condition)
        {
            default:
                qWarning() << "CCriteria::matchCriteria() : la condition n'est pas gérée";
                return false;

            case CondNumberEqual      : return (num == m_value1.toInt());
            case CondNumberNotEqual   : return (num != m_value1.toInt());
            case CondNumberLessThan   : return (num < m_value1.toInt());
            case CondNumberGreaterThan: return (num > m_value1.toInt());
            case CondNumberBetween    : return (num > m_value1.toInt() && num < m_value2.toInt());
        }
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskTime)
    {
        QTime time = QTime(0, 0);

        switch (m_type)
        {
            default:
                qWarning() << "CCriteria::matchCriteria() : le type de critère n'est pas géré";
                return false;

            case TypeDuration: time = time.addMSecs(song->getDuration()); break;
        }

        if ((m_condition >> 8) != ICriteria::CondMaskTime)
        {
            qWarning() << "CCriteria::matchCriteria() : la condition et le type ne correspondent pas";
            return false;
        }

        switch (m_condition)
        {
            default:
                qWarning() << "CCriteria::matchCriteria() : la condition n'est pas gérée";
                return false;

            case CondTimeIs         : return (time == m_value1.toTime());
            case CondTimeIsNot      : return (time != m_value1.toTime());
            case CondTimeLessThan   : return (time < m_value1.toTime());
            case CondTimeGreaterThan: return (time > m_value1.toTime());
            case CondTimeBetween    : return (time > m_value1.toTime() && time < m_value2.toTime());
        }
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskDate)
    {
        QDateTime date;

        switch (m_type)
        {
            default:
                qWarning() << "CCriteria::matchCriteria() : le type de critère n'est pas géré";
                return false;

            case TypeLastPlayTime: date = song->getLastPlay(); break;
            case TypeAdded       : date = song->getCreationDate(); break;
            case TypeModified    : date = song->getModificationDate(); break;
        }

        if ((m_condition >> 8) != ICriteria::CondMaskDate)
        {
            qWarning() << "CCriteria::matchCriteria() : la condition et le type ne correspondent pas";
            return false;
        }

        switch (m_condition)
        {
            default:
                qWarning() << "CCriteria::matchCriteria() : la condition n'est pas gérée";
                return false;

            case CondDateIs       : return (date == m_value1.toDateTime());
            case CondDateIsNot    : return (date != m_value1.toDateTime());
            case CondDateBefore   : return (date < m_value1.toDateTime());
            case CondDateAfter    : return (date > m_value1.toDateTime());
            case CondDateBetween  : return (date > m_value1.toDateTime() && date < m_value2.toDateTime());

            case CondDateInLast:
            {
                QDateTime dateCmp1 = QDateTime::currentDateTime();
                QDateTime dateCmp2 = dateCmp1.addDays(-m_value2.toInt()); //TODO: gérer l'unité de temps
                return (date <= dateCmp1 && date >= dateCmp2);
            }

            case CondDateNotInLast:
            {
                QDateTime dateCmp1 = QDateTime::currentDateTime();
                QDateTime dateCmp2 = dateCmp1.addDays(-m_value2.toInt()); //TODO: gérer l'unité de temps
                return !(date <= dateCmp1 && date >= dateCmp2);
            }
        }
    }

    return false;
}

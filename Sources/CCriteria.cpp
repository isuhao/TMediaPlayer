
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


#include "CWidgetCriteria.hpp" // Si on l'inclut avant, l'intellisense bug complètement...


/// \todo Implémentation.
IWidgetCriteria * CCriteria::getWidget(void) const
{
    CWidgetCriteria * widget = new CWidgetCriteria(NULL);

    switch (m_type)
    {
        case TypeMultiCriterion:
            qWarning() << "CCriteria::getWidget() : un critère simple ne peut pas avoir le type TypeMultiCriterion";

        default:
        case TypeInvalid:

        case TypeTitle       : widget->m_uiWidget->listType->setCurrentIndex( 0); break;
        case TypeArtist      : widget->m_uiWidget->listType->setCurrentIndex( 1); break;
        case TypeAlbum       : widget->m_uiWidget->listType->setCurrentIndex( 2); break;
        case TypeAlbumArtist : widget->m_uiWidget->listType->setCurrentIndex( 3); break;
        case TypeComposer    : widget->m_uiWidget->listType->setCurrentIndex( 4); break;
        case TypeGenre       : widget->m_uiWidget->listType->setCurrentIndex( 5); break;
        case TypeComments    : widget->m_uiWidget->listType->setCurrentIndex( 6); break;
        case TypeLyrics      : widget->m_uiWidget->listType->setCurrentIndex( 7); break;
        case TypeFileName    : widget->m_uiWidget->listType->setCurrentIndex( 8); break;
        case TypeYear        : widget->m_uiWidget->listType->setCurrentIndex( 9); break;
        case TypeTrackNumber : widget->m_uiWidget->listType->setCurrentIndex(10); break;
        case TypeDiscNumber  : widget->m_uiWidget->listType->setCurrentIndex(11); break;
        case TypeBitRate     : widget->m_uiWidget->listType->setCurrentIndex(12); break;
        case TypeSampleRate  : widget->m_uiWidget->listType->setCurrentIndex(13); break;
        case TypePlayCount   : widget->m_uiWidget->listType->setCurrentIndex(14); break;
        case TypeChannels    : widget->m_uiWidget->listType->setCurrentIndex(15); break;
        case TypeRating      : widget->m_uiWidget->listType->setCurrentIndex(16); break;
        case TypeFileSize    : widget->m_uiWidget->listType->setCurrentIndex(17); break;
        case TypeDuration    : widget->m_uiWidget->listType->setCurrentIndex(18); break;
        case TypeLastPlayTime: widget->m_uiWidget->listType->setCurrentIndex(19); break;
        case TypeAdded       : widget->m_uiWidget->listType->setCurrentIndex(20); break;
        case TypeModified    : widget->m_uiWidget->listType->setCurrentIndex(21); break;
        case TypeLanguage    : widget->m_uiWidget->listType->setCurrentIndex(22); break;
        case TypePlayList    : widget->m_uiWidget->listType->setCurrentIndex(23); break;
        case TypeFormat      : widget->m_uiWidget->listType->setCurrentIndex(24); break;
    }

    if ((m_type >> 8) == TypeMaskBoolean)
    {
        switch (m_condition)
        {
            default:
            case CondIs   : widget->m_uiWidget->listConditionBoolean->setCurrentIndex(0); break;
            case ConsIsNot: widget->m_uiWidget->listConditionBoolean->setCurrentIndex(1); break;
        }
    }
    else if ((m_type >> 8) == TypeMaskString)
    {
        switch (m_condition)
        {
            default:
            case CondStringEqual      : widget->m_uiWidget->listConditionString->setCurrentIndex(0); break;
            case CondStringNotEqual   : widget->m_uiWidget->listConditionString->setCurrentIndex(1); break;
            case CondStringContains   : widget->m_uiWidget->listConditionString->setCurrentIndex(2); break;
            case CondStringNotContains: widget->m_uiWidget->listConditionString->setCurrentIndex(3); break;
            case CondStringStartsWith : widget->m_uiWidget->listConditionString->setCurrentIndex(4); break;
            case CondStringEndsWith   : widget->m_uiWidget->listConditionString->setCurrentIndex(5); break;
        }

        widget->m_uiWidget->editValue1String->setText(m_value1.toString());
    }
    else if ((m_type >> 8) == TypeMaskNumber)
    {
        switch (m_condition)
        {
            default:
            case CondNumberEqual      : widget->m_uiWidget->listConditionNumber->setCurrentIndex(0); break;
            case CondNumberNotEqual   : widget->m_uiWidget->listConditionNumber->setCurrentIndex(1); break;
            case CondNumberLessThan   : widget->m_uiWidget->listConditionNumber->setCurrentIndex(2); break;
            case CondNumberGreaterThan: widget->m_uiWidget->listConditionNumber->setCurrentIndex(3); break;
            case CondNumberBetween    : widget->m_uiWidget->listConditionNumber->setCurrentIndex(4); break;
        }

        widget->m_uiWidget->editValue1Number->setValue(m_value1.toInt());
        widget->m_uiWidget->editValue1Number->setValue(m_value2.toInt());
    }
    else if ((m_type >> 8) == TypeMaskTime)
    {
        switch (m_condition)
        {
            default:
            case CondTimeIs         : widget->m_uiWidget->listConditionTime->setCurrentIndex(0); break;
            case CondTimeIsNot      : widget->m_uiWidget->listConditionTime->setCurrentIndex(1); break;
            case CondTimeLessThan   : widget->m_uiWidget->listConditionTime->setCurrentIndex(2); break;
            case CondTimeGreaterThan: widget->m_uiWidget->listConditionTime->setCurrentIndex(3); break;
            case CondTimeBetween    : widget->m_uiWidget->listConditionTime->setCurrentIndex(4); break;
        }

        widget->m_uiWidget->editValue1Time->setTime(m_value1.toTime());
        widget->m_uiWidget->editValue2Time->setTime(m_value2.toTime());
    }
    else if ((m_type >> 8) == TypeMaskDate)
    {
        switch (m_condition)
        {
            default:
            case CondDateIs       : widget->m_uiWidget->listConditionDate->setCurrentIndex(0); break;
            case CondDateIsNot    : widget->m_uiWidget->listConditionDate->setCurrentIndex(1); break;
            case CondDateBefore   : widget->m_uiWidget->listConditionDate->setCurrentIndex(2); break;
            case CondDateAfter    : widget->m_uiWidget->listConditionDate->setCurrentIndex(3); break;
            case CondDateInLast   : widget->m_uiWidget->listConditionDate->setCurrentIndex(4); break;
            case CondDateNotInLast: widget->m_uiWidget->listConditionDate->setCurrentIndex(5); break;
            case CondDateBetween  : widget->m_uiWidget->listConditionDate->setCurrentIndex(6); break;
        }

        widget->m_uiWidget->editValue1Number->setValue(m_value1.toInt());
        widget->m_uiWidget->listDateUnit->setCurrentIndex(m_value2.toInt());
        widget->m_uiWidget->editValue1Date->setDateTime(m_value1.toDateTime());
        widget->m_uiWidget->editValue2Date->setDateTime(m_value2.toDateTime());
    }

    return widget;
}

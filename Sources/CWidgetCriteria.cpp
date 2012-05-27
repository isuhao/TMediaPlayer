
#include "CWidgetCriteria.hpp"
#include "CCriteria.hpp"
#include "CSong.hpp"

#include <QtDebug>


/**
 * Construit le widget.
 *
 * \todo Remplir la liste des types de critères dans le code.
 *
 * \param parent Widget parent.
 */

CWidgetCriteria::CWidgetCriteria(QWidget * parent) :
    IWidgetCriteria (parent),
    m_uiWidget      (new Ui::WidgetCriteria())
{
    m_uiWidget->setupUi(this);

    //TODO: remplir la liste de type ici !

    changeType(0);

    connect(m_uiWidget->listType, SIGNAL(currentIndexChanged(int)), this, SLOT(changeType(int)));
    connect(m_uiWidget->listConditionBoolean, SIGNAL(currentIndexChanged(int)), this, SLOT(changeConditionBoolean(int)));
    connect(m_uiWidget->listConditionString,  SIGNAL(currentIndexChanged(int)), this, SLOT(changeConditionString(int)));
    connect(m_uiWidget->listConditionNumber,  SIGNAL(currentIndexChanged(int)), this, SLOT(changeConditionNumber(int)));
    connect(m_uiWidget->listConditionTime,    SIGNAL(currentIndexChanged(int)), this, SLOT(changeConditionTime(int)));
    connect(m_uiWidget->listConditionDate,    SIGNAL(currentIndexChanged(int)), this, SLOT(changeConditionDate(int)));
}


/**
 * Détruit le widget.
 */

CWidgetCriteria::~CWidgetCriteria()
{
    qDebug() << "CWidgetCriteria::~CWidgetCriteria()";
    delete m_uiWidget;
}


/**
 * Retourne le critère définis par le widget.
 *
 * \todo Gérer le critère "Playlist".
 *
 * \return Pointeur sur le critère.
 */

ICriteria * CWidgetCriteria::getCriteria(void)
{
    qDebug() << "CWidgetCriteria::getCriteria()";

    CCriteria * criteria = new CCriteria(this);
    criteria->m_type      = m_type;
    criteria->m_condition = m_condition;

    if ((m_type >> 8) == ICriteria::TypeMaskBoolean)
    {
        if (m_type == ICriteria::TypeLanguage)
        {
            criteria->m_value1 = CSong::getISO2CodeForLanguage(CSong::getLanguageFromInteger(m_uiWidget->listLanguage->currentIndex()));
        }
        else if (m_type == ICriteria::TypePlayList)
        {
            //TODO...
            //criteria->m_value1 = 
        }
        else if (m_type == ICriteria::TypeFormat)
        {
            criteria->m_value1 = m_uiWidget->listFormat->currentIndex();
        }
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskString)
    {
        criteria->m_value1 = m_uiWidget->editValue1String->text();
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskNumber)
    {
        criteria->m_value1 = m_uiWidget->editValue1Number->text();
        criteria->m_value2 = m_uiWidget->editValue2Number->text();
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskTime)
    {
        criteria->m_value1 = m_uiWidget->editValue1Time->date();
        criteria->m_value2 = m_uiWidget->editValue2Time->date();
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskDate)
    {
        criteria->m_value1 = m_uiWidget->editValue1Date->date();
        criteria->m_value2 = m_uiWidget->editValue2Date->date();
        //criteria->m_value2 = m_uiWidget->listDateUnit->currentIndex();
    }

    return criteria;
}


void CWidgetCriteria::changeType(int num)
{
    switch (num)
    {
        default: m_type = ICriteria::TypeInvalid;      break;
        case  0: m_type = ICriteria::TypeTitle;        break;
        case  1: m_type = ICriteria::TypeArtist;       break;
        case  2: m_type = ICriteria::TypeAlbum;        break;
        case  3: m_type = ICriteria::TypeAlbumArtist;  break;
        case  4: m_type = ICriteria::TypeComposer;     break;
        case  5: m_type = ICriteria::TypeGenre;        break;
        case  6: m_type = ICriteria::TypeComments;     break;
        case  7: m_type = ICriteria::TypeLyrics;       break;
        case  8: m_type = ICriteria::TypeFileName;     break;
        case  9: m_type = ICriteria::TypeYear;         break;
        case 10: m_type = ICriteria::TypeTrackNumber;  break;
        case 11: m_type = ICriteria::TypeDiscNumber;   break;
        case 12: m_type = ICriteria::TypeBitRate;      break;
        case 13: m_type = ICriteria::TypeSampleRate;   break;
        case 14: m_type = ICriteria::TypePlayCount;    break;
        case 15: m_type = ICriteria::TypeChannels;     break;
        case 16: m_type = ICriteria::TypeRating;       break;
        case 17: m_type = ICriteria::TypeFileSize;     break;
        case 18: m_type = ICriteria::TypeDuration;     break;
        case 19: m_type = ICriteria::TypeLastPlayTime; break;
        case 20: m_type = ICriteria::TypeAdded;        break;
        case 21: m_type = ICriteria::TypeModified;     break;
        case 22: m_type = ICriteria::TypeLanguage;     break;
        case 23: m_type = ICriteria::TypePlayList;     break;
        case 24: m_type = ICriteria::TypeFormat;       break;
    }

    if ((m_type >> 8) == ICriteria::TypeMaskBoolean)
    {
        m_uiWidget->listConditionString->hide();
        m_uiWidget->listConditionNumber->hide();
        m_uiWidget->listConditionTime->hide();
        m_uiWidget->listConditionDate->hide();
        m_uiWidget->listConditionBoolean->show();

        changeConditionBoolean(0);
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskString)
    {
        m_uiWidget->listConditionString->show();
        m_uiWidget->listConditionNumber->hide();
        m_uiWidget->listConditionTime->hide();
        m_uiWidget->listConditionDate->hide();
        m_uiWidget->listConditionBoolean->hide();

        changeConditionString(0);
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskNumber)
    {
        m_uiWidget->listConditionString->hide();
        m_uiWidget->listConditionNumber->show();
        m_uiWidget->listConditionTime->hide();
        m_uiWidget->listConditionDate->hide();
        m_uiWidget->listConditionBoolean->hide();

        changeConditionNumber(0);
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskTime)
    {
        m_uiWidget->listConditionString->hide();
        m_uiWidget->listConditionNumber->hide();
        m_uiWidget->listConditionTime->show();
        m_uiWidget->listConditionDate->hide();
        m_uiWidget->listConditionBoolean->hide();

        changeConditionTime(0);
    }
    else if ((m_type >> 8) == ICriteria::TypeMaskDate)
    {
        m_uiWidget->listConditionString->hide();
        m_uiWidget->listConditionNumber->hide();
        m_uiWidget->listConditionTime->hide();
        m_uiWidget->listConditionDate->show();
        m_uiWidget->listConditionBoolean->hide();

        changeConditionDate(0);
    }
}

void CWidgetCriteria::changeConditionBoolean(int num)
{
    if ((m_type >> 8) != ICriteria::TypeMaskBoolean)
    {
        return;
    }

    switch (num)
    {
        default: m_condition = ICriteria::CondInvalid; break;
        case  0: m_condition = ICriteria::CondIs;      break;
        case  1: m_condition = ICriteria::ConsIsNot;   break;
    }

    m_uiWidget->editValue1String->hide();
    m_uiWidget->editValue1Number->hide();
    m_uiWidget->editValue1Time->hide();
    m_uiWidget->editValue1Date->hide();
    m_uiWidget->editValue2Number->hide();
    m_uiWidget->editValue2Time->hide();
    m_uiWidget->editValue2Date->hide();
    m_uiWidget->listDateUnit->hide();
    m_uiWidget->lblBetween->hide();

    m_uiWidget->listFormat->setVisible(m_type == ICriteria::TypeFormat);
    m_uiWidget->listLanguage->setVisible(m_type == ICriteria::TypeLanguage);
    m_uiWidget->listPlayList->setVisible(m_type == ICriteria::TypePlayList);
}

void CWidgetCriteria::changeConditionString(int num)
{
    if ((m_type >> 8) != ICriteria::TypeMaskString)
    {
        return;
    }

    switch (num)
    {
        default: m_condition = ICriteria::CondInvalid;           break;
        case  0: m_condition = ICriteria::CondStringEqual;       break;
        case  1: m_condition = ICriteria::CondStringNotEqual;    break;
        case  2: m_condition = ICriteria::CondStringContains;    break;
        case  3: m_condition = ICriteria::CondStringNotContains; break;
        case  4: m_condition = ICriteria::CondStringStartsWith;  break;
        case  5: m_condition = ICriteria::CondStringEndsWith;    break;
    }

    m_uiWidget->editValue1String->show();
    m_uiWidget->editValue1Number->hide();
    m_uiWidget->editValue1Time->hide();
    m_uiWidget->editValue1Date->hide();
    m_uiWidget->editValue2Number->hide();
    m_uiWidget->editValue2Time->hide();
    m_uiWidget->editValue2Date->hide();
    m_uiWidget->listDateUnit->hide();
    m_uiWidget->lblBetween->hide();

    m_uiWidget->listFormat->hide();
    m_uiWidget->listLanguage->hide();
    m_uiWidget->listPlayList->hide();
}


void CWidgetCriteria::changeConditionNumber(int num)
{
    if ((m_type >> 8) != ICriteria::TypeMaskNumber)
    {
        return;
    }

    switch (num)
    {
        default: m_condition = ICriteria::CondInvalid;           break;
        case  0: m_condition = ICriteria::CondNumberEqual;       break;
        case  1: m_condition = ICriteria::CondNumberNotEqual;    break;
        case  2: m_condition = ICriteria::CondNumberLessThan;    break;
        case  3: m_condition = ICriteria::CondNumberGreaterThan; break;
        case  4: m_condition = ICriteria::CondNumberBetween;     break;
    }

    m_uiWidget->editValue1String->hide();
    m_uiWidget->editValue1Number->show();
    m_uiWidget->editValue1Time->hide();
    m_uiWidget->editValue1Date->hide();
    m_uiWidget->editValue2Number->setVisible(m_condition == ICriteria::CondNumberBetween);
    m_uiWidget->editValue2Time->hide();
    m_uiWidget->editValue2Date->hide();
    m_uiWidget->listDateUnit->hide();
    m_uiWidget->lblBetween->setVisible(m_condition == ICriteria::CondNumberBetween);

    m_uiWidget->listFormat->hide();
    m_uiWidget->listLanguage->hide();
    m_uiWidget->listPlayList->hide();
}


void CWidgetCriteria::changeConditionTime(int num)
{
    if ((m_type >> 8) != ICriteria::TypeMaskTime)
    {
        return;
    }

    switch (num)
    {
        default: m_condition = ICriteria::CondInvalid;         break;
        case  0: m_condition = ICriteria::CondTimeIs;          break;
        case  1: m_condition = ICriteria::CondTimeIsNot;       break;
        case  2: m_condition = ICriteria::CondTimeLessThan;    break;
        case  3: m_condition = ICriteria::CondTimeGreaterThan; break;
        case  4: m_condition = ICriteria::CondTimeBetween;     break;
    }

    m_uiWidget->editValue1String->hide();
    m_uiWidget->editValue1Number->hide();
    m_uiWidget->editValue1Time->show();
    m_uiWidget->editValue1Date->hide();
    m_uiWidget->editValue2Number->hide();
    m_uiWidget->editValue2Time->setVisible(m_condition == ICriteria::CondTimeBetween);
    m_uiWidget->editValue2Date->hide();
    m_uiWidget->listDateUnit->hide();
    m_uiWidget->lblBetween->setVisible(m_condition == ICriteria::CondTimeBetween);

    m_uiWidget->listFormat->hide();
    m_uiWidget->listLanguage->hide();
    m_uiWidget->listPlayList->hide();
}


void CWidgetCriteria::changeConditionDate(int num)
{
    if ((m_type >> 8) != ICriteria::TypeMaskDate)
    {
        return;
    }

    switch (num)
    {
        default: m_condition = ICriteria::CondInvalid;       break;
        case  0: m_condition = ICriteria::CondDateIs;        break;
        case  1: m_condition = ICriteria::CondDateIsNot;     break;
        case  2: m_condition = ICriteria::CondDateBefore;    break;
        case  3: m_condition = ICriteria::CondDateAfter;     break;
        case  4: m_condition = ICriteria::CondDateInLast;    break;
        case  5: m_condition = ICriteria::CondDateNotInLast; break;
        case  6: m_condition = ICriteria::CondDateBetween;   break;
    }

    m_uiWidget->editValue1String->hide();
    m_uiWidget->editValue1Number->setVisible(m_condition == ICriteria::CondDateInLast || m_condition == ICriteria::CondDateNotInLast);
    m_uiWidget->editValue1Time->hide();
    m_uiWidget->editValue1Date->setVisible(m_condition != ICriteria::CondDateInLast && m_condition != ICriteria::CondDateNotInLast);
    m_uiWidget->editValue2Number->hide();
    m_uiWidget->editValue2Time->hide();
    m_uiWidget->editValue2Date->setVisible(m_condition == ICriteria::CondDateBetween);
    m_uiWidget->listDateUnit->setVisible(m_condition == ICriteria::CondDateInLast || m_condition == ICriteria::CondDateNotInLast);
    m_uiWidget->lblBetween->setVisible(m_condition == ICriteria::CondDateBetween);

    m_uiWidget->listFormat->hide();
    m_uiWidget->listLanguage->hide();
    m_uiWidget->listPlayList->hide();
}

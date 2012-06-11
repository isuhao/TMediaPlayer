
#ifndef FILE_C_WIDGET_CRITERIA
#define FILE_C_WIDGET_CRITERIA

#include "IWidgetCriteria.hpp"
#include "ui_WidgetCriteria.h"


/**
 * Widget permettant d'éditer un critère d'une liste dynamique.
 */

class CWidgetCriteria : public IWidgetCriteria
{
    Q_OBJECT

    friend class CCriteria;

public:

    explicit CWidgetCriteria(CApplication * application, QWidget * parent = NULL);
    virtual ~CWidgetCriteria();

    virtual ICriteria * getCriteria(void);

protected slots:

    void changeType(int num);
    void changeConditionBoolean(int num);
    void changeConditionString(int num);
    void changeConditionNumber(int num);
    void changeConditionTime(int num);
    void changeConditionDate(int num);

private:

    Ui::WidgetCriteria * m_uiWidget;
};

#endif // FILE_C_WIDGET_CRITERIA


#ifndef FILE_C_WIDGET_CRITERIA
#define FILE_C_WIDGET_CRITERIA

#include "IWidgetCriteria.hpp"
#include "ui_WidgetCriteria.h"


class CWidgetCriteria : public IWidgetCriteria
{
    Q_OBJECT

public:

    explicit CWidgetCriteria(QWidget * parent = NULL);
    ~CWidgetCriteria();

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

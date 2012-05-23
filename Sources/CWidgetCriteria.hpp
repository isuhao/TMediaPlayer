
#ifndef FILE_CWIDGETCRITERIA
#define FILE_CWIDGETCRITERIA

#include "IWidgetCriteria.hpp"
#include "ui_WidgetCriteria.h"


class CWidgetCriteria : public IWidgetCriteria
{
    Q_OBJECT

public:

    CWidgetCriteria(QWidget * parent = NULL);
    ~CWidgetCriteria();

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

#endif // FILE_CWIDGETCRITERIA

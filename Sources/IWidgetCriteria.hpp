
#ifndef FILE_I_WIDGET_CRITERIA
#define FILE_I_WIDGET_CRITERIA

#include <QWidget>
#include "ICriteria.hpp"


class IWidgetCriteria : public QWidget
{
    Q_OBJECT

public:

    explicit IWidgetCriteria(QWidget * parent = NULL);
    ~IWidgetCriteria();

    virtual ICriteria * getCriteria(void) = 0;
    
protected:

    ICriteria::TType m_type;
    ICriteria::TCondition m_condition;
};

#endif // FILE_I_WIDGET_CRITERIA


#ifndef FILE_I_WIDGET_CRITERIA
#define FILE_I_WIDGET_CRITERIA

#include <QWidget>
#include "ICriteria.hpp"


class ICriteria;


class IWidgetCriteria : public QWidget
{
    Q_OBJECT

public:

    IWidgetCriteria(QWidget * parent = NULL);
    ~IWidgetCriteria();

    inline ICriteria * getCriteria(void) const;
    
protected:

    ICriteria::TType m_type;
    ICriteria::TCondition m_condition;

private:

    ICriteria * m_criteria;
};


inline ICriteria * IWidgetCriteria::getCriteria(void) const
{
    return m_criteria;
}

#endif // FILE_I_WIDGET_CRITERIA


#include "IWidgetCriteria.hpp"


IWidgetCriteria::IWidgetCriteria(QWidget * parent) :
    QWidget     (parent),
    m_type      (ICriteria::TypeInvalid),
    m_condition (ICriteria::CondInvalid),
    m_criteria  (NULL)
{

}


IWidgetCriteria::~IWidgetCriteria()
{

}

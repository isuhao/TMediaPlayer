
#include "IWidgetCriteria.hpp"


IWidgetCriteria::IWidgetCriteria(CApplication * application, QWidget * parent) :
    QWidget       (parent),
    m_type        (ICriteria::TypeInvalid),
    m_condition   (ICriteria::CondInvalid),
    m_application (application)
{
    Q_CHECK_PTR(application);
}


IWidgetCriteria::~IWidgetCriteria()
{

}

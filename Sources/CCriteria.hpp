
#ifndef FILE_C_CRITERIA
#define FILE_C_CRITERIA

#include "ICriteria.hpp"


/**
 * Critère simple.
 */

class CCriteria : public ICriteria
{
    Q_OBJECT

public:

    explicit CCriteria(QObject * parent = NULL);
    ~CCriteria();

    virtual bool matchCriteria(CSong * song) const;
    virtual IWidgetCriteria * getWidget(void) const;
};

#endif // FILE_C_CRITERIA

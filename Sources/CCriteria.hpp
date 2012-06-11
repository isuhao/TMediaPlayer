
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

    explicit CCriteria(CApplication * application, QObject * parent = NULL);
    virtual ~CCriteria();

    virtual bool matchCriteria(CSong * song) const;
    virtual QList<CSong *> getSongs(const QList<CSong *>& from, const QList<CSong *>& with = QList<CSong *>()) const;
    virtual TUpdateConditions getUpdateConditions(void) const;
    virtual IWidgetCriteria * getWidget(void) const;
};

#endif // FILE_C_CRITERIA

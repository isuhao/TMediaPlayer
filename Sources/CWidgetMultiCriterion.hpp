
#ifndef FILE_C_WIDGET_MULTI_CRITERION
#define FILE_C_WIDGET_MULTI_CRITERION

#include <QList>
#include <QMap>
#include "IWidgetCriteria.hpp"
#include "ui_WidgetMultiCriterion.h"


class CMultiCriterion;
class QPushButton;


class CWidgetMultiCriterion : public IWidgetCriteria
{
    Q_OBJECT

public:

    explicit CWidgetMultiCriterion(QWidget * parent = NULL);
    ~CWidgetMultiCriterion();

    virtual ICriteria * getCriteria(void);

protected slots:

    void addCriteria(void);
    void addMultiCriterion(void);
    void removeCriteria(int row);
    void removeCriteriaFromButton(void);

private:

    void addCriteria(IWidgetCriteria * criteriaWidget);

    Ui::WidgetMultiCriterion * m_uiWidget;
    QList<IWidgetCriteria *> m_children;
    QMap<QPushButton *, int> m_btnRemove;
};

#endif // FILE_C_WIDGET_MULTI_CRITERION

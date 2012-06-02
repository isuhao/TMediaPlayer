
#ifndef FILE_C_WIDGET_MULTI_CRITERION
#define FILE_C_WIDGET_MULTI_CRITERION

#include "IWidgetCriteria.hpp"
#include "CMultiCriterion.hpp" // Si on ne l'inclut pas, l'intellisense bug...
#include "ui_WidgetMultiCriterion.h"
#include <QList>
#include <QMap>


class CMultiCriterion;
class QPushButton;


/**
 * Widget permettant d'éditer un sous-critère d'une liste dynamique.
 */

class CWidgetMultiCriterion : public IWidgetCriteria
{
    Q_OBJECT

    friend class CMultiCriterion;

public:

    explicit CWidgetMultiCriterion(QWidget * parent = NULL);
    virtual ~CWidgetMultiCriterion();

    virtual ICriteria * getCriteria(void);

protected slots:

    void setMultiCriterionType(int type);
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

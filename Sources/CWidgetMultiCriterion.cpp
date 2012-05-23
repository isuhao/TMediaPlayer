
#include "CWidgetMultiCriterion.hpp"
#include "CWidgetCriteria.hpp"
#include <QPushButton>


CWidgetMultiCriterion::CWidgetMultiCriterion(QWidget * parent) :
    IWidgetCriteria (parent),
    m_uiWidget      (new Ui::WidgetMultiCriterion())
{
    m_type = ICriteria::TypeMultiCriterion;
    m_uiWidget->setupUi(this);

    addCriteria();

    connect(m_uiWidget->btnAddCriteria, SIGNAL(clicked()), this, SLOT(addCriteria()));
    connect(m_uiWidget->btnAddMultiCriterion, SIGNAL(clicked()), this, SLOT(addMultiCriterion()));
}


CWidgetMultiCriterion::~CWidgetMultiCriterion()
{
    delete m_uiWidget;
}


void CWidgetMultiCriterion::addCriteria(void)
{
    addCriteria(new CWidgetCriteria(this));
}


void CWidgetMultiCriterion::addMultiCriterion(void)
{
    addCriteria(new CWidgetMultiCriterion(this));
}


void CWidgetMultiCriterion::removeCriteria(int row)
{
    Q_ASSERT(row >= 0 && row < m_uiWidget->layoutChildren->rowCount());

    // Suppression du widget
    QLayoutItem * itemWidget = m_uiWidget->layoutChildren->itemAtPosition(row, 0);
    if (!itemWidget) return;
    IWidgetCriteria * child = qobject_cast<IWidgetCriteria *>(itemWidget->widget());
    if (!child) return;

    m_uiWidget->layoutChildren->removeItem(itemWidget);
    delete itemWidget;
    m_children.removeOne(child);
    delete child;

    // Suppression du bouton
    QLayoutItem * itemButton = m_uiWidget->layoutChildren->itemAtPosition(row, 1);
    if (!itemButton) return;
    QPushButton * btnRemove = qobject_cast<QPushButton *>(itemButton->widget());
    if (!btnRemove) return;

    m_uiWidget->layoutChildren->removeItem(itemButton);
    delete itemButton;
    delete btnRemove;
    
    m_btnRemove.remove(btnRemove);
}


void CWidgetMultiCriterion::removeCriteriaFromButton(void)
{
    QPushButton * btnRemove = qobject_cast<QPushButton *>(sender());

    if (btnRemove && m_btnRemove.contains(btnRemove))
    {
        removeCriteria(m_btnRemove.value(btnRemove));
    }
}


void CWidgetMultiCriterion::addCriteria(IWidgetCriteria * criteriaWidget)
{
    Q_CHECK_PTR(criteriaWidget);

    const int row = m_uiWidget->layoutChildren->rowCount();

    m_children.append(criteriaWidget);
    m_uiWidget->layoutChildren->addWidget(criteriaWidget, row, 0);

    QPushButton * btnRemove = new QPushButton(tr("-"));
    btnRemove->setMaximumWidth(30);
    m_uiWidget->layoutChildren->addWidget(btnRemove, row, 1, Qt::AlignTop);

    m_btnRemove[btnRemove] = row;
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeCriteriaFromButton()));
}

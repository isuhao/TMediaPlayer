
#include "CWidgetMultiCriterion.hpp"
#include "CWidgetCriteria.hpp"
#include "CMultiCriterion.hpp"
#include "CDialogEditDynamicList.hpp"
#include <QPushButton>

#include <QDebug>


/**
 * Construit le widget.
 * Un sous-crit�re simple est ajout�.
 *
 * \param parent Widget parent.
 */

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


/**
 * D�truit le widget.
 */

CWidgetMultiCriterion::~CWidgetMultiCriterion()
{
    qDebug() << "CWidgetMultiCriterion::~CWidgetMultiCriterion()";
    delete m_uiWidget;
}


/**
 * Retourne le crit�re d�finis par le widget.
 *
 * \return Pointeur sur le crit�re.
 */

ICriteria * CWidgetMultiCriterion::getCriteria(void)
{
    qDebug() << "CWidgetMultiCriterion::getCriteria()";
    CMultiCriterion * criteria = new CMultiCriterion(this);
    criteria->setMultiCriterionType(CMultiCriterion::getMultiCriterionTypeFromInteger(m_uiWidget->listUnion->currentIndex()));

    foreach (IWidgetCriteria * child, m_children)
    {
        criteria->addChild(child->getCriteria());
    }

    return criteria;
}


/**
 * Ajoute un sous-crit�re.
 */

void CWidgetMultiCriterion::addCriteria(void)
{
    addCriteria(new CWidgetCriteria(this));
}


/**
 * Ajoute un sous-crit�re de type multi-crit�res.
 */

void CWidgetMultiCriterion::addMultiCriterion(void)
{
    addCriteria(new CWidgetMultiCriterion(this));
}


/**
 * Enl�ve un sous-crit�re.
 *
 * \param row Num�ro de la ligne � enlever.
 */

void CWidgetMultiCriterion::removeCriteria(int row)
{
    Q_ASSERT(row >= 0 && row < m_uiWidget->layoutChildren->rowCount());

    // On garde au moins un crit�re
    if (m_children.size() <= 1)
    {
        return;
    }

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

    CDialogEditDynamicList * dialogList = qobject_cast<CDialogEditDynamicList *>(window());

    if (dialogList)
    {
        dialogList->resizeWindow();
    }
}


/**
 * M�thode appel�e lorsqu'on clique sur un bouton pour supprimer un sous-crit�re.
 * Le num�ro de la ligne est d�termin�e � partir du widget ayant envoy� le signal.
 */

void CWidgetMultiCriterion::removeCriteriaFromButton(void)
{
    QPushButton * btnRemove = qobject_cast<QPushButton *>(sender());

    if (btnRemove && m_btnRemove.contains(btnRemove))
    {
        removeCriteria(m_btnRemove.value(btnRemove));
    }
}


/**
 * Ajoute un sous-crit�re au widget.
 *
 * \param criteriaWidget Widget du sous-crit�re � ajouter.
 */

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

    CDialogEditDynamicList * dialogList = qobject_cast<CDialogEditDynamicList *>(window());

    if (dialogList)
    {
        dialogList->resizeWindow();
    }
}

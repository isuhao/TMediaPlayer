/*
Copyright (C) 2012 Teddy Michel

This file is part of TMediaPlayer.

TMediaPlayer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TMediaPlayer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TMediaPlayer. If not, see <http://www.gnu.org/licenses/>.
*/

#include "CWidgetMultiCriterion.hpp"
#include "CWidgetCriteria.hpp"
#include "CMultiCriterion.hpp"
#include "Dialog/CDialogEditDynamicList.hpp"
#include <QPushButton>

#include <QDebug>


/**
 * Construit le widget.
 * Un sous-critère simple est ajouté.
 *
 * \param parent Widget parent.
 */

CWidgetMultiCriterion::CWidgetMultiCriterion(CApplication * application, QWidget * parent) :
    IWidgetCriteria (application, parent),
    m_uiWidget      (new Ui::WidgetMultiCriterion())
{
    m_type = ICriteria::TypeUnion;
    m_uiWidget->setupUi(this);

    addCriteria();

    connect(m_uiWidget->btnAddCriteria, SIGNAL(clicked()), this, SLOT(addCriteria()));
    connect(m_uiWidget->btnAddMultiCriterion, SIGNAL(clicked()), this, SLOT(addMultiCriterion()));
}


/**
 * Détruit le widget.
 */

CWidgetMultiCriterion::~CWidgetMultiCriterion()
{
    //qDebug() << "CWidgetMultiCriterion::~CWidgetMultiCriterion()";
    delete m_uiWidget;
}


/**
 * Retourne le critère définis par le widget.
 *
 * \return Pointeur sur le critère.
 */

ICriteria * CWidgetMultiCriterion::getCriteria(void)
{
    qDebug() << "CWidgetMultiCriterion::getCriteria()";
    CMultiCriterion * criteria = new CMultiCriterion(m_application, this);
    criteria->setMultiCriterionType(CMultiCriterion::getMultiCriterionTypeFromInteger(m_uiWidget->listUnion->currentIndex()));

    foreach (IWidgetCriteria * child, m_children)
    {
        criteria->addChild(child->getCriteria());
    }

    return criteria;
}


void CWidgetMultiCriterion::setMultiCriterionType(int type)
{
    m_uiWidget->listUnion->setCurrentIndex(type - 1);
}


/**
 * Ajoute un sous-critère.
 */

void CWidgetMultiCriterion::addCriteria(void)
{
    addCriteria(new CWidgetCriteria(m_application, this));
}


/**
 * Ajoute un sous-critère de type multi-critères.
 */

void CWidgetMultiCriterion::addMultiCriterion(void)
{
    addCriteria(new CWidgetMultiCriterion(m_application, this));
}


/**
 * Enlève un sous-critère.
 *
 * \param row Numéro de la ligne à enlever.
 */

void CWidgetMultiCriterion::removeCriteria(int row)
{
    Q_ASSERT(row >= 0 && row < m_uiWidget->layoutChildren->rowCount());

    // On garde au moins un critère
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
 * Méthode appelée lorsqu'on clique sur un bouton pour supprimer un sous-critère.
 * Le numéro de la ligne est déterminée à partir du widget ayant envoyé le signal.
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
 * Ajoute un sous-critère au widget.
 *
 * \param criteriaWidget Widget du sous-critère à ajouter.
 */

void CWidgetMultiCriterion::addCriteria(IWidgetCriteria * criteriaWidget)
{
    Q_CHECK_PTR(criteriaWidget);

    criteriaWidget->setParent(this);

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

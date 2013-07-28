/*
Copyright (C) 2012-2013 Teddy Michel

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

#include "CWidgetMultiCriteria.hpp"
#include "CWidgetCriterion.hpp"
#include "CMultiCriteria.hpp"
#include "Dialog/CDialogEditDynamicList.hpp"
#include <QPushButton>

#include <QDebug>


/**
 * Construit le widget.
 * Un sous-critère simple est ajouté.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 * \param parent     Widget parent.
 */

CWidgetMultiCriteria::CWidgetMultiCriteria(CMainWindow * mainWindow, QWidget * parent) :
IWidgetCriterion (mainWindow, parent),
m_uiWidget       (new Ui::WidgetMultiCriteria())
{
    m_type = ICriterion::TypeUnion;
    m_uiWidget->setupUi(this);

    addCriterion();

    connect(m_uiWidget->btnAddCriterion, SIGNAL(clicked()), this, SLOT(addCriterion()));
    connect(m_uiWidget->btnAddMultiCriteria, SIGNAL(clicked()), this, SLOT(addMultiCriteria()));
}


/**
 * Détruit le widget.
 */

CWidgetMultiCriteria::~CWidgetMultiCriteria()
{
    delete m_uiWidget;
}


/**
 * Retourne le critère définis par le widget.
 *
 * \return Pointeur sur le critère.
 */

ICriterion * CWidgetMultiCriteria::getCriterion()
{
    CMultiCriteria * criteria = new CMultiCriteria(m_mainWindow, this);
    criteria->setMultiCriteriaType(CMultiCriteria::getMultiCriteriaTypeFromInteger(m_uiWidget->listUnion->currentIndex()));

    for (QList<IWidgetCriterion *>::ConstIterator child = m_children.begin(); child != m_children.end(); ++child)
    {
        criteria->addChild((*child)->getCriterion());
    }

    return criteria;
}


void CWidgetMultiCriteria::setMultiCriteriaType(int type)
{
    m_uiWidget->listUnion->setCurrentIndex(type - 1);
}


/**
 * Ajoute un sous-critère.
 */

void CWidgetMultiCriteria::addCriterion()
{
    addCriterion(new CWidgetCriterion(m_mainWindow, this));
}


/**
 * Ajoute un sous-critère de type multi-critères.
 */

void CWidgetMultiCriteria::addMultiCriteria()
{
    addCriterion(new CWidgetMultiCriteria(m_mainWindow, this));
}


/**
 * Enlève un sous-critère.
 *
 * \param row Numéro de la ligne à enlever.
 */

void CWidgetMultiCriteria::removeCriterion(int row)
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
    IWidgetCriterion * child = qobject_cast<IWidgetCriterion *>(itemWidget->widget());
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

void CWidgetMultiCriteria::removeCriterionFromButton()
{
    QPushButton * btnRemove = qobject_cast<QPushButton *>(sender());

    if (btnRemove && m_btnRemove.contains(btnRemove))
    {
        removeCriterion(m_btnRemove.value(btnRemove));
    }
}


/**
 * Ajoute un sous-critère au widget.
 *
 * \param criteriaWidget Widget du sous-critère à ajouter.
 */

void CWidgetMultiCriteria::addCriterion(IWidgetCriterion * criteriaWidget)
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
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeCriterionFromButton()));

    CDialogEditDynamicList * dialogList = qobject_cast<CDialogEditDynamicList *>(window());

    if (dialogList)
    {
        dialogList->resizeWindow();
    }
}

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

#include "CDialogEditDynamicList.hpp"
#include "../CDynamicList.hpp"
#include "../CWidgetMultiCriteria.hpp"
#include "../CMainWindow.hpp"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QPushButton>


/**
 * Constructeur de la boite de dialogue d'édition des listes de lecture dynamiques.
 *
 * \param playList    Pointeur sur la liste à modifier, ou nullptr pour une nouvelle liste.
 * \param application Pointeur sur l'application.
 * \param folder      Dossier contenant la liste de lecture.
 */

CDialogEditDynamicList::CDialogEditDynamicList(CDynamicList * playList, CMainWindow * application, CFolder * folder) :
QDialog          (application),
m_uiWidget       (new Ui::DialogEditDynamicPlayList()),
m_widgetCriteria (nullptr),
m_playList       (playList),
m_application    (application),
m_folder         (folder)
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    if (m_playList)
    {
        m_widgetCriteria = m_playList->getWidget();

        if (m_widgetCriteria)
        {
            m_uiWidget->verticalLayout->insertWidget(1, m_widgetCriteria);
            m_widgetCriteria->setParent(this);
        }
        else
        {
            m_widgetCriteria = new CWidgetMultiCriteria(m_application, this);
            m_uiWidget->verticalLayout->insertWidget(1, m_widgetCriteria);
        }
    }
    else
    {
        m_playList = new CDynamicList(m_application);

        m_widgetCriteria = new CWidgetMultiCriteria(m_application, this);
        m_uiWidget->verticalLayout->insertWidget(1, m_widgetCriteria);
    }

    m_uiWidget->editName->setText(m_playList->getName());
    m_uiWidget->editLimit->setChecked(m_playList->getNumItems() > 0);
    m_uiWidget->editNumItems->setValue(m_playList->getNumItems());
    m_uiWidget->editOnlyChecked->setChecked(m_playList->getOnlyChecked());
    m_uiWidget->editUpdate->setChecked(m_playList->isAutoUpdate());

    resizeWindow();

    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Détruit le widget.
 */

CDialogEditDynamicList::~CDialogEditDynamicList()
{
    delete m_uiWidget;
}


/**
 * Redimensionne la boite de dialogue à chaque ajout ou suppression de critère.
 */

void CDialogEditDynamicList::resizeWindow()
{
    setMinimumSize(0, 0);
    resize(size().width(), 1);
}


/**
 * Enregistre les paramètres de la liste de lecture dynamique.
 */

void CDialogEditDynamicList::save()
{
    QString name = m_uiWidget->editName->text();

    if (name.isEmpty())
    {
        QMessageBox::warning(this, QString(), tr("You need to choose a name for the playlist."));
        return;
    }

    m_playList->setName(name);
    m_playList->setFolder(m_folder);
    m_playList->setCriterion(m_widgetCriteria->getCriterion());
    m_playList->setAutoUpdate(m_uiWidget->editUpdate->isChecked());
    m_playList->setOnlyChecked(m_uiWidget->editOnlyChecked->isChecked());

    if (!m_playList->updateDatabase())
    {
        return;
    }

    m_application->addPlayList(m_playList);
    m_playList->updateList();

    close();
}

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

#include "CDialogNotifications.hpp"
#include "CApplication.hpp"
#include <QPushButton>


CDialogNotifications::CDialogNotifications(CApplication * application) :
    QDialog       (application),
    m_uiWidget    (new Ui::DialogNotifications()),
    m_application (application)
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    m_uiWidget->table->setSortingEnabled(false);

    // Remplissage du tableau
    QList<CApplication::TNotification> notifications = m_application->getNotifications();
    m_uiWidget->table->setRowCount(notifications.size());

    for (int row = 0; row < notifications.size(); ++row)
    {
        QTableWidgetItem * item;
        
        item = new QTableWidgetItem(notifications.at(row).date.toString("dd/MM/yyyy hh:mm:ss"));
        m_uiWidget->table->setItem(row, 0, item);
        
        item = new QTableWidgetItem(notifications.at(row).message);
        m_uiWidget->table->setItem(row, 1, item);
    }

    m_uiWidget->table->setSortingEnabled(true);
    m_uiWidget->table->sortByColumn(0, Qt::DescendingOrder);
    
    // Connexions des signaux des boutons
    QPushButton * btnClose = m_uiWidget->buttonBox->addButton(tr("Close"), QDialogButtonBox::AcceptRole);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
}


CDialogNotifications::~CDialogNotifications()
{
    delete m_uiWidget;
}

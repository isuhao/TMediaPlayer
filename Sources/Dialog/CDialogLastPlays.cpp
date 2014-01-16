/*
Copyright (C) 2012-2014 Teddy Michel

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

#include "CDialogLastPlays.hpp"
#include "../CMainWindow.hpp"
#include "../CMediaManager.hpp"
#include "../CSong.hpp"
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>


/**
 * Constructeur de la boite de dialogue.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 */

CDialogLastPlays::CDialogLastPlays(CMainWindow * mainWindow) :
QDialog      (mainWindow),
m_uiWidget   (new Ui::DialogLastPlays()),
m_mainWindow (mainWindow)
{
    Q_CHECK_PTR(m_mainWindow);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    m_uiWidget->table->setSortingEnabled(false);

    QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());

    if (!query.exec("SELECT song_id, play_time_utc FROM play WHERE play_time_utc IS NOT NULL ORDER BY play_time_utc DESC"))
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        // Remplissage du tableau
        for (int row = 0; row < 100 && query.next(); ++row)
        {
            m_uiWidget->table->setRowCount(row + 1);

            const int songId = query.value(0).toInt();
            const CSong * song = m_mainWindow->getSongFromId(songId);

            if (!song)
                continue;

            QDateTime playTimeUTC = query.value(1).toDateTime();
            playTimeUTC.setTimeSpec(Qt::UTC);

            QTableWidgetItem * item;

            item = new QTableWidgetItem(playTimeUTC.toLocalTime().toString("dd/MM/yyyy HH:mm:ss"));
            m_uiWidget->table->setItem(row, 0, item);

            item = new QTableWidgetItem(song->getTitle());
            m_uiWidget->table->setItem(row, 1, item);

            item = new QTableWidgetItem(song->getArtistName());
            m_uiWidget->table->setItem(row, 2, item);

            item = new QTableWidgetItem(song->getAlbumTitle());
            m_uiWidget->table->setItem(row, 3, item);
        }
    }

    m_uiWidget->table->setSortingEnabled(true);
    m_uiWidget->table->sortByColumn(0, Qt::DescendingOrder);

    // Connexions des signaux des boutons
    QPushButton * btnClose = m_uiWidget->buttonBox->addButton(tr("Close"), QDialogButtonBox::AcceptRole);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Détruit la boite de dialogue.
 */

CDialogLastPlays::~CDialogLastPlays()
{
    delete m_uiWidget;
}


void CDialogLastPlays::closeEvent(QCloseEvent * event)
{
    emit closed();
}


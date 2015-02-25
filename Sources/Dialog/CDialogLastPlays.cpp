/*
Copyright (C) 2012-2015 Teddy Michel

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
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
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
m_mainWindow (mainWindow),
m_model      (nullptr)
{
    Q_CHECK_PTR(m_mainWindow);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    // Connexions des signaux des boutons
    QPushButton * btnClose = m_uiWidget->buttonBox->addButton(tr("Close"), QDialogButtonBox::AcceptRole);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(QStringList() << /*tr("Num") <<*/ tr("Time") << tr("Title") << tr("Artist") << tr("Album"));

    QSortFilterProxyModel * proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_model);
    proxyModel->setSortRole(Qt::UserRole + 2);
    proxyModel->setDynamicSortFilter(true);
    m_uiWidget->table->setModel(proxyModel);
    m_uiWidget->table->sortByColumn(0, Qt::DescendingOrder);

    resetList();
    m_uiWidget->table->resizeColumnsToContents();
    m_uiWidget->table->setAlternatingRowColors(true);
    m_uiWidget->table->setShowGrid(false);

    connect(m_uiWidget->editMaxPlays, SIGNAL(valueChanged(int)), this, SLOT(changeMaxPlays(int)));
    connect(m_mainWindow, SIGNAL(songPlayEnd(CSong *)), this, SLOT(resetList()));
}


/**
 * Détruit la boite de dialogue.
 */

CDialogLastPlays::~CDialogLastPlays()
{
    delete m_uiWidget;
}


void CDialogLastPlays::changeMaxPlays(int maxPlays)
{
    m_uiWidget->editMaxPlays->setValue(maxPlays);
    resetList();
}


void CDialogLastPlays::resetList()
{
    int colWidth0 = m_uiWidget->table->columnWidth(0);
    int colWidth1 = m_uiWidget->table->columnWidth(1);
    int colWidth2 = m_uiWidget->table->columnWidth(2);
    int colWidth3 = m_uiWidget->table->columnWidth(3);
    //int colWidth4 = m_uiWidget->table->columnWidth(4);

    m_model->removeRows(0, m_model->rowCount());

    QSqlQuery query(m_mainWindow->getMediaManager()->getDataBase());

    if (!query.exec("SELECT song_id, play_time_utc, play_id FROM play WHERE play_time_utc IS NOT NULL ORDER BY play_time_utc DESC"))
    {
        m_mainWindow->getMediaManager()->logDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
    }
    else
    {
        // Remplissage du tableau
        for (int row = 0; row < m_uiWidget->editMaxPlays->value() && query.next(); ++row)
        {
            const int songId = query.value(0).toInt();
            const int playId = query.value(2).toInt();
            const CSong * song = m_mainWindow->getSongFromId(songId);

            if (song == nullptr)
            {
                continue;
            }

            QDateTime playTimeUTC = query.value(1).toDateTime();
            playTimeUTC.setTimeSpec(Qt::UTC);

            QList<QStandardItem *> itemList;

            //QStandardItem * itemNum = new QStandardItem(row + 1);
            QStandardItem * itemTime = new QStandardItem(playTimeUTC.toLocalTime().toString("dd/MM/yyyy HH:mm:ss"));
            QStandardItem * itemTitle = new QStandardItem(song->getTitle());
            QStandardItem * itemArtist = new QStandardItem(song->getArtistName());
            QStandardItem * itemAlbum = new QStandardItem(song->getAlbumTitle());

            //itemNum->setData(playId, Qt::UserRole + 1);
            itemTime->setData(playId, Qt::UserRole + 1);
            itemTitle->setData(playId, Qt::UserRole + 1);
            itemArtist->setData(playId, Qt::UserRole + 1);
            itemAlbum->setData(playId, Qt::UserRole + 1);

            //itemNum->setData(row, Qt::UserRole + 2);
            itemTime->setData(playTimeUTC, Qt::UserRole + 2);
            itemTitle->setData(song->getTitleSort(false), Qt::UserRole + 2);
            itemArtist->setData(song->getArtistNameSort(false), Qt::UserRole + 2);
            itemAlbum->setData(song->getAlbumTitleSort(false), Qt::UserRole + 2);

            //itemList.append(itemNum);
            itemList.append(itemTime);
            itemList.append(itemTitle);
            itemList.append(itemArtist);
            itemList.append(itemAlbum);

            m_model->appendRow(itemList);
        }
    }

    m_uiWidget->table->setColumnWidth(0, colWidth0 > 20 ? colWidth0 : 20);
    m_uiWidget->table->setColumnWidth(1, colWidth1 > 20 ? colWidth1 : 20);
    m_uiWidget->table->setColumnWidth(2, colWidth2 > 20 ? colWidth2 : 20);
    m_uiWidget->table->setColumnWidth(3, colWidth3 > 20 ? colWidth3 : 20);
    //m_uiWidget->table->setColumnWidth(4, colWidth4 > 20 ? colWidth4 : 20);
}


void CDialogLastPlays::closeEvent(QCloseEvent * event)
{
    emit closed();
}


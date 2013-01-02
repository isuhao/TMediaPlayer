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

#include "CDialogEditStaticPlayList.hpp"
#include "../CStaticPlayList.hpp"
#include "../CApplication.hpp"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QPushButton>

#include <QtDebug>


CDialogEditStaticPlayList::CDialogEditStaticPlayList(CStaticPlayList * playList, CApplication * application, CFolder * folder, const QList<CSong *>& songs) :
    QDialog       (application),
    m_uiWidget    (new Ui::DialogEditStaticPlayList()),
    m_playList    (playList),
    m_application (application),
    m_folder      (folder),
    m_songs       (songs)
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    if (!m_playList)
    {
        m_playList = new CStaticPlayList(m_application);
    }

    m_uiWidget->editName->setText(m_playList->getName());


    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Détruit le widget.
 */

CDialogEditStaticPlayList::~CDialogEditStaticPlayList()
{
    delete m_uiWidget;
}


/**
 * Enregistre les modifications.
 *
 * \todo Ajouter les morceaux à la liste.
 */

void CDialogEditStaticPlayList::save()
{
    QString name = m_uiWidget->editName->text();

    if (name.isEmpty())
    {
        QMessageBox::warning(this, QString(), tr("You need to choose a name for the playlist."));
        return;
    }

    m_playList->setName(name);
    m_playList->setFolder(m_folder);

    if (!m_playList->updateDatabase())
    {
        return;
    }

    m_application->addPlayList(m_playList);

    // Ajout des morceaux à la liste
    m_playList->addSongs(m_songs, false);

    close();
}

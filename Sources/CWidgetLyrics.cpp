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

#include "CWidgetLyrics.hpp"
#include "CMainWindow.hpp"
#include "CSong.hpp"
#include "CLyricWiki.hpp"

#include <QTextEdit>
#include <QPushButton>
#include <QGridLayout>


CWidgetLyrics::CWidgetLyrics(CMainWindow * mainWindow) :
QWidget      (mainWindow),
m_mainWindow (mainWindow),
m_song       (nullptr)
{
    Q_CHECK_PTR(m_mainWindow);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setDisabled(true);

    m_buttonFind = new QPushButton(tr("Find"));
    m_buttonEdit = new QPushButton(tr("Edit"));
    m_buttonSave = new QPushButton(tr("Save"));
    m_buttonCancel = new QPushButton(tr("Cancel"));

    connect(m_buttonFind, SIGNAL(clicked()), this, SLOT(findLyrics()));
    connect(m_buttonEdit, SIGNAL(clicked()), this, SLOT(editLyrics()));
    connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(saveLyrics()));
    connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(cancelEdit()));

    m_layout = new QGridLayout(this);
    setLayout(m_layout);
    m_layout->setMargin(0);

    m_layout->addWidget(m_textEdit, 0, 0, 1, 2);

    m_layout->addWidget(m_buttonFind, 1, 0);
    m_layout->addWidget(m_buttonEdit, 1, 1);

    m_layout->addWidget(m_buttonSave, 2, 0);
    m_layout->addWidget(m_buttonCancel, 2, 1);

    m_buttonSave->hide();
    m_buttonCancel->hide();

    m_buttonFind->show();
    m_buttonEdit->show();

    m_buttonFind->setDisabled(true);
    m_buttonEdit->setDisabled(true);
}


CWidgetLyrics::~CWidgetLyrics()
{
    delete m_buttonFind;
    delete m_buttonEdit;
    delete m_buttonSave;
    delete m_buttonCancel;
}


void CWidgetLyrics::setSong(CSong * song)
{
    if (m_song != song)
    {
        m_song = song;
        cancelEdit();

        m_buttonFind->setText(tr("Find"));

        if (m_song)
        {
            m_textEdit->setDisabled(false);
            m_buttonFind->setDisabled(false);
            m_buttonEdit->setDisabled(false);
        }
        else
        {
            m_textEdit->setDisabled(true);
            m_buttonFind->setDisabled(true);
            m_buttonEdit->setDisabled(true);
        }
    }
}


/**
 * Recherche les paroles du morceau sur Internet.
 */

void CWidgetLyrics::findLyrics()
{
    if (m_song)
    {
        CLyricWiki * query = new CLyricWiki(m_mainWindow->getMediaManager(), m_song);
        connect(query, SIGNAL(lyricsFound(const QString&)), this, SLOT(onLyricsFound(const QString&)));
        connect(query, SIGNAL(lyricsNotFound()), this, SLOT(onLyricsNotFound()));

        m_buttonFind->setText(tr("Finding lyrics..."));
        m_buttonFind->setEnabled(false);
    }
}


void CWidgetLyrics::editLyrics()
{
    m_textEdit->setReadOnly(false);

    m_buttonFind->hide();
    m_buttonEdit->hide();

    m_buttonSave->show();
    m_buttonCancel->show();
}


void CWidgetLyrics::saveLyrics()
{
    if (m_song)
    {
        m_song->setLyrics(m_textEdit->toPlainText());
        m_song->writeTags();
        m_song->updateDatabase();
    }

    m_textEdit->setReadOnly(true);

    m_buttonSave->hide();
    m_buttonCancel->hide();

    m_buttonFind->show();
    m_buttonEdit->show();
}


void CWidgetLyrics::cancelEdit()
{
    m_textEdit->setText(m_song ? m_song->getLyrics() : QString());
    m_textEdit->setReadOnly(true);

    m_buttonSave->hide();
    m_buttonCancel->hide();

    m_buttonFind->show();
    m_buttonEdit->show();
}


void CWidgetLyrics::onLyricsFound(const QString& lyrics)
{
    CLyricWiki * query = qobject_cast<CLyricWiki *>(sender());

    if (query)
    {
        // On vérifie que le morceau n'a pas changé entre temps
        if (query->getSong() == m_song)
        {
            m_song->setLyrics(lyrics);
            m_textEdit->setText(lyrics);

            m_song->writeTags();
            m_song->updateDatabase();

            m_buttonFind->setText(tr("Find"));
            m_buttonFind->setEnabled(true);
        }

        query->deleteLater();
    }
}


void CWidgetLyrics::onLyricsNotFound()
{
    CLyricWiki * query = qobject_cast<CLyricWiki *>(sender());

    if (query)
    {
        // On vérifie que le morceau n'a pas changé entre temps
        if (query->getSong() == m_song)
        {
            m_buttonFind->setText(tr("Find"));
            m_buttonFind->setEnabled(true);
        }

        query->deleteLater();
    }
}

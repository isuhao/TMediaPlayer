
#include "CWidgetLyrics.hpp"
#include "CApplication.hpp"
#include "CSong.hpp"
#include "CLyricWiki.hpp"

#include <QTextEdit>
#include <QPushButton>
#include <QGridLayout>


CWidgetLyrics::CWidgetLyrics(CApplication * application) :
    QWidget       (application),
    m_application (application),
    m_song        (NULL)
{
    Q_CHECK_PTR(application);

    m_textEdit = new QTextEdit();

    m_buttonFind = new QPushButton(tr("Find"));
    m_buttonEdit = new QPushButton(tr("Edit"));
    m_buttonSave = new QPushButton(tr("Save"));
    m_buttonCancel = new QPushButton(tr("Cancel"));

    connect(m_buttonFind, SIGNAL(clicked()), this, SLOT(findLyrics()));
    connect(m_buttonEdit, SIGNAL(clicked()), this, SLOT(editLyrics()));
    connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(saveLyrics()));
    connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(cancelEdit()));

    m_layout = new QGridLayout();
    setLayout(m_layout);
    m_layout->setMargin(0);

    m_layout->addWidget(m_textEdit, 0, 0, 1, 2);

    m_layout->addWidget(m_buttonFind, 1, 0);
    m_layout->addWidget(m_buttonEdit, 1, 1);

    m_layout->addWidget(m_buttonSave, 2, 0);
    m_layout->addWidget(m_buttonCancel, 2, 1);
}


void CWidgetLyrics::setSong(CSong * song)
{
    m_song = song;
    cancelEdit();

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

    
void CWidgetLyrics::findLyrics(void)
{
    if (m_song)
    {
        CLyricWiki * query = new CLyricWiki(m_application, m_song);
        connect(query, SIGNAL(lyricsFound(const QString&)), this, SLOT(onLyricsFound(const QString&)));
    }
}


void CWidgetLyrics::editLyrics(void)
{
    m_textEdit->setReadOnly(false);

    m_buttonFind->hide();
    m_buttonEdit->hide();

    m_buttonSave->show();
    m_buttonCancel->show();
}


void CWidgetLyrics::saveLyrics(void)
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


void CWidgetLyrics::cancelEdit(void)
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
        }

        query->deleteLater();
    }
}

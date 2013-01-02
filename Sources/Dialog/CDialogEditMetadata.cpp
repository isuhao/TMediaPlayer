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

#include "CDialogEditMetadata.hpp"
#include "../CSong.hpp"
#include "../CApplication.hpp"
#include <QPushButton>
#include <QStandardItemModel>
#include <QMessageBox>

// TagLib
#include <fileref.h>
#include <tag.h>
#include <flacfile.h>
#include <mpegfile.h>
#include <vorbisfile.h>
#include <tmap.h>
#include <id3v1tag.h>
#include <id3v1genres.h>
#include <id3v2tag.h>
#include <apetag.h>
#include <xiphcomment.h>
#include <textidentificationframe.h>
#include <urllinkframe.h>
#include <unsynchronizedlyricsframe.h>
#include <commentsframe.h>

#include <QtDebug>


/**
 * Construit la boite de dialogue.
 *
 * \param application Pointeur sur l'application.
 * \param song        Morceau dont on veut afficher les métadonnées.
 */

CDialogEditMetadata::CDialogEditMetadata(CApplication * application, CSong * song) :
    QDialog       (application),
    m_uiWidget    (new Ui::DialogEditMetadata()),
    m_application (application),
    m_song        (song)
{
    Q_CHECK_PTR(application);
    Q_CHECK_PTR(song);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    const QString songTitle = m_song->getTitle();
    const QString songArtist = m_song->getArtistName();

    if (songArtist.isEmpty())
        setWindowTitle(tr("Metadata") + " - " + songTitle);
    else
        setWindowTitle(tr("Metadata") + " - " + songTitle + " - " + songArtist);


    // Genres ID3v1
    TagLib::StringList genreList = TagLib::ID3v1::genreList();

    for (TagLib::StringList::ConstIterator it = genreList.begin(); it != genreList.end(); ++it)
    {
        m_uiWidget->editID3v1Genre->addItem(QString::fromUtf8(it->toCString(true)));
    }

    m_uiWidget->editID3v1Genre->setCurrentIndex(-1);


    // Modèles
    m_modelID3v2Text = new QStandardItemModel(this);
    m_uiWidget->tableID3v2Text->setModel(m_modelID3v2Text);
    m_modelID3v2URL = new QStandardItemModel(this);
    m_uiWidget->tableID3v2URL->setModel(m_modelID3v2URL);
    m_modelID3v2Lyrics = new QStandardItemModel(this);
    m_uiWidget->tableID3v2Lyrics->setModel(m_modelID3v2Lyrics);
    m_modelID3v2Comments = new QStandardItemModel(this);
    m_uiWidget->tableID3v2Comments->setModel(m_modelID3v2Comments);
    m_modelID3v2Pictures = new QStandardItemModel(this);
    m_uiWidget->tableID3v2Pictures->setModel(m_modelID3v2Pictures);

    m_modelAPE = new QStandardItemModel(this);
    m_uiWidget->tableAPE->setModel(m_modelAPE);

    m_modelXiphComment = new QStandardItemModel(this);
    m_uiWidget->tableXiphComment->setModel(m_modelXiphComment);

    
    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    QPushButton * btnApply = m_uiWidget->buttonBox->addButton(tr("Apply"), QDialogButtonBox::ApplyRole);
    QPushButton * btnReset = m_uiWidget->buttonBox->addButton(tr("Reset"), QDialogButtonBox::ResetRole);

    // TODO: Enlever ces 2 lignes quand tout sera fonctionnel
    btnSave->setEnabled(false);
    btnApply->setEnabled(false);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(btnReset, SIGNAL(clicked()), this, SLOT(reset()));

    connect(m_uiWidget->enableID3v1, SIGNAL(clicked(bool)), this, SLOT(enableTagID3v1(bool)));
    connect(m_uiWidget->enableID3v2, SIGNAL(clicked(bool)), this, SLOT(enableTagID3v2(bool)));
    connect(m_uiWidget->enableAPE, SIGNAL(clicked(bool)), this, SLOT(enableTagAPE(bool)));
    connect(m_uiWidget->enableXiphComment, SIGNAL(clicked(bool)), this, SLOT(enableTagXiphComment(bool)));

    reset();
}


/**
 * Détruit la boite de dialogue.
 */

CDialogEditMetadata::~CDialogEditMetadata()
{
    delete m_uiWidget;
}


/**
 * Enregistre les modifications effectuées sur le morceau.
 *
 * \todo Implémentation.
 */

void CDialogEditMetadata::apply()
{
    //...
}


/**
 * Enregistre les modifications effectuées sur le morceau et ferme la boite de dialogue.
 */

void CDialogEditMetadata::save()
{
    apply();
    close();
}


/**
 * Recharge les métadonnées du morceau.
 */

void CDialogEditMetadata::reset()
{
    // Titres des colonnes
    m_modelID3v2Text->clear();
    m_modelID3v2Text->setHorizontalHeaderLabels(QStringList() << tr("Key") << tr("Value"));
    m_modelID3v2URL->clear();
    m_modelID3v2URL->setHorizontalHeaderLabels(QStringList() << tr("Key") << tr("Value"));
    m_modelID3v2Lyrics->clear();
    m_modelID3v2Lyrics->setHorizontalHeaderLabels(QStringList() << tr("Description") << tr("Language") << tr("Lyrics"));
    m_modelID3v2Comments->clear();
    m_modelID3v2Comments->setHorizontalHeaderLabels(QStringList() << tr("Description") << tr("Language") << tr("Comments"));
    m_modelID3v2Pictures->clear();
    m_modelID3v2Pictures->setHorizontalHeaderLabels(QStringList() << tr("Description") << tr("Type") << tr("Format") << tr("Size"));

    m_modelAPE->clear();
    m_modelAPE->setHorizontalHeaderLabels(QStringList() << tr("Key") << tr("Value"));

    m_modelXiphComment->clear();
    m_modelXiphComment->setHorizontalHeaderLabels(QStringList() << tr("Key") << tr("Value"));

    switch (m_song->getFormat())
    {
        default:
            m_application->logError(tr("unknown format"), __FUNCTION__, __FILE__, __LINE__);
            QMessageBox::warning(m_application, QString(), tr("Error while loading tags."), QMessageBox::Ok);
            close();
            return;

        case CSong::FormatMP3:
        {
#ifdef Q_OS_WIN32

#if QT_VERSION >= 0x050000
            TagLib::MPEG::File file(reinterpret_cast<const wchar_t *>(m_song->getFileName().constData()));
#else
            std::wstring fileNameWString = m_song->getFileName().toStdWString();
            TagLib::MPEG::File file(fileNameWString.c_str(), false);
#endif

#else
            TagLib::MPEG::File file(qPrintable(m_song->getFileName()), false);
#endif

            if (!file.isValid())
            {
                m_application->logError(tr("can't read the MP3 file \"%1\"").arg(m_song->getFileName()), __FUNCTION__, __FILE__, __LINE__);
                QMessageBox::warning(m_application, QString(), tr("Error while loading tags."), QMessageBox::Ok);
                close();
                return;
            }

            initTagID3v1(file.ID3v1Tag(true));
            initTagID3v2(file.ID3v2Tag(true));
            initTagAPE(file.APETag(false));
            initTagXiphComment(NULL);
            //m_uiWidget->tabWidget->setTabEnabled(3, false);
            m_uiWidget->tabWidget->removeTab(3);

            break;
        }

        case CSong::FormatOGG:
        {
#ifdef Q_OS_WIN32

#if QT_VERSION >= 0x050000
            TagLib::Ogg::Vorbis::File file(reinterpret_cast<const wchar_t *>(m_song->getFileName().constData()));
#else
            std::wstring fileNameWString = m_song->getFileName().toStdWString();
            TagLib::Ogg::Vorbis::File file(fileNameWString.c_str(), false);
#endif

#else
            TagLib::Ogg::Vorbis::File file(qPrintable(m_song->getFileName()), false);
#endif

            if (!file.isValid())
            {
                m_application->logError(tr("can't read the Ogg file \"%1\"").arg(m_song->getFileName()), __FUNCTION__, __FILE__, __LINE__);
                QMessageBox::warning(m_application, QString(), tr("Error while loading tags."), QMessageBox::Ok);
                close();
                return;
            }
            
            //m_uiWidget->tabWidget->setTabEnabled(0, false);
            m_uiWidget->tabWidget->removeTab(0);
            //m_uiWidget->tabWidget->setTabEnabled(1, false);
            m_uiWidget->tabWidget->removeTab(1);
            //m_uiWidget->tabWidget->setTabEnabled(2, false);
            m_uiWidget->tabWidget->removeTab(2);
            initTagID3v1(NULL);
            initTagID3v2(NULL);
            initTagAPE(NULL);
            initTagXiphComment(file.tag());

            break;
        }

        case CSong::FormatFLAC:
        {
#ifdef Q_OS_WIN32

#if QT_VERSION >= 0x050000
            TagLib::FLAC::File file(reinterpret_cast<const wchar_t *>(m_song->getFileName().constData()));
#else
            std::wstring fileNameWString = m_song->getFileName().toStdWString();
            TagLib::FLAC::File file(fileNameWString.c_str(), false);
#endif

#else
            TagLib::FLAC::File file(qPrintable(m_song->getFileName()), false);
#endif

            if (!file.isValid())
            {
                m_application->logError(tr("can't read the FLAC file \"%1\"").arg(m_song->getFileName()), __FUNCTION__, __FILE__, __LINE__);
                QMessageBox::warning(m_application, QString(), tr("Error while loading tags."), QMessageBox::Ok);
                close();
                return;
            }
            
            initTagID3v1(file.ID3v1Tag(false));
            initTagID3v2(file.ID3v2Tag(false));
            //m_uiWidget->tabWidget->setTabEnabled(2, false);
            m_uiWidget->tabWidget->removeTab(2);
            initTagAPE(NULL);
            initTagXiphComment(file.xiphComment(true));

            break;
        }
    }
}


/**
 * Active ou désactive les tags ID3 v1.
 */

void CDialogEditMetadata::enableTagID3v1(bool enable)
{
    m_uiWidget->enableID3v1->setChecked(enable);

    m_uiWidget->editID3v1Title->setEnabled(enable);
    m_uiWidget->editID3v1Artist->setEnabled(enable);
    m_uiWidget->editID3v1Album->setEnabled(enable);
    m_uiWidget->editID3v1Year->setEnabled(enable);
    m_uiWidget->editID3v1Comments->setEnabled(enable);
    m_uiWidget->editID3v1Track->setEnabled(enable);
    m_uiWidget->editID3v1Genre->setEnabled(enable);
}


/**
 * Active ou désactive les tags ID3 v2.
 */

void CDialogEditMetadata::enableTagID3v2(bool enable)
{
    m_uiWidget->enableID3v2->setChecked(enable);

    m_uiWidget->tabID3v2Type->setEnabled(enable);
}


/**
 * Active ou désactive les tags APE.
 */

void CDialogEditMetadata::enableTagAPE(bool enable)
{
    m_uiWidget->enableAPE->setChecked(enable);

    m_uiWidget->tableAPE->setEnabled(enable);
}


/**
 * Active ou désactive les tags XiphComment.
 */

void CDialogEditMetadata::enableTagXiphComment(bool enable)
{
    m_uiWidget->enableXiphComment->setChecked(enable);

    m_uiWidget->tableXiphComment->setEnabled(enable);
}


/**
 * Initialise l'onglet avec les tags ID3 version 1.
 *
 * \param tags Tags à afficher.
 */

void CDialogEditMetadata::initTagID3v1(TagLib::ID3v1::Tag * tags)
{
    enableTagID3v1(tags);

    if (!tags)
        return;

    m_uiWidget->editID3v1Title->setText(QString::fromUtf8(tags->title().toCString(true)));
    m_uiWidget->editID3v1Artist->setText(QString::fromUtf8(tags->artist().toCString(true)));
    m_uiWidget->editID3v1Album->setText(QString::fromUtf8(tags->album().toCString(true)));
    m_uiWidget->editID3v1Year->setValue(tags->year());
    m_uiWidget->editID3v1Comments->setText(QString::fromUtf8(tags->comment().toCString(true)));
    m_uiWidget->editID3v1Track->setValue(tags->track());
    m_uiWidget->editID3v1Genre->setCurrentIndex(TagLib::ID3v1::genreIndex(tags->genre()));
}


/**
 * Initialise l'onglet avec les tags ID3 version 2.
 *
 * \todo Implémentation complète.
 *
 * \param tags Tags à afficher.
 */

void CDialogEditMetadata::initTagID3v2(TagLib::ID3v2::Tag * tags)
{
    enableTagID3v2(tags);

    if (!tags)
        return;

    TagLib::ID3v2::FrameListMap tagMap = tags->frameListMap();

    // Liste des tags
    for (TagLib::ID3v2::FrameListMap::ConstIterator it = tagMap.begin(); it != tagMap.end(); ++it)
    {
        QString tagKey = QByteArray(it->first.data(), it->first.size());

        for (TagLib::ID3v2::FrameList::ConstIterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            // Texte
            {
                TagLib::ID3v2::TextIdentificationFrame * frame = dynamic_cast<TagLib::ID3v2::TextIdentificationFrame *>(*it2);
                if (frame)
                {
                    QList<QStandardItem *> itemList;

                    itemList.append(new QStandardItem(tagKey));
                    itemList.append(new QStandardItem(QString::fromUtf8(frame->toString().toCString(true))));

                    m_modelID3v2Text->appendRow(itemList);
                    continue;
                }
            }

            // URL
            {
                TagLib::ID3v2::UrlLinkFrame * frame = dynamic_cast<TagLib::ID3v2::UrlLinkFrame *>(*it2);
                if (frame)
                {
                    QList<QStandardItem *> itemList;

                    itemList.append(new QStandardItem(tagKey));
                    itemList.append(new QStandardItem(QString::fromUtf8(frame->url().toCString(true))));

                    m_modelID3v2URL->appendRow(itemList);
                    continue;
                }
            }

            // Paroles
            {
                TagLib::ID3v2::UnsynchronizedLyricsFrame * frame = dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame *>(*it2);
                if (frame)
                {
                    QList<QStandardItem *> itemList;

                    itemList.append(new QStandardItem(QString::fromUtf8(frame->description().toCString(true))));
                    itemList.append(new QStandardItem(getLanguageName(getLanguageForISO3Code(QByteArray(frame->language().data(), 3)))));
                    itemList.append(new QStandardItem(QString::fromUtf8(frame->text().toCString(true))));

                    m_modelID3v2Lyrics->appendRow(itemList);
                    continue;
                }
            }

            // Commentaires
            {
                TagLib::ID3v2::CommentsFrame * frame = dynamic_cast<TagLib::ID3v2::CommentsFrame *>(*it2);
                if (frame)
                {
                    QList<QStandardItem *> itemList;

                    itemList.append(new QStandardItem(QString::fromUtf8(frame->description().toCString(true))));
                    itemList.append(new QStandardItem(getLanguageName(getLanguageForISO3Code(QByteArray(frame->language().data(), 3)))));
                    itemList.append(new QStandardItem(QString::fromUtf8(frame->text().toCString(true))));

                    m_modelID3v2Comments->appendRow(itemList);
                    continue;
                }
            }

            // Illustrations
            {
                TagLib::ID3v2::AttachedPictureFrame * frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(*it2);
                if (frame)
                {
                    QList<QStandardItem *> itemList;

                    itemList.append(new QStandardItem(QString::fromUtf8(frame->description().toCString(true))));
                    itemList.append(new QStandardItem(pictureType(frame->type())));
                    itemList.append(new QStandardItem(QString::fromUtf8(frame->mimeType().toCString(true))));
                    itemList.append(new QStandardItem(QString::number(frame->picture().size())));

                    m_modelID3v2Pictures->appendRow(itemList);
                    continue;
                }
            }

            //...
        }
    }
}


/**
 * Retourne une chaine correspondant à un type d'illustration.
 *
 * \param type Type d'illustration.
 * \return Chaine de caractères.
 */

QString CDialogEditMetadata::pictureType(TagLib::ID3v2::AttachedPictureFrame::Type type) const
{
    switch (type)
    {
        default:
        case TagLib::ID3v2::AttachedPictureFrame::Other:              return tr("Other");
        case TagLib::ID3v2::AttachedPictureFrame::FileIcon:           return tr("FileIcon");
        case TagLib::ID3v2::AttachedPictureFrame::OtherFileIcon:      return tr("OtherFileIcon");
        case TagLib::ID3v2::AttachedPictureFrame::FrontCover:         return tr("FrontCover");
        case TagLib::ID3v2::AttachedPictureFrame::BackCover:          return tr("BackCover");
        case TagLib::ID3v2::AttachedPictureFrame::LeafletPage:        return tr("LeafletPage");
        case TagLib::ID3v2::AttachedPictureFrame::Media:              return tr("Media");
        case TagLib::ID3v2::AttachedPictureFrame::LeadArtist:         return tr("LeadArtist");
        case TagLib::ID3v2::AttachedPictureFrame::Artist:             return tr("Artist");
        case TagLib::ID3v2::AttachedPictureFrame::Conductor:          return tr("Conductor");
        case TagLib::ID3v2::AttachedPictureFrame::Band:               return tr("Band");
        case TagLib::ID3v2::AttachedPictureFrame::Composer:           return tr("Composer");
        case TagLib::ID3v2::AttachedPictureFrame::Lyricist:           return tr("Lyricist");
        case TagLib::ID3v2::AttachedPictureFrame::RecordingLocation:  return tr("RecordingLocation");
        case TagLib::ID3v2::AttachedPictureFrame::DuringRecording:    return tr("DuringRecording");
        case TagLib::ID3v2::AttachedPictureFrame::DuringPerformance:  return tr("DuringPerformance");
        case TagLib::ID3v2::AttachedPictureFrame::MovieScreenCapture: return tr("MovieScreenCapture");
        case TagLib::ID3v2::AttachedPictureFrame::ColouredFish:       return tr("ColouredFish");
        case TagLib::ID3v2::AttachedPictureFrame::Illustration:       return tr("Illustration");
        case TagLib::ID3v2::AttachedPictureFrame::BandLogo:           return tr("BandLogo");
        case TagLib::ID3v2::AttachedPictureFrame::PublisherLogo:      return tr("PublisherLogo");
    }
}


/**
 * Initialise l'onglet avec les tags APE.
 *
 * \todo Afficher les tags binaires et locator.
 *
 * \param tags Tags APE.
 */

void CDialogEditMetadata::initTagAPE(TagLib::APE::Tag * tags)
{
    enableTagAPE(tags);

    if (!tags)
        return;

    TagLib::APE::ItemListMap tagMap = tags->itemListMap();

    // Liste des tags
    for (TagLib::APE::ItemListMap::ConstIterator it = tagMap.begin(); it != tagMap.end(); ++it)
    {
        switch (it->second.type())
        {
            case TagLib::APE::Item::Text:
            {
                QString tagValue = QString::fromUtf8(it->second.values().toString().toCString(true)).replace('\r', ' ').replace('\n', ' ');

                QList<QStandardItem *> itemList;

                itemList.append(new QStandardItem(QString::fromUtf8(it->first.toCString(true))));

                if (tagValue.size() > 100)
                    itemList.append(new QStandardItem(tagValue.left(100-6) + " [...]"));
                else
                    itemList.append(new QStandardItem(tagValue));

                m_modelAPE->appendRow(itemList);
                break;
            }

            case TagLib::APE::Item::Binary:
                //...
                break;

            case TagLib::APE::Item::Locator:
                //...
                break;
        }
    }
}


/**
 * Initialise l'onglet avec les tags xiphComment.
 *
 * \param tags Tags xiphComment.
 */

void CDialogEditMetadata::initTagXiphComment(TagLib::Ogg::XiphComment * tags)
{
    enableTagXiphComment(tags);

    if (!tags)
        return;

    TagLib::Ogg::FieldListMap tagMap = tags->fieldListMap();

    for (TagLib::Ogg::FieldListMap::ConstIterator it = tagMap.begin(); it != tagMap.end(); ++it)
    {
        QString tagKey = QString::fromUtf8(it->first.toCString(true));

        for (TagLib::StringList::ConstIterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            QString tagValue = QString::fromUtf8(it2->toCString(true)).replace('\r', ' ').replace('\n', ' ');

            QList<QStandardItem *> itemList;

            itemList.append(new QStandardItem(tagKey));

            if (tagValue.size() > 100)
                itemList.append(new QStandardItem(tagValue.left(100-6) + " [...]"));
            else
                itemList.append(new QStandardItem(tagValue));

            m_modelXiphComment->appendRow(itemList);
        }
    }
}

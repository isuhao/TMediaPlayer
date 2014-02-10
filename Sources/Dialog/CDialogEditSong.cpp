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

#include "CDialogEditSong.hpp"
#include "../CMediaTableView.hpp"
#include "../CMainWindow.hpp"
#include "../CMediaManager.hpp"
#include "../CRatingEditor.hpp"
#include "../Utils.hpp"

// Qt
#include <QStandardItemModel>
#include <QCloseEvent>
#include <QSettings>

// FMOD
#include <mpegfile.h>
#include <id3v2tag.h>

#include <QtDebug>


/**
 * Construit la boite de dialogue pour modifier les informations d'un morceau.
 *
 * \param songItem   Morceau à modifier.
 * \param songTable  Liste contenant le morceau, pour pouvoir naviguer parmi les morceaux.
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 */

CDialogEditSong::CDialogEditSong(CMediaTableItem * songItem, CMediaTableView * songTable, CMainWindow * mainWindow) :
QDialog        (mainWindow),
m_uiWidget     (new Ui::DialogEditSong()),
m_ratingEditor (new CRatingEditor()),
m_songTable    (songTable),
m_songItem     (songItem),
m_mainWindow   (mainWindow)
{
    Q_CHECK_PTR(m_songItem);
    Q_CHECK_PTR(m_songTable);
    Q_CHECK_PTR(m_mainWindow);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    m_ratingEditor->setDelegate(false);
    m_uiWidget->layoutInfos->addWidget(m_ratingEditor, 16, 1, 1, 3);


    // Liste des langues
    m_uiWidget->editLanguage->addItems(getLanguageList());


    // Liste des genres
    QStringList genres = m_mainWindow->getMediaManager()->getGenreList();
    m_uiWidget->editGenre->addItems(genres);


    // Synchronisation des champs avec tri
    connect(m_uiWidget->editTitle, SIGNAL(textEdited(const QString&)), m_uiWidget->editTitle_2, SLOT(setText(const QString&)));
    connect(m_uiWidget->editTitle_2, SIGNAL(textEdited(const QString&)), m_uiWidget->editTitle, SLOT(setText(const QString&)));

    connect(m_uiWidget->editArtist, SIGNAL(textEdited(const QString&)), m_uiWidget->editArtist_2, SLOT(setText(const QString&)));
    connect(m_uiWidget->editArtist_2, SIGNAL(textEdited(const QString&)), m_uiWidget->editArtist, SLOT(setText(const QString&)));

    connect(m_uiWidget->editAlbum, SIGNAL(textEdited(const QString&)), m_uiWidget->editAlbum_2, SLOT(setText(const QString&)));
    connect(m_uiWidget->editAlbum_2, SIGNAL(textEdited(const QString&)), m_uiWidget->editAlbum, SLOT(setText(const QString&)));

    connect(m_uiWidget->editAlbumArtist, SIGNAL(textEdited(const QString&)), m_uiWidget->editAlbumArtist_2, SLOT(setText(const QString&)));
    connect(m_uiWidget->editAlbumArtist_2, SIGNAL(textEdited(const QString&)), m_uiWidget->editAlbumArtist, SLOT(setText(const QString&)));

    connect(m_uiWidget->editComposer, SIGNAL(textEdited(const QString&)), m_uiWidget->editComposer_2, SLOT(setText(const QString&)));
    connect(m_uiWidget->editComposer_2, SIGNAL(textEdited(const QString&)), m_uiWidget->editComposer, SLOT(setText(const QString&)));


    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    QPushButton * btnApply = m_uiWidget->buttonBox->addButton(tr("Apply"), QDialogButtonBox::ApplyRole);

    connect(m_uiWidget->btnPrevious, SIGNAL(clicked()), this, SLOT(previousSong()));
    connect(m_uiWidget->btnNext, SIGNAL(clicked()), this, SLOT(nextSong()));

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));

    updateInfos();

    CMediaTableItem * songItemTest = m_songTable->getPreviousSong(m_songItem, false);
    m_uiWidget->btnPrevious->setEnabled(songItemTest);

    songItemTest = m_songTable->getNextSong(m_songItem, false);
    m_uiWidget->btnNext->setEnabled(songItemTest);
}


/**
 * Détruit la boite de dialogue.
 */

CDialogEditSong::~CDialogEditSong()
{
    delete m_uiWidget;
}


/**
 * Modifie le morceau ou la liste de lecture en cours d'édition.
 *
 * \param songItem  Pointeur sur le nouveau morceau dans la liste.
 * \param songTable Pointeur sur la liste de morceaux.
 */

void CDialogEditSong::setSongItem(CMediaTableItem * songItem, CMediaTableView * songTable)
{
    Q_CHECK_PTR(songTable);

    m_songItem = songItem;
    m_songTable = songTable;
}


/**
 * Méthode appellée lorsqu'on ferme la boite de dialogue
 *
 * \param event Évènement de fermeture.
 */

void CDialogEditSong::closeEvent(QCloseEvent * event)
{
    Q_CHECK_PTR(event);
    event->accept();
    emit closed();
}


/**
 * Applique les modifications apportées au morceau.
 */

void CDialogEditSong::applyChanges()
{
    if (m_songItem == nullptr)
    {
        return;
    }

    CSong * song = m_songItem->getSong();

    song->setTitle(m_uiWidget->editTitle->text());
    song->setSubTitle(m_uiWidget->editSubTitle->text());
    song->setGrouping(m_uiWidget->editGrouping->text());
    song->setArtistName(m_uiWidget->editArtist->text());
    song->setAlbumTitle(m_uiWidget->editAlbum->text());
    song->setAlbumArtist(m_uiWidget->editAlbumArtist->text());
    song->setComposer(m_uiWidget->editComposer->text());

    song->setTitleSort(m_uiWidget->editTitleSort->text());
    song->setArtistNameSort(m_uiWidget->editArtistSort->text());
    song->setAlbumTitleSort(m_uiWidget->editAlbumSort->text());
    song->setAlbumArtistSort(m_uiWidget->editAlbumArtistSort->text());
    song->setComposerSort(m_uiWidget->editComposerSort->text());

    song->setBPM(m_uiWidget->editBPM->text().toInt());
    song->setYear(m_uiWidget->editYear->text().toInt());
    song->setTrackNumber(m_uiWidget->editTrackNumber->text().toInt());
    song->setTrackCount(m_uiWidget->editTrackCount->text().toInt());
    song->setDiscNumber(m_uiWidget->editDiscNumber->text().toInt());
    song->setDiscCount(m_uiWidget->editDiscCount->text().toInt());
    song->setComments(m_uiWidget->editComments->toPlainText());
    song->setGenre(m_uiWidget->editGenre->currentText());
    song->setRating(m_ratingEditor->getRatingValue());
    song->setLyrics(m_uiWidget->editLyrics->toPlainText());
    song->setLanguage(getLanguageFromInteger(m_uiWidget->editLanguage->currentIndex()));
    song->setLyricist(m_uiWidget->editLyricist->text());

    song->setEnabled(m_uiWidget->editEnabled->isChecked());
    song->setSkipShuffle(m_uiWidget->editSkipShuffle->isChecked());
    song->setCompilation(m_uiWidget->editCompilation->isChecked());

    song->writeTags();
    song->updateDatabase();
}


/**
 * Recharge les informations affichées dans l'onglet "Résumé".
 *
 * \todo Afficher les illustrations.
 */

void CDialogEditSong::resetSummary()
{
    if (m_songItem == nullptr)
    {
        return;
    }

    CSong * song = m_songItem->getSong();

    const QString songTitle = song->getTitle();
    const QString songArtist = song->getArtistName();

    if (songArtist.isEmpty())
        setWindowTitle(tr("Song infos") + " - " + songTitle);
    else
        setWindowTitle(tr("Song infos") + " - " + songTitle + " - " + songArtist);


    // Résumé

    const int duration = song->getDuration();
    QTime durationTime(0, 0);
    durationTime = durationTime.addMSecs(duration);

    m_uiWidget->valueTitle->setText(songTitle + QString(" (%1)").arg(durationTime.toString("m:ss"))); /// \todo Stocker dans les settings
    m_uiWidget->valueArtist->setText(songArtist);
    m_uiWidget->valueAlbum->setText(song->getAlbumTitle());

    m_uiWidget->valueFileName->setText(song->getFileName());
    m_uiWidget->valueFileSize->setText(getFileSize(song->getFileSize()));
    m_uiWidget->valueCreation->setText(song->getCreationDate().toString("dd/MM/yyyy HH:mm:ss"));
    m_uiWidget->valueModification->setText(song->getModificationDate().toString("dd/MM/yyyy HH:mm:ss"));

    m_uiWidget->valueBitRate->setText(tr("%1 kbit/s").arg(song->getBitRate()));
    m_uiWidget->valueFormat->setText(CSong::getFormatName(song->getFormat()));
    m_uiWidget->valueChannels->setText(QString::number(song->getNumChannels()));
    m_uiWidget->valueSampleRate->setText(tr("%1 Hz").arg(song->getSampleRate()));

    QDateTime lastPlay = song->getLastPlay();

    if (lastPlay.isNull())
        m_uiWidget->valueLastPlayTime->setText(tr("never"));
    else
        m_uiWidget->valueLastPlayTime->setText(song->getLastPlay().toString("dd/MM/yyyy HH:mm:ss"));

    m_uiWidget->valuePlayCount->setText(QString::number(song->getNumPlays()));

    // Illustration
    if (song->getFormat() == CSong::FormatMP3)
    {
#ifdef Q_OS_WIN32

#if QT_VERSION >= 0x050000
        TagLib::MPEG::File file(reinterpret_cast<const wchar_t *>(song->getFileName().constData()));
#else
        std::wstring fileNameWString = song->getFileName().toStdWString();
        TagLib::MPEG::File file(fileNameWString.c_str(), false);
#endif

#else
        TagLib::MPEG::File file(qPrintable(song->getFileName()), false);
#endif

        if (file.isValid())
        {
            TagLib::ID3v2::Tag * tags = file.ID3v2Tag(true);
            TagLib::ID3v2::FrameListMap tagMap = tags->frameListMap();
            TagLib::ID3v2::FrameList tagList = tags->frameList("APIC");

            // Recherche dans le dossier
            if (tagList.isEmpty())
            {
                m_uiWidget->valueCover->setPixmap(QPixmap::fromImage(song->getCoverImage()));
            }
            // Recherche dans les métadonnées
            else
            {
                //...
                // Recherche de l'illustration la plus appropriée (s'il y en a plusieurs)
                //m_uiWidget->valueCover->setPixmap(QImage(?));
            }
        }
        else
        {
            m_mainWindow->getMediaManager()->logError(tr("impossible de lire le fichier MP3 \"%1\"").arg(song->getFileName()), __FUNCTION__, __FILE__, __LINE__);
            m_uiWidget->valueCover->setPixmap(QPixmap::fromImage(song->getCoverImage()));
        }
    }
    else
    {
        m_uiWidget->valueCover->setPixmap(QPixmap::fromImage(song->getCoverImage()));
    }

    //TagLib::ID3v2::AttachedPictureFrame
}


/**
 * Affiche les informations du morceau précédent dans la liste.
 * Les modifications sont enregistrées si l'option EditSongAutoSave est activée.
 */

void CDialogEditSong::previousSong()
{
    // Morceau précédent
    CMediaTableItem * songItem = m_songTable->getPreviousSong(m_songItem, false);

    QSettings * settings = m_mainWindow->getMediaManager()->getSettings();
    Q_CHECK_PTR(settings);

    if (settings->value("Preferences/EditSongAutoSave", false).toBool())
    {
        applyChanges();

        // La liste a changé et le morceau courant ne s'y trouve plus
        if (m_songItem == nullptr && songItem != nullptr)
        {
            songItem = m_songTable->getFirstSongItem(songItem->getSong());
        }
    }

    if (songItem)
    {
        m_songItem = songItem;
        updateInfos();

        songItem = m_songTable->getPreviousSong(m_songItem, false);
        m_uiWidget->btnPrevious->setEnabled(songItem != nullptr);
        songItem = m_songTable->getNextSong(m_songItem, false);
        m_uiWidget->btnNext->setEnabled(songItem != nullptr);
    }
}


/**
 * Affiche les informations du morceau suivant dans la liste.
 * Les modifications sont enregistrées si l'option EditSongAutoSave est activée.
 */

void CDialogEditSong::nextSong()
{
    // Morceau suivant
    CMediaTableItem * songItem = m_songTable->getNextSong(m_songItem, false);

    QSettings * settings = m_mainWindow->getMediaManager()->getSettings();
    Q_CHECK_PTR(settings);

    if (settings->value("Preferences/EditSongAutoSave", false).toBool())
    {
        applyChanges();

        // La liste a changé et le morceau courant ne s'y trouve plus
        if (m_songItem == nullptr && songItem != nullptr)
        {
            songItem = m_songTable->getFirstSongItem(songItem->getSong());
        }
    }

    if (songItem)
    {
        m_songItem = songItem;
        updateInfos();

        songItem = m_songTable->getPreviousSong(m_songItem, false);
        m_uiWidget->btnPrevious->setEnabled(songItem != nullptr);
        songItem = m_songTable->getNextSong(m_songItem, false);
        m_uiWidget->btnNext->setEnabled(songItem != nullptr);
    }
}


/**
 * Enregistre les modifications effectuées sur le morceau.
 */

void CDialogEditSong::apply()
{
    applyChanges();
    resetSummary();
}


/**
 * Enregistre les modifications effectuées sur le morceau et ferme la boite de dialogue.
 */

void CDialogEditSong::save()
{
    applyChanges();
    close();
}


/**
 * Met à jour la boite de dialogue avec les informations du morceau.
 */

void CDialogEditSong::updateInfos()
{
    if (m_songItem == nullptr)
    {
        return;
    }

    CSong * song = m_songItem->getSong();

    // Rechargement des métadonnées
    song->loadTags();
    song->updateDatabase();


    // Résumé
    resetSummary();


    // Informations et tri

    QString songTitle = song->getTitle();
    m_uiWidget->editTitle->setText(songTitle);
    m_uiWidget->editTitle_2->setText(songTitle);
    m_uiWidget->editTitleSort->setText(song->getTitleSort());

    m_uiWidget->editSubTitle->setText(song->getSubTitle());

    QString songArtist = song->getArtistName();
    m_uiWidget->editArtist->setText(songArtist);
    m_uiWidget->editArtist_2->setText(songArtist);
    m_uiWidget->editArtistSort->setText(song->getArtistNameSort());

    QString songAlbum = song->getAlbumTitle();
    m_uiWidget->editAlbum->setText(songAlbum);
    m_uiWidget->editAlbum_2->setText(songAlbum);
    m_uiWidget->editAlbumSort->setText(song->getAlbumTitleSort());

    QString songAlbumArtist = song->getAlbumArtist();
    m_uiWidget->editAlbumArtist->setText(songAlbumArtist);
    m_uiWidget->editAlbumArtist_2->setText(songAlbumArtist);
    m_uiWidget->editAlbumArtistSort->setText(song->getAlbumArtistSort());

    m_uiWidget->editGrouping->setText(song->getGrouping());
    const int bpm = song->getBPM();
    m_uiWidget->editBPM->setText(bpm > 0 ? QString::number(bpm) : QString());

    QString songComposer = song->getComposer();
    m_uiWidget->editComposer->setText(songComposer);
    m_uiWidget->editComposer_2->setText(songComposer);
    m_uiWidget->editComposerSort->setText(song->getComposerSort());

    // Année
    const int year = song->getYear();
    m_uiWidget->editYear->setText(year > 0 ? QString::number(year) : QString());
    m_uiWidget->editYear->setValidator(new QIntValidator(0, 9999, this));

    // Numéro de piste
    const int trackNumber = song->getTrackNumber();
    m_uiWidget->editTrackNumber->setText(trackNumber > 0 ? QString::number(trackNumber) : QString());
    m_uiWidget->editTrackNumber->setValidator(new QIntValidator(0, 999, this));

    const int trackCount = song->getTrackCount();
    m_uiWidget->editTrackCount->setText(trackCount > 0 ? QString::number(trackCount) : QString());
    m_uiWidget->editTrackCount->setValidator(new QIntValidator(0, 999, this));

    // Numéro de disque
    const int discNumber = song->getDiscNumber();
    m_uiWidget->editDiscNumber->setText(discNumber > 0 ? QString::number(discNumber) : QString());
    m_uiWidget->editDiscNumber->setValidator(new QIntValidator(0, 999, this));

    const int discCount = song->getDiscCount();
    m_uiWidget->editDiscCount->setText(discCount > 0 ? QString::number(discCount) : QString());
    m_uiWidget->editDiscCount->setValidator(new QIntValidator(0, 999, this));

    // Commentaires
    m_uiWidget->editComments->setText(song->getComments());

    // Genres
    int genreIndex = m_uiWidget->editGenre->findText(song->getGenre());
    if (genreIndex < 0)
    {
        m_uiWidget->editGenre->addItem(song->getGenre());
        genreIndex = m_uiWidget->editGenre->findText(song->getGenre());
    }

    m_uiWidget->editGenre->setCurrentIndex(genreIndex);

    // Note
    //m_uiWidget->editRating->setValue(song->getRating());
    m_ratingEditor->setRatingValue(song->getRating());


    // Paroles
    m_uiWidget->editLyrics->setText(song->getLyrics());
    m_uiWidget->editLanguage->setCurrentIndex(song->getLanguage());
    m_uiWidget->editLyricist->setText(song->getLyricist());


    // Options
    m_uiWidget->editEnabled->setChecked(song->isEnabled());
    m_uiWidget->editSkipShuffle->setChecked(song->isSkipShuffle());
    m_uiWidget->editCompilation->setChecked(song->isCompilation());

    float trackGain = song->getTrackGain();
    if (trackGain == std::numeric_limits<float>::infinity())
        m_uiWidget->editTrackGain->setText(QString());
    else
        m_uiWidget->editTrackGain->setText(tr("%1 dB").arg(trackGain));

    float trackPeak = song->getTrackPeak();
    if (trackPeak == std::numeric_limits<float>::infinity())
        m_uiWidget->editTrackPeak->setText(QString());
    else
        m_uiWidget->editTrackPeak->setText(QString::number(trackPeak));

    float albumGain = song->getAlbumGain();
    if (albumGain == std::numeric_limits<float>::infinity())
        m_uiWidget->editAlbumGain->setText(QString());
    else
        m_uiWidget->editAlbumGain->setText(tr("%1 dB").arg(albumGain));

    float albumPeak = song->getAlbumPeak();
    if (albumPeak == std::numeric_limits<float>::infinity())
        m_uiWidget->editAlbumPeak->setText(QString());
    else
        m_uiWidget->editAlbumPeak->setText(QString::number(albumPeak));


    // Illustrations
    //...


    // Lectures
    QStandardItemModel * model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList() << tr("Local time") << tr("UTC"));
    m_uiWidget->listPlays->setModel(model);

    for (QList<CSong::TSongPlay>::ConstIterator it = song->m_plays.begin(); it != song->m_plays.end(); ++it)
    {
        QList<QStandardItem *> itemList;
        itemList << new QStandardItem(it->time.toString(tr("dd/MM/yyyy HH:mm:ss")));
        itemList << new QStandardItem(it->timeUTC.toString(tr("dd/MM/yyyy HH:mm:ss")));
        model->appendRow(itemList);
    }

    m_uiWidget->listPlays->resizeColumnsToContents();

    qlonglong totalDuration = song->m_plays.size() * song->getDuration();
    QTime durationTime(0, 0);
    durationTime = durationTime.addMSecs(totalDuration % 86400000);

    if (totalDuration > 86400000)
    {
        int numDays = static_cast<int>(totalDuration / 86400000);
        m_uiWidget->lblTotalDuration->setText(tr("Total duration: %1").arg(tr("%n day(s) %1", "", numDays).arg(durationTime.toString())));
    }
    else if (totalDuration > 3600000)
    {
        m_uiWidget->lblTotalDuration->setText(tr("Total duration: %1").arg(durationTime.toString()));
    }
    else
    {
        m_uiWidget->lblTotalDuration->setText(tr("Total duration: %1").arg(durationTime.toString("m:ss")));
    }
}

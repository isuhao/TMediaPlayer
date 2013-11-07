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

#include "CDialogEditSongs.hpp"
#include "../CMainWindow.hpp"
#include "../CMediaManager.hpp"
#include "../CSpecialSpinBox.hpp"
#include "../CMediaTableItem.hpp"
#include "../CWidgetLineEditMultiple.hpp"
#include "../CSong.hpp"
#include "../Language.hpp"
#include "../Utils.hpp"

#include <QStandardItemModel>
#include <QPushButton>
#include <QProgressDialog>


/**
 * Construit la boite de dialogue pour modifier les informations de plusieurs morceaux.
 *
 * \param songItemList Liste des morceaux à modifier.
 * \param mainWindow   Pointeur sur la fenêtre principale de l'application.
 */

CDialogEditSongs::CDialogEditSongs(QList<CMediaTableItem *>& songItemList, CMainWindow * mainWindow) :
QDialog             (mainWindow),
m_uiWidget          (new Ui::DialogEditSongs()),
m_editRating        (nullptr),
m_differentComments (false),
m_differentLyrics   (false),
m_songItemList      (songItemList)
{
    Q_CHECK_PTR(mainWindow);
    Q_ASSERT(songItemList.size() > 1);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);


    // Titre de la fenêtre
    setWindowTitle(tr("Informations (%1 songs)").arg(songItemList.size()));


    // Synchronisation des champs
    connect(m_uiWidget->chTitle, SIGNAL(toggled(bool)), m_uiWidget->chTitle_2, SLOT(setChecked(bool)));
    connect(m_uiWidget->chTitle_2, SIGNAL(toggled(bool)), m_uiWidget->chTitle, SLOT(setChecked(bool)));

    connect(m_uiWidget->chArtist, SIGNAL(toggled(bool)), m_uiWidget->chArtist_2, SLOT(setChecked(bool)));
    connect(m_uiWidget->chArtist_2, SIGNAL(toggled(bool)), m_uiWidget->chArtist, SLOT(setChecked(bool)));

    connect(m_uiWidget->chAlbum, SIGNAL(toggled(bool)), m_uiWidget->chAlbum_2, SLOT(setChecked(bool)));
    connect(m_uiWidget->chAlbum_2, SIGNAL(toggled(bool)), m_uiWidget->chAlbum, SLOT(setChecked(bool)));

    connect(m_uiWidget->chAlbumArtist, SIGNAL(toggled(bool)), m_uiWidget->chAlbumArtist_2, SLOT(setChecked(bool)));
    connect(m_uiWidget->chAlbumArtist_2, SIGNAL(toggled(bool)), m_uiWidget->chAlbumArtist, SLOT(setChecked(bool)));

    connect(m_uiWidget->chComposer, SIGNAL(toggled(bool)), m_uiWidget->chComposer_2, SLOT(setChecked(bool)));
    connect(m_uiWidget->chComposer_2, SIGNAL(toggled(bool)), m_uiWidget->chComposer, SLOT(setChecked(bool)));


    // Liste des langues
    m_uiWidget->editLanguage->addItems(getLanguageList());


    // Résumé
    int durationMin;
    int durationMax = 0;
    qlonglong durationTotal = 0;

    qlonglong fileSizeMin;
    qlonglong fileSizeMax = 0;
    qlonglong fileSizeTotal = 0;

    QDateTime creationMin;
    QDateTime creationMax;

    QDateTime modificationMin;
    QDateTime modificationMax;

    int bitRateMin;
    int bitRateMax = 0;
    qlonglong bitRateAverage = 0;

    int sampleRateMin;
    int sampleRateMax = 0;

    QMap<CSong::TFormat, int> formats;

    int numChannelsMin;
    int numChannelsMax = 0;

    QList<TPlay> plays;

    // Recherche des données similaires pour tous les éléments
    //TODO: Remplacer par des maps
    QMap<QString, int> songTitle_V2;
    QMap<QString, int> songTitleSort_V2;
    QMap<QString, int> songArtist_V2;
    QMap<QString, int> songArtistSort_V2;
    QMap<QString, int> songAlbum_V2;
    QMap<QString, int> songAlbumSort_V2;
    QMap<QString, int> songAlbumArtist_V2;
    QMap<QString, int> songAlbumArtistSort_V2;
    QMap<QString, int> songComposer_V2;
    QMap<QString, int> songComposerSort_V2;
    QString songSubTitle; bool songSubTitleSim = true;
    QString songGrouping; bool songGroupingSim = true;
    QString songComments; bool songCommentsSim = true;
    QString songGenre;    bool songGenreSim    = true;
    QString songLyrics;   bool songLyricsSim   = true;
    QString songLyricist; bool songLyricistSim = true;

    int songYear        = 0; bool songYearSim        = true;
    int songTrackNumber = 0; bool songTrackNumberSim = true;
    int songTrackCount  = 0; bool songTrackCountSim  = true;
    int songDiscNumber  = 0; bool songDiscNumberSim  = true;
    int songDiscCount   = 0; bool songDiscCountSim   = true;
    int songBPM         = 0; bool songBPMSim         = true;
    int songRating      = 0; bool songRatingSim      = true;

    TLanguage songLanguage = LangUnknown;
    bool songLanguageSim = true;

    bool songEnabled     = false; bool songEnabledSim     = true;
    bool songSkipShuffle = false; bool songSkipShuffleSim = true;
    bool songCompilation = false; bool songCompilationSim = true;


    bool first = true;

    // On parcourt la liste des morceaux sélectionnés pour trouver les informations à afficher
    for (QList<CMediaTableItem *>::ConstIterator it = m_songItemList.begin(); it != m_songItemList.end(); ++it)
    {
        Q_CHECK_PTR(*it);
        CSong * song = (*it)->getSong();

        // Durée du morceau
        int songDuration = song->getDuration();
        durationTotal += songDuration;

        if (first || songDuration < durationMin)
            durationMin = songDuration;

        if (songDuration > durationMax)
            durationMax = songDuration;

        // Taille du fichier
        qlonglong songFileSize = song->getFileSize();
        fileSizeTotal += songFileSize;

        if (first || songFileSize < fileSizeMin)
            fileSizeMin = songFileSize;

        if (songFileSize > fileSizeMax)
            fileSizeMax = songFileSize;

        // Date de création
        QDateTime songCreation = song->getCreationDate();

        if (first || songCreation < creationMin)
            creationMin = songCreation;

        if (songCreation > creationMax)
            creationMax = songCreation;

        // Date de modification
        QDateTime songModification = song->getModificationDate();

        if (first || songModification < modificationMin)
            modificationMin = songModification;

        if (songModification > modificationMax)
            modificationMax = songModification;

        // Débit binaire
        int songBitRate = song->getBitRate();
        bitRateAverage += static_cast<qlonglong>(songBitRate) * static_cast<qlonglong>(songDuration);

        if (first || songBitRate < bitRateMin)
            bitRateMin = songBitRate;

        if (songBitRate > bitRateMax)
            bitRateMax = songBitRate;

        // Fréquence d'échantillonnage
        int songSampleRate = song->getSampleRate();

        if (first || songSampleRate < sampleRateMin)
            sampleRateMin = songSampleRate;

        if (songSampleRate > sampleRateMax)
            sampleRateMax = songSampleRate;

        // Formats
        CSong::TFormat format = song->getFormat();

        if (formats.contains(format))
            ++formats[format];
        else
            formats[format] = 1;

        // Fréquence d'échantillonnage
        int songNumChannels = song->getNumChannels();

        if (first || songNumChannels < numChannelsMin)
            numChannelsMin = songNumChannels;

        if (songNumChannels > numChannelsMax)
            numChannelsMax = songNumChannels;

        // Lectures
        QList<CSong::TSongPlay> songPlays = song->getPlays();

        for (QList<CSong::TSongPlay>::ConstIterator play = songPlays.begin(); play != songPlays.end(); ++play)
        {
            TPlay p;
            p.song    = song;
            p.time    = play->time;
            p.timeUTC = play->timeUTC;

            plays.append(p);
        }

        const QString songTitle_V2_Song = song->getTitle();
        if (songTitle_V2.contains(songTitle_V2_Song))
            ++songTitle_V2[songTitle_V2_Song];
        else
            songTitle_V2[songTitle_V2_Song] = 1;

        const QString songTitleSort_V2_Song = song->getTitleSort();
        if (songTitleSort_V2.contains(songTitleSort_V2_Song))
            ++songTitleSort_V2[songTitleSort_V2_Song];
        else
            songTitleSort_V2[songTitleSort_V2_Song] = 1;

        const QString songArtist_V2_Song = song->getArtistName();
        if (songArtist_V2.contains(songArtist_V2_Song))
            ++songArtist_V2[songArtist_V2_Song];
        else
            songArtist_V2[songArtist_V2_Song] = 1;

        const QString songArtistSort_V2_Song = song->getArtistNameSort();
        if (songArtistSort_V2.contains(songArtistSort_V2_Song))
            ++songArtistSort_V2[songArtistSort_V2_Song];
        else
            songArtistSort_V2[songArtistSort_V2_Song] = 1;

        const QString songAlbum_V2_Song = song->getAlbumTitle();
        if (songAlbum_V2.contains(songAlbum_V2_Song))
            ++songAlbum_V2[songAlbum_V2_Song];
        else
            songAlbum_V2[songAlbum_V2_Song] = 1;

        const QString songAlbumSort_V2_Song = song->getAlbumTitleSort();
        if (songAlbumSort_V2.contains(songAlbumSort_V2_Song))
            ++songAlbumSort_V2[songAlbumSort_V2_Song];
        else
            songAlbumSort_V2[songAlbumSort_V2_Song] = 1;

        const QString songAlbumArtist_V2_Song = song->getAlbumArtist();
        if (songAlbumArtist_V2.contains(songAlbumArtist_V2_Song))
            ++songAlbumArtist_V2[songAlbumArtist_V2_Song];
        else
            songAlbumArtist_V2[songAlbumArtist_V2_Song] = 1;

        const QString songAlbumArtistSort_V2_Song = song->getAlbumArtistSort();
        if (songAlbumArtistSort_V2.contains(songAlbumArtistSort_V2_Song))
            ++songAlbumArtistSort_V2[songAlbumArtistSort_V2_Song];
        else
            songAlbumArtistSort_V2[songAlbumArtistSort_V2_Song] = 1;

        const QString songComposer_V2_Song = song->getComposer();
        if (songComposer_V2.contains(songComposer_V2_Song))
            ++songComposer_V2[songComposer_V2_Song];
        else
            songComposer_V2[songComposer_V2_Song] = 1;

        const QString songComposerSort_V2_Song = song->getComposerSort();
        if (songComposerSort_V2.contains(songComposerSort_V2_Song))
            ++songComposerSort_V2[songComposerSort_V2_Song];
        else
            songComposerSort_V2[songComposerSort_V2_Song] = 1;

        if (first)
        {
            songSubTitle    = song->getSubTitle();
            songGrouping    = song->getGrouping();
            songComments    = song->getComments();
            songGenre       = song->getGenre();
            songLyrics      = song->getLyrics();
            songLyricist    = song->getLyricist();

            songYear        = song->getYear();
            songTrackNumber = song->getTrackNumber();
            songTrackCount  = song->getTrackCount();
            songDiscNumber  = song->getDiscNumber();
            songDiscCount   = song->getDiscCount();
            songBPM         = song->getBPM();
            songRating      = song->getRating();

            songLanguage    = song->getLanguage();

            songEnabled     = song->isEnabled();
            songSkipShuffle = song->isSkipShuffle();
            songCompilation = song->isCompilation();

            first = false;
        }
        else
        {
            if (songSubTitleSim    && song->getSubTitle()    != songSubTitle    ) { songSubTitleSim    = false; songSubTitle.clear(); }
            if (songGroupingSim    && song->getGrouping()    != songGrouping    ) { songGroupingSim    = false; songGrouping.clear(); }
            if (songCommentsSim    && song->getComments()    != songComments    ) { songCommentsSim    = false; songComments.clear(); }
            if (songGenreSim       && song->getGenre()       != songGenre       ) { songGenreSim       = false; songGenre   .clear(); }
            if (songLyricsSim      && song->getLyrics()      != songLyrics      ) { songLyricsSim      = false; songLyrics  .clear(); }
            if (songLyricistSim    && song->getLyricist()    != songLyricist    ) { songLyricistSim    = false; songLyricist.clear(); }

            if (songYearSim        && song->getYear()        != songYear        ) { songYearSim        = false; songYear        = 0; }
            if (songTrackNumberSim && song->getTrackNumber() != songTrackNumber ) { songTrackNumberSim = false; songTrackNumber = 0; }
            if (songTrackCountSim  && song->getTrackCount()  != songTrackCount  ) { songTrackCountSim  = false; songTrackCount  = 0; }
            if (songDiscNumberSim  && song->getDiscNumber()  != songDiscNumber  ) { songDiscNumberSim  = false; songDiscNumber  = 0; }
            if (songDiscCountSim   && song->getDiscCount()   != songDiscCount   ) { songDiscCountSim   = false; songDiscCount   = 0; }
            if (songBPMSim         && song->getBPM()         != songBPM         ) { songBPMSim         = false; songBPM         = 0; }
            if (songRatingSim      && song->getRating()      != songRating      ) { songRatingSim      = false; songRating      = 0; }

            if (songLanguageSim && song->getLanguage() != songLanguage)
            {
                songLanguageSim = false;
                songLanguage = LangUnknown;
            }

            if (songEnabledSim     && song->isEnabled()     != songEnabled    ) songEnabledSim     = false;
            if (songSkipShuffleSim && song->isSkipShuffle() != songSkipShuffle) songSkipShuffleSim = false;
            if (songCompilationSim && song->isCompilation() != songCompilation) songCompilationSim = false;
        }
    }

    qSort(plays.begin(), plays.end(), CDialogEditSongs::comparePlay);


    // Résumé
    if (durationMin == durationMax)
    {
        m_uiWidget->valueDuration->setText(tr("%1 (total: %2)").arg(durationToString(durationMin))
                                                               .arg(durationToString(durationTotal)));
    }
    else
    {
        m_uiWidget->valueDuration->setText(tr("between %1 and %2 (total: %3)").arg(durationToString(durationMin))
                                                                              .arg(durationToString(durationMax))
                                                                              .arg(durationToString(durationTotal)));
    }

    QString fileSizeMinStr = getFileSize(fileSizeMin);
    QString fileSizeMaxStr = getFileSize(fileSizeMax);

    if (fileSizeMinStr == fileSizeMaxStr)
    {
        m_uiWidget->valueFilesSize->setText(tr("%1 (total: %2)").arg(fileSizeMinStr)
                                                                .arg(getFileSize(fileSizeTotal)));
    }
    else
    {
        m_uiWidget->valueFilesSize->setText(tr("between %1 and %2 (total: %3)").arg(fileSizeMinStr)
                                                                               .arg(fileSizeMaxStr)
                                                                               .arg(getFileSize(fileSizeTotal)));
    }

    m_uiWidget->valueCreation->setText(tr("%1 - %2").arg(creationMin.toString("dd/MM/yyyy HH:mm:ss"))
                                                    .arg(creationMax.toString("dd/MM/yyyy HH:mm:ss")));

    m_uiWidget->valueModification->setText(tr("%1 - %2").arg(modificationMin.toString("dd/MM/yyyy HH:mm:ss"))
                                                        .arg(modificationMax.toString("dd/MM/yyyy HH:mm:ss")));

    bitRateAverage /= durationTotal;

    if (bitRateMin == bitRateMax)
    {
        m_uiWidget->valueBitRate->setText(tr("%1 kbit/s").arg(bitRateMin));
    }
    else
    {
        m_uiWidget->valueBitRate->setText(tr("%1 - %2 (average: %3)").arg(tr("%1 kbit/s").arg(bitRateMin))
                                                                     .arg(tr("%1 kbit/s").arg(bitRateMax))
                                                                     .arg(tr("%1 kbit/s").arg(bitRateAverage)));
    }

    // Formats
    QString formatStr;

    for (int f = 0; f < formats.size(); ++f)
    {
        CSong::TFormat m = CSong::FormatUnknown;
        int valMax = 0;

        for (QMap<CSong::TFormat, int>::Iterator it = formats.begin(); it != formats.end(); ++it)
        {
            if (it.value() > valMax)
            {
                valMax = it.value();
                m = it.key();
                it.value() = 0;
            }
        }

        if (!formatStr.isEmpty())
            formatStr += ", ";

        formatStr += CSong::getFormatName(m);
    }

    m_uiWidget->valueFormat->setText(formatStr);

    if (numChannelsMin == numChannelsMax)
    {
        m_uiWidget->valueChannels->setText(QString::number(numChannelsMin));
    }
    else
    {
        m_uiWidget->valueChannels->setText(QString("%1 - %2").arg(numChannelsMin)
                                                             .arg(numChannelsMax));
    }

    if (sampleRateMin == sampleRateMax)
    {
        m_uiWidget->valueSampleRate->setText(tr("%1 Hz").arg(sampleRateMax));
    }
    else
    {
        m_uiWidget->valueSampleRate->setText(tr("%1 - %2").arg(tr("%1 Hz").arg(sampleRateMin))
                                                          .arg(tr("%1 Hz").arg(sampleRateMax)));
    }

    if (plays.isEmpty())
        m_uiWidget->valueLastPlayTime->setText(tr("never"));
    else
        m_uiWidget->valueLastPlayTime->setText(plays.first().time.toString("dd/MM/yyyy HH:mm:ss"));

    m_uiWidget->valuePlayCount->setText(QString::number(plays.size()));


    // Lectures
    QStandardItemModel * model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList() << tr("Song") << tr("Local time") << tr("UTC"));
    m_uiWidget->listPlays->setModel(model);

    for (QList<TPlay>::ConstIterator it = plays.begin(); it != plays.end(); ++it)
    {
        QList<QStandardItem *> itemList;

        const QString songTitle = it->song->getTitle();
        const QString songArtist = it->song->getArtistName();
        const QString songAlbum = it->song->getAlbumTitle();

        if (songAlbum.isEmpty())
        {
            if (songArtist.isEmpty())
                itemList << new QStandardItem(songTitle);
            else
                itemList << new QStandardItem(songArtist + " - " + songTitle);
        }
        else
        {
            if (songArtist.isEmpty())
                itemList << new QStandardItem(QString("%1 (%2)").arg(songTitle).arg(songAlbum));
            else
                itemList << new QStandardItem(QString("%1 - %2 (%3)").arg(songArtist).arg(songTitle).arg(songAlbum));
        }

        itemList << new QStandardItem(it->time.toString(tr("dd/MM/yyyy HH:mm:ss")));
        itemList << new QStandardItem(it->timeUTC.toString(tr("dd/MM/yyyy HH:mm:ss")));
        model->appendRow(itemList);
    }

    m_uiWidget->listPlays->resizeColumnsToContents();


    const QString notSimText = tr("Different values");


    // Titre
    m_editTitle = new CWidgetLineEditMultiple();
    m_editTitle->setContent(songTitle_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutInfosTitle->addWidget(m_editTitle, 1, 1, 1, 1);
    connect(m_editTitle, SIGNAL(textChanged(const QString&)), this, SLOT(onTitleChange(const QString&)));

    m_editTitle2 = new CWidgetLineEditMultiple();
    m_editTitle2->setContent(songTitle_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutSorting->addWidget(m_editTitle2, 1, 1, 1, 1);
    connect(m_editTitle2, SIGNAL(textChanged(const QString&)), this, SLOT(onTitleChange(const QString&)));

    connect(m_editTitle, SIGNAL(textChanged(const QString&)), m_editTitle2, SLOT(setText(const QString&)));
    connect(m_editTitle2, SIGNAL(textChanged(const QString&)), m_editTitle, SLOT(setText(const QString&)));
/*
    if (songTitle_V2.size() > 1)
    {
        m_editTitleCB = new QComboBox(m_uiWidget->tabInfos);
        m_editTitleCB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editTitleCB->setEditable(true);
        m_editTitleLE = m_editTitleCB->lineEdit();
        m_editTitleCB->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutInfosTitle->addWidget(m_editTitleCB, 1, 1, 1, 1);

        m_editTitleCB_2 = new QComboBox(m_uiWidget->tabSorting);
        m_editTitleCB_2->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editTitleCB_2->setEditable(true);
        m_editTitleLE_2 = m_editTitleCB_2->lineEdit();
        m_editTitleCB_2->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutSorting->addWidget(m_editTitleCB_2, 1, 1, 1, 1);

        for (QMap<QString, int>::ConstIterator it = songTitle_V2.begin(); it != songTitle_V2.end(); ++it)
        {
            m_editTitleCB->addItem(it.key());
            m_editTitleCB_2->addItem(it.key());
        }

        m_editTitleCB->setCurrentIndex(-1);
        m_editTitleCB_2->setCurrentIndex(-1);

        m_editTitleLE->setPlaceholderText(notSimText);
        m_editTitleLE_2->setPlaceholderText(notSimText);

        connect(m_editTitleCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(onTitleChange(const QString&)));
        connect(m_editTitleCB_2, SIGNAL(editTextChanged(const QString&)), this, SLOT(onTitleChange(const QString&)));

        connect(m_editTitleCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onTitleChange(const QString&)));
        connect(m_editTitleCB_2, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onTitleChange(const QString&)));
    }
    else
    {
        m_editTitleLE = new QLineEdit(m_uiWidget->tabInfos);
        m_uiWidget->layoutInfosTitle->addWidget(m_editTitleLE, 1, 1, 1, 1);

        m_editTitleLE_2 = new QLineEdit(m_uiWidget->tabSorting);
        m_uiWidget->layoutSorting->addWidget(m_editTitleLE_2, 1, 1, 1, 1);

        if (songTitle_V2.size() == 1)
        {
            const QString songTitle_V2_Song = songTitle_V2.keys().at(0);
            m_editTitleLE->setText(songTitle_V2_Song);
            m_editTitleLE_2->setText(songTitle_V2_Song);
        }

        connect(m_editTitleLE, SIGNAL(textEdited(const QString&)), this, SLOT(onTitleChange(const QString&)));
        connect(m_editTitleLE_2, SIGNAL(textEdited(const QString&)), this, SLOT(onTitleChange(const QString&)));
    }

    connect(m_editTitleLE, SIGNAL(textEdited(const QString&)), m_editTitleLE_2, SLOT(setText(const QString&)));
    connect(m_editTitleLE_2, SIGNAL(textEdited(const QString&)), m_editTitleLE, SLOT(setText(const QString&)));
*/
    connect(m_uiWidget->chTitle, SIGNAL(clicked(bool)), this, SLOT(onTitleChecked(bool)));
    connect(m_uiWidget->chTitle_2, SIGNAL(clicked(bool)), this, SLOT(onTitleChecked(bool)));


    // Titre pour le tri
    m_editTitleSort = new CWidgetLineEditMultiple();
    m_editTitleSort->setContent(songTitleSort_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutSorting->addWidget(m_editTitleSort, 1, 3, 1, 1);
    connect(m_editTitleSort, SIGNAL(textChanged(const QString&)), this, SLOT(onTitleSortChange(const QString&)));
/*
    if (songTitleSort_V2.size() > 1)
    {
        m_editTitleSortCB = new QComboBox(m_uiWidget->tabSorting);
        m_editTitleSortCB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editTitleSortCB->setEditable(true);
        m_editTitleSortLE = m_editTitleSortCB->lineEdit();
        m_editTitleSortCB->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutSorting->addWidget(m_editTitleSortCB, 1, 3, 1, 1);

        for (QMap<QString, int>::ConstIterator it = songTitleSort_V2.begin(); it != songTitleSort_V2.end(); ++it)
        {
            m_editTitleSortCB->addItem(it.key());
        }

        m_editTitleSortCB->setCurrentIndex(-1);
        m_editTitleSortLE->setPlaceholderText(notSimText);

        connect(m_editTitleSortCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(onTitleSortChange(const QString&)));
        connect(m_editTitleSortCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onTitleSortChange(const QString&)));
    }
    else
    {
        m_editTitleSortLE = new QLineEdit(m_uiWidget->tabSorting);
        m_uiWidget->layoutSorting->addWidget(m_editTitleSortLE, 1, 3, 1, 1);

        if (songTitleSort_V2.size() == 1)
        {
            const QString songTitleSort_V2_Song = songTitleSort_V2.keys().at(0);
            m_editTitleSortLE->setText(songTitleSort_V2_Song);
        }

        connect(m_editTitleSortLE, SIGNAL(textEdited(const QString&)), this, SLOT(onTitleSortChange(const QString&)));
    }
*/
    connect(m_uiWidget->chTitleSort, SIGNAL(clicked(bool)), this, SLOT(onTitleSortChecked(bool)));


    // Artiste
    m_editArtist = new CWidgetLineEditMultiple();
    m_editArtist->setContent(songArtist_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutInfos->addWidget(m_editArtist, 2, 1, 1, 1);
    connect(m_editArtist, SIGNAL(textChanged(const QString&)), this, SLOT(onArtistChange(const QString&)));

    m_editArtist2 = new CWidgetLineEditMultiple();
    m_editArtist2->setContent(songArtist_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutSorting->addWidget(m_editArtist2, 3, 1, 1, 1);
    connect(m_editArtist2, SIGNAL(textChanged(const QString&)), this, SLOT(onArtistChange(const QString&)));

    connect(m_editArtist, SIGNAL(textChanged(const QString&)), m_editArtist2, SLOT(setText(const QString&)));
    connect(m_editArtist2, SIGNAL(textChanged(const QString&)), m_editArtist, SLOT(setText(const QString&)));
/*
    if (songArtist_V2.size() > 1)
    {
        m_editArtistCB = new QComboBox(m_uiWidget->tabInfos);
        m_editArtistCB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editArtistCB->setEditable(true);
        m_editArtistLE = m_editArtistCB->lineEdit();
        m_editArtistCB->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutInfos->addWidget(m_editArtistCB, 2, 1, 1, 1);

        m_editArtistCB_2 = new QComboBox(m_uiWidget->tabSorting);
        m_editArtistCB_2->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editArtistCB_2->setEditable(true);
        m_editArtistLE_2 = m_editArtistCB_2->lineEdit();
        m_editArtistCB_2->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutSorting->addWidget(m_editArtistCB_2, 3, 1, 1, 1);

        for (QMap<QString, int>::ConstIterator it = songArtist_V2.begin(); it != songArtist_V2.end(); ++it)
        {
            m_editArtistCB->addItem(it.key());
            m_editArtistCB_2->addItem(it.key());
        }

        m_editArtistCB->setCurrentIndex(-1);
        m_editArtistCB_2->setCurrentIndex(-1);

        m_editArtistLE->setPlaceholderText(notSimText);
        m_editArtistLE_2->setPlaceholderText(notSimText);

        connect(m_editArtistCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(onArtistChange(const QString&)));
        connect(m_editArtistCB_2, SIGNAL(editTextChanged(const QString&)), this, SLOT(onArtistChange(const QString&)));

        connect(m_editArtistCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onArtistChange(const QString&)));
        connect(m_editArtistCB_2, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onArtistChange(const QString&)));
    }
    else
    {
        m_editArtistLE = new QLineEdit(m_uiWidget->tabInfos);
        m_uiWidget->layoutInfos->addWidget(m_editArtistLE, 2, 1, 1, 1);

        m_editArtistLE_2 = new QLineEdit(m_uiWidget->tabSorting);
        m_uiWidget->layoutSorting->addWidget(m_editArtistLE_2, 3, 1, 1, 1);

        if (songArtist_V2.size() == 1)
        {
            const QString songArtist_V2_Song = songArtist_V2.keys().at(0);
            m_editArtistLE->setText(songArtist_V2_Song);
            m_editArtistLE_2->setText(songArtist_V2_Song);
        }

        connect(m_editArtistLE, SIGNAL(textEdited(const QString&)), this, SLOT(onArtistChange(const QString&)));
        connect(m_editArtistLE_2, SIGNAL(textEdited(const QString&)), this, SLOT(onArtistChange(const QString&)));
    }

    connect(m_editArtistLE, SIGNAL(textEdited(const QString&)), m_editArtistLE_2, SLOT(setText(const QString&)));
    connect(m_editArtistLE_2, SIGNAL(textEdited(const QString&)), m_editArtistLE, SLOT(setText(const QString&)));
*/
    connect(m_uiWidget->chArtist, SIGNAL(clicked(bool)), this, SLOT(onArtistChecked(bool)));
    connect(m_uiWidget->chArtist_2, SIGNAL(clicked(bool)), this, SLOT(onArtistChecked(bool)));


    // Artiste pour le tri
    m_editArtistSort = new CWidgetLineEditMultiple();
    m_editArtistSort->setContent(songArtistSort_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutSorting->addWidget(m_editArtistSort, 3, 3, 1, 1);
    connect(m_editArtistSort, SIGNAL(textChanged(const QString&)), this, SLOT(onArtistSortChange(const QString&)));
/*
    if (songArtistSort_V2.size() > 1)
    {
        m_editArtistSortCB = new QComboBox(m_uiWidget->tabSorting);
        m_editArtistSortCB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editArtistSortCB->setEditable(true);
        m_editArtistSortLE = m_editArtistSortCB->lineEdit();
        m_editArtistSortCB->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutSorting->addWidget(m_editArtistSortCB, 3, 3, 1, 1);

        for (QMap<QString, int>::ConstIterator it = songArtistSort_V2.begin(); it != songArtistSort_V2.end(); ++it)
        {
            m_editArtistSortCB->addItem(it.key());
        }

        m_editArtistSortCB->setCurrentIndex(-1);
        m_editArtistSortLE->setPlaceholderText(notSimText);

        connect(m_editArtistSortCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(onArtistSortChange(const QString&)));
        connect(m_editArtistSortCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onArtistSortChange(const QString&)));
    }
    else
    {
        m_editArtistSortLE = new QLineEdit(m_uiWidget->tabSorting);
        m_uiWidget->layoutSorting->addWidget(m_editArtistSortLE, 3, 3, 1, 1);

        if (songArtistSort_V2.size() == 1)
        {
            const QString songArtistSort_V2_Song = songArtistSort_V2.keys().at(0);
            m_editArtistSortLE->setText(songArtistSort_V2_Song);
        }

        connect(m_editArtistSortLE, SIGNAL(textEdited(const QString&)), this, SLOT(onArtistSortChange(const QString&)));
    }
*/
    connect(m_uiWidget->chArtistSort, SIGNAL(clicked(bool)), this, SLOT(onArtistSortChecked(bool)));


    // Album
    m_editAlbum = new CWidgetLineEditMultiple();
    m_editAlbum->setContent(songAlbum_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutInfos->addWidget(m_editAlbum, 4, 1, 1, 1);
    connect(m_editAlbum, SIGNAL(textChanged(const QString&)), this, SLOT(onAlbumChange(const QString&)));

    m_editAlbum2 = new CWidgetLineEditMultiple();
    m_editAlbum2->setContent(songAlbum_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutSorting->addWidget(m_editAlbum2, 5, 1, 1, 1);
    connect(m_editAlbum2, SIGNAL(textChanged(const QString&)), this, SLOT(onAlbumChange(const QString&)));

    connect(m_editAlbum, SIGNAL(textChanged(const QString&)), m_editAlbum2, SLOT(setText(const QString&)));
    connect(m_editAlbum2, SIGNAL(textChanged(const QString&)), m_editAlbum, SLOT(setText(const QString&)));
/*
    if (songAlbum_V2.size() > 1)
    {
        m_editAlbumCB = new QComboBox(m_uiWidget->tabInfos);
        m_editAlbumCB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editAlbumCB->setEditable(true);
        m_editAlbumLE = m_editAlbumCB->lineEdit();
        m_editAlbumCB->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutInfos->addWidget(m_editAlbumCB, 4, 1, 1, 1);

        m_editAlbumCB_2 = new QComboBox(m_uiWidget->tabSorting);
        m_editAlbumCB_2->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editAlbumCB_2->setEditable(true);
        m_editAlbumLE_2 = m_editAlbumCB_2->lineEdit();
        m_editAlbumCB_2->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutSorting->addWidget(m_editAlbumCB_2, 5, 1, 1, 1);

        for (QMap<QString, int>::ConstIterator it = songAlbum_V2.begin(); it != songAlbum_V2.end(); ++it)
        {
            m_editAlbumCB->addItem(it.key());
            m_editAlbumCB_2->addItem(it.key());
        }

        m_editAlbumCB->setCurrentIndex(-1);
        m_editAlbumCB_2->setCurrentIndex(-1);

        m_editAlbumLE->setPlaceholderText(notSimText);
        m_editAlbumLE_2->setPlaceholderText(notSimText);

        connect(m_editAlbumCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(onAlbumChange(const QString&)));
        connect(m_editAlbumCB_2, SIGNAL(editTextChanged(const QString&)), this, SLOT(onAlbumChange(const QString&)));

        connect(m_editAlbumCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onAlbumChange(const QString&)));
        connect(m_editAlbumCB_2, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onAlbumChange(const QString&)));
    }
    else
    {
        m_editAlbumLE = new QLineEdit(m_uiWidget->tabInfos);
        m_uiWidget->layoutInfos->addWidget(m_editAlbumLE, 4, 1, 1, 1);

        m_editAlbumLE_2 = new QLineEdit(m_uiWidget->tabSorting);
        m_uiWidget->layoutSorting->addWidget(m_editAlbumLE_2, 5, 1, 1, 1);

        if (songAlbum_V2.size() == 1)
        {
            const QString songAlbum_V2_Song = songAlbum_V2.keys().at(0);
            m_editAlbumLE->setText(songAlbum_V2_Song);
            m_editAlbumLE_2->setText(songAlbum_V2_Song);
        }

        connect(m_editAlbumLE, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumChange(const QString&)));
        connect(m_editAlbumLE_2, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumChange(const QString&)));
    }

    connect(m_editAlbumLE, SIGNAL(textEdited(const QString&)), m_editAlbumLE_2, SLOT(setText(const QString&)));
    connect(m_editAlbumLE_2, SIGNAL(textEdited(const QString&)), m_editAlbumLE, SLOT(setText(const QString&)));
*/
    connect(m_uiWidget->chAlbum, SIGNAL(clicked(bool)), this, SLOT(onAlbumChecked(bool)));
    connect(m_uiWidget->chAlbum_2, SIGNAL(clicked(bool)), this, SLOT(onAlbumChecked(bool)));


    // Album pour le tri
    m_editAlbumSort = new CWidgetLineEditMultiple();
    m_editAlbumSort->setContent(songAlbumSort_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutSorting->addWidget(m_editAlbumSort, 5, 3, 1, 1);
    connect(m_editAlbumSort, SIGNAL(textChanged(const QString&)), this, SLOT(onAlbumSortChange(const QString&)));
/*
    if (songAlbumSort_V2.size() > 1)
    {
        m_editAlbumSortCB = new QComboBox(m_uiWidget->tabSorting);
        m_editAlbumSortCB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editAlbumSortCB->setEditable(true);
        m_editAlbumSortLE = m_editAlbumSortCB->lineEdit();
        m_editAlbumSortCB->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutSorting->addWidget(m_editAlbumSortCB, 5, 3, 1, 1);

        for (QMap<QString, int>::ConstIterator it = songAlbumSort_V2.begin(); it != songAlbumSort_V2.end(); ++it)
        {
            m_editAlbumSortCB->addItem(it.key());
        }

        m_editAlbumSortCB->setCurrentIndex(-1);
        m_editAlbumSortLE->setPlaceholderText(notSimText);

        connect(m_editAlbumSortCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(onAlbumSortChange(const QString&)));
        connect(m_editAlbumSortCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onAlbumSortChange(const QString&)));
    }
    else
    {
        m_editAlbumSortLE = new QLineEdit(m_uiWidget->tabSorting);
        m_uiWidget->layoutSorting->addWidget(m_editAlbumSortLE, 5, 3, 1, 1);

        if (songAlbumSort_V2.size() == 1)
        {
            const QString songAlbumSort_V2_Song = songAlbumSort_V2.keys().at(0);
            m_editAlbumSortLE->setText(songAlbumSort_V2_Song);
        }

        connect(m_editAlbumSortLE, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumSortChange(const QString&)));
    }
*/
    connect(m_uiWidget->chAlbumSort, SIGNAL(clicked(bool)), this, SLOT(onAlbumSortChecked(bool)));


    // Artiste de l'album
    m_editAlbumArtist = new CWidgetLineEditMultiple();
    m_editAlbumArtist->setContent(songAlbumArtist_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutInfos->addWidget(m_editAlbumArtist, 6, 1, 1, 1);
    connect(m_editAlbumArtist, SIGNAL(textChanged(const QString&)), this, SLOT(onAlbumArtistChange(const QString&)));

    m_editAlbumArtist2 = new CWidgetLineEditMultiple();
    m_editAlbumArtist2->setContent(songAlbumArtist_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutSorting->addWidget(m_editAlbumArtist2, 7, 1, 1, 1);
    connect(m_editAlbumArtist2, SIGNAL(textChanged(const QString&)), this, SLOT(onAlbumArtistChange(const QString&)));

    connect(m_editAlbumArtist, SIGNAL(textChanged(const QString&)), m_editAlbumArtist2, SLOT(setText(const QString&)));
    connect(m_editAlbumArtist2, SIGNAL(textChanged(const QString&)), m_editAlbumArtist, SLOT(setText(const QString&)));
/*
    if (songAlbumArtist_V2.size() > 1)
    {
        m_editAlbumArtistCB = new QComboBox(m_uiWidget->tabInfos);
        m_editAlbumArtistCB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editAlbumArtistCB->setEditable(true);
        m_editAlbumArtistLE = m_editAlbumArtistCB->lineEdit();
        m_editAlbumArtistCB->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutInfos->addWidget(m_editAlbumArtistCB, 6, 1, 1, 1);

        m_editAlbumArtistCB_2 = new QComboBox(m_uiWidget->tabSorting);
        m_editAlbumArtistCB_2->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editAlbumArtistCB_2->setEditable(true);
        m_editAlbumArtistLE_2 = m_editAlbumArtistCB_2->lineEdit();
        m_editAlbumArtistCB_2->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutSorting->addWidget(m_editAlbumArtistCB_2, 7, 1, 1, 1);

        for (QMap<QString, int>::ConstIterator it = songAlbumArtist_V2.begin(); it != songAlbumArtist_V2.end(); ++it)
        {
            m_editAlbumArtistCB->addItem(it.key());
            m_editAlbumArtistCB_2->addItem(it.key());
        }

        m_editAlbumArtistCB->setCurrentIndex(-1);
        m_editAlbumArtistCB_2->setCurrentIndex(-1);

        m_editAlbumArtistLE->setPlaceholderText(notSimText);
        m_editAlbumArtistLE_2->setPlaceholderText(notSimText);

        connect(m_editAlbumArtistCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(onAlbumArtistChange(const QString&)));
        connect(m_editAlbumArtistCB_2, SIGNAL(editTextChanged(const QString&)), this, SLOT(onAlbumArtistChange(const QString&)));

        connect(m_editAlbumArtistCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onAlbumArtistChange(const QString&)));
        connect(m_editAlbumArtistCB_2, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onAlbumArtistChange(const QString&)));
    }
    else
    {
        m_editAlbumArtistLE = new QLineEdit(m_uiWidget->tabInfos);
        m_uiWidget->layoutInfos->addWidget(m_editAlbumArtistLE, 6, 1, 1, 1);

        m_editAlbumArtistLE_2 = new QLineEdit(m_uiWidget->tabSorting);
        m_uiWidget->layoutSorting->addWidget(m_editAlbumArtistLE_2, 7, 1, 1, 1);

        if (songAlbumArtist_V2.size() == 1)
        {
            const QString songAlbumArtist_V2_Song = songAlbumArtist_V2.keys().at(0);
            m_editAlbumArtistLE->setText(songAlbumArtist_V2_Song);
            m_editAlbumArtistLE_2->setText(songAlbumArtist_V2_Song);
        }

        connect(m_editAlbumArtistLE, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumArtistChange(const QString&)));
        connect(m_editAlbumArtistLE_2, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumArtistChange(const QString&)));
    }

    connect(m_editAlbumArtistLE, SIGNAL(textEdited(const QString&)), m_editAlbumArtistLE_2, SLOT(setText(const QString&)));
    connect(m_editAlbumArtistLE_2, SIGNAL(textEdited(const QString&)), m_editAlbumArtistLE, SLOT(setText(const QString&)));
*/
    connect(m_uiWidget->chAlbumArtist, SIGNAL(clicked(bool)), this, SLOT(onAlbumArtistChecked(bool)));
    connect(m_uiWidget->chAlbumArtist_2, SIGNAL(clicked(bool)), this, SLOT(onAlbumArtistChecked(bool)));


    // Artiste de l'album pour le tri
    m_editAlbumArtistSort = new CWidgetLineEditMultiple();
    m_editAlbumArtistSort->setContent(songAlbumArtistSort_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutSorting->addWidget(m_editAlbumArtistSort, 7, 3, 1, 1);
    connect(m_editAlbumArtistSort, SIGNAL(textChanged(const QString&)), this, SLOT(onAlbumArtistSortChange(const QString&)));
/*
    if (songAlbumArtistSort_V2.size() > 1)
    {
        m_editAlbumArtistSortCB = new QComboBox(m_uiWidget->tabSorting);
        m_editAlbumArtistSortCB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editAlbumArtistSortCB->setEditable(true);
        m_editAlbumArtistSortLE = m_editAlbumArtistSortCB->lineEdit();
        m_editAlbumArtistSortCB->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutSorting->addWidget(m_editAlbumArtistSortCB, 7, 3, 1, 1);

        for (QMap<QString, int>::ConstIterator it = songAlbumArtistSort_V2.begin(); it != songAlbumArtistSort_V2.end(); ++it)
        {
            m_editAlbumArtistSortCB->addItem(it.key());
        }

        m_editAlbumArtistSortCB->setCurrentIndex(-1);
        m_editAlbumArtistSortLE->setPlaceholderText(notSimText);

        connect(m_editAlbumArtistSortCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(onAlbumArtistSortChange(const QString&)));
        connect(m_editAlbumArtistSortCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onAlbumArtistSortChange(const QString&)));
    }
    else
    {
        m_editAlbumArtistSortLE = new QLineEdit(m_uiWidget->tabSorting);
        m_uiWidget->layoutSorting->addWidget(m_editAlbumArtistSortLE, 7, 3, 1, 1);

        if (songAlbumArtistSort_V2.size() == 1)
        {
            const QString songAlbumArtistSort_V2_Song = songAlbumArtistSort_V2.keys().at(0);
            m_editAlbumArtistSortLE->setText(songAlbumArtistSort_V2_Song);
        }

        connect(m_editAlbumArtistSortLE, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumArtistSortChange(const QString&)));
    }
*/
    connect(m_uiWidget->chAlbumArtistSort, SIGNAL(clicked(bool)), this, SLOT(onAlbumArtistSortChecked(bool)));


    // Compositeur
    m_editComposer = new CWidgetLineEditMultiple();
    m_editComposer->setContent(songComposer_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutInfos->addWidget(m_editComposer, 10, 1, 1, 6);
    connect(m_editComposer, SIGNAL(textChanged(const QString&)), this, SLOT(onComposerChange(const QString&)));

    m_editComposer2 = new CWidgetLineEditMultiple();
    m_editComposer2->setContent(songComposer_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutSorting->addWidget(m_editComposer2, 9, 1, 1, 1);
    connect(m_editComposer2, SIGNAL(textChanged(const QString&)), this, SLOT(onComposerChange(const QString&)));

    connect(m_editComposer, SIGNAL(textChanged(const QString&)), m_editComposer2, SLOT(setText(const QString&)));
    connect(m_editComposer2, SIGNAL(textChanged(const QString&)), m_editComposer, SLOT(setText(const QString&)));
/*
    if (songComposer_V2.size() > 1)
    {
        m_editComposerCB = new QComboBox(m_uiWidget->tabInfos);
        m_editComposerCB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editComposerCB->setEditable(true);
        m_editComposerLE = m_editComposerCB->lineEdit();
        m_editComposerCB->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutInfos->addWidget(m_editComposerCB, 10, 1, 1, 6);

        m_editComposerCB_2 = new QComboBox(m_uiWidget->tabSorting);
        m_editComposerCB_2->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editComposerCB_2->setEditable(true);
        m_editComposerLE_2 = m_editComposerCB_2->lineEdit();
        m_editComposerCB_2->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutSorting->addWidget(m_editComposerCB_2, 9, 1, 1, 1);

        for (QMap<QString, int>::ConstIterator it = songComposer_V2.begin(); it != songComposer_V2.end(); ++it)
        {
            m_editComposerCB->addItem(it.key());
            m_editComposerCB_2->addItem(it.key());
        }

        m_editComposerCB->setCurrentIndex(-1);
        m_editComposerCB_2->setCurrentIndex(-1);

        m_editComposerLE->setPlaceholderText(notSimText);
        m_editComposerLE_2->setPlaceholderText(notSimText);

        connect(m_editComposerCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(onComposerChange(const QString&)));
        connect(m_editComposerCB_2, SIGNAL(editTextChanged(const QString&)), this, SLOT(onComposerChange(const QString&)));

        connect(m_editComposerCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onComposerChange(const QString&)));
        connect(m_editComposerCB_2, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onComposerChange(const QString&)));
    }
    else
    {
        m_editComposerLE = new QLineEdit(m_uiWidget->tabInfos);
        m_uiWidget->layoutInfos->addWidget(m_editComposerLE, 10, 1, 1, 6);

        m_editComposerLE_2 = new QLineEdit(m_uiWidget->tabSorting);
        m_uiWidget->layoutSorting->addWidget(m_editComposerLE_2, 9, 1, 1, 1);

        if (songComposer_V2.size() == 1)
        {
            const QString songComposer_V2_Song = songComposer_V2.keys().at(0);
            m_editComposerLE->setText(songComposer_V2_Song);
            m_editComposerLE_2->setText(songComposer_V2_Song);
        }

        connect(m_editComposerLE, SIGNAL(textEdited(const QString&)), this, SLOT(onComposerChange(const QString&)));
        connect(m_editComposerLE_2, SIGNAL(textEdited(const QString&)), this, SLOT(onComposerChange(const QString&)));
    }

    connect(m_editComposerLE, SIGNAL(textEdited(const QString&)), m_editComposerLE_2, SLOT(setText(const QString&)));
    connect(m_editComposerLE_2, SIGNAL(textEdited(const QString&)), m_editComposerLE, SLOT(setText(const QString&)));
*/
    connect(m_uiWidget->chComposer, SIGNAL(clicked(bool)), this, SLOT(onComposerChecked(bool)));
    connect(m_uiWidget->chComposer_2, SIGNAL(clicked(bool)), this, SLOT(onComposerChecked(bool)));


    // Compositeur pour le tri
    m_editComposerSort = new CWidgetLineEditMultiple();
    m_editComposerSort->setContent(songComposerSort_V2.uniqueKeys(), notSimText);
    m_uiWidget->layoutSorting->addWidget(m_editComposerSort, 9, 3, 1, 1);
    connect(m_editComposerSort, SIGNAL(textChanged(const QString&)), this, SLOT(onComposerSortChange(const QString&)));
/*
    if (songComposerSort_V2.size() > 1)
    {
        m_editComposerSortCB = new QComboBox(m_uiWidget->tabSorting);
        m_editComposerSortCB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_editComposerSortCB->setEditable(true);
        m_editComposerSortLE = m_editComposerSortCB->lineEdit();
        m_editComposerSortCB->setInsertPolicy(QComboBox::NoInsert);
        m_uiWidget->layoutSorting->addWidget(m_editComposerSortCB, 9, 3, 1, 1);

        for (QMap<QString, int>::ConstIterator it = songComposerSort_V2.begin(); it != songComposerSort_V2.end(); ++it)
        {
            m_editComposerSortCB->addItem(it.key());
        }

        m_editComposerSortCB->setCurrentIndex(-1);
        m_editComposerSortLE->setPlaceholderText(notSimText);

        connect(m_editComposerSortCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(onComposerSortChange(const QString&)));
        connect(m_editComposerSortCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onComposerSortChange(const QString&)));
    }
    else
    {
        m_editComposerSortLE = new QLineEdit(m_uiWidget->tabSorting);
        m_uiWidget->layoutSorting->addWidget(m_editComposerSortLE, 9, 3, 1, 1);

        if (songComposerSort_V2.size() == 1)
        {
            const QString songComposerSort_V2_Song = songComposerSort_V2.keys().at(0);
            m_editComposerSortLE->setText(songComposerSort_V2_Song);
        }

        connect(m_editComposerSortLE, SIGNAL(textEdited(const QString&)), this, SLOT(onComposerSortChange(const QString&)));
    }
*/
    connect(m_uiWidget->chComposerSort, SIGNAL(clicked(bool)), this, SLOT(onComposerSortChecked(bool)));


    // Sous-titre
    m_uiWidget->editSubTitle->setText(songSubTitle);

    if (!songSubTitleSim)
    {
        m_uiWidget->editSubTitle->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editSubTitle, SIGNAL(textEdited(const QString&)), this, SLOT(onSubTitleChange(const QString&)));
    connect(m_uiWidget->chSubTitle, SIGNAL(clicked(bool)), this, SLOT(onSubTitleChecked(bool)));

    // Regroupement
    m_uiWidget->editGrouping->setText(songGrouping);

    if (!songGroupingSim)
    {
        m_uiWidget->editGrouping->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editGrouping, SIGNAL(textEdited(const QString&)), this, SLOT(onGroupingChange(const QString&)));
    connect(m_uiWidget->chGrouping, SIGNAL(clicked(bool)), this, SLOT(onGroupingChecked(bool)));

    // Commentaires
    m_uiWidget->editComments->setText(songComments);

    if (!songCommentsSim)
    {
        m_uiWidget->editComments->setText("<span style='color:grey;'>" + notSimText + "</span>");
        m_differentComments = true;
    }

    connect(m_uiWidget->editComments, SIGNAL(textChanged()), this, SLOT(onCommentsChange()));
    connect(m_uiWidget->chComments, SIGNAL(clicked(bool)), this, SLOT(onCommentsChecked(bool)));

    // Année
    m_uiWidget->editYear->setText(songYear > 0 ? QString::number(songYear) : QString());
    m_uiWidget->editYear->setValidator(new QIntValidator(0, 9999, this));

    if (!songYearSim)
    {
        m_uiWidget->editYear->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editYear, SIGNAL(textEdited(const QString&)), this, SLOT(onYearChange(const QString&)));
    connect(m_uiWidget->chYear, SIGNAL(clicked(bool)), this, SLOT(onYearChecked(bool)));

    // Numéro de piste
    m_uiWidget->editTrackNumber->setText(songTrackNumber > 0 ? QString::number(songTrackNumber) : QString());
    m_uiWidget->editTrackNumber->setValidator(new QIntValidator(0, 999, this));

    if (!songTrackNumberSim)
    {
        m_uiWidget->editTrackNumber->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editTrackNumber, SIGNAL(textEdited(const QString&)), this, SLOT(onTrackNumberChange(const QString&)));
    connect(m_uiWidget->chTrackNumber, SIGNAL(clicked(bool)), this, SLOT(onTrackNumberChecked(bool)));

    // Nombre de pistes
    m_uiWidget->editTrackCount->setText(songTrackCount > 0 ? QString::number(songTrackCount) : QString());
    m_uiWidget->editTrackCount->setValidator(new QIntValidator(0, 999, this));

    if (!songTrackCountSim)
    {
        m_uiWidget->editTrackCount->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editTrackCount, SIGNAL(textEdited(const QString&)), this, SLOT(onTrackCountChange(const QString&)));
    connect(m_uiWidget->chTrackCount, SIGNAL(clicked(bool)), this, SLOT(onTrackCountChecked(bool)));

    // Numéro de disque
    m_uiWidget->editDiscNumber->setText(songDiscNumber > 0 ? QString::number(songDiscNumber) : QString());
    m_uiWidget->editDiscNumber->setValidator(new QIntValidator(0, 999, this));

    if (!songDiscNumberSim)
    {
        m_uiWidget->editDiscNumber->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editDiscNumber, SIGNAL(textEdited(const QString&)), this, SLOT(onDiscNumberChange(const QString&)));
    connect(m_uiWidget->chDiscNumber, SIGNAL(clicked(bool)), this, SLOT(onDiscNumberChecked(bool)));

    // Nombre de disques
    m_uiWidget->editDiscCount->setText(songDiscCount > 0 ? QString::number(songDiscCount) : QString());
    m_uiWidget->editDiscCount->setValidator(new QIntValidator(0, 999, this));

    if (!songDiscCountSim)
    {
        m_uiWidget->editDiscCount->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editDiscCount, SIGNAL(textEdited(const QString&)), this, SLOT(onDiscCountChange(const QString&)));
    connect(m_uiWidget->chDiscCount, SIGNAL(clicked(bool)), this, SLOT(onDiscCountChecked(bool)));

    // BPM
    m_uiWidget->editBPM->setText(songBPM > 0 ? QString::number(songBPM) : QString());
    m_uiWidget->editBPM->setValidator(new QIntValidator(0, 999, this));

    if (!songBPMSim)
    {
        m_uiWidget->editBPM->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editBPM, SIGNAL(textEdited(const QString&)), this, SLOT(onBPMChange(const QString&)));
    connect(m_uiWidget->chBPM, SIGNAL(clicked(bool)), this, SLOT(onBPMChecked(bool)));

    // Genre
    QStringList genres = mainWindow->getMediaManager()->getGenreList();
    m_uiWidget->editGenre->addItems(genres);
    m_uiWidget->editGenre->lineEdit()->setText(songGenre);
    m_uiWidget->editGenre->setCurrentIndex(m_uiWidget->editGenre->findText(songGenre));

    if (!songGenreSim)
    {
        m_uiWidget->editGenre->lineEdit()->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editGenre, SIGNAL(editTextChanged(const QString&)), this, SLOT(onGenreChange(const QString&)));
    connect(m_uiWidget->editGenre, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onGenreChange(const QString&)));
    connect(m_uiWidget->chGenre, SIGNAL(clicked(bool)), this, SLOT(onGenreChecked(bool)));

    // Note
    m_editRating = new CSpecialSpinBox();
    //m_editRating->setObjectName(QString::fromUtf8("editRating"));
    m_editRating->setMaximum(5);
    m_uiWidget->layoutInfos->addWidget(m_editRating, 14, 3, 1, 4);

    if (songRatingSim)
    {
        m_editRating->setValue(songRating);
    }
    else
    {
        m_editRating->setSpecialValue(10);
        m_editRating->setPlaceholderText(notSimText);
    }

    connect(m_editRating, SIGNAL(valueChanged(int)), this, SLOT(onRatingChange(int)));
    connect(m_uiWidget->chRating, SIGNAL(clicked(bool)), this, SLOT(onGenreChecked(bool)));

    // Paroles
    m_uiWidget->editLyrics->setText(songLyrics);

    if (!songLyricsSim)
    {
        m_uiWidget->editLyrics->setText("<span style='color:grey;'>" + notSimText + "</span>");
        m_differentLyrics = true;
    }

    connect(m_uiWidget->editLyrics, SIGNAL(textChanged()), this, SLOT(onLyricsChange()));
    connect(m_uiWidget->chLyrics, SIGNAL(clicked(bool)), this, SLOT(onLyricsChecked(bool)));

    // Langue
    if (songLanguageSim)
    {
        m_uiWidget->editLanguage->setCurrentIndex(songLanguage);
    }
    else
    {
        m_uiWidget->editLanguage->setCurrentIndex(-1);
    }

    connect(m_uiWidget->editLanguage, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onLanguageChange(const QString&)));
    connect(m_uiWidget->chLanguage, SIGNAL(clicked(bool)), this, SLOT(onLanguageChecked(bool)));

    // Parolier
    m_uiWidget->editLyricist->setText(songLyricist);

    if (!songLyricistSim)
    {
        m_uiWidget->editLyricist->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editLyricist, SIGNAL(textEdited(const QString&)), this, SLOT(onLyricistChange(const QString&)));
    connect(m_uiWidget->chLyricist, SIGNAL(clicked(bool)), this, SLOT(onLyricistChecked(bool)));

    // Morceau coché
    if (songEnabledSim)
    {
        m_uiWidget->editEnabled->setChecked(songEnabled);
    }
    else
    {
        m_uiWidget->editEnabled->setCheckState(Qt::PartiallyChecked);
    }

    // Ne pas lire en mode aléatoire
    if (songSkipShuffleSim)
    {
        m_uiWidget->editSkipShuffle->setChecked(songSkipShuffle);
    }
    else
    {
        m_uiWidget->editSkipShuffle->setCheckState(Qt::PartiallyChecked);
    }

    // Compilation
    if (songCompilationSim)
    {
        m_uiWidget->editCompilation->setChecked(songCompilation);
    }
    else
    {
        m_uiWidget->editCompilation->setCheckState(Qt::PartiallyChecked);
    }


    connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)), this, SLOT(onFocusChange(QWidget *, QWidget *)));


    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    QPushButton * btnApply = m_uiWidget->buttonBox->addButton(tr("Apply"), QDialogButtonBox::ApplyRole);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
}


/**
 * Détruit la boite de dialogue.
 */

CDialogEditSongs::~CDialogEditSongs()
{
    delete m_uiWidget;
}


/**
 * Enregistre les modifications effectuées sur les morceaux.
 */

void CDialogEditSongs::apply()
{
    QProgressDialog progress(tr("Apply changes..."), tr("Abort"), 0, m_songItemList.size(), this);
    progress.setModal(true);
    progress.setMinimumDuration(2000);
    int i = 0;

    // Modification de chaque morceau
    for (QList<CMediaTableItem *>::ConstIterator it = m_songItemList.begin(); it != m_songItemList.end(); ++it)
    {
        progress.setValue(i++);
        CSong * song = (*it)->getSong();

        if (m_uiWidget->chTitle->isChecked())
        {
            song->setTitle(m_editTitle->text());
        }

        if (m_uiWidget->chSubTitle->isChecked())
        {
            song->setSubTitle(m_uiWidget->editSubTitle->text());
        }

        if (m_uiWidget->chGrouping->isChecked())
        {
            song->setGrouping(m_uiWidget->editGrouping->text());
        }

        if (m_uiWidget->chArtist->isChecked())
        {
            song->setArtistName(m_editArtist->text());
        }

        if (m_uiWidget->chAlbum->isChecked())
        {
            song->setAlbumTitle(m_editAlbum->text());
        }

        if (m_uiWidget->chAlbumArtist->isChecked())
        {
            song->setAlbumArtist(m_editAlbumArtist->text());
        }

        if (m_uiWidget->chComposer->isChecked())
        {
            song->setComposer(m_editComposer->text());
        }

        if (m_uiWidget->chTitleSort->isChecked())
        {
            song->setTitleSort(m_editTitleSort->text());
        }

        if (m_uiWidget->chArtistSort->isChecked())
        {
            song->setArtistNameSort(m_editArtistSort->text());
        }

        if (m_uiWidget->chAlbumSort->isChecked())
        {
            song->setAlbumTitleSort(m_editAlbumSort->text());
        }

        if (m_uiWidget->chAlbumArtistSort->isChecked())
        {
            song->setAlbumArtistSort(m_editAlbumArtistSort->text());
        }

        if (m_uiWidget->chComposerSort->isChecked())
        {
            song->setComposerSort(m_editComposerSort->text());
        }

        if (m_uiWidget->chBPM->isChecked())
        {
            song->setBPM(m_uiWidget->editBPM->text().toInt());
        }

        if (m_uiWidget->chYear->isChecked())
        {
            song->setYear(m_uiWidget->editYear->text().toInt());
        }

        if (m_uiWidget->chTrackNumber->isChecked())
        {
            song->setTrackNumber(m_uiWidget->editTrackNumber->text().toInt());
        }

        if (m_uiWidget->chTrackCount->isChecked())
        {
            song->setTrackCount(m_uiWidget->editTrackCount->text().toInt());
        }

        if (m_uiWidget->chDiscNumber->isChecked())
        {
            song->setDiscNumber(m_uiWidget->editDiscNumber->text().toInt());
        }

        if (m_uiWidget->chDiscCount->isChecked())
        {
            song->setDiscCount(m_uiWidget->editDiscCount->text().toInt());
        }

        if (m_uiWidget->chComments->isChecked())
        {
            song->setComments(m_uiWidget->editComments->toPlainText());
        }

        if (m_uiWidget->chGenre->isChecked())
        {
            song->setGenre(m_uiWidget->editGenre->currentText());
        }

        if (m_uiWidget->chRating->isChecked())
        {
            song->setRating(m_editRating->value());
        }

        if (m_uiWidget->chLyrics->isChecked())
        {
            song->setLyrics(m_uiWidget->editLyrics->toPlainText());
        }

        if (m_uiWidget->chLanguage->isChecked())
        {
            song->setLanguage(getLanguageFromInteger(m_uiWidget->editLanguage->currentIndex()));
        }

        if (m_uiWidget->chLyricist->isChecked())
        {
            song->setLyricist(m_uiWidget->editLyricist->text());
        }

        if (m_uiWidget->editEnabled->checkState() != Qt::PartiallyChecked)
        {
            song->setEnabled(m_uiWidget->editEnabled->isChecked());
        }

        if (m_uiWidget->editSkipShuffle->checkState() != Qt::PartiallyChecked)
        {
            song->setSkipShuffle(m_uiWidget->editSkipShuffle->isChecked());
        }

        if (m_uiWidget->editCompilation->checkState() != Qt::PartiallyChecked)
        {
            song->setCompilation(m_uiWidget->editCompilation->isChecked());
        }

        song->writeTags();
        qApp->processEvents();

        song->updateDatabase();
        qApp->processEvents();

        if (progress.wasCanceled())
        {
            break;
        }
    }
}


/**
 * Enregistre les modifications effectuées sur les morceaux et ferme la boite de dialogue.
 */

void CDialogEditSongs::save()
{
    apply();
    close();
}


void CDialogEditSongs::onTitleChange(const QString& title)
{
    m_editTitle->setPlaceholderText(QString());
    m_editTitle2->setPlaceholderText(QString());

    m_uiWidget->chTitle->setChecked(true);
    m_uiWidget->chTitle_2->setChecked(true);

    // Synchronisation pour les combo-box
    if (m_editTitle->text() != title)
    {
        m_editTitle->setText(title);
    }

    if (m_editTitle2->text() != title)
    {
        m_editTitle2->setText(title);
    }
}


void CDialogEditSongs::onTitleSortChange(const QString& title)
{
    m_editTitleSort->setPlaceholderText(QString());
    m_uiWidget->chTitleSort->setChecked(true);
}


void CDialogEditSongs::onArtistChange(const QString& artistName)
{
    m_editArtist->setPlaceholderText(QString());
    m_editArtist2->setPlaceholderText(QString());

    m_uiWidget->chArtist->setChecked(true);
    m_uiWidget->chArtist_2->setChecked(true);

    // Synchronisation pour les combo-box
    if (m_editArtist->text() != artistName)
        m_editArtist->setText(artistName);

    if (m_editArtist2->text() != artistName)
        m_editArtist2->setText(artistName);
}


void CDialogEditSongs::onArtistSortChange(const QString& artistName)
{
    m_editArtistSort->setPlaceholderText(QString());
    m_uiWidget->chArtistSort->setChecked(true);
}


void CDialogEditSongs::onAlbumChange(const QString& albumTitle)
{
    m_editAlbum->setPlaceholderText(QString());
    m_editAlbum2->setPlaceholderText(QString());

    m_uiWidget->chAlbum->setChecked(true);
    m_uiWidget->chAlbum_2->setChecked(true);

    // Synchronisation pour les combo-box
    if (m_editAlbum->text() != albumTitle)
        m_editAlbum->setText(albumTitle);

    if (m_editAlbum2->text() != albumTitle)
        m_editAlbum2->setText(albumTitle);
}


void CDialogEditSongs::onAlbumSortChange(const QString& albumTitle)
{
    m_editAlbumSort->setPlaceholderText(QString());
    m_uiWidget->chAlbumSort->setChecked(true);
}


void CDialogEditSongs::onAlbumArtistChange(const QString& albumArtist)
{
    m_editAlbumArtist->setPlaceholderText(QString());
    m_editAlbumArtist2->setPlaceholderText(QString());

    m_uiWidget->chAlbumArtist->setChecked(true);
    m_uiWidget->chAlbumArtist_2->setChecked(true);

    // Synchronisation pour les combo-box
    if (m_editAlbumArtist->text() != albumArtist)
        m_editAlbumArtist->setText(albumArtist);

    if (m_editAlbumArtist2->text() != albumArtist)
        m_editAlbumArtist2->setText(albumArtist);
}


void CDialogEditSongs::onAlbumArtistSortChange(const QString& albumArtist)
{
    m_editAlbumArtistSort->setPlaceholderText(QString());
    m_uiWidget->chAlbumArtistSort->setChecked(true);
}


void CDialogEditSongs::onComposerChange(const QString& composer)
{
    m_editComposer->setPlaceholderText(QString());
    m_editComposer2->setPlaceholderText(QString());

    m_uiWidget->chComposer->setChecked(true);
    m_uiWidget->chComposer_2->setChecked(true);

    // Synchronisation pour les combo-box
    if (m_editComposer->text() != composer)
        m_editComposer->setText(composer);

    if (m_editComposer2->text() != composer)
        m_editComposer2->setText(composer);
}


void CDialogEditSongs::onComposerSortChange(const QString& composer)
{
    m_editComposerSort->setPlaceholderText(QString());
    m_uiWidget->chComposerSort->setChecked(true);
}


void CDialogEditSongs::onSubTitleChange(const QString& subTitle)
{
    m_uiWidget->editSubTitle->setPlaceholderText(QString());
    m_uiWidget->chSubTitle->setChecked(true);
}


void CDialogEditSongs::onGroupingChange(const QString& grouping)
{
    m_uiWidget->editGrouping->setPlaceholderText(QString());
    m_uiWidget->chGrouping->setChecked(true);
}


void CDialogEditSongs::onCommentsChange()
{
    if (m_differentComments)
    {
        m_differentComments = false;
    }

    m_uiWidget->chComments->setChecked(true);
}


void CDialogEditSongs::onYearChange(const QString& year)
{
    m_uiWidget->editYear->setPlaceholderText(QString());
    m_uiWidget->chYear->setChecked(true);
}


void CDialogEditSongs::onTrackNumberChange(const QString& trackNumber)
{
    m_uiWidget->editTrackNumber->setPlaceholderText(QString());
    m_uiWidget->chTrackNumber->setChecked(true);
}


void CDialogEditSongs::onTrackCountChange(const QString& trackCount)
{
    m_uiWidget->editTrackCount->setPlaceholderText(QString());
    m_uiWidget->chTrackCount->setChecked(true);
}


void CDialogEditSongs::onDiscNumberChange(const QString& discNumber)
{
    m_uiWidget->editDiscNumber->setPlaceholderText(QString());
    m_uiWidget->chDiscNumber->setChecked(true);
}


void CDialogEditSongs::onDiscCountChange(const QString& discCount)
{
    m_uiWidget->editDiscCount->setPlaceholderText(QString());
    m_uiWidget->chDiscCount->setChecked(true);
}


void CDialogEditSongs::onBPMChange(const QString& bpm)
{
    m_uiWidget->editBPM->setPlaceholderText(QString());
    m_uiWidget->chBPM->setChecked(true);
}


void CDialogEditSongs::onGenreChange(const QString& genre)
{
    m_uiWidget->editGenre->lineEdit()->setPlaceholderText(QString());
    m_uiWidget->chGenre->setChecked(true);
}


void CDialogEditSongs::onRatingChange(int rating)
{
    m_editRating->setPlaceholderText(QString());
    m_uiWidget->chRating->setChecked(true);
}


void CDialogEditSongs::onLyricsChange()
{
    if (m_differentLyrics)
    {
        m_differentLyrics = false;
    }

    m_uiWidget->chLyrics->setChecked(true);
}


void CDialogEditSongs::onLyricistChange(const QString& lyricist)
{
    m_uiWidget->editLyricist->setPlaceholderText(QString());
    m_uiWidget->chLyricist->setChecked(true);
}


void CDialogEditSongs::onLanguageChange(const QString& language)
{
    m_uiWidget->chLanguage->setChecked(true);
}


void CDialogEditSongs::onTitleChecked(bool checked)
{
    m_editTitle->setPlaceholderText(QString());
    m_editTitle2->setPlaceholderText(QString());
}


void CDialogEditSongs::onTitleSortChecked(bool checked)
{
    m_editTitleSort->setPlaceholderText(QString());
}


void CDialogEditSongs::onArtistChecked(bool checked)
{
    m_editArtist->setPlaceholderText(QString());
    m_editArtist2->setPlaceholderText(QString());
}


void CDialogEditSongs::onArtistSortChecked(bool checked)
{
    m_editArtistSort->setPlaceholderText(QString());
}


void CDialogEditSongs::onAlbumChecked(bool checked)
{
    m_editAlbum->setPlaceholderText(QString());
    m_editAlbum2->setPlaceholderText(QString());
}


void CDialogEditSongs::onAlbumSortChecked(bool checked)
{
    m_editAlbumSort->setPlaceholderText(QString());
}


void CDialogEditSongs::onAlbumArtistChecked(bool checked)
{
    m_editAlbumArtist->setPlaceholderText(QString());
    m_editAlbumArtist2->setPlaceholderText(QString());
}


void CDialogEditSongs::onAlbumArtistSortChecked(bool checked)
{
    m_editAlbumArtistSort->setPlaceholderText(QString());
}


void CDialogEditSongs::onComposerChecked(bool checked)
{
    m_editComposer->setPlaceholderText(QString());
    m_editComposer2->setPlaceholderText(QString());
}


void CDialogEditSongs::onComposerSortChecked(bool checked)
{
    m_editComposerSort->setPlaceholderText(QString());
}


void CDialogEditSongs::onSubTitleChecked(bool checked)
{
    m_uiWidget->editSubTitle->setPlaceholderText(QString());
}


void CDialogEditSongs::onGroupingChecked(bool checked)
{
    m_uiWidget->editGrouping->setPlaceholderText(QString());
}


void CDialogEditSongs::onCommentsChecked(bool checked)
{
    if (m_differentComments)
    {
        m_uiWidget->editComments->setText(QString());
        m_differentComments = false;
    }
}


void CDialogEditSongs::onYearChecked(bool checked)
{
    m_uiWidget->editYear->setPlaceholderText(QString());
}


void CDialogEditSongs::onTrackNumberChecked(bool checked)
{
    m_uiWidget->editTrackNumber->setPlaceholderText(QString());
}


void CDialogEditSongs::onTrackCountChecked(bool checked)
{
    m_uiWidget->editTrackCount->setPlaceholderText(QString());
}


void CDialogEditSongs::onDiscNumberChecked(bool checked)
{
    m_uiWidget->editDiscNumber->setPlaceholderText(QString());
}


void CDialogEditSongs::onDiscCountChecked(bool checked)
{
    m_uiWidget->editDiscCount->setPlaceholderText(QString());
}


void CDialogEditSongs::onBPMChecked(bool checked)
{
    m_uiWidget->editBPM->setPlaceholderText(QString());
}


void CDialogEditSongs::onGenreChecked(bool checked)
{
    m_uiWidget->editGenre->lineEdit()->setPlaceholderText(QString());
}


void CDialogEditSongs::onRatingChecked(bool checked)
{
    m_editRating->setPlaceholderText(QString());
}


void CDialogEditSongs::onLyricsChecked(bool checked)
{
    if (m_differentLyrics)
    {
        m_uiWidget->editLyrics->setText(QString());
        m_differentLyrics = false;
    }
}


void CDialogEditSongs::onLyricistChecked(bool checked)
{
    m_uiWidget->editLyricist->setPlaceholderText(QString());
}


/**
 * Méthode appellée quand on coche la case de modification de la langue.
 *
 * \param checked True si la case est cochée, false sinon.
 */

void CDialogEditSongs::onLanguageChecked(bool checked)
{
    if (m_uiWidget->editLanguage->currentIndex() == -1)
    {
        m_uiWidget->editLanguage->setCurrentIndex(0);
    }
}


/**
 * Méthode appellée quand le focus sur le widget change.
 *
 * \param old Ancien widget ayant le focus.
 * \param now Nouveau widget ayant le focus.
 */

void CDialogEditSongs::onFocusChange(QWidget * old, QWidget * now)
{
    if (m_differentComments)
    {
        if (now == m_uiWidget->editComments)
        {
            m_uiWidget->editComments->setText(QString());
            m_differentComments = true;
            m_uiWidget->chComments->setChecked(false);
            return;
        }
        else if (old == m_uiWidget->editComments)
        {
            m_uiWidget->editComments->setText("<span style='color:grey;'>" + tr("Different values") + "</span>");
            m_differentComments = true;
            m_uiWidget->chComments->setChecked(false);
            return;
        }
    }

    if (m_differentLyrics)
    {
        if (now == m_uiWidget->editLyrics)
        {
            m_uiWidget->editLyrics->setText(QString());
            m_differentLyrics = true;
            m_uiWidget->chLyrics->setChecked(false);
            return;
        }
        else if (old == m_uiWidget->editLyrics)
        {
            m_uiWidget->editLyrics->setText("<span style='color:grey;'>" + tr("Different values") + "</span>");
            m_differentLyrics = true;
            m_uiWidget->chLyrics->setChecked(false);
            return;
        }
    }
}

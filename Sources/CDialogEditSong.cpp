
#include "CDialogEditSong.hpp"
#include "CSongTable.hpp"
#include <QStandardItemModel>

#include <QtDebug>


CDialogEditSong::CDialogEditSong(CSongTableItem * songItem, CSongTable * songTable) :
    QDialog     (songTable),
    m_uiWidget  (new Ui::DialogEditSong()),
    m_songTable (songTable),
    m_songItem  (songItem)
{
    Q_CHECK_PTR(songItem);
    Q_CHECK_PTR(songTable);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);


    // Liste des langues
    m_uiWidget->editLanguage->addItems(CSong::getLanguageList());


    // Liste des genres
    /// \todo Déplacer dans une fonction
    /// \todo Gérer tous les genres de base
    /// \todo Trier la liste
    /// \todo Ajouter tous les genres utilisés dans la médiathèque
    QStringList genreList;
    genreList << tr("Classical");
    genreList << tr("Reggae");
    genreList << tr("Rock");
    m_uiWidget->editGenre->addItems(genreList);


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
}


CDialogEditSong::~CDialogEditSong()
{
    delete m_uiWidget;
}


/**
 * Affiche les informations du morceau précédent dans la liste.
 *
 * \todo Implémentation.
 */

void CDialogEditSong::previousSong(void)
{
    CSongTableItem * songItem = m_songTable->getPreviousSong(m_songItem, false);

    if (songItem)
    {
        m_songItem = songItem;
        updateInfos();
    }
}


/**
 * Affiche les informations du morceau suivant dans la liste.
 *
 * \todo Implémentation.
 */

void CDialogEditSong::nextSong(void)
{
    CSongTableItem * songItem = m_songTable->getNextSong(m_songItem, false);

    if (songItem)
    {
        m_songItem = songItem;
        updateInfos();
    }
}


/**
 * Enregistre les modifications effectuées sur le morceau.
 */

void CDialogEditSong::apply(void)
{
    qDebug() << "CDialogEditSong::apply";

    CSong * song = m_songItem->getSong();
    song->startMultiModification();

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
    song->setTrackTotal(m_uiWidget->editTrackTotal->text().toInt());
    song->setDiscNumber(m_uiWidget->editDiscNumber->text().toInt());
    song->setDiscTotal(m_uiWidget->editDiscTotal->text().toInt());
    song->setComments(m_uiWidget->editComments->toPlainText());
    song->setGenre(m_uiWidget->editGenre->currentText());
    song->setRating(m_uiWidget->editRating->value());
    song->setLyrics(m_uiWidget->editLyrics->toPlainText());
    song->setLanguage(CSong::getLanguageFromInteger(m_uiWidget->editLanguage->currentIndex()));

    song->writeTags();
    song->updateDatabase();
}


/**
 * Enregistre les modifications effectuées sur le morceau et ferme la boite de dialogue.
 */

void CDialogEditSong::save(void)
{
    apply();
    close();
}


/**
 * Met à jour la boite de dialogue avec les informations du morceau.
 *
 * \todo Afficher les illustrations.
 * \todo Modifier le titre de la fenêtre.
 */

void CDialogEditSong::updateInfos()
{
    CSong * song = m_songItem->getSong();

    const QString songTitle = song->getTitle();
    const QString songArtist = song->getArtistName();

    if (songArtist.isEmpty())
    {
        setWindowTitle(tr("Song infos") + " - " + songTitle);
    }
    else
    {
        setWindowTitle(tr("Song infos") + " - " + songTitle + " - " + songArtist);
    }

    // Résumé

    const int duration = song->getDuration();
    QTime durationTime(0, 0);
    durationTime = durationTime.addMSecs(duration);

    m_uiWidget->valueTitle->setText(songTitle + QString(" (%1)").arg(durationTime.toString("m:ss"))); /// \todo Stocker dans les settings
    m_uiWidget->valueArtist->setText(songArtist);
    m_uiWidget->valueAlbum->setText(song->getAlbumTitle());

    m_uiWidget->valueFileName->setText(song->getFileName());
    m_uiWidget->valueFileSize->setText(CSong::getFileSize(song->getFileSize()));
    m_uiWidget->valueCreation->setText(song->getCreationDate().toString());
    m_uiWidget->valueModification->setText(song->getModificationDate().toString());

    m_uiWidget->valueBitRate->setText(tr("%1 kbit/s").arg(song->getBitRate()));
    m_uiWidget->valueFormat->setText(CSong::getFormatName(song->getFormat()));
    m_uiWidget->valueChannels->setText(QString::number(song->getNumChannels()));
    m_uiWidget->valueSampleRate->setText(tr("%1 kHz").arg(song->getSampleRate()));

    m_uiWidget->valueLastPlayTime->setText(song->getLastPlay().toString());
    m_uiWidget->valuePlayCount->setText(QString::number(song->getNumPlays()));
    

    // Informations et tri

    m_uiWidget->editTitle->setText(songTitle);
    m_uiWidget->editTitle_2->setText(songTitle);
    m_uiWidget->editTitleSort->setText(song->getTitleSort());

    m_uiWidget->editSubTitle->setText(song->getSubTitle());

    m_uiWidget->editArtist->setText(songArtist);
    m_uiWidget->editArtist_2->setText(songArtist);
    m_uiWidget->editArtistSort->setText(song->getArtistNameSort());

    m_uiWidget->editAlbum->setText(song->getAlbumTitle());
    m_uiWidget->editAlbum_2->setText(song->getAlbumTitle());
    m_uiWidget->editAlbumSort->setText(song->getAlbumTitleSort());

    m_uiWidget->editAlbumArtist->setText(song->getAlbumArtist());
    m_uiWidget->editAlbumArtist_2->setText(song->getAlbumArtist());
    m_uiWidget->editAlbumArtistSort->setText(song->getAlbumArtistSort());

    m_uiWidget->editGrouping->setText(song->getGrouping());
    const int bpm = song->getBPM();
    m_uiWidget->editBPM->setText(bpm > 0 ? QString::number(bpm) : "");

    m_uiWidget->editComposer->setText(song->getComposer());
    m_uiWidget->editComposer_2->setText(song->getComposer());
    m_uiWidget->editComposerSort->setText(song->getComposerSort());

    const int year = song->getYear();
    m_uiWidget->editYear->setText(year > 0 ? QString::number(year) : "");

    const int trackNumber = song->getTrackNumber();
    m_uiWidget->editTrackNumber->setText(trackNumber > 0 ? QString::number(trackNumber) : "");

    const int trackTotal = song->getTrackTotal();
    m_uiWidget->editTrackTotal->setText(trackTotal > 0 ? QString::number(trackTotal) : "");

    const int discNumber = song->getDiscNumber();
    m_uiWidget->editDiscNumber->setText(discNumber > 0 ? QString::number(discNumber) : "");

    const int discTotal = song->getDiscTotal();
    m_uiWidget->editDiscTotal->setText(discTotal > 0 ? QString::number(discTotal) : "");

    m_uiWidget->editComments->setText(song->getComments());

    int genreIndex = m_uiWidget->editGenre->findText(song->getGenre());
    if (genreIndex < 0)
    {
        m_uiWidget->editGenre->addItem(song->getGenre());
        genreIndex = m_uiWidget->editGenre->findText(song->getGenre());
    }

    m_uiWidget->editGenre->setCurrentIndex(genreIndex);

    m_uiWidget->editRating->setValue(song->getRating());


    // Paroles

    m_uiWidget->editLyrics->setText(song->getLyrics());
    m_uiWidget->editLanguage->setCurrentIndex(song->getLanguage());


    // Lectures

    QStandardItemModel * model = new QStandardItemModel();
    m_uiWidget->listPlayings->setModel(model);

    foreach (QDateTime playTime, song->m_plays)
    {
        QStandardItem * item = new QStandardItem(playTime.toString());
        model->appendRow(item);
    }
}

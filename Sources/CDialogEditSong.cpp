
#include "CDialogEditSong.hpp"
#include "CSongTable.hpp"
#include <QStandardItemModel>

#include <QtDebug>


CDialogEditSong::CDialogEditSong(CSongTableModel::TSongItem * songItem, CSongTable * songTable) :
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
    /// \todo Déplacer dans une fonction
    m_uiWidget->editLanguage->addItem(tr("Unknown"), CSong::LangUnknown);
    m_uiWidget->editLanguage->addItem(tr("English"), CSong::LangEnglish);
    m_uiWidget->editLanguage->addItem(tr("French") , CSong::LangFrench );
    m_uiWidget->editLanguage->addItem(tr("German") , CSong::LangGerman );
    m_uiWidget->editLanguage->addItem(tr("Italian"), CSong::LangItalian);


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
    //m_songItem = m_songTable->getPreviousSong(m_songItem);
    updateInfos();
}


/**
 * Affiche les informations du morceau suivant dans la liste.
 *
 * \todo Implémentation.
 */

void CDialogEditSong::nextSong(void)
{
    //m_songItem = m_songTable->getNextSong(m_songItem);
    updateInfos();
}


/// \todo Implémentation
void CDialogEditSong::apply(void)
{
    qDebug() << "CDialogEditSong::apply";

    m_songItem->song->setTitle(m_uiWidget->editTitle->text());
    m_songItem->song->setArtistName(m_uiWidget->editArtist->text());
    m_songItem->song->setAlbumTitle(m_uiWidget->editAlbum->text());
    m_songItem->song->setAlbumArtist(m_uiWidget->editAlbumArtist->text());
    m_songItem->song->setComposer(m_uiWidget->editComposer->text());

    m_songItem->song->setTitleSort(m_uiWidget->editTitleSort->text());
    m_songItem->song->setArtistNameSort(m_uiWidget->editArtistSort->text());
    m_songItem->song->setAlbumTitleSort(m_uiWidget->editAlbumSort->text());
    m_songItem->song->setAlbumArtistSort(m_uiWidget->editAlbumArtistSort->text());
    m_songItem->song->setComposerSort(m_uiWidget->editComposerSort->text());

    m_songItem->song->setYear(m_uiWidget->editYear->text().toInt());
    m_songItem->song->setTrackNumber(m_uiWidget->editTrackNumber->text().toInt());
    m_songItem->song->setTrackTotal(m_uiWidget->editTrackTotal->text().toInt());
    m_songItem->song->setDiscNumber(m_uiWidget->editDiscNumber->text().toInt());
    m_songItem->song->setDiscTotal(m_uiWidget->editDiscTotal->text().toInt());
    m_songItem->song->setComments(m_uiWidget->editComments->toPlainText());
    m_songItem->song->setGenre(m_uiWidget->editGenre->currentText());
    m_songItem->song->setRating(m_uiWidget->editRating->value());
    m_songItem->song->setLyrics(m_uiWidget->editLyrics->toPlainText());
    m_songItem->song->setLanguage(CSong::LangFromInt(m_uiWidget->editLanguage->itemData(m_uiWidget->editLanguage->currentIndex()).toInt()));

    m_songItem->song->updateDatabase();
}


void CDialogEditSong::save(void)
{
    qDebug() << "CDialogEditSong::save";
    apply();
    close();
}


/// \todo Implémentation complète.
void CDialogEditSong::updateInfos()
{
    // Résumé

    const int duration = m_songItem->song->getDuration();
    QTime durationTime(0, 0);
    durationTime = durationTime.addMSecs(duration);

    m_uiWidget->valueTitle->setText(m_songItem->song->getTitle() + QString(" (%1)").arg(durationTime.toString("m:ss"))); /// \todo Stocker dans les settings
    m_uiWidget->valueArtist->setText(m_songItem->song->getArtistName());
    m_uiWidget->valueAlbum->setText(m_songItem->song->getAlbumTitle());

    m_uiWidget->valueFileName->setText(m_songItem->song->getFileName());

    int fileSize = m_songItem->song->getFileSize();

    if (fileSize >= 1024)
    {
        if (fileSize >= 1024 * 1024)
        {
            if (fileSize >= 1024 * 1024 * 1024)
            {
                m_uiWidget->valueFileSize->setText(tr("%1 Gio").arg(fileSize / (1024*1024*1024)));
            }
            else
            {
                // Moins de 1 Gio
                m_uiWidget->valueFileSize->setText(tr("%1 Mio").arg(fileSize / (1024*1024)));
            }
        }
        else
        {
            // Moins de 1 Mio
            m_uiWidget->valueFileSize->setText(tr("%1 Kio").arg(fileSize / 1024));
        }
    }
    else
    {
        // Moins de 1 Kio
        m_uiWidget->valueFileSize->setText(tr("%1 bytes").arg(fileSize));
    }

    m_uiWidget->valueCreation->setText(m_songItem->song->getCreationDate().toString());
    m_uiWidget->valueModification->setText(m_songItem->song->getModificationDate().toString());

    m_uiWidget->valueBitRate->setText(tr("%1 kbit/s").arg(m_songItem->song->getBitRate()));

    switch (m_songItem->song->getFileType())
    {
        default:
        case CSong::TypeUnknown:
            m_uiWidget->valueFormat->setText(tr("Unknown"));
            break;

        case CSong::TypeMP3:
            m_uiWidget->valueFormat->setText(tr("MP3"));
            break;

        case CSong::TypeOGG:
            m_uiWidget->valueFormat->setText(tr("OGG Vorbis"));
            break;

        case CSong::TypeFlac:
            m_uiWidget->valueFormat->setText(tr("Flac"));
            break;
    }

    m_uiWidget->valueChannels->setText(QString::number(m_songItem->song->getNumChannels()));
    m_uiWidget->valueSampleRate->setText(tr("%1 kHz").arg(m_songItem->song->getSampleRate()));
    m_uiWidget->valueEncodedWith->setText(m_songItem->song->getEncoder());
    

    // Informations et tri

    m_uiWidget->editTitle->setText(m_songItem->song->getTitle());
    m_uiWidget->editTitle_2->setText(m_songItem->song->getTitle());
    m_uiWidget->editTitleSort->setText(m_songItem->song->getTitleSort());

    m_uiWidget->editArtist->setText(m_songItem->song->getArtistName());
    m_uiWidget->editArtist_2->setText(m_songItem->song->getArtistName());
    m_uiWidget->editArtistSort->setText(m_songItem->song->getArtistNameSort());

    m_uiWidget->editAlbum->setText(m_songItem->song->getAlbumTitle());
    m_uiWidget->editAlbum_2->setText(m_songItem->song->getAlbumTitle());
    m_uiWidget->editAlbumSort->setText(m_songItem->song->getAlbumTitleSort());

    m_uiWidget->editAlbumArtist->setText(m_songItem->song->getAlbumArtist());
    m_uiWidget->editAlbumArtist_2->setText(m_songItem->song->getAlbumArtist());
    m_uiWidget->editAlbumArtistSort->setText(m_songItem->song->getAlbumArtistSort());

    m_uiWidget->editComposer->setText(m_songItem->song->getComposer());
    m_uiWidget->editComposer_2->setText(m_songItem->song->getComposer());
    m_uiWidget->editComposerSort->setText(m_songItem->song->getComposerSort());

    const int year = m_songItem->song->getYear();
    m_uiWidget->editYear->setText(year > 0 ? QString::number(year) : "");

    const int trackNumber = m_songItem->song->getTrackNumber();
    m_uiWidget->editTrackNumber->setText(trackNumber > 0 ? QString::number(trackNumber) : "");

    const int trackTotal = m_songItem->song->getTrackTotal();
    m_uiWidget->editTrackTotal->setText(trackTotal > 0 ? QString::number(trackTotal) : "");

    const int discNumber = m_songItem->song->getDiscNumber();
    m_uiWidget->editDiscNumber->setText(discNumber > 0 ? QString::number(discNumber) : "");

    const int discTotal = m_songItem->song->getDiscTotal();
    m_uiWidget->editDiscTotal->setText(discTotal > 0 ? QString::number(discTotal) : "");

    m_uiWidget->editComments->setText(m_songItem->song->getComments());
    
    QStringList genreList;
    genreList << m_songItem->song->getGenre();
    m_uiWidget->editGenre->addItems(genreList);

    m_uiWidget->editRating->setValue(m_songItem->song->getRating());
        
    // Paroles
    m_uiWidget->editLyrics->setText(m_songItem->song->getLyrics());
    m_uiWidget->editLanguage->setCurrentIndex(0);
    // Langue

    //...


    // Métadonnées

    QStandardItemModel * modelMetaData = new QStandardItemModel();

    QStringList headerList;
    headerList << tr("Tag") << tr("Value");
    modelMetaData->setHorizontalHeaderLabels(headerList);

    m_uiWidget->tableMetaData->setModel(modelMetaData);


    // Lectures

    //...
}


#include "CDialogEditSong.hpp"

#include <QStandardItemModel>


CDialogEditSong::CDialogEditSong(CSongTableModel::TSongItem * songItem, QWidget * parent) :
    QDialog    (parent),
    m_uiWidget (new Ui::DialogEditSong()),
    m_songItem (songItem)
{
    Q_CHECK_PTR(songItem);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);


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
        m_uiWidget->valueFileSize->setText(tr("%1 octets").arg(fileSize));
    }

    m_uiWidget->valueCreation->setText(m_songItem->song->getCreationDate().toString());
    m_uiWidget->valueModification->setText(m_songItem->song->getModificationDate().toString());

    m_uiWidget->valueBitRate->setText(tr("%1 kbit/s").arg(m_songItem->song->getBitRate()));

    switch (m_songItem->song->getFileType())
    {
        default:
        case CSong::TypeUnknown:
            m_uiWidget->valueFormat->setText(tr("Inconnu"));
            break;

        case CSong::TypeMP3:
            m_uiWidget->valueFormat->setText(tr("MP3"));
            break;

        case CSong::TypeOGG:
            m_uiWidget->valueFormat->setText(tr("OGG Vorbis"));
            break;

        case CSong::TypeFlac:
            m_uiWidget->valueFormat->setText(tr("Inconnu"));
            break;
    }

    m_uiWidget->valueChannels->setText(QString::number(m_songItem->song->getNumChannels()));
    m_uiWidget->valueFrequency->setText("?");

    //...


    // Informations et tri

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
    
    //Genre...
    m_uiWidget->editRating->setValue(m_songItem->song->getRating());
        
    // Paroles
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


CDialogEditSong::~CDialogEditSong()
{
    delete m_uiWidget;
}

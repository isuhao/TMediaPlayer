
#include "CDialogEditSong.hpp"
#include "CSong.hpp"
#include <QStandardItemModel>


CDialogEditSong::CDialogEditSong(CSong * song, QWidget * parent) :
    QDialog    (parent),
    m_uiWidget (new Ui::DialogEditSong()),
    m_song     (song)
{
    Q_CHECK_PTR(song);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);


    // Résumé

    const int duration = m_song->getDuration();
    QTime durationTime(0, 0);
    durationTime = durationTime.addMSecs(duration);

    m_uiWidget->valueTitle->setText(m_song->getTitle() + QString(" (%1)").arg(durationTime.toString("m:ss"))); /// \todo Stocker dans les settings
    m_uiWidget->valueArtist->setText(m_song->getArtistName());
    m_uiWidget->valueAlbum->setText(m_song->getAlbumTitle());

    m_uiWidget->valueFileName->setText(m_song->getFileName());

    int fileSize = m_song->getFileSize();

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

    m_uiWidget->valueCreation->setText(m_song->getCreationDate().toString());
    m_uiWidget->valueModification->setText(m_song->getModificationDate().toString());

    m_uiWidget->valueBitRate->setText(tr("%1 kbit/s").arg(m_song->getBitRate()));

    switch (m_song->getFileType())
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

    m_uiWidget->valueChannels->setText(QString::number(m_song->getNumChannels()));

    //...
    //...
    //...
    //...
    //...


    // Informations et tri

    m_uiWidget->editTitle->setText(m_song->getTitle());
    m_uiWidget->editTitle_2->setText(m_song->getTitle());
    m_uiWidget->editTitleSort->setText(m_song->getTitleSort());

    m_uiWidget->editArtist->setText(m_song->getArtistName());
    m_uiWidget->editArtist_2->setText(m_song->getArtistName());
    m_uiWidget->editArtistSort->setText(m_song->getArtistNameSort());

    m_uiWidget->editAlbum->setText(m_song->getAlbumTitle());
    m_uiWidget->editAlbum_2->setText(m_song->getAlbumTitle());
    m_uiWidget->editAlbumSort->setText(m_song->getAlbumTitleSort());

    m_uiWidget->editAlbumArtist->setText(m_song->getAlbumArtist());
    m_uiWidget->editAlbumArtist_2->setText(m_song->getAlbumArtist());
    m_uiWidget->editAlbumArtistSort->setText(m_song->getAlbumArtistSort());

    m_uiWidget->editComposer->setText(m_song->getComposer());
    m_uiWidget->editComposer_2->setText(m_song->getComposer());
    m_uiWidget->editComposerSort->setText(m_song->getComposerSort());

    const int year = m_song->getYear();
    m_uiWidget->editYear->setText(year > 0 ? QString::number(year) : "");

    const int trackNumber = m_song->getTrackNumber();
    m_uiWidget->editTrackNumber->setText(trackNumber > 0 ? QString::number(trackNumber) : "");

    const int trackTotal = m_song->getTrackTotal();
    m_uiWidget->editTrackTotal->setText(trackTotal > 0 ? QString::number(trackTotal) : "");

    const int discNumber = m_song->getDiscNumber();
    m_uiWidget->editDiscNumber->setText(discNumber > 0 ? QString::number(discNumber) : "");

    const int discTotal = m_song->getDiscTotal();
    m_uiWidget->editDiscTotal->setText(discTotal > 0 ? QString::number(discTotal) : "");

    m_uiWidget->editComments->setText(m_song->getComments());

    //Genre...
    //Note...
    
    // Paroles
    // Langue

    //...


    // Métadonnées

    QStandardItemModel * modelMetaData = new QStandardItemModel();

    QStringList headerList;
    headerList << tr("Tag") << tr("Value");
    modelMetaData->setHorizontalHeaderLabels(headerList);

    m_uiWidget->tableMetaData->setModel(modelMetaData);

    //TODO...


    // Lectures

    //...
}


CDialogEditSong::~CDialogEditSong()
{
    delete m_uiWidget;
}

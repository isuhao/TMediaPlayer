
#include "CDialogEditSongs.hpp"

#include <QStandardItemModel>


CDialogEditSongs::CDialogEditSongs(QList<CSongTableModel::TSongItem *> songItemList, QWidget * parent) :
    QDialog        (parent),
    m_uiWidget     (new Ui::DialogEditSongs()),
    m_songItemList (songItemList)
{
    Q_ASSERT(songItemList.size() > 1);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);


    // Liste des langues
    /// \todo Déplacer dans une fonction
    m_uiWidget->editLanguage->addItem(tr("Unknown"), CSong::LangUnknown);
    m_uiWidget->editLanguage->addItem(tr("English"), CSong::LangEnglish);
    m_uiWidget->editLanguage->addItem(tr("French") , CSong::LangFrench );
    m_uiWidget->editLanguage->addItem(tr("German") , CSong::LangGerman );
    m_uiWidget->editLanguage->addItem(tr("Italian"), CSong::LangItalian);


    QString songTitle;       bool songTitleSim       = true;
    QString songTitleSort;   bool songTitleSortSim   = true;
    QString songArtist;      bool songArtistSim      = true;
    QString songAlbum;       bool songAlbumSim       = true;
    QString songAlbumArtist; bool songAlbumArtistSim = true;
    QString songComposer;    bool songComposerSim    = true;
    //...

    bool first = true;

    // Recherche des données similaires pour tous les éléments
    foreach (CSongTableModel::TSongItem * songItem, m_songItemList)
    {
        Q_CHECK_PTR(songItem);
        Q_CHECK_PTR(songItem->song);

        if (first)
        {
            songTitle       = songItem->song->getTitle();
            songTitleSort   = songItem->song->getTitleSort();
            songArtist      = songItem->song->getArtistName();
            songAlbum       = songItem->song->getAlbumTitle();
            songAlbumArtist = songItem->song->getAlbumArtist();
            songComposer    = songItem->song->getComposer();
            //...

            first = false;
        }
        else
        {
            if (songTitleSim       && songItem->song->getTitle()       != songTitle      ) { songTitleSim       = false; songTitle      .clear(); }
            if (songTitleSortSim   && songItem->song->getTitleSort()   != songTitleSort  ) { songTitleSortSim   = false; songTitleSort  .clear(); }
            if (songArtistSim      && songItem->song->getArtistName()  != songArtist     ) { songArtistSim      = false; songArtist     .clear(); }
            if (songAlbumSim       && songItem->song->getAlbumTitle()  != songAlbum      ) { songAlbumSim       = false; songAlbum      .clear(); }
            if (songAlbumArtistSim && songItem->song->getAlbumArtist() != songAlbumArtist) { songAlbumArtistSim = false; songAlbumArtist.clear(); }
            if (songComposerSim    && songItem->song->getComposer()    != songComposer   ) { songComposerSim    = false; songComposer   .clear(); }
            //...
        }
    }

    const QString notSimText = tr("<Different values>");


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
    

    // Informations et tri

    m_uiWidget->editTitle->setText(songTitleSim ? songTitle : notSimText);
    m_uiWidget->editTitle_2->setText(songTitleSim ? songTitle : notSimText);
    m_uiWidget->editTitleSort->setText(songTitleSortSim ? songTitleSort : notSimText);
    
    m_uiWidget->editArtist->setText(songArtistSim ? songArtist : notSimText);
    m_uiWidget->editArtist_2->setText(songArtistSim ? songArtist : notSimText);
    //m_uiWidget->editArtistSort->setText(m_songItem->song->getArtistNameSort());

    m_uiWidget->editAlbum->setText(songAlbumSim ? songAlbum : notSimText);
    m_uiWidget->editAlbum_2->setText(songAlbumSim ? songAlbum : notSimText);
    //m_uiWidget->editAlbumSort->setText(m_songItem->song->getAlbumTitleSort());

    m_uiWidget->editAlbumArtist->setText(songAlbumArtistSim ? songAlbumArtist : notSimText);
    m_uiWidget->editAlbumArtist_2->setText(songAlbumArtistSim ? songAlbumArtist : notSimText);
    //m_uiWidget->editAlbumArtistSort->setText(m_songItem->song->getAlbumArtistSort());

    m_uiWidget->editComposer->setText(songComposerSim ? songComposer : notSimText);
    m_uiWidget->editComposer_2->setText(songComposerSim ? songComposer : notSimText);
    //m_uiWidget->editComposerSort->setText(m_songItem->song->getComposerSort());
/*
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
*/
    // Paroles
    // Langue

    //...
}


CDialogEditSongs::~CDialogEditSongs()
{
    delete m_uiWidget;
}

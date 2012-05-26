
#include "CDialogEditSongs.hpp"
#include <QStandardItemModel>
#include <QPushButton>


CDialogEditSongs::CDialogEditSongs(QList<CSongTableItem *> songItemList, QWidget * parent) :
    QDialog        (parent),
    m_uiWidget     (new Ui::DialogEditSongs()),
    m_songItemList (songItemList)
{
    Q_ASSERT(songItemList.size() > 1);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);


    // Synchronisation des champs
    connect(m_uiWidget->editTitle, SIGNAL(textEdited(const QString&)), m_uiWidget->editTitle_2, SLOT(setText(const QString&)));
    connect(m_uiWidget->editTitle_2, SIGNAL(textEdited(const QString&)), m_uiWidget->editTitle, SLOT(setText(const QString&)));

    connect(m_uiWidget->chTitle, SIGNAL(toggled(bool)), m_uiWidget->chTitle_2, SLOT(setChecked(bool)));
    connect(m_uiWidget->chTitle_2, SIGNAL(toggled(bool)), m_uiWidget->chTitle, SLOT(setChecked(bool)));

    connect(m_uiWidget->editArtist, SIGNAL(textEdited(const QString&)), m_uiWidget->editArtist_2, SLOT(setText(const QString&)));
    connect(m_uiWidget->editArtist_2, SIGNAL(textEdited(const QString&)), m_uiWidget->editArtist, SLOT(setText(const QString&)));

    connect(m_uiWidget->chArtist, SIGNAL(toggled(bool)), m_uiWidget->chArtist_2, SLOT(setChecked(bool)));
    connect(m_uiWidget->chArtist_2, SIGNAL(toggled(bool)), m_uiWidget->chArtist, SLOT(setChecked(bool)));

    connect(m_uiWidget->editAlbum, SIGNAL(textEdited(const QString&)), m_uiWidget->editAlbum_2, SLOT(setText(const QString&)));
    connect(m_uiWidget->editAlbum_2, SIGNAL(textEdited(const QString&)), m_uiWidget->editAlbum, SLOT(setText(const QString&)));

    connect(m_uiWidget->chAlbum, SIGNAL(toggled(bool)), m_uiWidget->chAlbum_2, SLOT(setChecked(bool)));
    connect(m_uiWidget->chAlbum_2, SIGNAL(toggled(bool)), m_uiWidget->chAlbum, SLOT(setChecked(bool)));

    connect(m_uiWidget->editAlbumArtist, SIGNAL(textEdited(const QString&)), m_uiWidget->editAlbumArtist_2, SLOT(setText(const QString&)));
    connect(m_uiWidget->editAlbumArtist_2, SIGNAL(textEdited(const QString&)), m_uiWidget->editAlbumArtist, SLOT(setText(const QString&)));

    connect(m_uiWidget->chAlbumArtist, SIGNAL(toggled(bool)), m_uiWidget->chAlbumArtist_2, SLOT(setChecked(bool)));
    connect(m_uiWidget->chAlbumArtist_2, SIGNAL(toggled(bool)), m_uiWidget->chAlbumArtist, SLOT(setChecked(bool)));

    connect(m_uiWidget->editComposer, SIGNAL(textEdited(const QString&)), m_uiWidget->editComposer_2, SLOT(setText(const QString&)));
    connect(m_uiWidget->editComposer_2, SIGNAL(textEdited(const QString&)), m_uiWidget->editComposer, SLOT(setText(const QString&)));

    connect(m_uiWidget->chComposer, SIGNAL(toggled(bool)), m_uiWidget->chComposer_2, SLOT(setChecked(bool)));
    connect(m_uiWidget->chComposer_2, SIGNAL(toggled(bool)), m_uiWidget->chComposer, SLOT(setChecked(bool)));


    // Liste des langues
    m_uiWidget->editLanguage->addItems(CSong::getLanguageList());

    
    // Recherche des données similaires pour tous les éléments
    QString songTitle;           bool songTitleSim           = true;
    QString songTitleSort;       bool songTitleSortSim       = true;
    QString songArtist;          bool songArtistSim          = true;
    QString songArtistSort;      bool songArtistSortSim      = true;
    QString songAlbum;           bool songAlbumSim           = true;
    QString songAlbumSort;       bool songAlbumSortSim       = true;
    QString songAlbumArtist;     bool songAlbumArtistSim     = true;
    QString songAlbumArtistSort; bool songAlbumArtistSortSim = true;
    QString songComposer;        bool songComposerSim        = true;
    QString songComposerSort;    bool songComposerSortSim    = true;

    int songYear; bool songYearSim = true;
    //...

    bool first = true;

    foreach (CSongTableItem * songItem, m_songItemList)
    {
        Q_CHECK_PTR(songItem);
        CSong * song = songItem->getSong();

        if (first)
        {
            songTitle           = song->getTitle();
            songTitleSort       = song->getTitleSort();
            songArtist          = song->getArtistName();
            songArtistSort      = song->getArtistNameSort();
            songAlbum           = song->getAlbumTitle();
            songAlbumSort       = song->getAlbumTitleSort();
            songAlbumArtist     = song->getAlbumArtist();
            songAlbumArtistSort = song->getAlbumArtistSort();
            songComposer        = song->getComposer();
            songComposerSort    = song->getComposerSort();

            songYear = song->getYear();
            //...

            first = false;
        }
        else
        {
            if (songTitleSim           && song->getTitle()           != songTitle          ) { songTitleSim           = false; songTitle          .clear(); }
            if (songTitleSortSim       && song->getTitleSort()       != songTitleSort      ) { songTitleSortSim       = false; songTitleSort      .clear(); }
            if (songArtistSim          && song->getArtistName()      != songArtist         ) { songArtistSim          = false; songArtist         .clear(); }
            if (songArtistSortSim      && song->getArtistNameSort()  != songArtistSort     ) { songArtistSortSim      = false; songArtistSort     .clear(); }
            if (songAlbumSim           && song->getAlbumTitle()      != songAlbum          ) { songAlbumSim           = false; songAlbum          .clear(); }
            if (songAlbumSortSim       && song->getAlbumTitleSort()  != songAlbumSort      ) { songAlbumSortSim       = false; songAlbumSort      .clear(); }
            if (songAlbumArtistSim     && song->getAlbumArtist()     != songAlbumArtist    ) { songAlbumArtistSim     = false; songAlbumArtist    .clear(); }
            if (songAlbumArtistSortSim && song->getAlbumArtistSort() != songAlbumArtistSort) { songAlbumArtistSortSim = false; songAlbumArtistSort.clear(); }
            if (songComposerSim        && song->getComposer()        != songComposer       ) { songComposerSim        = false; songComposer       .clear(); }
            if (songComposerSortSim    && song->getComposerSort()    != songComposerSort   ) { songComposerSortSim    = false; songComposerSort   .clear(); }

            if (songYearSim && song->getYear() != songYear ) { songYearSim = false; songYear = 0; }
            //...
        }
    }

    const QString notSimText = tr("Different values");
   

    // Titre
    m_uiWidget->editTitle->setText(songTitle);
    m_uiWidget->editTitle_2->setText(songTitle);

    if (!songTitleSim)
    {
        m_uiWidget->editTitle->setPlaceholderText(notSimText);
        m_uiWidget->editTitle_2->setPlaceholderText(notSimText);
    }

    // Titre pour le tri
    m_uiWidget->editTitleSort->setText(songTitleSort);

    if (!songTitleSortSim)
    {
        m_uiWidget->editTitleSort->setPlaceholderText(notSimText);
    }

    // Artiste
    m_uiWidget->editArtist->setText(songArtist);
    m_uiWidget->editArtist_2->setText(songArtist);

    if (!songArtistSim)
    {
        m_uiWidget->editArtist->setPlaceholderText(notSimText);
        m_uiWidget->editArtist_2->setPlaceholderText(notSimText);
    }

    // Artiste pour le tri
    m_uiWidget->editArtistSort->setText(songArtistSort);

    if (!songArtistSortSim)
    {
        m_uiWidget->editArtistSort->setPlaceholderText(notSimText);
    }

    // Album
    m_uiWidget->editAlbum->setText(songAlbum);
    m_uiWidget->editAlbum_2->setText(songAlbum);

    if (!songAlbumSim)
    {
        m_uiWidget->editAlbum->setPlaceholderText(notSimText);
        m_uiWidget->editAlbum_2->setPlaceholderText(notSimText);
    }

    // Album pour le tri
    m_uiWidget->editAlbumSort->setText(songAlbumSort);

    if (!songAlbumSortSim)
    {
        m_uiWidget->editAlbumSort->setPlaceholderText(notSimText);
    }

    // Artiste de l'album
    m_uiWidget->editAlbumArtist->setText(songAlbumArtist);
    m_uiWidget->editAlbumArtist_2->setText(songAlbumArtist);

    if (!songAlbumArtistSim)
    {
        m_uiWidget->editAlbumArtist->setPlaceholderText(notSimText);
        m_uiWidget->editAlbumArtist_2->setPlaceholderText(notSimText);
    }

    // Artiste de l'album pour le tri
    m_uiWidget->editAlbumArtistSort->setText(songAlbumArtistSort);

    if (!songAlbumArtistSortSim)
    {
        m_uiWidget->editAlbumArtistSort->setPlaceholderText(notSimText);
    }

    // Compositeur
    m_uiWidget->editComposer->setText(songComposer);
    m_uiWidget->editComposer_2->setText(songComposer);

    if (!songComposerSim)
    {
        m_uiWidget->editComposer->setPlaceholderText(notSimText);
        m_uiWidget->editComposer_2->setPlaceholderText(notSimText);
    }

    // Compositeur pour le tri
    m_uiWidget->editComposerSort->setText(songComposerSort);

    if (!songComposerSortSim)
    {
        m_uiWidget->editComposerSort->setPlaceholderText(notSimText);
    }

    // Année
    m_uiWidget->editYear->setText(QString::number(songYear));

    if (!songYearSim)
    {
        m_uiWidget->editYear->setPlaceholderText(notSimText);
    }

/*
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


    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    QPushButton * btnApply = m_uiWidget->buttonBox->addButton(tr("Apply"), QDialogButtonBox::ApplyRole);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
}


CDialogEditSongs::~CDialogEditSongs()
{
    delete m_uiWidget;
}


/**
 * Enregistre les modifications effectuées sur les morceaux.
 *
 * \todo Implémentation.
 */

void CDialogEditSongs::apply(void)
{
    //...
}


/**
 * Enregistre les modifications effectuées sur les morceaux et ferme la boite de dialogue.
 */

void CDialogEditSongs::save(void)
{
    apply();
    close();
}

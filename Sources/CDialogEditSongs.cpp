
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

    
    // Recherche des données similaires pour tous les éléments (23 éléments)
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
    QString songSubTitle;        bool songSubTitleSim        = true;
    QString songGrouping;        bool songGroupingSim        = true;
    QString songComments;        bool songCommentsSim        = true;

    int songYear; bool songYearSim = true;
    //...

    bool first = true;

    for (QList<CSongTableItem *>::const_iterator it = m_songItemList.begin(); it != m_songItemList.end(); ++it)
    {
        Q_CHECK_PTR(*it);
        CSong * song = (*it)->getSong();

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
            songSubTitle        = song->getSubTitle();
            songGrouping        = song->getGrouping();
            songComments        = song->getComments();

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
            if (songSubTitleSim        && song->getSubTitle()        != songSubTitle       ) { songSubTitleSim        = false; songSubTitle       .clear(); }
            if (songGroupingSim        && song->getGrouping()        != songGrouping       ) { songGroupingSim        = false; songGrouping       .clear(); }
            if (songCommentsSim        && song->getComments()        != songComments       ) { songCommentsSim        = false; songComments       .clear(); }

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

    connect(m_uiWidget->editTitle, SIGNAL(textEdited(const QString&)), this, SLOT(onTitleChange(const QString&)));
    connect(m_uiWidget->editTitle_2, SIGNAL(textEdited(const QString&)), this, SLOT(onTitleChange(const QString&)));
    connect(m_uiWidget->chTitle, SIGNAL(clicked(bool)), this, SLOT(onTitleChecked(bool)));
    connect(m_uiWidget->chTitle_2, SIGNAL(clicked(bool)), this, SLOT(onTitleChecked(bool)));

    // Titre pour le tri
    m_uiWidget->editTitleSort->setText(songTitleSort);

    if (!songTitleSortSim)
    {
        m_uiWidget->editTitleSort->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editTitleSort, SIGNAL(textEdited(const QString&)), this, SLOT(onTitleSortChange(const QString&)));
    connect(m_uiWidget->chTitleSort, SIGNAL(clicked(bool)), this, SLOT(onTitleSortChecked(bool)));

    // Artiste
    m_uiWidget->editArtist->setText(songArtist);
    m_uiWidget->editArtist_2->setText(songArtist);

    if (!songArtistSim)
    {
        m_uiWidget->editArtist->setPlaceholderText(notSimText);
        m_uiWidget->editArtist_2->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editArtist, SIGNAL(textEdited(const QString&)), this, SLOT(onArtistChange(const QString&)));
    connect(m_uiWidget->editArtist_2, SIGNAL(textEdited(const QString&)), this, SLOT(onArtistChange(const QString&)));
    connect(m_uiWidget->chArtist, SIGNAL(clicked(bool)), this, SLOT(onArtistChecked(bool)));
    connect(m_uiWidget->chArtist_2, SIGNAL(clicked(bool)), this, SLOT(onArtistChecked(bool)));

    // Artiste pour le tri
    m_uiWidget->editArtistSort->setText(songArtistSort);

    if (!songArtistSortSim)
    {
        m_uiWidget->editArtistSort->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editArtistSort, SIGNAL(textEdited(const QString&)), this, SLOT(onArtistSortChange(const QString&)));
    connect(m_uiWidget->chArtistSort, SIGNAL(clicked(bool)), this, SLOT(onArtistSortChecked(bool)));

    // Album
    m_uiWidget->editAlbum->setText(songAlbum);
    m_uiWidget->editAlbum_2->setText(songAlbum);

    if (!songAlbumSim)
    {
        m_uiWidget->editAlbum->setPlaceholderText(notSimText);
        m_uiWidget->editAlbum_2->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editAlbum, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumChange(const QString&)));
    connect(m_uiWidget->editAlbum_2, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumChange(const QString&)));
    connect(m_uiWidget->chAlbum, SIGNAL(clicked(bool)), this, SLOT(onAlbumChecked(bool)));
    connect(m_uiWidget->chAlbum_2, SIGNAL(clicked(bool)), this, SLOT(onAlbumChecked(bool)));

    // Album pour le tri
    m_uiWidget->editAlbumSort->setText(songAlbumSort);

    if (!songAlbumSortSim)
    {
        m_uiWidget->editAlbumSort->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editAlbumSort, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumSortChange(const QString&)));
    connect(m_uiWidget->chAlbumSort, SIGNAL(clicked(bool)), this, SLOT(onAlbumSortChecked(bool)));

    // Artiste de l'album
    m_uiWidget->editAlbumArtist->setText(songAlbumArtist);
    m_uiWidget->editAlbumArtist_2->setText(songAlbumArtist);

    if (!songAlbumArtistSim)
    {
        m_uiWidget->editAlbumArtist->setPlaceholderText(notSimText);
        m_uiWidget->editAlbumArtist_2->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editAlbumArtist, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumArtistChange(const QString&)));
    connect(m_uiWidget->editAlbumArtist_2, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumArtistChange(const QString&)));
    connect(m_uiWidget->chAlbumArtist, SIGNAL(clicked(bool)), this, SLOT(onAlbumArtistChecked(bool)));
    connect(m_uiWidget->chAlbumArtist_2, SIGNAL(clicked(bool)), this, SLOT(onAlbumArtistChecked(bool)));

    // Artiste de l'album pour le tri
    m_uiWidget->editAlbumArtistSort->setText(songAlbumArtistSort);

    if (!songAlbumArtistSortSim)
    {
        m_uiWidget->editAlbumArtistSort->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editAlbumArtistSort, SIGNAL(textEdited(const QString&)), this, SLOT(onAlbumArtistSortChange(const QString&)));
    connect(m_uiWidget->chAlbumArtistSort, SIGNAL(clicked(bool)), this, SLOT(onAlbumArtistSortChecked(bool)));

    // Compositeur
    m_uiWidget->editComposer->setText(songComposer);
    m_uiWidget->editComposer_2->setText(songComposer);

    if (!songComposerSim)
    {
        m_uiWidget->editComposer->setPlaceholderText(notSimText);
        m_uiWidget->editComposer_2->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editComposer, SIGNAL(textEdited(const QString&)), this, SLOT(onComposerChange(const QString&)));
    connect(m_uiWidget->editComposer_2, SIGNAL(textEdited(const QString&)), this, SLOT(onComposerChange(const QString&)));
    connect(m_uiWidget->chComposer, SIGNAL(clicked(bool)), this, SLOT(onComposerChecked(bool)));
    connect(m_uiWidget->chComposer_2, SIGNAL(clicked(bool)), this, SLOT(onComposerChecked(bool)));

    // Compositeur pour le tri
    m_uiWidget->editComposerSort->setText(songComposerSort);

    if (!songComposerSortSim)
    {
        m_uiWidget->editComposerSort->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editComposerSort, SIGNAL(textEdited(const QString&)), this, SLOT(onComposerSortChange(const QString&)));
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
        //m_uiWidget->editComments->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editComments, SIGNAL(textEdited(const QString&)), this, SLOT(onCommentsChange(const QString&)));
    connect(m_uiWidget->chComments, SIGNAL(clicked(bool)), this, SLOT(onCommentsChecked(bool)));

    // Année
    m_uiWidget->editYear->setText(QString::number(songYear));
    m_uiWidget->editYear->setValidator(new QIntValidator(0, 9999, this));

    if (!songYearSim)
    {
        m_uiWidget->editYear->setPlaceholderText(notSimText);
    }

    connect(m_uiWidget->editYear, SIGNAL(textEdited(const QString&)), this, SLOT(onYearChange(const QString&)));
    connect(m_uiWidget->chYear, SIGNAL(clicked(bool)), this, SLOT(onYearChecked(bool)));

    // Numéro de piste
    m_uiWidget->editTrackNumber->setValidator(new QIntValidator(0, 999, this));
    m_uiWidget->editTrackCount->setValidator(new QIntValidator(0, 999, this));

    m_uiWidget->editDiscNumber->setValidator(new QIntValidator(0, 999, this));
    m_uiWidget->editDiscCount->setValidator(new QIntValidator(0, 999, this));

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


/**
 * Détruit la boite de dialogue.
 */

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
    //TODO: mettre à jour les données cochées
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


void CDialogEditSongs::onTitleChange(const QString& title)
{
    m_uiWidget->editTitle->setPlaceholderText(QString());
    m_uiWidget->editTitle_2->setPlaceholderText(QString());

    m_uiWidget->chTitle->setChecked(true);
    m_uiWidget->chTitle_2->setChecked(true);
}


void CDialogEditSongs::onTitleSortChange(const QString& title)
{
    m_uiWidget->editTitleSort->setPlaceholderText(QString());

    m_uiWidget->chTitleSort->setChecked(true);
}


void CDialogEditSongs::onArtistChange(const QString& artistName)
{
    m_uiWidget->editArtist->setPlaceholderText(QString());
    m_uiWidget->editArtist_2->setPlaceholderText(QString());

    m_uiWidget->chArtist->setChecked(true);
    m_uiWidget->chArtist_2->setChecked(true);
}


void CDialogEditSongs::onArtistSortChange(const QString& artistName)
{
    m_uiWidget->editArtistSort->setPlaceholderText(QString());

    m_uiWidget->chArtistSort->setChecked(true);
}


void CDialogEditSongs::onAlbumChange(const QString& albumTitle)
{
    m_uiWidget->editAlbum->setPlaceholderText(QString());
    m_uiWidget->editAlbum_2->setPlaceholderText(QString());

    m_uiWidget->chAlbum->setChecked(true);
    m_uiWidget->chAlbum_2->setChecked(true);
}


void CDialogEditSongs::onAlbumSortChange(const QString& albumTitle)
{
    m_uiWidget->editAlbumSort->setPlaceholderText(QString());

    m_uiWidget->chAlbumSort->setChecked(true);
}


void CDialogEditSongs::onTitleChecked(bool checked)
{
    m_uiWidget->editTitle->setPlaceholderText(QString());
    m_uiWidget->editTitle_2->setPlaceholderText(QString());
}


void CDialogEditSongs::onTitleSortChecked(bool checked)
{
    m_uiWidget->editTitleSort->setPlaceholderText(QString());
}


void CDialogEditSongs::onArtistChecked(bool checked)
{
    m_uiWidget->editArtist->setPlaceholderText(QString());
    m_uiWidget->editArtist_2->setPlaceholderText(QString());
}


void CDialogEditSongs::onArtistSortChecked(bool checked)
{
    m_uiWidget->editArtistSort->setPlaceholderText(QString());
}


void CDialogEditSongs::onAlbumChecked(bool checked)
{
    m_uiWidget->editAlbum->setPlaceholderText(QString());
    m_uiWidget->editAlbum_2->setPlaceholderText(QString());
}


void CDialogEditSongs::onAlbumSortChecked(bool checked)
{
    m_uiWidget->editAlbumSort->setPlaceholderText(QString());
}

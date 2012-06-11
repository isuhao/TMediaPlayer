
#include "CDialogEditMetadata.hpp"
#include "CSong.hpp"
#include <QPushButton>


/**
 * Construit la boite de dialogue.
 *
 * \param song Morceau dont on veut afficher les métadonnées.
 */

CDialogEditMetadata::CDialogEditMetadata(CSong * song) :
    m_uiWidget (new Ui::DialogEditMetadata()),
    m_song     (song)
{
    Q_CHECK_PTR(song);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    const QString songTitle = m_song->getTitle();
    const QString songArtist = m_song->getArtistName();

    if (songArtist.isEmpty())
    {
        setWindowTitle(tr("Metadata") + " - " + songTitle);
    }
    else
    {
        setWindowTitle(tr("Metadata") + " - " + songTitle + " - " + songArtist);
    }
    
    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    QPushButton * btnApply = m_uiWidget->buttonBox->addButton(tr("Apply"), QDialogButtonBox::ApplyRole);
    QPushButton * btnReset = m_uiWidget->buttonBox->addButton(tr("Reset"), QDialogButtonBox::ResetRole);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(btnReset, SIGNAL(clicked()), this, SLOT(reset()));

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

void CDialogEditMetadata::apply(void)
{
    //...
}


/**
 * Enregistre les modifications effectuées sur le morceau et ferme la boite de dialogue.
 */

void CDialogEditMetadata::save(void)
{
    apply();
    close();
}


/**
 * Recharge les métadonnées du morceau.
 *
 * \todo Implémentation.
 */

void CDialogEditMetadata::reset(void)
{
    //...

    // TODO: récupérer les tags du morceau suivant son format
    // TODO: masquer les onglets non-gérés par le format

    //initTabID3v1(tagID3v1...);
    //initTabID3v2(void);
    //initTabAPE(void);
    //initTabXiphComment(void);
}
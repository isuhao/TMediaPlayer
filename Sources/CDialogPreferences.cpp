
#include "CDialogPreferences.hpp"
#include "CApplication.hpp"
#include <QSettings>


/**
 * Construit la boite de dialogue des préférences.
 *
 * \todo Gérer toutes les préferences.
 */

CDialogPreferences::CDialogPreferences(CApplication * application, QSettings * settings) :
    QDialog       (application),
    m_uiWidget    (new Ui::DialogPreferences()),
    m_application (application),
    m_settings    (settings)
{
    Q_CHECK_PTR(application);
    Q_CHECK_PTR(settings);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    // Récupération des paramètres
    m_uiWidget->editRowHeight->setValue(m_settings->value("Preferences/RowHeight", 19).toInt());
    m_uiWidget->editShowButtonStop->setChecked(m_settings->value("Preferences/ShowButtonStop", true).toBool());

    // Connexions des signaux des boutons
    QPushButton * btnOK = m_uiWidget->buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    connect(btnOK, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Détruit la boite de dialogue.
 */

CDialogPreferences::~CDialogPreferences()
{
    delete m_uiWidget;
}


/**
 * Enregistre les préférences et ferme la boite de dialogue.
 *
 * \todo Gérer toutes les préferences.
 */

void CDialogPreferences::save(void)
{
    m_application->setRowHeight(m_uiWidget->editRowHeight->value());
    m_application->showButtonStop(m_uiWidget->editShowButtonStop->isChecked());

    close();
}

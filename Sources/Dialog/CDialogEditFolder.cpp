
#include "CDialogEditFolder.hpp"
#include "CApplication.hpp"
#include "CListFolder.hpp"
#include <QMessageBox>
#include <QPushButton>


CDialogEditFolder::CDialogEditFolder(CListFolder * folder, CApplication * application, CListFolder * folderParent) :
    QDialog        (application),
    m_uiWidget     (new Ui::DialogEditFolder()),
    m_folder       (folder),
    m_application  (application),
    m_folderParent (folderParent)
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    if (!m_folder)
    {
        m_folder = new CListFolder(m_application);
    }

    m_uiWidget->editName->setText(m_folder->getName());


    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Détruit le widget.
 */

CDialogEditFolder::~CDialogEditFolder()
{
    delete m_uiWidget;
}


void CDialogEditFolder::save(void)
{
    QString name = m_uiWidget->editName->text();

    if (name.isEmpty())
    {
        QMessageBox::warning(this, QString(), tr("You need to choose a name for the folder."));
        return;
    }

    m_folder->setName(name);
    m_folder->setFolder(m_folderParent);

    if (!m_folder->updateDatabase())
    {
        return;
    }

    m_application->addFolder(m_folder);

    close();
}

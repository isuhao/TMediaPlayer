
#include "CDialogRemoveFolder.hpp"
#include "CApplication.hpp"
#include "CFolder.hpp"
#include <QPushButton>

#include <QtDebug>


CDialogRemoveFolder::CDialogRemoveFolder(CApplication * application, CFolder * folder) :
    QDialog       (application, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    m_uiWidget    (new Ui::DialogRemoveFolder()),
    m_application (application),
    m_folder      (folder)
{
    Q_CHECK_PTR(application);
    Q_CHECK_PTR(folder);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);
    
    // Connexions des signaux des boutons
    QPushButton * btnYes = m_uiWidget->buttonBox->addButton(tr("Yes"), QDialogButtonBox::YesRole);
    QPushButton * btnNo = m_uiWidget->buttonBox->addButton(tr("No"), QDialogButtonBox::NoRole);
    btnNo->setDefault(true);

    connect(btnYes, SIGNAL(clicked()), this, SLOT(removeFolder()));
    connect(btnNo, SIGNAL(clicked()), this, SLOT(close()));

    //...
}


CDialogRemoveFolder::~CDialogRemoveFolder()
{
    delete m_uiWidget;
}


void CDialogRemoveFolder::removeFolder(void)
{
    qDebug() << "Suppression du dossier...";

    //...

    close();
}


#include "CImporterITunes.hpp"
#include "CApplication.hpp"
#include <QFileDialog>


CImporterITunes::CImporterITunes(CApplication * application) :
    QWizard       (application),
    m_application (application),
    m_uiWidget    (new Ui::DialogImportITunes())
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    connect(m_uiWidget->btnFileName, SIGNAL(clicked()), this, SLOT(chooseFile()));
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(onPageChanged(int)));
}

    
/**
 * Détruit la boite de dialogue.
 */

CImporterITunes::~CImporterITunes()
{
    delete m_uiWidget;
}


void CImporterITunes::chooseFile(void)
{
    const QString fileName = QFileDialog::getOpenFileName(this, QString(), "iTunes Music Library.xml");

    //...
}


void CImporterITunes::onPageChanged(int page)
{
    if (page == 1)
    {
        //...
    }

    //...
}

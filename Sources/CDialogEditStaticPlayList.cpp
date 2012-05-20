
#include "CDialogEditStaticPlayList.hpp"
#include "CStaticPlayList.hpp"
#include "CApplication.hpp"
#include <QStandardItemModel>
#include <QMessageBox>


CDialogEditStaticPlayList::CDialogEditStaticPlayList(CStaticPlayList * playList, CApplication * application) :
    QDialog       (application),
    m_uiWidget    (new Ui::DialogEditStaticPlayList()),
    m_application (application),
    m_playList    (playList)
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    if (!m_playList)
    {
        m_playList = new CStaticPlayList(m_application);
    }

    m_uiWidget->editName->setText(m_playList->getName());

    connect(m_uiWidget->buttonBox, SIGNAL(accepted()), this, SLOT(onSave()));
    connect(m_uiWidget->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}


CDialogEditStaticPlayList::~CDialogEditStaticPlayList()
{
    delete m_uiWidget;
}


/// \todo Implémentation
void CDialogEditStaticPlayList::onSave(void)
{
    QString name = m_uiWidget->editName->text();

    if (name.isEmpty())
    {
        QMessageBox::warning(this, QString(), tr("You need to choose a name for the playlist."));
        return;
    }

    m_playList->setName(name);

    if (!m_playList->updateDatabase())
    {
        return;
    }

    m_application->addPlayList(m_playList);

    close();
}

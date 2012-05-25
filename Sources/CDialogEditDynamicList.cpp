
#include "CDialogEditDynamicList.hpp"
#include "CDynamicPlayList.hpp"
#include "CWidgetMultiCriterion.hpp"
#include "CApplication.hpp"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QPushButton>


/**
 * Constructeur de la boite de dialogue d'édition des listes de lecture dynamiques.
 *
 * \param playList Pointeur sur la liste à modifier, ou NULL pour une nouvelle liste.
 */

CDialogEditDynamicList::CDialogEditDynamicList(CDynamicPlayList * playList, CApplication * application) :
    QDialog       (application),
    m_uiWidget    (new Ui::DialogEditDynamicPlayList()),
    m_playList    (playList),
    m_application (application)
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    CWidgetMultiCriterion * widgetCriterion = new CWidgetMultiCriterion(this);
    //m_uiWidget->scrollArea->setWidget(widgetCriterion);
    m_uiWidget->verticalLayout->insertWidget(1, widgetCriterion);

    if (m_playList)
    {
        m_uiWidget->editName->setText(m_playList->getName());
    }

    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}


CDialogEditDynamicList::~CDialogEditDynamicList()
{
    delete m_uiWidget;
}


/// \todo Implémentation
void CDialogEditDynamicList::save(void)
{
    QString name = m_uiWidget->editName->text();

    if (name.isEmpty())
    {
        QMessageBox::warning(this, QString(), tr("You need to choose a name for the playlist."));
        return;
    }

    if (!m_playList)
    {
        m_playList = new CDynamicPlayList(m_application);
    }

    m_playList->setName(name);
    //...

    if (!m_playList->updateDatabase())
    {
        delete m_playList;
        m_playList = NULL;
        return;
    }

    m_application->addPlayList(m_playList);

    close();
}


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
 * \todo Charger les critères si la liste existe déjà.
 *
 * \param playList    Pointeur sur la liste à modifier, ou NULL pour une nouvelle liste.
 * \param application Pointeur sur l'application.
 */

CDialogEditDynamicList::CDialogEditDynamicList(CDynamicPlayList * playList, CApplication * application) :
    QDialog           (application),
    m_uiWidget        (new Ui::DialogEditDynamicPlayList()),
    m_widgetCriterion (NULL),
    m_playList        (playList),
    m_application     (application)
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    m_uiWidget->setupUi(this);

    if (m_playList)
    {
        m_widgetCriterion = m_playList->getWidget();

        if (m_widgetCriterion)
        {
            m_uiWidget->verticalLayout->insertWidget(1, m_widgetCriterion);
            m_widgetCriterion->setParent(this);
        }
        else
        {
            m_widgetCriterion = new CWidgetMultiCriterion(m_application, this);
            m_uiWidget->verticalLayout->insertWidget(1, m_widgetCriterion);
        }
    }
    else
    {
        m_playList = new CDynamicPlayList(m_application);

        m_widgetCriterion = new CWidgetMultiCriterion(m_application, this);
        m_uiWidget->verticalLayout->insertWidget(1, m_widgetCriterion);
    }

    m_uiWidget->editName->setText(m_playList->getName());
    resizeWindow();

    // Connexions des signaux des boutons
    QPushButton * btnSave = m_uiWidget->buttonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    QPushButton * btnCancel = m_uiWidget->buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}


/**
 * Détruit le widget.
 */

CDialogEditDynamicList::~CDialogEditDynamicList()
{
    delete m_uiWidget;
}


/**
 * Redimensionne la boite de dialogue à chaque ajout ou suppression de critère.
 */

void CDialogEditDynamicList::resizeWindow(void)
{
    setMinimumSize(0, 0);
    resize(size().width(), 1);
}


/**
 * Enregistre les paramètres de la liste de lecture dynamique.
 */

void CDialogEditDynamicList::save(void)
{
    QString name = m_uiWidget->editName->text();

    if (name.isEmpty())
    {
        QMessageBox::warning(this, QString(), tr("You need to choose a name for the playlist."));
        return;
    }

    m_playList->setName(name);
    m_playList->setCriteria(m_widgetCriterion->getCriteria());

    if (!m_playList->updateDatabase())
    {
        return;
    }

    m_application->addPlayList(m_playList);

    close();
}

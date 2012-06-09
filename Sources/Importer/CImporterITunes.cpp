
#include "CImporterITunes.hpp"
#include "CApplication.hpp"
#include <QFileDialog>
#include <QStandardItem>
#include <QStandardItemModel>

#include <QtDebug>


CImporterITunes::CImporterITunes(CApplication * application) :
    QWizard       (application),
    m_application (application),
    m_library     (new CITunesLibrary(this)),
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
    m_uiWidget->editFileName->setText(QFileDialog::getOpenFileName(this, QString(), "iTunes Music Library.xml"));
}


void CImporterITunes::onPageChanged(int page)
{
    if (page == 1)
    {
        qDebug() << "CImporterITunes::onPageChanged() : page 1";
        QStandardItemModel * model = new QStandardItemModel(this);
        m_uiWidget->treeView->setModel(model);

        m_library->loadFile(m_uiWidget->editFileName->text());
        m_library->initModelWithLists(model);
    }
}


CITunesLibrary::CITunesLibrary(QObject * parent) :
    QObject    (parent),
    m_isLoaded (false)
{

}


CITunesLibrary::~CITunesLibrary()
{

}


bool CITunesLibrary::loadFile(const QString& fileName)
{
    m_isLoaded = false;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "CITunesLibrary::CITunesLibrary() : erreur lors de l'ouverture du fichier " << fileName;
        return false;
    }

    if (!m_document.setContent(&file))
    {
        file.close();
        qWarning() << "CITunesLibrary::CITunesLibrary() : le fichier " << fileName << " n'est pas un document XML valide";
        return false;
    }

    file.close();

    m_isLoaded = true;
    return true;
}


void CITunesLibrary::initModelWithLists(QStandardItemModel * model) const
{
    Q_CHECK_PTR(model);

    if (!m_isLoaded)
    {
        return;
    }

    QDomElement node = m_document.documentElement();

    if (node.tagName() != "plist")
    {
        qWarning() << "CITunesLibrary::initModelWithLists() : le fichier n'est pas valide (élément 'plist' attendu)";
        return;
    }

    node = node.firstChildElement();

    if (node.tagName() != "dict")
    {
        qWarning() << "CITunesLibrary::initModelWithLists() : le fichier n'est pas valide (élément 'dict' attendu)";
        return;
    }


    struct TList
    {
        QString listID;
        QString listName;
        QString folderID;
        bool isFolder;
        bool isDynamic;
        QStandardItem * item;
        QList<TList> children;

        TList(void) : isFolder(false), isDynamic(false), item(NULL) { }
    };

    QMap<QString, TList> playLists;


    for (node = node.firstChildElement(); !node.isNull(); node = node.nextSibling().toElement())
    {
        if (node.tagName() != "key")
        {
            qWarning() << "CITunesLibrary::initModelWithLists() : le fichier n'est pas valide (élément 'key' attendu)";
            continue;
        }

        if (node.text() == "Playlists")
        {
            node = node.nextSibling().toElement();

            if (node.tagName() != "array")
            {
                qWarning() << "CITunesLibrary::initModelWithLists() : le fichier n'est pas valide (élément 'array' attendu)";
                continue;
            }
            
            // Parcours des listes de lecture
            for (QDomElement nodeList = node.firstChildElement(); !nodeList.isNull(); nodeList = nodeList.nextSibling().toElement())
            {
                if (nodeList.tagName() != "dict")
                {
                    qWarning() << "CITunesLibrary::initModelWithLists() : le fichier n'est pas valide (élément 'dict' attendu)";
                    continue;
                }

                bool playListOK = true;
                bool isFolder = false;
                bool isDynamic = false;
                QString playListName;
                QString playListID;
                QString folderID;
                
                // Parcours des attributs de la liste de lecture
                for (QDomElement nodeListAttr = nodeList.firstChildElement(); !nodeListAttr.isNull(); nodeListAttr = nodeListAttr.nextSibling().toElement())
                {
                    if (nodeListAttr.tagName() == "array")
                    {
                        //...
                        continue;
                    }
                    else if (nodeListAttr.tagName() != "key")
                    {
                        qWarning() << "CITunesLibrary::initModelWithLists() : le fichier n'est pas valide (élément 'key' attendu)";
                        continue;
                    }

                    QDomElement nodeListAttrValue = nodeListAttr.nextSibling().toElement();

                    if (nodeListAttr.text() == "Master" || nodeListAttr.text() == "Music")
                    {
                        playListOK = false;
                        break;
                    }
                    else if (nodeListAttr.text() == "Folder")
                    {
                        isFolder = true;
                    }
                    else if (nodeListAttr.text() == "Parent Persistent ID")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::initModelWithLists() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        folderID = nodeListAttrValue.text();
                    }
                    else if (nodeListAttr.text() == "Playlist Persistent ID")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::initModelWithLists() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        playListID = nodeListAttrValue.text();
                    }
                    else if (nodeListAttr.text() == "Smart Info" || nodeListAttr.text() == "Smart Criteria")
                    {
                        isDynamic = true;
                    }
                    else if (nodeListAttr.text() == "Name")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::initModelWithLists() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        playListName = nodeListAttrValue.text();
                    }

                    nodeListAttr = nodeListAttr.nextSibling().toElement();
                }

                if (playListOK && !playListName.isEmpty() && !playListID.isEmpty())
                {
                    TList l;

                    l.listName  = playListName;
                    l.listID    = playListID;
                    l.folderID  = folderID;
                    l.isFolder  = isFolder;
                    l.isDynamic = isDynamic;
                    l.item      = new QStandardItem(playListName);
                    l.item->setCheckable(true);
                    l.item->setEnabled(false);

                    if (isDynamic)
                        l.item->setIcon(QPixmap(":/icons/dynamic_list"));
                    else if (isFolder)
                        l.item->setIcon(QPixmap(":/icons/folder_close"));
                    else
                        l.item->setIcon(QPixmap(":/icons/playlist"));

                    playLists[playListID] = l;
                }
            }
        }
        else
        {
            node = node.nextSibling().toElement();
        }
    }

    // Remplissage du modèle
    QStandardItem * item = new QStandardItem(QPixmap(":/icons/library"), tr("Songs"));
    item->setCheckable(true);
    item->setCheckState(Qt::Checked);
    model->appendRow(item);

    QMap<QString, TList> playListsCopy = playLists;

    for (QMap<QString, TList>::const_iterator it = playListsCopy.begin(); it != playListsCopy.end(); ++it)
    {
        if (!it->folderID.isEmpty())
        {
            if (it->isFolder)
            {
                qWarning() << "CITunesLibrary::initModelWithLists() : un dossier ne peut pas avoir de dossier parent";
                continue;
            }
            else if (!playLists[it->folderID].isFolder)
            {
                qWarning() << "CITunesLibrary::initModelWithLists() : dossier invalide";
                continue;
            }
            else
            {
                playLists[it->folderID].children.append(*it);
                playLists.remove(it.key());
                //playLists[it->folderID].item->appendRow(it->item);
            }
        }
        else
        {
            //model->appendRow(it->item);
        }
    }

    // Ajout des dossiers
    for (QMap<QString, TList>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        if (it->isFolder)
        {
            QMap<QString, TList> lists;

            // Ajout des listes dynamiques
            for (QList<TList>::const_iterator it2 = it->children.begin(); it2 != it->children.end(); ++it2)
            {
                if (it2->isDynamic)
                {
                    lists[it2->listName] = *it2;
                }
            }

            for (QMap<QString, TList>::const_iterator it2 = lists.begin(); it2 != lists.end(); ++it2)
            {
                it->item->appendRow(it2->item);
            }

            lists.clear();

            // Ajout des listes statiques
            for (QList<TList>::const_iterator it2 = it->children.begin(); it2 != it->children.end(); ++it2)
            {
                if (!it2->isDynamic)
                {
                    lists[it2->listName] = *it2;
                }
            }

            for (QMap<QString, TList>::const_iterator it2 = lists.begin(); it2 != lists.end(); ++it2)
            {
                it->item->appendRow(it2->item);
            }

            model->appendRow(it->item);
        }
    }

    QMap<QString, TList> lists;

    // Ajout des listes dynamiques
    for (QMap<QString, TList>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        if (it->isDynamic)
        {
            lists[it->listName] = *it;
        }
    }

    for (QMap<QString, TList>::const_iterator it = lists.begin(); it != lists.end(); ++it)
    {
        model->appendRow(it->item);
    }

    lists.clear();

    // Ajout des listes statiques
    for (QMap<QString, TList>::const_iterator it = playLists.begin(); it != playLists.end(); ++it)
    {
        if (!it->isFolder && !it->isDynamic)
        {
            lists[it->listName] = *it;
        }
    }

    for (QMap<QString, TList>::const_iterator it = lists.begin(); it != lists.end(); ++it)
    {
        model->appendRow(it->item);
    }
}


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


/**
 * Chargement du fichier library de iTunes.
 *
 * \param fileName Localisation du fichier XML.
 * \return Booléen indiquant si le chargement s'est bien passé.
 */

bool CITunesLibrary::loadFile(const QString& fileName)
{
    if (m_isLoaded && m_fileName == fileName)
    {
        return true;
    }

    m_isLoaded = false;
    m_fileName = QString();
    m_songs.clear();
    m_folders.clear();
    m_dynamicLists.clear();
    m_staticLists.clear();

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "CITunesLibrary::loadFile() : erreur lors de l'ouverture du fichier " << fileName;
        return false;
    }

    if (!m_document.setContent(&file))
    {
        file.close();
        qWarning() << "CITunesLibrary::loadFile() : le fichier " << fileName << " n'est pas un document XML valide";
        return false;
    }

    file.close();

    QDomElement node = m_document.documentElement();

    if (node.tagName() != "plist")
    {
        qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'plist' attendu)";
        return false;
    }

    node = node.firstChildElement();

    if (node.tagName() != "dict")
    {
        qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'dict' attendu)";
        return false;
    }


    // Chargement du fichier
    for (node = node.firstChildElement(); !node.isNull(); node = node.nextSibling().toElement())
    {
        if (node.tagName() != "key")
        {
            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'key' attendu)";
            continue;
        }
        
        // Liste des morceaux
        if (node.text() == "Tracks")
        {
            node = node.nextSibling().toElement();

            if (node.tagName() != "dict")
            {
                qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'dict' attendu)";
                continue;
            }
            
            // Parcours de la liste des morceaux
            for (QDomElement nodeList = node.firstChildElement(); !nodeList.isNull(); nodeList = nodeList.nextSibling().toElement())
            {
                if (nodeList.tagName() != "key")
                {
                    qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'key' attendu)";
                    continue;
                }

                nodeList = nodeList.nextSibling().toElement();

                if (nodeList.tagName() != "dict")
                {
                    qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'dict' attendu)";
                    continue;
                }

                TSong song;
            
                // Parcours de la liste des attributs du morceau
                for (QDomElement nodeListAttr = nodeList.firstChildElement(); !nodeListAttr.isNull(); nodeListAttr = nodeListAttr.nextSibling().toElement())
                {
                    if (nodeListAttr.tagName() != "key")
                    {
                        qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'key' attendu)";
                        continue;
                    }

                    QDomElement nodeListAttrValue = nodeListAttr.nextSibling().toElement();

                    if (nodeListAttr.text() == "Track ID")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        song.id = nodeListAttrValue.text().toInt();
                    }
                    else if (nodeListAttr.text() == "Name")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Artist")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Album Artist")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Album")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Genre")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Kind")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Size")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Total Time")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Track Number")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Track Count")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Year")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Date Modified")
                    {
                        if (nodeListAttrValue.tagName() != "date")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'date' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Date Added")
                    {
                        if (nodeListAttrValue.tagName() != "date")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'date' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Bit Rate")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Sample Rate")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Play Count")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        song.playCount = nodeListAttrValue.text().toInt();
                    }
                    else if (nodeListAttr.text() == "Play Date")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Play Date UTC")
                    {
                        if (nodeListAttrValue.tagName() != "date")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'date' attendu)";
                            continue;
                        }

                        song.lastPlayed = QDateTime::fromString(nodeListAttrValue.text(), "yyyy-MM-ddTHH:mm:ssZ");
                    }
                    else if (nodeListAttr.text() == "Skip Count")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Skip Date")
                    {
                        if (nodeListAttrValue.tagName() != "date")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'date' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Rating")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        song.rating = nodeListAttrValue.text().toInt() / 20;
                    }
                    else if (nodeListAttr.text() == "Album Rating")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Album Rating Computed")
                    {
                        if (nodeListAttrValue.tagName() != "true")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'true' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Sort Album")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Persistent ID")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Track Type")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //File

                        //...
                    }
                    else if (nodeListAttr.text() == "Location")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        song.fileName = nodeListAttrValue.text();
                    }
                    else if (nodeListAttr.text() == "File Folder Count")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Library Folder Count")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Comments")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Grouping")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Composer")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Sort Name")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Sort Artist")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Sort Composer")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Sort Album Artist")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Disc Number")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Disc Count")
                    {
                        if (nodeListAttrValue.tagName() != "integer")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'integer' attendu)";
                            continue;
                        }

                        //...
                    }
                    else if (nodeListAttr.text() == "Disabled")
                    {
                        if (nodeListAttrValue.tagName() != "true")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'true' attendu)";
                            continue;
                        }

                        song.enabled = false;
                    }
                    //Compilation
                    //Part Of Gapless Album
                    //Volume Adjustment
                    //Start Time
                    //BPM
                    else
                    {
                        qDebug() << "CITunesLibrary::loadFile() : unknown key " << nodeListAttr.text() << " for a song";
                    }

                    nodeListAttr = nodeListAttr.nextSibling().toElement();
                }

                if (song.id > 0)
                {
                    m_songs[song.id] = song;
                }
            }
        }
        // Listes de lecture
        else if (node.text() == "Playlists")
        {
            node = node.nextSibling().toElement();

            if (node.tagName() != "array")
            {
                qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'array' attendu)";
                continue;
            }

            QMap<QString, QString> dynamicListParents;
            QMap<QString, QString> staticListParents;
            QMap<QString, TDynamicList> dynamicLists;
            QMap<QString, TStaticList> staticLists;
            
            // Parcours des listes de lecture
            for (QDomElement nodeList = node.firstChildElement(); !nodeList.isNull(); nodeList = nodeList.nextSibling().toElement())
            {
                if (nodeList.tagName() != "dict")
                {
                    qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'dict' attendu)";
                    continue;
                }

                bool playListOK = true;
                bool isFolder = false;
                bool isDynamic = false;
                QString playListName;
                QString playListID;
                QString folderID;
                QList<int> songs;
                
                // Parcours des attributs de la liste de lecture
                for (QDomElement nodeListAttr = nodeList.firstChildElement(); !nodeListAttr.isNull(); nodeListAttr = nodeListAttr.nextSibling().toElement())
                {
                    if (nodeListAttr.tagName() == "array")
                    {
/*
			<array>
				<dict><key>Track ID</key><integer>11390</integer></dict>
				<dict><key>Track ID</key><integer>26862</integer></dict>
			</array>
*/
                        continue;
                    }
                    else if (nodeListAttr.tagName() != "key")
                    {
                        qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'key' attendu)";
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
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        folderID = nodeListAttrValue.text();
                    }
                    else if (nodeListAttr.text() == "Playlist Persistent ID")
                    {
                        if (nodeListAttrValue.tagName() != "string")
                        {
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
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
                            qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément 'string' attendu)";
                            continue;
                        }

                        playListName = nodeListAttrValue.text();
                    }

                    nodeListAttr = nodeListAttr.nextSibling().toElement();
                }

                if (playListOK && !playListName.isEmpty() && !playListID.isEmpty())
                {
                    if (isFolder)
                    {
                        TFolder folder;
                        folder.name = playListName;
                        folder.id   = playListID;
                        m_folders.append(folder);
                    }
                    else if (isDynamic)
                    {
                        if (folderID.isEmpty())
                        {
                            TDynamicList list;
                            list.name = playListName;
                            list.id   = playListID;
                            m_dynamicLists.append(list);
                        }
                        else
                        {
                            dynamicListParents[playListID] = folderID;
                            dynamicLists[playListID].name = playListName;
                            dynamicLists[playListID].id   = playListID;
                        }
                    }
                    else
                    {
                        if (folderID.isEmpty())
                        {
                            TStaticList list;
                            list.name = playListName;
                            list.id   = playListID;
                            m_staticLists.append(list);
                        }
                        else
                        {
                            staticListParents[playListID] = folderID;
                            staticLists[playListID].name = playListName;
                            staticLists[playListID].id   = playListID;
                        }
                    }
                }
            }

            for (QMap<QString, TDynamicList>::const_iterator it = dynamicLists.begin(); it != dynamicLists.end(); ++it)
            {
                int folderIndex = -1, index = 0;
                QString parentID = dynamicListParents.value(it.key());

                for (QList<TFolder>::const_iterator it2 = m_folders.begin(); it2 != m_folders.end(); ++it2, ++index)
                {
                    if (it2->id == parentID)
                    {
                        folderIndex = index;
                        break;
                    }
                }

                if (folderIndex == -1)
                {
                    qWarning() << "CITunesLibrary::loadFile() : dossier introuvable";
                    m_dynamicLists.append(it.value());
                }
                else
                {
                    m_folders[folderIndex].dynamicLists.append(it.value());
                }
            }

            for (QMap<QString, TStaticList>::const_iterator it = staticLists.begin(); it != staticLists.end(); ++it)
            {
                int folderIndex = -1, index = 0;
                QString parentID = staticListParents.value(it.key());

                for (QList<TFolder>::const_iterator it2 = m_folders.begin(); it2 != m_folders.end(); ++it2, ++index)
                {
                    if (it2->id == parentID)
                    {
                        folderIndex = index;
                        break;
                    }
                }

                if (folderIndex == -1)
                {
                    qWarning() << "CITunesLibrary::loadFile() : dossier introuvable";
                    m_staticLists.append(it.value());
                }
                else
                {
                    m_folders[folderIndex].staticLists.append(it.value());
                }
            }
        }
        else
        {
            node = node.nextSibling().toElement();
        }
    }

    m_fileName = fileName;
    m_isLoaded = true;
    return true;
}

/*
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
*/

/**
 * Initialise un modèle avec les données de la médiathèque de iTunes.
 * Les dossiers et les listes de lecture sont ajoutés sous forme d'arborescence.
 *
 * \todo Trier les dossiers et les listes par nom.
 *
 * \param model Modèle à initialiser.
 */

void CITunesLibrary::initModelWithLists(QStandardItemModel * model) const
{
    Q_CHECK_PTR(model);

    if (!m_isLoaded)
    {
        return;
    }

    // Remplissage du modèle
    QStandardItem * item = new QStandardItem(QPixmap(":/icons/library"), tr("%1 (%n song(s))", "", m_songs.size()).arg(tr("Library")));
    item->setCheckable(true);
    item->setCheckState(Qt::Checked);
    model->appendRow(item);

/*
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
*/

    // Dossiers
    for (QList<TFolder>::const_iterator it = m_folders.begin(); it != m_folders.end(); ++it)
    {
        QStandardItem * itemFolder = new QStandardItem(QPixmap(":/icons/folder_close"), it->name);
        itemFolder->setData(it->id, Qt::UserRole + 1);
        itemFolder->setCheckable(true);
        itemFolder->setEnabled(false);
        model->appendRow(itemFolder);

        // Listes dynamiques
        for (QList<TDynamicList>::const_iterator it2 = it->dynamicLists.begin(); it2 != it->dynamicLists.end(); ++it2)
        {
            QStandardItem * item = new QStandardItem(QPixmap(":/icons/dynamic_list"), it2->name);
            item->setData(it2->id, Qt::UserRole + 1);
            item->setCheckable(true);
            item->setEnabled(false);
            itemFolder->appendRow(item);
        }

        // Listes statiques
        for (QList<TStaticList>::const_iterator it2 = it->staticLists.begin(); it2 != it->staticLists.end(); ++it2)
        {
            QStandardItem * item = new QStandardItem(QPixmap(":/icons/playlist"), tr("%1 (%n song(s))", "", it2->songs.size()).arg(it2->name));
            item->setData(it2->id, Qt::UserRole + 1);
            item->setCheckable(true);
            item->setEnabled(false);
            itemFolder->appendRow(item);
        }
    }

    // Listes dynamiques
    for (QList<TDynamicList>::const_iterator it = m_dynamicLists.begin(); it != m_dynamicLists.end(); ++it)
    {
        QStandardItem * item = new QStandardItem(QPixmap(":/icons/dynamic_list"), it->name);
        item->setData(it->id, Qt::UserRole + 1);
        model->appendRow(item);
    }

    // Listes statiques
    for (QList<TStaticList>::const_iterator it = m_staticLists.begin(); it != m_staticLists.end(); ++it)
    {
        QStandardItem * item = new QStandardItem(QPixmap(":/icons/static_list"), tr("%1 (%n song(s))", "", it->songs.size()).arg(it->name));
        item->setData(it->id, Qt::UserRole + 1);
        model->appendRow(item);
    }
}

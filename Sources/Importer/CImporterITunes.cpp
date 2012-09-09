/*
Copyright (C) 2012 Teddy Michel

This file is part of TMediaPlayer.

TMediaPlayer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TMediaPlayer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TMediaPlayer. If not, see <http://www.gnu.org/licenses/>.
*/

#include "CImporterITunes.hpp"
#include "CApplication.hpp"
#include <QFileDialog>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>
#include <QLineEdit>
#include <QWizardPage>
#include <QUrl>
#include <QSqlQuery>
#include <QSqlError>

#include <QtDebug>


CImporterITunes::CImporterITunes(CApplication * application) :
    QWizard       (application),
    m_application (application),
    m_library     (new CITunesLibrary(application, this))
{
    Q_CHECK_PTR(application);

    setAttribute(Qt::WA_DeleteOnClose);
    resize(400, 300);
    setModal(true);
    setOptions(QWizard::HelpButtonOnRight|QWizard::NoBackButtonOnStartPage);
    setWindowTitle(tr("Import from iTunes"));

    m_page1 = new CITunesWizardPage1(m_library, this);
    m_page2 = new CITunesWizardPage2(m_library, this);
    m_page3 = new CITunesWizardPage3(m_library, this);
    m_page4 = new CITunesWizardPage4(m_application, m_library, this);

    addPage(m_page1);
    addPage(m_page2);
    addPage(m_page3);
    addPage(m_page4);

    setButtonText(QWizard::BackButton, tr("&Back"));
    setButtonText(QWizard::NextButton, tr("&Next"));
    setButtonText(QWizard::FinishButton, tr("&Finish"));
    setButtonText(QWizard::CommitButton, tr("&Import"));
    setButtonText(QWizard::CancelButton, tr("Cancel"));
}

    
/**
 * Détruit la boite de dialogue.
 */

CImporterITunes::~CImporterITunes()
{

}


QStringList CImporterITunes::getSelectedItems(void) const
{
    return m_page2->getSelectedItems();
}


bool CImporterITunes::needToImportSongs(void) const
{
    return m_page2->needToImportSongs();
}


CITunesWizardPage1::CITunesWizardPage1(CITunesLibrary * library, QWidget * parent) :
    QWizardPage (parent),
    m_library   (library)
{
    Q_CHECK_PTR(library);

    setTitle(tr("Library localisation"));
    setSubTitle(tr("Select the file which contains the iTunes library."));

    m_gridLayout = new QGridLayout(this);
    m_lblFileName = new QLabel(tr("Library:"), this);
    m_gridLayout->addWidget(m_lblFileName, 0, 0, 1, 1);
    m_editFileName = new QLineEdit(this);
    registerField("fileName", m_editFileName);
    m_gridLayout->addWidget(m_editFileName, 0, 1, 1, 1);
    m_btnFileName = new QToolButton(this);
    m_btnFileName->setText(tr("..."));
    m_gridLayout->addWidget(m_btnFileName, 0, 2, 1, 1);

    connect(m_editFileName, SIGNAL(textChanged(const QString&)), this, SIGNAL(completeChanged()));
    connect(m_btnFileName, SIGNAL(clicked()), this, SLOT(chooseFile()));
}

    
bool CITunesWizardPage1::isComplete(void) const
{
    return (!m_editFileName->text().isEmpty() && QWizardPage::isComplete());
}


void CITunesWizardPage1::chooseFile(void)
{
    m_editFileName->setText(QFileDialog::getOpenFileName(this, QString(), "iTunes Music Library.xml"));
}


CITunesWizardPage2::CITunesWizardPage2(CITunesLibrary * library, QWidget * parent) :
    QWizardPage (parent),
    m_library   (library)
{
    Q_CHECK_PTR(library);

    setTitle(tr("Items to import"));
    setSubTitle(tr("Select items you want to import in your library."));
    setCommitPage(true);

    m_gridLayout = new QGridLayout(this);
    m_treeView = new QTreeView(this);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->setUniformRowHeights(true);
    m_treeView->header()->setVisible(false);
    m_gridLayout->addWidget(m_treeView, 0, 0, 1, 1);

    m_model = new QStandardItemModel(this);
    m_treeView->setModel(m_model);
}


void CITunesWizardPage2::initializePage(void)
{
    QString fileName = field("fileName").toString();
    m_library->loadFile(fileName);
    m_library->initModelWithLists(m_model);
}


void CITunesWizardPage2::cleanupPage(void)
{
    m_model->clear();
}


QStringList CITunesWizardPage2::getSelectedItems(void) const
{
    QStringList res;

    for (int row = 1; row < m_model->rowCount(); ++row)
    {
        QStandardItem * item = m_model->item(row);

        if (item->checkState() == Qt::Checked)
        {
            res.append(item->data(Qt::UserRole + 1).toString());
        }
    }

    return res;
}


bool CITunesWizardPage2::needToImportSongs(void) const
{
    if (m_model)
        return (m_model->item(0)->checkState() == Qt::Checked);

    return false;
}


CITunesWizardPage3::CITunesWizardPage3(CITunesLibrary * library, QWidget * parent) :
    QWizardPage (parent),
    m_library   (library),
    m_uiWidget  (new Ui::ImporterITunesPage3())
{
    Q_CHECK_PTR(library);

    setTitle(tr("Solving problems"));
    setSubTitle(tr("What do you want to deal with songs already in your library?"));
    setCommitPage(true);

    m_uiWidget->setupUi(this);

    registerField("dataNeverChange", m_uiWidget->dataNeverChange);
    registerField("dataAlwaysChange", m_uiWidget->dataAlwaysChange);
    registerField("dataChangeWhenNeeded", m_uiWidget->dataChangeWhenNeeded);

    registerField("playsNeverChange", m_uiWidget->playsNeverChange);
    registerField("playsAlwaysChange", m_uiWidget->playsAlwaysChange);
    registerField("playsMerge", m_uiWidget->playsMerge);
}


CITunesWizardPage3::~CITunesWizardPage3()
{
    delete m_uiWidget;
}


CITunesWizardPage4::CITunesWizardPage4(CApplication * application, CITunesLibrary * library, QWidget * parent) :
    QWizardPage   (parent),
    m_library     (library),
    m_application (application)
{
    Q_CHECK_PTR(library);
    Q_CHECK_PTR(application);

    setTitle(tr("Import done"));
    setSubTitle(tr("All selected items have been imported into the library."));
    setCommitPage(true);
}


/// \todo Implémentation.
void CITunesWizardPage4::initializePage(void)
{
    QSqlQuery query(m_application->getDataBase());
    QMap<int, CSong *> songs;

    // Ajout des morceaux
    if (qobject_cast<CImporterITunes *>(wizard())->needToImportSongs())
    {
        QMap<int, CITunesLibrary::TSong> songsToLoad = m_library->getSongs();

        for (QMap<int, CITunesLibrary::TSong>::const_iterator it2 = songsToLoad.begin(); it2 != songsToLoad.end(); ++it2)
        {
            songs[it2.key()] = m_application->getSongFromId(CSong::getId(m_application, it2->fileName));

            if (songs[it2.key()])
            {
                if (field("dataAlwaysChange").toBool())
                {
                    query.prepare("UPDATE song SET song_enabled = ?, song_compilation = ?, song_rating = ? "
                                  "WHERE song_id = ?");

                    query.bindValue(0, it2->enabled);
                    query.bindValue(1, it2->compilation);
                    query.bindValue(2, it2->rating);
                    query.bindValue(3, songs[it2.key()]->getId());

                    if (!query.exec())
                    {
                        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                        return;
                    }
                }
                else if (field("dataChangeWhenNeeded").toBool())
                {
                    query.prepare("UPDATE song SET song_enabled = ?, song_compilation = ?, song_rating = ? "
                                  "WHERE song_id = ?");

                    bool songEnable = songs[it2.key()]->isEnabled();
                    bool songCompilation = songs[it2.key()]->isCompilation();
                    int songRating = songs[it2.key()]->getRating();

                    query.bindValue(0, songEnable ? true : it2->enabled);
                    query.bindValue(1, songCompilation ? true : it2->compilation);
                    query.bindValue(2, songRating != 0 ? songRating : it2->rating);
                    query.bindValue(3, songs[it2.key()]->getId());

                    if (!query.exec())
                    {
                        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                        return;
                    }
                }

                if (field("playsAlwaysChange").toBool())
                {
                    query.prepare("UPDATE song SET song_play_count = ?, song_play_time = ? "
                                  "WHERE song_id = ?");

                    query.bindValue(0, it2->playCount);
                    query.bindValue(1, it2->lastPlayed);
                    query.bindValue(2, songs[it2.key()]->getId());

                    if (!query.exec())
                    {
                        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                        return;
                    }
                    
                    // Ajout des lectures
                    query.prepare("DELETE FROM play WHERE song_id = ?");
                    query.bindValue(0, songs[it2.key()]->getId());

                    if (!query.exec())
                    {
                        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                        return;
                    }

                    query.prepare("INSERT INTO play (song_id, play_time) VALUES (?, ?)");
                    query.bindValue(0, songs[it2.key()]->getId());

                    for (int play = 0; play < it2->playCount; ++play)
                    {
                        if (play == it2->playCount - 1)
                            query.bindValue(1, it2->lastPlayed);
                        else
                            query.bindValue(1, QDateTime());

                        if (!query.exec())
                        {
                            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                            return;
                        }
                    }
                }
                else if (field("playsMerge").toBool())
                {
                    int songPlayCount = songs[it2.key()]->getNumPlays() + it2->playCount;
                    QDateTime songPlayTime = qMax(songs[it2.key()]->getLastPlay(), it2->lastPlayed);
                    
                    query.prepare("UPDATE song SET song_play_count = ?, song_play_time = ? "
                                  "WHERE song_id = ?");

                    query.bindValue(0, songPlayCount);
                    query.bindValue(1, songPlayTime);
                    query.bindValue(2, songs[it2.key()]->getId());

                    if (!query.exec())
                    {
                        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                        return;
                    }

                    // Ajout des lectures
                    query.prepare("INSERT INTO play (song_id, play_time) VALUES (?, ?)");
                    query.bindValue(0, songs[it2.key()]->getId());

                    for (int play = 0; play < it2->playCount; ++play)
                    {
                        if (play == it2->playCount - 1)
                            query.bindValue(1, it2->lastPlayed);
                        else
                            query.bindValue(1, QDateTime());

                        if (!query.exec())
                        {
                            m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                            return;
                        }
                    }
                }
            }
            else
            {
                songs[it2.key()] = m_application->addSong(it2->fileName);

                query.prepare("UPDATE song SET song_enabled = ?, song_compilation = ?, song_rating = ?, song_play_count = ?, song_play_time = ? "
                              "WHERE song_id = ?");

                query.bindValue(0, it2->enabled);
                query.bindValue(1, it2->compilation);
                query.bindValue(2, it2->rating);
                query.bindValue(3, it2->playCount);
                query.bindValue(4, it2->lastPlayed);
                query.bindValue(5, songs[it2.key()]->getId());

                if (!query.exec())
                {
                    m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                    return;
                }

                // Ajout des lectures
                query.prepare("INSERT INTO play (song_id, play_time) VALUES (?, ?)");
                query.bindValue(0, songs[it2.key()]->getId());

                for (int play = 0; play < it2->playCount; ++play)
                {
                    if (play == it2->playCount - 1)
                        query.bindValue(1, it2->lastPlayed);
                    else
                        query.bindValue(1, QDateTime());

                    if (!query.exec())
                    {
                        m_application->showDatabaseError(query.lastError().text(), query.lastQuery(), __FILE__, __LINE__);
                        return;
                    }
                }
            }

            // Rechargement du morceau
            songs[it2.key()]->loadFromDatabase();
        }
    }

    // Ajout des listes de lecture et des dossiers
    QStringList list = qobject_cast<CImporterITunes *>(wizard())->getSelectedItems();

    for (QStringList::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        //...
    }
}


/*
bool CITunesWizardPage1::validateCurrentPage(void)
{
    return QWizardPage::validateCurrentPage();
}
*/

CITunesLibrary::CITunesLibrary(CApplication * application, QObject * parent) :
    QObject       (parent),
    m_isLoaded    (false),
    m_application (application)
{
    Q_CHECK_PTR(application);
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
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        song.id = nodeListAttrValue.text().toInt();
                    }
                    else if (nodeListAttr.text() == "Name")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Artist")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Album Artist")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Album")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Genre")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Kind")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Size")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Total Time")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Track Number")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Track Count")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Year")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Date Modified")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "date"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Date Added")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "date"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Bit Rate")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Sample Rate")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Play Count")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        song.playCount = nodeListAttrValue.text().toInt();
                    }
                    else if (nodeListAttr.text() == "Play Date")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Play Date UTC")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "date"))
                            continue;

                        song.lastPlayed = QDateTime::fromString(nodeListAttrValue.text(), "yyyy-MM-ddTHH:mm:ssZ");
                    }
                    else if (nodeListAttr.text() == "Skip Count")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Skip Date")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "date"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Rating")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        song.rating = nodeListAttrValue.text().toInt() / 20;
                    }
                    else if (nodeListAttr.text() == "Album Rating")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Album Rating Computed")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "true"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Sort Album")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Persistent ID")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Track Type")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //File
                    }
                    else if (nodeListAttr.text() == "Location")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        song.fileName = QUrl(nodeListAttrValue.text()).toLocalFile();

                        if (song.fileName.startsWith("//localhost/"))
                        {
                            song.fileName = song.fileName.mid(12).replace('\\', '/');
                        }
                        else
                        {
                            qWarning() << "Nom de fichier incorrect";
                        }
                    }
                    else if (nodeListAttr.text() == "File Folder Count")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Library Folder Count")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Comments")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Grouping")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Composer")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Sort Name")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Sort Artist")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Sort Composer")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Sort Album Artist")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Disc Number")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Disc Count")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Disabled")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "true"))
                            continue;

                        song.enabled = false;
                    }
                    else if (nodeListAttr.text() == "Compilation")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "true"))
                            continue;

                        song.compilation = true;
                    }
                    else if (nodeListAttr.text() == "Part Of Gapless Album")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "true"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Volume Adjustment")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "Start Time")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else if (nodeListAttr.text() == "BPM")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "integer"))
                            continue;

                        //...
                    }
                    else
                    {
                        m_application->logError(tr("unknown key %1 for a song").arg(nodeListAttr.text()), __FUNCTION__, __FILE__, __LINE__);
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
            
            if (testLoadingXMLElementError(node.tagName(), "array"))
                continue;

            QMap<QString, QString> dynamicListParents;
            QMap<QString, QString> staticListParents;
            QMap<QString, TDynamicList> dynamicLists;
            QMap<QString, TStaticList> staticLists;
            
            // Parcours des listes de lecture
            for (QDomElement nodeList = node.firstChildElement(); !nodeList.isNull(); nodeList = nodeList.nextSibling().toElement())
            {
                if (testLoadingXMLElementError(nodeList.tagName(), "dict"))
                    continue;

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
                    if (testLoadingXMLElementError(nodeListAttr.tagName(), "key"))
                        continue;

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
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        folderID = nodeListAttrValue.text();
                    }
                    else if (nodeListAttr.text() == "Playlist Persistent ID")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

                        playListID = nodeListAttrValue.text();
                    }
                    else if (nodeListAttr.text() == "Smart Info" || nodeListAttr.text() == "Smart Criteria")
                    {
                        isDynamic = true;
                    }
                    else if (nodeListAttr.text() == "Playlist Items")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "array"))
                            continue;

                        // Parcours de la liste des morceaux
                        for (QDomElement nodeListSongs = nodeListAttrValue.firstChildElement(); !nodeListSongs.isNull(); nodeListSongs = nodeListSongs.nextSibling().toElement())
                        {
                            if (testLoadingXMLElementError(nodeListSongs.tagName(), "dict"))
                                continue;

                            QDomElement nodeListSong = nodeListSongs.firstChildElement();

                            if (testLoadingXMLElementError(nodeListSong.tagName(), "key"))
                                continue;

                            if (nodeListSong.text() != "Track ID")
                            {
                                qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (valeur 'Track ID' attendue)";
                                continue;
                            }

                            nodeListSong = nodeListSong.nextSibling().toElement();
                            
                            if (testLoadingXMLElementError(nodeListSong.tagName(), "integer"))
                                continue;

                            songs.append(nodeListSong.text().toInt());
                        }
                    }
                    else if (nodeListAttr.text() == "Name")
                    {
                        if (testLoadingXMLElementError(nodeListAttrValue.tagName(), "string"))
                            continue;

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
                            list.name  = playListName;
                            list.id    = playListID;
                            list.songs = songs;
                            m_staticLists.append(list);
                        }
                        else
                        {
                            staticListParents[playListID] = folderID;
                            staticLists[playListID].name  = playListName;
                            staticLists[playListID].id    = playListID;
                            staticLists[playListID].songs = songs;
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
    item->setData("", Qt::UserRole + 1);
    item->setCheckable(true);
    item->setCheckState(Qt::Checked);
    model->appendRow(item);

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
        item->setCheckable(true);
        item->setEnabled(false);
        model->appendRow(item);
    }

    // Listes statiques
    for (QList<TStaticList>::const_iterator it = m_staticLists.begin(); it != m_staticLists.end(); ++it)
    {
        QStandardItem * item = new QStandardItem(QPixmap(":/icons/playlist"), tr("%1 (%n song(s))", "", it->songs.size()).arg(it->name));
        item->setData(it->id, Qt::UserRole + 1);
        item->setCheckable(true);
        item->setEnabled(false);
        model->appendRow(item);
    }
}


bool CITunesLibrary::testLoadingXMLElementError(const QString& element, const QString& expected) const
{
    if (element != expected)
    {
        qWarning() << "CITunesLibrary::loadFile() : le fichier n'est pas valide (élément " << expected << " attendu)";
        return true;
    }

    return false;
}

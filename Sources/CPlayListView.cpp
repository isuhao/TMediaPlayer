/*
Copyright (C) 2012-2013 Teddy Michel

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

#include "CPlayListView.hpp"
#include "CListModel.hpp"
#include "IPlayList.hpp"
#include "CStaticPlayList.hpp"
#include "CDynamicList.hpp"
#include "CApplication.hpp"
#include "CQueuePlayList.hpp"
#include "CFolder.hpp"
#include "CCDRomDrive.hpp"
#include "Utils.hpp"
#include "Dialog/CDialogCDRomDriveInfos.hpp"

#include <QHeaderView>
#include <QMenu>
#include <QDragMoveEvent>
#include <QMimeData>

#include <QtDebug>


/**
 * Constructeur de la vue.
 *
 * \todo Pouvoir renommer les éléments dans la vue.
 * \todo Ajouter une entrée dans le menu contextuel pour ouvrir une liste dans une nouvelle fenêtre.
 * \todo Ajouter une entrée dans le menu contextuel pour exporter une liste.
 *
 * \param application Pointeur sur l'application.
 */

CPlayListView::CPlayListView(CApplication * application) :
    QTreeView        (application),
    m_application    (application),
    m_model          (NULL),
    m_menuPlaylist   (NULL),
    m_menuFolder     (NULL),
    m_menuCDRomDrive (NULL),
    m_menuDefault    (NULL)
{
    Q_CHECK_PTR(application);

    header()->setVisible(false);
    setUniformRowHeights(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    //m_model = new CListModel(m_application);
    //setModel(m_model);

    // Glisser-déposer
    setDropIndicatorShown(true);
    setAcceptDrops(true);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);

    connect(this, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(onItemCollapsed(const QModelIndex&)));
    connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(onItemExpanded(const QModelIndex&)));

    // Menus contextuels
    m_menuPlaylist = new QMenu(this);
    m_menuPlaylist->addAction(tr("Edit..."), m_application, SLOT(editSelectedItem()));
    m_menuPlaylist->addAction(tr("Remove"), m_application, SLOT(removeSelectedItem()));

    m_menuFolder = new QMenu(this);
    m_menuFolder->addAction(tr("Edit..."), m_application, SLOT(editSelectedItem()));
    m_menuFolder->addAction(tr("Remove"), m_application, SLOT(removeSelectedItem()));
    m_menuFolder->addSeparator();
    m_menuFolder->addAction(tr("New playlist..."), this, SLOT(createStaticList()));
    m_menuFolder->addAction(tr("New dynamic playlist..."), this, SLOT(createDynamicList()));
    m_menuFolder->addAction(tr("New folder..."), this, SLOT(createFolder()));

    m_menuDefault = new QMenu(this);
    m_menuDefault->addAction(tr("New playlist..."), m_application, SLOT(openDialogCreateStaticList()));
    m_menuDefault->addAction(tr("New dynamic playlist..."), m_application, SLOT(openDialogCreateDynamicList()));
    m_menuDefault->addAction(tr("New folder..."), m_application, SLOT(openDialogCreateFolder()));

    m_menuCDRomDrive = new QMenu(this);
    m_menuCDRomDrive->addAction(tr("Eject"), this, SLOT(ejectCDRom()));
    m_menuCDRomDrive->addAction(tr("Informations..."), this, SLOT(informationsAboutCDRomDrive()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openCustomMenuProject(const QPoint&)));
}


/**
 * Retourne la liste de morceaux correspondant à un index.
 *
 * \param index Index de la liste.
 * \return Liste de morceaux, ou NULL.
 */

CSongTable * CPlayListView::getSongTable(const QModelIndex& index) const
{
    return m_model->data(index, Qt::UserRole + 1).value<CSongTable *>();
}


/**
 * Retourne le dossier correspondant à un index.
 *
 * \param index Index du dossier.
 * \return Dossier, ou NULL.
 */

CFolder * CPlayListView::getFolder(const QModelIndex& index) const
{
    return m_model->data(index, Qt::UserRole + 2).value<CFolder *>();
}


/**
 * Retourne la liste de morceaux actuellement sélectionnée.
 *
 * \return Liste de morceaux sélectionnée, ou NULL.
 */

CSongTable * CPlayListView::getSelectedSongTable() const
{
    QModelIndex index = selectionModel()->currentIndex();
    return (index.isValid() ? getSongTable(index) : NULL);
}


/**
 * Retourne le dossier actuellement sélectionné.
 *
 * \return Dossier sélectionné, ou NULL.
 */

CFolder * CPlayListView::getSelectedFolder() const
{
    QModelIndex index = selectionModel()->currentIndex();
    return (index.isValid() ? getFolder(index) : NULL);
}


/**
 * Retourne le lecteur de CD-ROM actuellement sélectionné.
 *
 * \return Lecteur de CD-ROM sélectionné, ou NULL.
 */

CCDRomDrive * CPlayListView::getSelectedCDRomDrive() const
{
    QModelIndex index = selectionModel()->currentIndex();

    if (!index.isValid())
        return NULL;

    CSongTable * songTable = getSongTable(index);

    if (!songTable)
        return NULL;

    CCDRomDrive * cdRomDrive = qobject_cast<CCDRomDrive *>(songTable);
    return cdRomDrive;
}


/**
 * Retourne l'index du modèle correspondant à une liste de morceaux.
 *
 * \param songTable Liste de morceaux à rechercher.
 * \return Index de la liste de morceaux.
 */

QModelIndex CPlayListView::getSongTableModelIndex(CSongTable * songTable) const
{
    if (!songTable)
        return QModelIndex();

    return m_model->getModelIndex(songTable);
}


/**
 * Retourne l'index du modèle correspondant à un dossier.
 *
 * \param folder Dossier à rechercher.
 * \return Index du dossier.
 */

QModelIndex CPlayListView::getFolderModelIndex(CFolder * folder) const
{
    if (!folder)
        return QModelIndex();
    
    return m_model->getModelIndex(folder);
}


/**
 * Modifie le modèle utilisé par la vue.
 *
 * \param model Pointeur sur le modèle à utiliser.
 */

void CPlayListView::setModel(CListModel * model)
{
    Q_CHECK_PTR(model);
    m_model = model;
    QTreeView::setModel(model);

    // Ouverture des dossiers
    QList<CFolder *> folders = m_model->getFolders();

    for (QList<CFolder *>::const_iterator it = folders.begin(); it != folders.end(); ++it)
    {
        setExpanded(m_model->getModelIndex(*it), (*it)->isOpen());
    }
    
    // Affiche ou masque les lecteurs de CD-ROM
    QList<CCDRomDrive *> drives = m_application->getCDRomDrives();

    for (QList<CCDRomDrive *>::const_iterator drive = drives.begin(); drive != drives.end(); ++drive)
    {
        QModelIndex index = m_model->getModelIndex(*drive);

        if (!index.isValid())
            continue;

        QStandardItem * item = m_model->item(index.row());

        if (item == NULL)
            continue;

        setRowHidden(index.row(), index.parent(), !item->isEnabled());
    }
}


/**
 * Met à jour les informations sur les lecteurs de CD-ROM.
 * Affiche ou masque les éléments correspondant aux lecteurs.
 */

void CPlayListView::updateCDRomDrives()
{
    Q_CHECK_PTR(m_model);
    m_model->updateCDRomDrives();
    
    QList<CCDRomDrive *> drives = m_application->getCDRomDrives();

    for (QList<CCDRomDrive *>::const_iterator drive = drives.begin(); drive != drives.end(); ++drive)
    {
        QModelIndex index = m_model->getModelIndex(*drive);

        if (!index.isValid())
            continue;

        QStandardItem * item = m_model->item(index.row());

        if (item == NULL)
            continue;

        setRowHidden(index.row(), index.parent(), !item->isEnabled());
    }
}


/**
 * Gestion des touches du clavier.
 * La touche Supprimer est gérée.
 *
 * \param event Évènement du clavier.
 */

void CPlayListView::keyPressEvent(QKeyEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->key() == Qt::Key_Delete)
    {
        event->accept();
        m_application->removeSelectedItem();
        return;
    }

    return QTreeView::keyPressEvent(event);
}


/**
 * Gestion du glisser-déposer.
 * Cette méthode est appelée à chaque fois qu'un objet QDrag est déplacé à l'intérieur de la vue.
 *
 * \param event Évènement.
 */

void CPlayListView::dragMoveEvent(QDragMoveEvent * event)
{
    Q_CHECK_PTR(event);

    if (event->keyboardModifiers() & Qt::ControlModifier)
    {
        // Traitement différent si Ctrl+Glisser (par exemple pour copier OU déplacer)
        // on verra plus tard...
        // Ctrl: Copier
        // Shift: Déplacer
    }

    event->setDropAction(Qt::CopyAction);

    QPoint point = event->pos();

    QModelIndex index = indexAt(point);

    if (index.isValid())
    {
#if QT_VERSION >= 0x050000
        const QMimeData * mimeData = event->mimeData();

        if (!mimeData)
        {
            event->ignore();
            return;
        }

        QByteArray dataList = mimeData->data("application/x-ted-media-list");
#else
        QByteArray dataList = event->encodedData("application/x-ted-media-list");
#endif

        if (dataList.isEmpty())
        {
            CSongTable * songTable = m_model->data(index, Qt::UserRole + 1).value<CSongTable *>();
            //CFolder * folder = m_model->data(index, Qt::UserRole + 2).value<CFolder *>();

            if (songTable && (qobject_cast<CStaticPlayList *>(songTable) || qobject_cast<CQueuePlayList *>(songTable)))
            {
                event->accept();
                event->acceptProposedAction();
            }
            /*else if (folder)
            {
                qDebug() << "CPlayListView::dragMoveEvent() : move in folder";
                //...
                event->ignore();
                return;
            }*/
            else
            {
                event->ignore();
                return;
            }
        }
        // Déplacement d'un dossier ou d'une liste de lecture
        else
        {
#ifdef ENABLE_DRAG_DROP_FOR_LISTS
            int playListId;
            int folderId;

            CListModel::decodeDataList(dataList, &playListId, &folderId);

            if (folderId > 0)
            {
                CFolder * folderSelected = m_application->getFolderFromId(folderId);

                CFolder * folder = m_model->data(index, Qt::UserRole + 2).value<CFolder *>();

                if (folder && folder->hasAncestor(folderSelected))
                {
                    event->ignore();
                    return;
                }

                CSongTable * songTable = m_model->data(index, Qt::UserRole + 1).value<CSongTable *>();

                if (songTable)
                {
                    event->ignore();
                    return;
                }
                /*
                if (folderId == folder->getId())
                {
                    event->ignore();
                    return;
                }*/
            }

            event->accept();
            event->acceptProposedAction()
#else
            // Désactivation
            event->ignore();
            return;
#endif
        }
    }
    else
    {
#if QT_VERSION >= 0x050000
        const QMimeData * mimeData = event->mimeData();

        if (!mimeData)
        {
            event->ignore();
            return;
        }

        QByteArray dataList = mimeData->data("application/x-ted-media-list");
#else
        QByteArray dataList = event->encodedData("application/x-ted-media-list");
#endif

        if (dataList.isEmpty())
        {
            event->accept();
            event->acceptProposedAction();
        }
        else
        {
            event->ignore();
            return;
        }
    }

    QTreeView::dragMoveEvent(event);
}


/**
 * Ouvre le menu contextuel de la vue.
 *
 * \param point Position du clic.
 */

void CPlayListView::openCustomMenuProject(const QPoint& point)
{
    QMenu * menu = m_menuDefault; // Menu à afficher
    QModelIndex index = indexAt(point);

    if (index.isValid())
    {
        QStandardItem * item = m_model->itemFromIndex(index);

        if (item)
        {
            CSongTable * songTable = item->data(Qt::UserRole + 1).value<CSongTable *>();

            if (songTable)
            {
                // Menu pour une liste de lecture
                if (qobject_cast<IPlayList *>(songTable))
                {
                    menu = m_menuPlaylist;
                }
                // Menu pour un lecteur de CD-ROM
                else if (qobject_cast<CCDRomDrive *>(songTable))
                {
                    menu = m_menuCDRomDrive;
                }
            }
            // Menu pour un dossier
            else if (item->data(Qt::UserRole + 2).value<CFolder *>())
            {
                menu = m_menuFolder;
            }
            else
            {
                m_application->logError(tr("the item isn't a playlist or a folder"), __FUNCTION__, __FILE__, __LINE__);
            }
        }
    }

    // Affichage du menu
    menu->move(getCorrectMenuPosition(menu, mapToGlobal(point)));
    menu->show();
}


void CPlayListView::onItemCollapsed(const QModelIndex& index)
{
    m_model->openFolder(index, false);
}


void CPlayListView::onItemExpanded(const QModelIndex& index)
{
    m_model->openFolder(index, true);
}


/**
 * Slot pour créer une liste statique à l'intérieur d'un dossier.
 */

void CPlayListView::createStaticList()
{
    CFolder * folder = getSelectedFolder();
    m_application->openDialogCreateStaticList(folder);
}


/**
 * Slot pour créer une liste dynamique à l'intérieur d'un dossier.
 */

void CPlayListView::createDynamicList()
{
    CFolder * folder = getSelectedFolder();
    m_application->openDialogCreateDynamicList(folder);
}


/**
 * Slot pour créer un dossier à l'intérieur d'un autre dossier.
 */

void CPlayListView::createFolder()
{
    CFolder * folder = getSelectedFolder();
    m_application->openDialogCreateFolder(folder);
}


/**
 * Éjecte le CD-ROM du lecteur de CD-ROM actuellement sélectionné.
 */

void CPlayListView::ejectCDRom()
{
    CCDRomDrive * cdRomDrive = getSelectedCDRomDrive();

    if (cdRomDrive)
    {
        cdRomDrive->ejectDisc();
    }
}


/**
 * Affiche les informations sur le lecteur de CD-ROM actuellement sélectionné.
 */

void CPlayListView::informationsAboutCDRomDrive()
{
    CCDRomDrive * cdRomDrive = getSelectedCDRomDrive();

    if (cdRomDrive)
    {
        CDialogCDRomDriveInfos * dialog = new CDialogCDRomDriveInfos(cdRomDrive, m_application);
        dialog->show();
    }
}

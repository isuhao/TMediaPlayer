/*
Copyright (C) 2012-2014 Teddy Michel

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

#ifndef FILE_C_PLAYLIST_VIEW
#define FILE_C_PLAYLIST_VIEW

#include <QTreeView>


class CLibraryModel;
class CMediaTableView;
class CCDRomDrive;
class CMainWindow;
class CFolder;


/**
 * Vue utilisée pour afficher les listes de lecture sous forme d'un arbre.
 */

class CLibraryView : public QTreeView
{
    Q_OBJECT

public:

    explicit CLibraryView(CMainWindow * mainWindow);

    CMediaTableView * getSongTable(const QModelIndex& index) const;
    CFolder * getFolder(const QModelIndex& index) const;
    CMediaTableView * getSelectedSongTable() const;
    CFolder * getSelectedFolder() const;
    CCDRomDrive * getSelectedCDRomDrive() const;
    QModelIndex getSongTableModelIndex(CMediaTableView * songTable) const;
    QModelIndex getFolderModelIndex(CFolder * folder) const;
    void setModel(CLibraryModel * model);
    void updateCDRomDrives();

protected:

    virtual void keyPressEvent(QKeyEvent * event);
    virtual void dragMoveEvent(QDragMoveEvent * event);

protected slots:

    virtual void openCustomMenuProject(const QPoint& point);
    void onItemCollapsed(const QModelIndex& index);
    void onItemExpanded(const QModelIndex& index);
    void createStaticList();
    void createDynamicList();
    void createFolder();
    void ejectCDRom();
    void informationsAboutCDRomDrive();
    void updateSelectedList();

private:

    CMainWindow * m_mainWindow;    ///< Pointeur sur la fenêtre principale de l'application.
    CLibraryModel * m_model;       ///< Modèle utilisé pour afficher les listes de lecture.
    QMenu * m_menuPlaylist;        ///< Menu contextuel pour les listes de lecture.
    QMenu * m_menuDynamicPlaylist; ///< Menu contextuel pour les listes de lecture dynamiques.
    QMenu * m_menuFolder;          ///< Menu contextuel pour les dossiers.
    QMenu * m_menuCDRomDrive;      ///< Menu contextuel pour les lecteurs de CD-ROM.
    QMenu * m_menuDefault;         ///< Menu contextuel par défaut.
};

#endif // FILE_C_PLAYLIST_VIEW

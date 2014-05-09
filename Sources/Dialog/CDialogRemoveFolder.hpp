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

#ifndef FILE_C_DIALOG_REMOVE_FOLDER
#define FILE_C_DIALOG_REMOVE_FOLDER

#include <QDialog>
#include "ui_DialogRemoveFolder.h"

class CMainWindow;
class CFolder;


/**
 * Boite de dialogue pour la suppression d'un dossier.
 */

class CDialogRemoveFolder : public QDialog
{
    Q_OBJECT

public:

    CDialogRemoveFolder(CMainWindow * mainWindow, CFolder * folder);
    virtual ~CDialogRemoveFolder();

    inline bool isResursive() const;

protected slots:

    void removeFolder();

private:

    Ui::DialogRemoveFolder * m_uiWidget;
    CMainWindow * m_mainWindow;   ///< Pointeur sur la fenêtre principale de l'application.
    CFolder * m_folder;           ///< Pointeur sur le dossier à supprimer.
    bool m_recursive;             ///< Indique si le contenu du dossier doit être supprimé récursivement.
};


/**
 * Indique si le contenu du dossier doit être supprimé récursivement.
 *
 * \return Booléen.
 */

inline bool CDialogRemoveFolder::isResursive() const
{
    return m_recursive;
}

#endif // FILE_C_DIALOG_REMOVE_FOLDER

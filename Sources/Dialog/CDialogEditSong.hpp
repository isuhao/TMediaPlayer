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

#ifndef FILE_C_DIALOG_EDIT_SONG
#define FILE_C_DIALOG_EDIT_SONG

#include <QDialog>
#include "CSongTableModel.hpp"
#include "ui_DialogEditSong.h"


class CSongTable;
class CApplication;


/**
 * Boite de dialogue pour modifier les informations d'un morceau.
 */

class CDialogEditSong : public QDialog
{
    Q_OBJECT

public:

    CDialogEditSong(CSongTableItem * songItem, CSongTable * songTable, CApplication * application);
    virtual ~CDialogEditSong();

protected slots:

    void previousSong(void);
    void nextSong(void);
    void apply(void);
    void save(void);

private:

    void updateInfos(void);
    
    Ui::DialogEditSong * m_uiWidget;
    CSongTable * m_songTable;
    CSongTableItem * m_songItem;
};

#endif // FILE_C_DIALOG_EDIT_SONG

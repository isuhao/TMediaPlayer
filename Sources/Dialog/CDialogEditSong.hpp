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

#ifndef FILE_C_DIALOG_EDIT_SONG
#define FILE_C_DIALOG_EDIT_SONG

#include <QDialog>
//#include "CMediaTableModel.hpp"
#include "ui_DialogEditSong.h"


class CMediaTableView;
class CMainWindow;
class CRatingEditor;
class CMediaTableItem;
class QCloseEvent;


/**
 * Boite de dialogue pour modifier les informations d'un morceau.
 */

class CDialogEditSong : public QDialog
{
    Q_OBJECT

public:

    CDialogEditSong(CMediaTableItem * songItem, CMediaTableView * songTable, CMainWindow * mainWindow);
    virtual ~CDialogEditSong();

    inline CMediaTableItem * getSongItem() const;
    inline CMediaTableView * getSongTable() const;
    void setSongItem(CMediaTableItem * songItem, CMediaTableView * songTable);

signals:

    void closed(); ///< Signal émis lorsque la boite de dialogue est fermée.

protected:

    virtual void closeEvent(QCloseEvent * event);
    void applyChanges();
    void resetSummary();

protected slots:

    void previousSong();
    void nextSong();
    void apply();
    void save();

private:

    void updateInfos();
    
    Ui::DialogEditSong * m_uiWidget;
    CRatingEditor * m_ratingEditor;
    CMediaTableView * m_songTable;
    CMediaTableItem * m_songItem;
    CMainWindow * m_mainWindow;
};


inline CMediaTableItem * CDialogEditSong::getSongItem() const
{
    return m_songItem;
}


inline CMediaTableView * CDialogEditSong::getSongTable() const
{
    return m_songTable;
}

#endif // FILE_C_DIALOG_EDIT_SONG

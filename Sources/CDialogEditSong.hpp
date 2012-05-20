
#ifndef FILE_CDIALOG_EDIT_SONG
#define FILE_CDIALOG_EDIT_SONG

#include <QDialog>
#include "CSongTableModel.hpp"
#include "ui_DialogEditSong.h"


class CDialogEditSong : public QDialog
{
    Q_OBJECT

public:

    CDialogEditSong(CSongTableModel::TSongItem * songItem, QWidget * parent = NULL);
    ~CDialogEditSong();

    //...

private:
    
    Ui::DialogEditSong * m_uiWidget;
    CSongTableModel::TSongItem * m_songItem;
};

#endif // FILE_CDIALOG_EDIT_SONG

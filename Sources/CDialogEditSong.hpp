
#ifndef FILE_CDIALOG_EDIT_SONG
#define FILE_CDIALOG_EDIT_SONG

#include <QDialog>
#include "ui_DialogEditSong.h"


class CSong;


class CDialogEditSong : public QDialog
{
    Q_OBJECT

public:

    CDialogEditSong(CSong * song, QWidget * parent = NULL);
    ~CDialogEditSong();

    //...

private:
    
    Ui::DialogEditSong * m_uiWidget;
    CSong * m_song;
};

#endif // FILE_CDIALOG_EDIT_SONG

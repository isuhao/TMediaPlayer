
#ifndef FILE_C_DIALOG_EDIT_SONG
#define FILE_C_DIALOG_EDIT_SONG

#include <QDialog>
#include "CSongTableModel.hpp"
#include "ui_DialogEditSong.h"


class CSongTable;


class CDialogEditSong : public QDialog
{
    Q_OBJECT

public:

    CDialogEditSong(CSongTableItem * songItem, CSongTable * songTable);
    ~CDialogEditSong();

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

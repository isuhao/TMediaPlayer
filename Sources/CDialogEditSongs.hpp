
#ifndef FILE_CDIALOG_EDIT_SONGS
#define FILE_CDIALOG_EDIT_SONGS

#include <QDialog>
#include "CSongTableModel.hpp"
#include "ui_DialogEditSongs.h"


class CDialogEditSongs : public QDialog
{
    Q_OBJECT

public:

    CDialogEditSongs(QList<CSongTableModel::TSongItem *> songItemList, QWidget * parent = NULL);
    ~CDialogEditSongs();

    //...

private:
    
    Ui::DialogEditSongs * m_uiWidget;
    QList<CSongTableModel::TSongItem *> m_songItemList;
};

#endif // FILE_CDIALOG_EDIT_SONGS

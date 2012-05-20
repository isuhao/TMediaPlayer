
#ifndef FILE_CDIALOG_EDIT_STATIC_PLAYLIST
#define FILE_CDIALOG_EDIT_STATIC_PLAYLIST

#include <QDialog>
#include "ui_DialogEditStaticPlayList.h"


class CStaticPlayList;
class CApplication;


class CDialogEditStaticPlayList : public QDialog
{
    Q_OBJECT

public:

    CDialogEditStaticPlayList(CStaticPlayList * playList, CApplication * application);
    ~CDialogEditStaticPlayList();

protected slots:

    void onSave(void);

private:
    
    Ui::DialogEditStaticPlayList * m_uiWidget;
    CApplication * m_application;
    CStaticPlayList * m_playList;
};

#endif // FILE_CDIALOG_EDIT_STATIC_PLAYLIST

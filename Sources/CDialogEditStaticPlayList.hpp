
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

    void save(void);

private:
    
    Ui::DialogEditStaticPlayList * m_uiWidget;
    CStaticPlayList * m_playList;
    CApplication * m_application;
};

#endif // FILE_CDIALOG_EDIT_STATIC_PLAYLIST

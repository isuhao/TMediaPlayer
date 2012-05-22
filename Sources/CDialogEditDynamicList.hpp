
#ifndef FILE_CDIALOG_EDIT_DYNAMIC_LIST
#define FILE_CDIALOG_EDIT_DYNAMIC_LIST

#include <QDialog>
#include "ui_DialogEditDynamicPlayList.h"


class CDynamicPlayList;
class CApplication;


class CDialogEditDynamicList : public QDialog
{
    Q_OBJECT

public:

    CDialogEditDynamicList(CDynamicPlayList * playList, CApplication * application);
    ~CDialogEditDynamicList();

protected slots:

    void save(void);

private:
    
    Ui::DialogEditDynamicPlayList * m_uiWidget;
    CApplication * m_application;
    CDynamicPlayList * m_playList;
};

#endif // FILE_CDIALOG_EDIT_DYNAMIC_LIST

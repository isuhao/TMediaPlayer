
#ifndef FILE_C_DIALOG_EDIT_DYNAMIC_LIST
#define FILE_C_DIALOG_EDIT_DYNAMIC_LIST

#include <QDialog>
#include "ui_DialogEditDynamicPlayList.h"


class CDynamicPlayList;
class CApplication;
class CWidgetMultiCriterion;


class CDialogEditDynamicList : public QDialog
{
    Q_OBJECT

public:

    CDialogEditDynamicList(CDynamicPlayList * playList, CApplication * application);
    virtual ~CDialogEditDynamicList();

public slots:

    void resizeWindow(void);

protected slots:

    void save(void);

private:
    
    Ui::DialogEditDynamicPlayList * m_uiWidget;
    CWidgetMultiCriterion * m_widgetCriterion;
    CDynamicPlayList * m_playList;
    CApplication * m_application;
};

#endif // FILE_C_DIALOG_EDIT_DYNAMIC_LIST

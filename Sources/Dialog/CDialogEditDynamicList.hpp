
#ifndef FILE_C_DIALOG_EDIT_DYNAMIC_LIST
#define FILE_C_DIALOG_EDIT_DYNAMIC_LIST

#include <QDialog>
#include "ui_DialogEditDynamicPlayList.h"


class CDynamicList;
class CApplication;
class CWidgetMultiCriterion;
class CFolder;


class CDialogEditDynamicList : public QDialog
{
    Q_OBJECT

public:

    CDialogEditDynamicList(CDynamicList * playList, CApplication * application, CFolder * folder = NULL);
    virtual ~CDialogEditDynamicList();

public slots:

    void resizeWindow(void);

protected slots:

    void save(void);

private:
    
    Ui::DialogEditDynamicPlayList * m_uiWidget;
    CWidgetMultiCriterion * m_widgetCriterion;
    CDynamicList * m_playList;
    CApplication * m_application;
    CFolder * m_folder;
};

#endif // FILE_C_DIALOG_EDIT_DYNAMIC_LIST

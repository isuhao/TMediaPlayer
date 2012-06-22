
#ifndef FILE_C_DIALOG_EDIT_FOLDER
#define FILE_C_DIALOG_EDIT_FOLDER

#include <QDialog>
#include "ui_DialogEditFolder.h"


class CApplication;
class CFolder;


class CDialogEditFolder : public QDialog
{
    Q_OBJECT

public:

    CDialogEditFolder(CFolder * folder, CApplication * application, CFolder * folderParent = NULL);
    virtual ~CDialogEditFolder();

protected slots:

    void save(void);

private:
    
    Ui::DialogEditFolder * m_uiWidget;
    CFolder * m_folder;
    CApplication * m_application;
    CFolder * m_folderParent;
};

#endif // FILE_C_DIALOG_EDIT_FOLDER

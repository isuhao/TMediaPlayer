
#ifndef FILE_C_DIALOG_EDIT_FOLDER
#define FILE_C_DIALOG_EDIT_FOLDER

#include <QDialog>
#include "ui_DialogEditFolder.h"


class CApplication;
class CListFolder;


class CDialogEditFolder : public QDialog
{
    Q_OBJECT

public:

    CDialogEditFolder(CListFolder * folder, CApplication * application, CListFolder * folderParent = NULL);
    virtual ~CDialogEditFolder();

protected slots:

    void save(void);

private:
    
    Ui::DialogEditFolder * m_uiWidget;
    CListFolder * m_folder;
    CApplication * m_application;
    CListFolder * m_folderParent;
};

#endif // FILE_C_DIALOG_EDIT_FOLDER

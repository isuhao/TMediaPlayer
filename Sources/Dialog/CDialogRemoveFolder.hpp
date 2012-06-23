
#ifndef FILE_C_DIALOG_REMOVE_FOLDER
#define FILE_C_DIALOG_REMOVE_FOLDER

#include <QDialog>
#include "ui_DialogRemoveFolder.h"

class CApplication;
class CFolder;


class CDialogRemoveFolder : public QDialog
{
    Q_OBJECT

public:

    CDialogRemoveFolder(CApplication * application, CFolder * folder);
    virtual ~CDialogRemoveFolder();

protected slots:

    void removeFolder(void);

private:

    Ui::DialogRemoveFolder * m_uiWidget;
    CApplication * m_application;
    CFolder * m_folder;
};

#endif // FILE_C_DIALOG_REMOVE_FOLDER

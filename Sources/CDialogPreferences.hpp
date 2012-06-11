
#ifndef FILE_C_DIALOG_PREFERENCES
#define FILE_C_DIALOG_PREFERENCES

#include <QDialog>
#include "ui_DialogPreferences.h"


class CApplication;
class QSettings;
class QNetworkReply;


class CDialogPreferences : public QDialog
{
    Q_OBJECT

public:

    CDialogPreferences(CApplication * application, QSettings * settings);
    virtual ~CDialogPreferences();

protected slots:

    void save(void);

private:

    Ui::DialogPreferences * m_uiWidget; ///< Widget utilisé par la boite de dialogue.
    CApplication * m_application;       ///< Pointeur sur l'application.
    QSettings * m_settings;
};

#endif // FILE_C_DIALOG_PREFERENCES

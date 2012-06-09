
#ifndef FILE_C_IMPORTER_ITUNES
#define FILE_C_IMPORTER_ITUNES

#include <QWizard>
#include "ui_DialogImportITunes.h"


class CApplication;


class CImporterITunes : public QWizard
{
    Q_OBJECT

public:

    explicit CImporterITunes(CApplication * application);
    virtual ~CImporterITunes();

protected slots:

    void chooseFile(void);
    void onPageChanged(int page);

private:

    CApplication * m_application;
    Ui::DialogImportITunes * m_uiWidget;
};

#endif // FILE_C_IMPORTER_ITUNES

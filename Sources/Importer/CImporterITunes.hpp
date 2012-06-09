
#ifndef FILE_C_IMPORTER_ITUNES
#define FILE_C_IMPORTER_ITUNES

#include <QWizard>
#include <QDomDocument>
#include "ui_DialogImportITunes.h"


class CApplication;
class CITunesLibrary;
class QStandardItemModel;


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
    CITunesLibrary * m_library;
    Ui::DialogImportITunes * m_uiWidget;
};


class CITunesLibrary : public QObject
{
    Q_OBJECT

public:

    CITunesLibrary(QObject * parent = NULL);
    ~CITunesLibrary();

    bool loadFile(const QString& fileName);
    void initModelWithLists(QStandardItemModel * model) const;

private:

    bool m_isLoaded;
    QDomDocument m_document;
};

#endif // FILE_C_IMPORTER_ITUNES

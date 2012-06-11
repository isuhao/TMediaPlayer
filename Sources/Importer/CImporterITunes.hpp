
#ifndef FILE_C_IMPORTER_ITUNES
#define FILE_C_IMPORTER_ITUNES

#include <QWizard>
#include <QDomDocument>
#include <QDateTime>
#include <QWizard>
#include <QMap>


class QGridLayout;
class QLabel;
class QLineEdit;
class QStandardItemModel;
class QToolButton;
class QTreeView;
class CApplication;
class CITunesLibrary;
class CITunesWizardPage1;
class CITunesWizardPage2;


class CImporterITunes : public QWizard
{
    Q_OBJECT

public:

    explicit CImporterITunes(CApplication * application);
    virtual ~CImporterITunes();

protected slots:

    void onPageChanged(int page);

private:

    CApplication * m_application;
    CITunesLibrary * m_library;
    CITunesWizardPage1 * m_page1;
    CITunesWizardPage2 * m_page2;
};


class CITunesWizardPage1 : public QWizardPage
{
    Q_OBJECT

public:

    explicit CITunesWizardPage1(CITunesLibrary * library, QWidget * parent = NULL);

    virtual bool isComplete(void) const;
    virtual void initializePage(void);

protected slots:

    void chooseFile(void);

private:

    CITunesLibrary * m_library;
    QGridLayout * m_gridLayout;
    QLabel * m_lblFileName;
    QLineEdit * m_editFileName;
    QToolButton * m_btnFileName;
};


class CITunesWizardPage2 : public QWizardPage
{
    Q_OBJECT

public:

    explicit CITunesWizardPage2(CITunesLibrary * library, QWidget * parent = NULL);

    virtual void initializePage(void);

private:
    
    CITunesLibrary * m_library;
    QGridLayout * m_gridLayout;
    QTreeView * m_treeView;
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

    struct TSong
    {
        int id;               ///< Identifiant du morceau.
        QString fileName;     ///< Adresse du fichier.
        int playCount;        ///< Nombre de lectures.
        QDateTime lastPlayed; ///< Date de la dernière lecture.
        int rating;           ///< Note du morceau.
        bool enabled;         ///< Indique si le morceau est coché ou pas.
        bool compilation;     ///< Indique si le morceau fait partie d'une compilation.

        TSong(void) : id(0), playCount(0), rating(0), enabled(true), compilation(false) { }
    };

    struct TDynamicList
    {
        QString name;
        QString id;
    };

    struct TStaticList
    {
        QString name;
        QString id;
        QList<int> songs;
    };

    struct TFolder
    {
        QString name;
        QString id;

        QList<TDynamicList> dynamicLists;
        QList<TStaticList> staticLists;
    };

    bool m_isLoaded;                    ///< Indique si la médiathèque a été chargée.
    QString m_fileName;                 ///< Fichier contenant la médiathèque.
    QDomDocument m_document;            ///< Document XML.
    QMap<int, TSong> m_songs;           ///< Liste des morceaux de la médiathèque.
    QList<TFolder> m_folders;           ///< Liste des dossiers.
    QList<TDynamicList> m_dynamicLists; ///< Liste des listes de lecture dynamiques.
    QList<TStaticList> m_staticLists;   ///< Liste des listes de lecture statiques.
};

#endif // FILE_C_IMPORTER_ITUNES

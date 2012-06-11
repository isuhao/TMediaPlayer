
#ifndef FILE_C_IMPORTER_ITUNES
#define FILE_C_IMPORTER_ITUNES

#include <QWizard>
#include <QDomDocument>
#include <QDateTime>
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

    struct TSong
    {
        int id;               ///< Identifiant du morceau.
        QString fileName;     ///< Adresse du fichier.
        int playCount;        ///< Nombre de lectures.
        QDateTime lastPlayed; ///< Date de la dernière lecture.
        int rating;           ///< Note du morceau.
        bool enabled;         ///< Indique si le morceau est coché ou pas.

        TSong(void) : id(0), playCount(0), rating(0), enabled(true) { }
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

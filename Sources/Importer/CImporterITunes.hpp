/*
Copyright (C) 2012-2013 Teddy Michel

This file is part of TMediaPlayer.

TMediaPlayer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TMediaPlayer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TMediaPlayer. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FILE_C_IMPORTER_ITUNES
#define FILE_C_IMPORTER_ITUNES

#include <QWizard>
#include <QDomDocument>
#include <QDateTime>
#include <QWizard>
#include <QMap>
#include "ui_ImporterITunesPage3.h"


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
class CITunesWizardPage3;
class CITunesWizardPage4;


class CImporterITunes : public QWizard
{
    Q_OBJECT

public:

    explicit CImporterITunes(CApplication * application);
    virtual ~CImporterITunes();

    QStringList getSelectedItems() const;
    bool needToImportSongs() const;

private:

    CApplication * m_application;
    CITunesLibrary * m_library;
    CITunesWizardPage1 * m_page1;
    CITunesWizardPage2 * m_page2;
    CITunesWizardPage3 * m_page3;
    CITunesWizardPage4 * m_page4;
};


class CITunesWizardPage1 : public QWizardPage
{
    Q_OBJECT

public:

    explicit CITunesWizardPage1(CITunesLibrary * library, QWidget * parent = NULL);

    virtual bool isComplete() const;

protected slots:

    void chooseFile();

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

    virtual void initializePage();
    virtual void cleanupPage();

    QStringList getSelectedItems() const;
    bool needToImportSongs() const;

private:
    
    CITunesLibrary * m_library;
    QGridLayout * m_gridLayout;
    QTreeView * m_treeView;
    QStandardItemModel * m_model;
};


class CITunesWizardPage3 : public QWizardPage
{
    Q_OBJECT

public:

    explicit CITunesWizardPage3(CITunesLibrary * library, QWidget * parent = NULL);
    virtual ~CITunesWizardPage3();

private:
    
    CITunesLibrary * m_library;
    Ui::ImporterITunesPage3 * m_uiWidget;
};


class CITunesWizardPage4 : public QWizardPage
{
    Q_OBJECT

public:

    CITunesWizardPage4(CApplication * application, CITunesLibrary * library, QWidget * parent = NULL);

    virtual void initializePage();

private:
    
    CITunesLibrary * m_library;
    CApplication * m_application;
};


class CITunesLibrary : public QObject
{
    Q_OBJECT

public:

    struct TSong
    {
        int id;               ///< Identifiant du morceau.
        QString fileName;     ///< Adresse du fichier.
        int playCount;        ///< Nombre de lectures.
        QDateTime lastPlayed; ///< Date de la dernière lecture.
        int rating;           ///< Note du morceau.
        bool enabled;         ///< Indique si le morceau est coché ou pas.
        bool compilation;     ///< Indique si le morceau fait partie d'une compilation.

        inline TSong() : id(0), playCount(0), rating(0), enabled(true), compilation(false) { }
    };

    explicit CITunesLibrary(CApplication * application, QObject * parent = NULL);
    virtual ~CITunesLibrary();

    bool loadFile(const QString& fileName);
    void initModelWithLists(QStandardItemModel * model) const;

    inline QMap<int, TSong> getSongs() const { return m_songs; }

private:

    bool testLoadingXMLElementError(const QString& element, const QString& expected) const;

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
    CApplication * m_application;
    QString m_fileName;                 ///< Fichier contenant la médiathèque.
    QDomDocument m_document;            ///< Document XML.
    QMap<int, TSong> m_songs;           ///< Liste des morceaux de la médiathèque.
    QList<TFolder> m_folders;           ///< Liste des dossiers.
    QList<TDynamicList> m_dynamicLists; ///< Liste des listes de lecture dynamiques.
    QList<TStaticList> m_staticLists;   ///< Liste des listes de lecture statiques.
};

#endif // FILE_C_IMPORTER_ITUNES

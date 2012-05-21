
#ifndef FILE_CDYNAMICPLAYLIST
#define FILE_CDYNAMICPLAYLIST

#include "CPlayList.hpp"


/**
 * Liste de lecture dynamique.
 * Contient une liste de critères de recherche.
 */

class CDynamicPlayList : public CPlayList
{
    Q_OBJECT

    friend class CDialogEditDynamicList;

public:

    CDynamicPlayList(CApplication * application);
    ~CDynamicPlayList();

public slots:

    void update(void);

protected slots:

    bool updateDatabase(void);
    //virtual void openCustomMenuProject(const QPoint& point);

signals:

    void listUpdated(); ///< Signal émis lorsque la liste a été mise à jour.

private:
    
    int m_id; ///< Identifiant de la liste en base de données.
    //QList<critères>
    //conditions de maj
};

#endif

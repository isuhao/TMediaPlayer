
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

public:

    CDynamicPlayList(void);
    ~CDynamicPlayList();

public slots:

    void update(void);

signals:

    void listUpdated(); ///< Signal émis lorsque la liste a été mise à jour.

private:

    //QList<critères>
    //conditions de maj
};

#endif

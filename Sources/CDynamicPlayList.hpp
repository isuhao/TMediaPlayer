
#ifndef FILE_C_DYNAMIC_PLAYLIST
#define FILE_C_DYNAMIC_PLAYLIST

#include "CPlayList.hpp"


class ICriteria;


/**
 * Liste de lecture dynamique.
 * Contient une liste de critères de recherche.
 */

class CDynamicPlayList : public CPlayList
{
    Q_OBJECT

    friend class CDialogEditDynamicList;
    friend class CApplication;

public:

    explicit CDynamicPlayList(CApplication * application, const QString& name = QString());
    ~CDynamicPlayList();

    inline int getId(void) const;

public slots:

    void update(void);

protected slots:

    //virtual void openCustomMenuProject(const QPoint& point);

signals:

    void listUpdated(void); ///< Signal émis lorsque la liste a été mise à jour.

protected:
    
    bool updateDatabase(void);
    void loadFromDatabase(void);
    void setCriteria(ICriteria * criteria);

private:
    
    int m_id; ///< Identifiant de la liste en base de données.
    ICriteria * m_mainCriteria;
    //QList<critères>
    //conditions de maj
};


inline int CDynamicPlayList::getId(void) const
{
    return m_id;
}

#endif // FILE_C_DYNAMIC_PLAYLIST

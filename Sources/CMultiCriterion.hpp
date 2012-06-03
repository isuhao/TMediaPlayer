
#ifndef FILE_C_MULTI_CRITERION
#define FILE_C_MULTI_CRITERION

#include "ICriteria.hpp"
#include <QList>


/**
 * Critère contenant des sous-critères.
 */

class CMultiCriterion : public ICriteria
{
    Q_OBJECT

public:

    enum TMultiCriterionType
    {
        Union        = 0, ///< N'importe quel critère.
        Intersection = 1  ///< Tous les critères.
    };

    static inline TMultiCriterionType getMultiCriterionTypeFromInteger(int type);


    explicit CMultiCriterion(CApplication * application, QObject * parent = NULL);
    virtual ~CMultiCriterion();

    TMultiCriterionType getMultiCriterionType(void) const;
    inline QList<ICriteria *> getChildren(void) const;
    inline int getNumChildren(void) const;

    void setMultiCriterionType(TMultiCriterionType type);
    void addChild(ICriteria * child);

    virtual bool matchCriteria(CSong * song) const;
    virtual QList<CSong *> getSongs(const QList<CSong *>& from, const QList<CSong *>& with = QList<CSong *>()) const;
    //virtual QList<int> getValidTypes(void) const;
    //virtual bool isValidType(int type) const;

    virtual IWidgetCriteria * getWidget(void) const;

protected:

    virtual void setPlayList(CDynamicPlayList * playList);
    virtual void insertIntoDatabase(CApplication * application);

private:

    QList<ICriteria *> m_children;
};


inline CMultiCriterion::TMultiCriterionType CMultiCriterion::getMultiCriterionTypeFromInteger(int type)
{
    switch (type)
    {
        default: return Intersection;
        case 0:  return Union;
        case 1:  return Intersection;
    }
}


inline QList<ICriteria *> CMultiCriterion::getChildren(void) const
{
    return m_children;
}


inline int CMultiCriterion::getNumChildren(void) const
{
    return m_children.size();
}

#endif // FILE_C_MULTI_CRITERION

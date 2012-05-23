
#ifndef FILE_CMULTICRITERION
#define FILE_CMULTICRITERION

#include "ICriteria.hpp"
#include <QList>


class CMultiCriterion : public ICriteria
{
public:

    enum TMultiCriterionType
    {
        Union,       ///< N'importe quel critère.
        Intersection ///< Tous les critères.
    };

    CMultiCriterion(QObject * parent = NULL);
    ~CMultiCriterion();

    inline TMultiCriterionType getMultiCriterionType(void) const;
    inline QList<ICriteria *> getChildren(void) const;
    inline int getNumChildren(void) const;

    void setMultiCriterionType(TMultiCriterionType type);
    void addChild(ICriteria * child);

    virtual bool matchCriteria(CSong * song) const;
    virtual QList<CSong *> getSongs(const QList<CSong *>& from, const QList<CSong *>& with = QList<CSong *>()) const;
    //virtual QList<int> getValidTypes(void) const;
    //virtual bool isValidType(int type) const;

private:

    TMultiCriterionType m_multi_type;
    QList<ICriteria *> m_children;
};


inline CMultiCriterion::TMultiCriterionType CMultiCriterion::getMultiCriterionType(void) const
{
    return m_multi_type;
}


inline QList<ICriteria *> CMultiCriterion::getChildren(void) const
{
    return m_children;
}


inline int CMultiCriterion::getNumChildren(void) const
{
    return m_children.size();
}

#endif // FILE_CMULTICRITERION

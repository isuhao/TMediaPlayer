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

    TMultiCriterionType getMultiCriterionType() const;
    inline QList<ICriteria *> getChildren() const;
    inline int getNumChildren() const;

    void setMultiCriterionType(TMultiCriterionType type);
    void addChild(ICriteria * child);

    virtual bool matchCriteria(CSong * song) const;
    virtual QList<CSong *> getSongs(const QList<CSong *>& from, const QList<CSong *>& with = QList<CSong *>()) const;
    virtual TUpdateConditions getUpdateConditions() const;

    virtual IWidgetCriteria * getWidget() const;

protected:

    virtual void setPlayList(CDynamicList * playList);
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


inline QList<ICriteria *> CMultiCriterion::getChildren() const
{
    return m_children;
}


inline int CMultiCriterion::getNumChildren() const
{
    return m_children.size();
}

#endif // FILE_C_MULTI_CRITERION

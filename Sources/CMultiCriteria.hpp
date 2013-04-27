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

#include "ICriterion.hpp"
#include <QList>


/**
 * Critère contenant des sous-critères.
 */

class CMultiCriteria : public ICriterion
{
    Q_OBJECT

public:

    enum TMultiCriteriaType
    {
        Union        = 0, ///< N'importe quel critère.
        Intersection = 1  ///< Tous les critères.
    };

    static inline TMultiCriteriaType getMultiCriteriaTypeFromInteger(int type);


    explicit CMultiCriteria(CMainWindow * mainWindow, QObject * parent = nullptr);
    virtual ~CMultiCriteria();

    TMultiCriteriaType getMultiCriteriaType() const;
    inline QList<ICriterion *> getChildren() const;
    inline int getNumChildren() const;

    void setMultiCriteriaType(TMultiCriteriaType type);
    void addChild(ICriterion * child);

    virtual bool matchCriterion(CSong * song) const;
    virtual QList<CSong *> getSongs(const QList<CSong *>& from, const QList<CSong *>& with = QList<CSong *>()) const;
    virtual TUpdateConditions getUpdateConditions() const;

    virtual IWidgetCriterion * getWidget() const;

protected:

    virtual void setPlayList(CDynamicList * playList);
    virtual void insertIntoDatabase();

private:

    QList<ICriterion *> m_children;
};


inline CMultiCriteria::TMultiCriteriaType CMultiCriteria::getMultiCriteriaTypeFromInteger(int type)
{
    switch (type)
    {
        default: return Intersection;
        case 0:  return Union;
        case 1:  return Intersection;
    }
}


inline QList<ICriterion *> CMultiCriteria::getChildren() const
{
    return m_children;
}


inline int CMultiCriteria::getNumChildren() const
{
    return m_children.size();
}

#endif // FILE_C_MULTI_CRITERION

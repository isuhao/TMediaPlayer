/*
Copyright (C) 2012 Teddy Michel

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

#ifndef FILE_C_DYNAMIC_PLAYLIST
#define FILE_C_DYNAMIC_PLAYLIST

#include "IPlayList.hpp"


class ICriteria;
class CWidgetMultiCriterion;


/**
 * Liste de lecture dynamique.
 * Contient une liste de critères de recherche.
 */

class CDynamicList : public IPlayList
{
    Q_OBJECT

    friend class CDialogEditDynamicList;
    friend class CApplication;
    friend class CListModel;

public:

    explicit CDynamicList(CApplication * application, const QString& name = QString());
    ~CDynamicList();

    virtual bool isModified(void) const;
    CWidgetMultiCriterion * getWidget(void) const;

    inline int getId(void) const;

public slots:

    void update(void);

protected slots:

    //virtual void openCustomMenuProject(const QPoint& point);

signals:

    void listModified(void); ///< Signal émis lorsque la liste a été modifiée.
    void listUpdated(void);  ///< Signal émis lorsque la liste a été mise à jour.

protected:
    
    virtual bool updateDatabase(void);
    virtual void romoveFromDatabase(void);
    void loadFromDatabase(void);
    void setCriteria(ICriteria * criteria);

private:
    
    int m_id; ///< Identifiant de la liste en base de données.
    ICriteria * m_mainCriteria;
    bool m_isDynamicListModified; ///< Indique si la liste a été modifiée.
    //QList<critères>
    //conditions de maj
};


inline int CDynamicList::getId(void) const
{
    return m_id;
}

#endif // FILE_C_DYNAMIC_PLAYLIST

/*
Copyright (C) 2012-2016 Teddy Michel

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


class ICriterion;
class CWidgetMultiCriteria;


/**
 * Liste de lecture dynamique.
 * Contient une liste de critères de recherche.
 */

class CDynamicList : public IPlayList
{
    Q_OBJECT

    friend class CDialogEditDynamicList;
    friend class CMainWindow;
    friend class CLibraryModel;

public:

    explicit CDynamicList(CMainWindow * mainWindow, const QString& name = QString());
    virtual ~CDynamicList();

    virtual bool isModified() const;
    CWidgetMultiCriteria * getWidget() const;

    inline int getId() const;
    inline bool isAutoUpdate() const;
    inline bool getOnlyChecked() const;
    inline int getNumItems() const;

public slots:

    void tryUpdateList();
    void updateList();

signals:

    void listModified(); ///< Signal émis lorsque la liste a été modifiée.
    void listUpdated();  ///< Signal émis lorsque la liste a été mise à jour.

protected:

    virtual bool updateDatabase();
    virtual void removeFromDatabase();
    void loadFromDatabase();
    void setCriterion(ICriterion * criteria);
    void setAutoUpdate(bool autoUpdate = true);
    void setOnlyChecked(bool onlyChecked = true);

private:

    int m_id;                     ///< Identifiant de la liste en base de données.
    bool m_needUpdate;            ///< Indique si une mise-à-jour doit être effectuée.
    ICriterion * m_mainCriterion; ///< Critère parent de la liste.
    bool m_isDynamicListModified; ///< Indique si la liste a été modifiée.
    bool m_autoUpdate;            ///< Indique si la liste doit être mise à jour automatiquement.
    bool m_onlyChecked;           ///< Indique si on doit utiliser uniquement les morceaux cochés.
    int m_numItems;
  //int m_itemsType;
  //int m_itemsSort;
};


/**
 * Retourne l'identifiant de la liste dynamique.
 *
 * \return Identifiant de la liste.
 */

inline int CDynamicList::getId() const
{
    return m_id;
}


inline bool CDynamicList::isAutoUpdate() const
{
    return m_autoUpdate;
}


inline bool CDynamicList::getOnlyChecked() const
{
    return m_onlyChecked;
}


inline int CDynamicList::getNumItems() const
{
    return m_numItems;
}

#endif // FILE_C_DYNAMIC_PLAYLIST

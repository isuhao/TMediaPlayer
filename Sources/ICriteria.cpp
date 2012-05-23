
#include "ICriteria.hpp"


ICriteria::ICriteria(QObject * parent) :
    QObject     (parent),
    m_type      (TypeInvalid),
    m_condition (CondInvalid),
    m_value1    (),
    m_value2    (),
    m_id        (-1),
    m_playList  (NULL)
{

}


ICriteria::~ICriteria()
{

}


/**
 * Retourne la liste des morceaux qui v�rifient le crit�re.
 * Cette impl�mentation de base parcourt la liste \a from est utilise la m�thode matchCriteria
 * pour tester si le morceau v�rifie le crit�re.
 *
 * \param from Liste de morceaux � analyser.
 * \param with Liste de morceaux � ajouter dans la liste.
 * \return Liste de morceaux qui v�rifient le crit�re, sans doublons, avec tous les
 *         �l�ments de \a with.
 */

QList<CSong *> ICriteria::getSongs(const QList<CSong *>& from, const QList<CSong *>& with) const
{
    QList<CSong *> songList = with;

    foreach (CSong * song, from)
    {
        if (matchCriteria(song) && !songList.contains(song))
        {
            songList.append(song);
        }
    }

    return songList;
}

/*
QList<int> ICriteria::getValidTypes(void) const
{
    return QList<int>();
}


bool ICriteria::isValidType(int type) const
{
    return false;
}


QList<int> ICriteria::getValidConditions(void) const
{
    return QList<int>();
}
*/
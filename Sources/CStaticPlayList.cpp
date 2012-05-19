
#include "CStaticPlayList.hpp"


CStaticPlayList::CStaticPlayList(const QString& name) :
    CPlayList (name)
{

}


CStaticPlayList::~CStaticPlayList()
{

}


/**
 * Ajoute une chanson à la liste.
 *
 * \param song Pointeur sur la chanson à ajouter.
 * \param pos  Position où ajouter la chanson. Si négatif, la chanson est ajoutée à la fin de la liste.
 */

void CStaticPlayList::addSong(CSong * song, int pos)
{
    Q_CHECK_PTR(song);
    CSongTable::addSong(song, pos);
    emit songAdded(song);
}


/**
 * Enlève une chanson de la liste.
 * Toutes les occurences de \a song sont enlevées de la liste.
 *
 * \param song Pointeur sur la chanson à enlever.
 */

void CStaticPlayList::removeSong(CSong * song)
{
    Q_CHECK_PTR(song);

    if (hasSong(song))
    {
        removeSong(song);
        CSongTable::removeSong(song);
        emit songRemoved(song);
        emit listModified();
    }
}


/**
 * Enlève une chanson de la liste.
 *
 * \param pos Position de la chanson dans la liste (à partir de 0).
 */

void CStaticPlayList::removeSong(int pos)
{
    Q_ASSERT(pos >= 0 && pos < getNumSongs());
    CSong * song = getSongForIndex(pos);
    CSongTable::removeSong(pos);
    emit songRemoved(song);
    emit listModified();
}


/**
 * Enlève les doublons de la liste.
 *
 * \todo Implémentation.
 */

void CStaticPlayList::removeDuplicateSongs(void)
{
    //TODO...
}

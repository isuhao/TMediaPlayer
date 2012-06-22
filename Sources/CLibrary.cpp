
#include "CLibrary.hpp"


CLibrary::CLibrary(CApplication * application) :
    CSongTable (application)
{

}


/**
 * Supprime tous les morceaux de la table. La mémoire est libérée.
 */

void CLibrary::deleteSongs(void)
{
    QList<CSong *> songs = m_model->getSongs();

    for (QList<CSong *>::const_iterator it = songs.begin(); it != songs.end(); ++it)
    {
        delete *it;
    }

    removeAllSongsFromTable();
}


/**
 * Ajoute un morceau à la table.
 *
 * \todo Supprimer cette méthode ?
 */

void CLibrary::addSong(CSong * song, int pos)
{
    addSongToTable(song, pos);
}


/**
 * Ajoute une liste de morceaux à la table.
 *
 * \todo Supprimer cette méthode ?
 */

void CLibrary::addSongs(const QList<CSong *>& songs)
{
    addSongsToTable(songs);
}


/**
 * Enlève un morceau de la table.
 *
 * \todo Supprimer cette méthode ?
 */

void CLibrary::removeSong(CSong * song)
{
    removeSongFromTable(song);
}


/**
 * Enlène un morceau de la table à partir de sa position.
 *
 * \todo Supprimer cette méthode ?
 */

void CLibrary::removeSong(int pos)
{
    removeSongFromTable(pos);
}

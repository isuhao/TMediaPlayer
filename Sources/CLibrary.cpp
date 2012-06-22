
#include "CLibrary.hpp"


CLibrary::CLibrary(CApplication * application) :
    CSongTable (application)
{

}


/**
 * Supprime tous les morceaux de la table. La m�moire est lib�r�e.
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
 * Ajoute un morceau � la table.
 *
 * \todo Supprimer cette m�thode ?
 */

void CLibrary::addSong(CSong * song, int pos)
{
    addSongToTable(song, pos);
}


/**
 * Ajoute une liste de morceaux � la table.
 *
 * \todo Supprimer cette m�thode ?
 */

void CLibrary::addSongs(const QList<CSong *>& songs)
{
    addSongsToTable(songs);
}


/**
 * Enl�ve un morceau de la table.
 *
 * \todo Supprimer cette m�thode ?
 */

void CLibrary::removeSong(CSong * song)
{
    removeSongFromTable(song);
}


/**
 * Enl�ne un morceau de la table � partir de sa position.
 *
 * \todo Supprimer cette m�thode ?
 */

void CLibrary::removeSong(int pos)
{
    removeSongFromTable(pos);
}

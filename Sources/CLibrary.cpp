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

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

#ifndef FILE_C_LIBRARY_FOLDER
#define FILE_C_LIBRARY_FOLDER


#include <QString>
#include <QObject>


class CLibraryFolder : public QObject
{
    Q_OBJECT

public:

    explicit CLibraryFolder(QObject * parent = NULL);

    QString convertFormatItemsToString() const;
    void convertStringToFormatItems(const QString& formatItems);

    int id;                  ///< Identifiant du r�pertoire en base de donn�es.
    QString pathName;        ///< Localisation du r�pertoire.
    bool keepOrganized;      ///< Indique si le r�pertoire est automatiquement organis�.
    QString format;          ///< Format des noms de fichier.

    QString titleDefault;    ///< Titre du morceau.
    QString titleEmpty;
    QString albumDefault;    ///< Nom de l'album.
    QString albumEmpty;
    QString artistDefault;   ///< Nom de l'artiste.
    QString artistEmpty;
    QString genreDefault;    ///< Genre.
    QString genreEmpty;
    QString yearDefault;     ///< Ann�e.
    QString yearEmpty;
    QString trackDefault;    ///< Num�ro de piste.
    QString trackEmpty;
    QString discDefault;     ///< Num�ro de disque.
    QString discEmpty;

    QString compilationName; ///< Nom du r�pertoire pour stocker les compilations.
};

#endif // FILE_C_LIBRARY_FOLDER

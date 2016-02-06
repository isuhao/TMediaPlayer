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

#include "CLibraryFolder.hpp"
#include <QStringList>


CLibraryFolder::CLibraryFolder(QObject * parent) :
QObject         (parent),
id              (-1),
keepOrganized   (false),
format          ("%2/%3%4/%6%5%1"),
titleDefault    ("%1"),
titleEmpty      (tr("Unknown title")),
albumDefault    ("%1"),
albumEmpty      (tr("Unknown album")),
artistDefault   ("%1"),
artistEmpty     (tr("Unknown artist")),
genreDefault    ("%1"),
genreEmpty      (tr("Unknown genre")),
yearDefault     (" (%1)"),
yearEmpty       (),
trackDefault    ("%1 "),
trackEmpty      (),
discDefault     ("%1-"),
discEmpty       (),
compilationName (tr("Compilations"))
{

}


QString CLibraryFolder::convertFormatItemsToString() const
{
    QString str;

    // Remarque : on utiliser deux caractères spéciaux interdits (: et |) comme séparateurs.
    str += QString("TitleDefault"   ) + QChar(':') + titleDefault    + QChar('|');
    str += QString("TitleEmpty"     ) + QChar(':') + titleEmpty      + QChar('|');
    str += QString("ArtistDefault"  ) + QChar(':') + artistDefault   + QChar('|');
    str += QString("ArtistEmpty"    ) + QChar(':') + artistEmpty     + QChar('|');
    str += QString("AlbumDefault"   ) + QChar(':') + albumDefault    + QChar('|');
    str += QString("AlbumEmpty"     ) + QChar(':') + albumEmpty      + QChar('|');
    str += QString("YearDefault"    ) + QChar(':') + yearDefault     + QChar('|');
    str += QString("YearEmpty"      ) + QChar(':') + yearEmpty       + QChar('|');
    str += QString("TrackDefault"   ) + QChar(':') + trackDefault    + QChar('|');
    str += QString("TrackEmpty"     ) + QChar(':') + trackEmpty      + QChar('|');
    str += QString("DiscDefault"    ) + QChar(':') + discDefault     + QChar('|');
    str += QString("DiscEmpty"      ) + QChar(':') + discEmpty       + QChar('|');
    str += QString("GenreDefault"   ) + QChar(':') + genreDefault    + QChar('|');
    str += QString("GenreEmpty"     ) + QChar(':') + genreEmpty      + QChar('|');
    str += QString("CompilationName") + QChar(':') + compilationName;

    return str;
}


void CLibraryFolder::convertStringToFormatItems(const QString& formatItems)
{
    QStringList items = formatItems.split('|');

    for (QStringList::ConstIterator item = items.begin(); item != items.end(); ++item)
    {
        QStringList itemSplit = item->split(':');

        if (itemSplit.size() != 2)
            continue;

        QString itemKey = itemSplit.at(0);
        QString itemVal = itemSplit.at(1);

        if (itemKey == "TitleDefault")
            titleDefault = itemVal;
        else if (itemKey == "TitleEmpty")
            titleEmpty = itemVal;
        else if (itemKey == "ArtistDefault")
            artistDefault = itemVal;
        else if (itemKey == "ArtistEmpty")
            artistEmpty = itemVal;
        else if (itemKey == "AlbumDefault")
            albumDefault = itemVal;
        else if (itemKey == "AlbumEmpty")
            albumEmpty = itemVal;
        else if (itemKey == "YearDefault")
            yearDefault = itemVal;
        else if (itemKey == "YearEmpty")
            yearEmpty = itemVal;
        else if (itemKey == "TrackDefault")
            trackDefault = itemVal;
        else if (itemKey == "TrackEmpty")
            trackEmpty = itemVal;
        else if (itemKey == "DiscDefault")
            discDefault = itemVal;
        else if (itemKey == "DiscEmpty")
            discEmpty = itemVal;
        else if (itemKey == "GenreDefault")
            genreDefault = itemVal;
        else if (itemKey == "GenreEmpty")
            genreEmpty = itemVal;

        else if (itemKey == "CompilationName")
            compilationName = itemVal;
    }
}

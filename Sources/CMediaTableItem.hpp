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

#ifndef FILE_C_MEDIA_TABLE_ITEM_HPP_
#define FILE_C_MEDIA_TABLE_ITEM_HPP_

class CSong;


class CMediaTableItem
{
    friend class CMediaTableModel;

public:

    inline CMediaTableItem();

    inline int getPosition() const;
    inline CSong * getSong() const;
    //inline IMediaTable * getMediaTable() const;
    inline bool isValid() const;

    inline bool operator==(const CMediaTableItem& other) const;
    inline bool operator!=(const CMediaTableItem& other) const;

private:

    inline CMediaTableItem(const CMediaTableItem& other);
    inline CMediaTableItem(int position, CSong * song/*, IMediaTable * mediaTable*/);

    inline CMediaTableItem& operator=(const CMediaTableItem& other);

    int m_position; ///< Position dans la liste.
    CSong * m_song; ///< Pointeur sur le morceau.
    //IMediaTable * m_mediaTable;
};


/**
 * Constructeur par défaut.
 */

inline CMediaTableItem::CMediaTableItem() :
m_position (-1),
m_song     (nullptr)
{

}


/**
 * Constructeur de copie.
 */

inline CMediaTableItem::CMediaTableItem(const CMediaTableItem& other) :
m_position   (other.m_position),
m_song       (other.m_song)/*,
m_mediaTable (other.m_mediaTable)*/
{

}


inline CMediaTableItem::CMediaTableItem(int position, CSong * song/*, IMediaTable * mediaTable*/) :
m_position   (position),
m_song       (song)/*,
m_mediaTable (mediaTable)*/
{
    Q_CHECK_PTR(song);
}


inline int CMediaTableItem::getPosition() const
{
    return m_position;
}


inline CSong * CMediaTableItem::getSong() const
{
    return m_song;
}

/*
inline IMediaTable * CMediaTableItem::getMediaTable() const
{
    return m_mediaTable;
}
*/

inline bool CMediaTableItem::isValid() const
{
    return (m_position >= 0 && m_song);
}


inline bool CMediaTableItem::operator==(const CMediaTableItem& other) const
{
    return (m_song == other.m_song /*&& m_mediaTable == other.m_mediaTable*/ && m_position == other.m_position);
}


inline bool CMediaTableItem::operator!=(const CMediaTableItem& other) const
{
    return !operator==(other);
}


/**
 * Opérateur d'affectation.
 */

inline CMediaTableItem& CMediaTableItem::operator=(const CMediaTableItem& other)
{
    m_song = other.m_song;
    /*m_mediaTable = other.m_mediaTable;*/
    m_position = other.m_position;

    return *this;
}

#endif // FILE_C_MEDIA_TABLE_ITEM_HPP_

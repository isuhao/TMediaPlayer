
#include "CSongTableModel.hpp"
#include "CSongTable.hpp"
#include <QMouseEvent>

#include <QtDebug>


CSongTableItem::CSongTableItem(void) :
    m_position (-1),
    m_song     (NULL)
{
    
}


CSongTableItem::CSongTableItem(int position, CSong * song) :
    m_position (position),
    m_song     (song)
{
    Q_CHECK_PTR(song);
}


CSongTableModel::CSongTableModel(const QList<CSong *>& data, QObject * parent) :
    QAbstractTableModel (parent),
    m_canDrop           (false)
{
    int i = 0;

    foreach (CSong * song, data)
    {
        m_data.append(new CSongTableItem(++i, song));
    }
}


void CSongTableModel::setCanDrop(bool canDrop)
{
    m_canDrop = canDrop;
}


void CSongTableModel::setSongs(const QList<CSong *>& data)
{
    emit layoutAboutToBeChanged();

    m_data.clear();
    int i = 0;

    foreach (CSong * song, data)
    {
        m_data.append(new CSongTableItem(++i, song));
    }

    emit layoutChanged();
}


QList<CSong *> CSongTableModel::getSongs(void) const
{
    QList<CSong *> songList;
    songList.reserve(m_data.size());

    foreach (CSongTableItem * item, m_data)
    {
        songList.append(item->getSong());
    }

    return songList;
}


bool CSongTableModel::hasSong(CSong * song) const
{
    foreach (CSongTableItem * songItem, m_data)
    {
        if (songItem->getSong() == song)
        {
            return true;
        }
    }

    return false;
}


int CSongTableModel::rowCount(const QModelIndex& parent) const
{
    return m_data.size();
}


int CSongTableModel::columnCount(const QModelIndex& parent) const
{
    return CSongTable::ColNumber;
}


QVariant CSongTableModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= m_data.size())
    {
        return QVariant::Invalid;
    }

    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
            case 0: return m_data.at(index.row())->getPosition();
            case 1: return m_data.at(index.row())->getSong()->getTitle();
            case 2: return m_data.at(index.row())->getSong()->getArtistName();
            case 3: return m_data.at(index.row())->getSong()->getAlbumTitle();
            case 4: return m_data.at(index.row())->getSong()->getAlbumArtist();
            case 5: return m_data.at(index.row())->getSong()->getComposer();

            // Année
            case 6:
            {
                int year = m_data.at(index.row())->getSong()->getYear();
                return (year > 0 ? year : QVariant(QString()));
            }

            // Numéro de piste
            case 7:
            {
                const int trackNumber = m_data.at(index.row())->getSong()->getTrackNumber();

                if (trackNumber <= 0)
                {
                    return QString();
                }

                const int trackTotal = m_data.at(index.row())->getSong()->getTrackTotal();

                if (trackTotal >= trackNumber)
                {
                    return QString("%1/%1").arg(trackNumber).arg(trackTotal);
                }
                else
                {
                    return trackNumber;
                }
            }

            // Numéro de disque
            case 8:
            {
                const int discNumber = m_data.at(index.row())->getSong()->getDiscNumber();

                if (discNumber <= 0)
                {
                    return QString();
                }

                const int discTotal = m_data.at(index.row())->getSong()->getDiscTotal();

                if (discTotal >= discNumber)
                {
                    return QString("%1/%1").arg(discNumber).arg(discTotal);
                }
                else
                {
                    return discNumber;
                }
            }

            case  9: return m_data.at(index.row())->getSong()->getGenre();
            case 10: return m_data.at(index.row())->getSong()->getRating();
            case 11: return m_data.at(index.row())->getSong()->getComments();
            case 12: return m_data.at(index.row())->getSong()->getNumPlays();
            case 13: return m_data.at(index.row())->getSong()->getLastPlay();
            case 14: return m_data.at(index.row())->getSong()->getFileName();
            case 15: return m_data.at(index.row())->getSong()->getBitRate();

            // Format
            case 16:
            {
                switch (m_data.at(index.row())->getSong()->getFileType())
                {
                    default:
                    case CSong::TypeUnknown: return tr("Unknown");
                    case CSong::TypeMP3:     return tr("MP3");
                    case CSong::TypeOGG:     return tr("OGG Vorbis");
                    case CSong::TypeFlac:    return tr("FLAC");
                }
            }

            // Durée
            case 17:
            {
                int duration = m_data.at(index.row())->getSong()->getDuration();

                QTime durationTime(0, 0);
                durationTime = durationTime.addMSecs(duration);
                return durationTime.toString("m:ss"); /// \todo Stocker le format dans les paramètres.
            }

            // Taux d'échantillonnage
            case 18:
                return m_data.at(index.row())->getSong()->getSampleRate();
                break;
        }
    }

    return QVariant::Invalid;
}


QVariant CSongTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant::Invalid;
    }

    if (orientation == Qt::Horizontal)
    {
        switch(section)
        {
            case  0: return QString(tr("#"));
            case  1: return QString(tr("Title"));
            case  2: return QString(tr("Artist"));
            case  3: return QString(tr("Album"));
            case  4: return QString(tr("Album artist"));
            case  5: return QString(tr("Composer"));
            case  6: return QString(tr("Year"));
            case  7: return QString(tr("Track"));
            case  8: return QString(tr("Disc"));
            case  9: return QString(tr("Genre"));
            case 10: return QString(tr("Rating"));
            case 11: return QString(tr("Comments"));
            case 12: return QString(tr("Plays"));
            case 13: return QString(tr("Last played"));
            case 14: return QString(tr("File name"));
            case 15: return QString(tr("Bit rate"));
            case 16: return QString(tr("Format"));
            case 17: return QString(tr("Duration"));
            case 18: return QString(tr("Sample rate"));
        }
    }

    return QVariant::Invalid;
}


void CSongTableModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();

    if (order == Qt::AscendingOrder)
    {
        switch(column)
        {
            case  0: qSort(m_data.begin(), m_data.end(), cmpSongPositionAsc   ); break;
            case  1: qSort(m_data.begin(), m_data.end(), cmpSongTitleAsc      ); break;
            case  2: qSort(m_data.begin(), m_data.end(), cmpSongArtistAsc     ); break;
            case  3: qSort(m_data.begin(), m_data.end(), cmpSongAlbumAsc      ); break;
            case  4: qSort(m_data.begin(), m_data.end(), cmpSongAlbumArtistAsc); break;
            case  5: qSort(m_data.begin(), m_data.end(), cmpSongComposerAsc   ); break;
            case  6: qSort(m_data.begin(), m_data.end(), cmpSongYearAsc       ); break;
            case  7: qSort(m_data.begin(), m_data.end(), cmpSongTrackAsc      ); break;
            case  8: qSort(m_data.begin(), m_data.end(), cmpSongDiscAsc       ); break;
            case  9: qSort(m_data.begin(), m_data.end(), cmpSongGenreAsc      ); break;
            case 10: qSort(m_data.begin(), m_data.end(), cmpSongRatingAsc     ); break;
            case 11: qSort(m_data.begin(), m_data.end(), cmpSongCommentsAsc   ); break;
            case 12: qSort(m_data.begin(), m_data.end(), cmpSongPlayCountAsc  ); break;
            case 13: qSort(m_data.begin(), m_data.end(), cmpSongLastPlayedAsc ); break;
            case 14: qSort(m_data.begin(), m_data.end(), cmpSongFileNameAsc   ); break;
            case 15: qSort(m_data.begin(), m_data.end(), cmpSongBitRateAsc    ); break;
            case 16: qSort(m_data.begin(), m_data.end(), cmpSongFormatAsc     ); break;
            case 17: qSort(m_data.begin(), m_data.end(), cmpSongDurationAsc   ); break;
            case 18: qSort(m_data.begin(), m_data.end(), cmpSongSampleRateAsc ); break;
        }
    }
    else
    {
        switch(column)
        {
            case  0: qSort(m_data.begin(), m_data.end(), cmpSongPositionDesc   ); break;
            case  1: qSort(m_data.begin(), m_data.end(), cmpSongTitleDesc      ); break;
            case  2: qSort(m_data.begin(), m_data.end(), cmpSongArtistDesc     ); break;
            case  3: qSort(m_data.begin(), m_data.end(), cmpSongAlbumDesc      ); break;
            case  4: qSort(m_data.begin(), m_data.end(), cmpSongAlbumArtistDesc); break;
            case  5: qSort(m_data.begin(), m_data.end(), cmpSongComposerDesc   ); break;
            case  6: qSort(m_data.begin(), m_data.end(), cmpSongYearDesc       ); break;
            case  7: qSort(m_data.begin(), m_data.end(), cmpSongTrackDesc      ); break;
            case  8: qSort(m_data.begin(), m_data.end(), cmpSongDiscDesc       ); break;
            case  9: qSort(m_data.begin(), m_data.end(), cmpSongGenreDesc      ); break;
            case 10: qSort(m_data.begin(), m_data.end(), cmpSongRatingDesc     ); break;
            case 11: qSort(m_data.begin(), m_data.end(), cmpSongCommentsDesc   ); break;
            case 12: qSort(m_data.begin(), m_data.end(), cmpSongPlayCountDesc  ); break;
            case 13: qSort(m_data.begin(), m_data.end(), cmpSongLastPlayedDesc ); break;
            case 14: qSort(m_data.begin(), m_data.end(), cmpSongFileNameDesc   ); break;
            case 15: qSort(m_data.begin(), m_data.end(), cmpSongBitRateDesc    ); break;
            case 16: qSort(m_data.begin(), m_data.end(), cmpSongFormatDesc     ); break;
            case 17: qSort(m_data.begin(), m_data.end(), cmpSongDurationDesc   ); break;
            case 18: qSort(m_data.begin(), m_data.end(), cmpSongSampleRateDesc ); break;
        }
    }

    emit layoutChanged();
    emit columnSorted(column, order);
}


Qt::ItemFlags CSongTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index) | Qt::ItemIsDragEnabled;

    if (index.isValid())
    {
        if (m_canDrop)
        {
            flags |= Qt::ItemIsDropEnabled;
        }
    }

    return flags;
}


QStringList CSongTableModel::mimeTypes(void) const
{
    QStringList types;
    types << "application/x-ted-media-songs";
    return types;
}


QMimeData * CSongTableModel::mimeData(const QModelIndexList& indexes) const
{
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    QList<int> rows;

    foreach (QModelIndex index, indexes)
    {
        if (index.isValid() && !rows.contains(index.row()))
        {
            rows.append(index.row());
        }
    }

    qSort(rows);

    stream << rows.size();

    foreach (int row, rows)
    {
        stream << m_data[row]->getSong()->getId();
    }

    qDebug() << "CSongTableModel::mimeData()...";
    
    QMimeData * mimeData = new QMimeData(); // = QTreeView::mimeData(indexes);
    mimeData->setData("application/x-ted-media-songs", encodedData);
    return mimeData;
}


bool CSongTableModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    if (!m_canDrop)
    {
        return false;
    }

    if (!data->hasFormat("application/x-ted-media-songs"))
    {
        return false;
    }

    qDebug() << "CSongTableModel::dropMimeData()";
    //...

    return false;
}


void CSongTableModel::insertRow(CSong * song, int pos)
{
    Q_CHECK_PTR(song);

    emit layoutAboutToBeChanged();

    if (pos < 0 || pos >= m_data.size())
    {
        m_data.append(new CSongTableItem(pos, song));
    }
    else
    {
        m_data.insert(pos, new CSongTableItem(pos, song));
    }

    emit layoutChanged();
}


void CSongTableModel::removeRow(int row)
{
    emit layoutAboutToBeChanged();

    Q_ASSERT(row >= 0 && row < m_data.size());

    delete m_data.takeAt(row);

    emit layoutChanged();
}


void CSongTableModel::clear(void)
{
    emit layoutAboutToBeChanged();

    foreach (CSongTableItem * songItem, m_data)
    {
        delete songItem;
    }

    m_data.clear();
    emit layoutChanged();
}


CSongTableItem * CSongTableModel::getSongItem(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return getSongItem(index.row());
    }

    return NULL;
}


CSongTableItem * CSongTableModel::getSongItem(int row) const
{
    Q_ASSERT(row >= 0);
    return (row < m_data.size() ? m_data.at(row) : NULL);
}


int CSongTableModel::getRowForSongItem(CSongTableItem * songItem) const
{
    Q_CHECK_PTR(songItem);
    return m_data.indexOf(songItem);
}


/// \todo Implémentation.
CSongTableItem * CSongTableModel::getPreviousSong(CSongTableItem * songItem, bool shuffle) const
{
    //...
    return NULL;
/*
    if (songItem)
    {
        if (shuffle)
        {
            //TODO...
            return NULL;
        }
        else
        {
            return (pos > 0 ? pos - 1 : NULL);
        }
    }
    else
    {
        if (shuffle)
        {
            //TODO...
            return NULL;
        }
        else
        {
            return NULL;
        }
    }
*/
}


/// \todo Implémentation.
CSongTableItem * CSongTableModel::getNextSong(CSongTableItem * songItem, bool shuffle) const
{
    if (songItem)
    {
        const int row = getRowForSongItem(songItem);

        if (shuffle)
        {
            //TODO...
            return NULL;
        }
        else
        {
            return (row == m_data.size() - 1 ? NULL : m_data.at(row + 1));
        }
    }
    else
    {
        if (shuffle)
        {
            //TODO...
            return NULL;
        }
        else
        {
            return (m_data.isEmpty() ? NULL : m_data.at(0));
        }
    }
}


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


CSongTableModel::CSongTableModel(const QList<CSong *>& data, QWidget * parent) :
    QAbstractTableModel (parent),
    m_canDrop           (false)
{
    int i = 0;

    foreach (CSong * song, data)
    {
        m_data.append(new CSongTableItem(++i, song));
    }
}


CSongTableModel::CSongTableModel(QWidget * parent) :
    QAbstractTableModel (parent),
    m_canDrop           (false)
{

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
        switch (index.column())
        {
            case CSongTable::ColPosition:
                if (m_canDrop)
                {
                    return m_data.at(index.row())->getPosition();
                }
                break;

            case CSongTable::ColTitle      : return m_data.at(index.row())->getSong()->getTitle();
            case CSongTable::ColArtist     : return m_data.at(index.row())->getSong()->getArtistName();
            case CSongTable::ColAlbum      : return m_data.at(index.row())->getSong()->getAlbumTitle();
            case CSongTable::ColAlbumArtist: return m_data.at(index.row())->getSong()->getAlbumArtist();
            case CSongTable::ColComposer   : return m_data.at(index.row())->getSong()->getComposer();

            // Année
            case CSongTable::ColYear:
            {
                int year = m_data.at(index.row())->getSong()->getYear();
                return (year > 0 ? year : QVariant(QString()));
            }

            // Numéro de piste
            case CSongTable::ColTrackNumber:
            {
                const int trackNumber = m_data.at(index.row())->getSong()->getTrackNumber();

                if (trackNumber <= 0)
                {
                    return QString();
                }

                const int trackTotal = m_data.at(index.row())->getSong()->getTrackTotal();

                if (trackTotal >= trackNumber)
                {
                    return QString("%1/%2").arg(trackNumber).arg(trackTotal);
                }
                else
                {
                    return trackNumber;
                }
            }

            // Numéro de disque
            case CSongTable::ColDiscNumber:
            {
                const int discNumber = m_data.at(index.row())->getSong()->getDiscNumber();

                if (discNumber <= 0)
                {
                    return QString();
                }

                const int discTotal = m_data.at(index.row())->getSong()->getDiscTotal();

                if (discTotal >= discNumber)
                {
                    return QString("%1/%2").arg(discNumber).arg(discTotal);
                }
                else
                {
                    return discNumber;
                }
            }

            case CSongTable::ColGenre       : return m_data.at(index.row())->getSong()->getGenre();
            case CSongTable::ColRating      : return m_data.at(index.row())->getSong()->getRating();
            case CSongTable::ColComments    : return m_data.at(index.row())->getSong()->getComments();
            case CSongTable::ColPlayCount   : return m_data.at(index.row())->getSong()->getNumPlays();
            case CSongTable::ColLastPlayTime: return m_data.at(index.row())->getSong()->getLastPlay();
            case CSongTable::ColFileName    : return m_data.at(index.row())->getSong()->getFileName();

            // Débit
            case CSongTable::ColBitRate:
                return QString("%1 kbit/s").arg(m_data.at(index.row())->getSong()->getBitRate());

            // Format
            case CSongTable::ColFormat:
                return CSong::getFormatName(m_data.at(index.row())->getSong()->getFormat());

            // Durée
            case CSongTable::ColDuration:
            {
                int duration = m_data.at(index.row())->getSong()->getDuration();

                QTime durationTime(0, 0);
                durationTime = durationTime.addMSecs(duration);
                return durationTime.toString("m:ss"); /// \todo Stocker le format dans les paramètres.
            }

            // Taux d'échantillonnage
            case CSongTable::ColSampleRate:
                return QString("%1 Hz").arg(m_data.at(index.row())->getSong()->getSampleRate());

            // Date de création
            case CSongTable::ColCreationDate:
                return m_data.at(index.row())->getSong()->getCreationDate();

            // Date de modification
            case CSongTable::ColModificationDate:
                return m_data.at(index.row())->getSong()->getModificationDate();

            // Nombre de canaux
            case CSongTable::ColChannels:
                return m_data.at(index.row())->getSong()->getNumChannels();

            // Taille du fichier
            case CSongTable::ColFileSize:
                return CSong::getFileSize(m_data.at(index.row())->getSong()->getFileSize());
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        switch (index.column())
        {
            default:
                return Qt::AlignLeft;

            case CSongTable::ColTrackNumber:
            case CSongTable::ColDiscNumber:
            case CSongTable::ColPlayCount:
            case CSongTable::ColBitRate:
            case CSongTable::ColDuration:
            case CSongTable::ColSampleRate:
            case CSongTable::ColFileSize:
                return Qt::AlignRight;
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
        CSongTable::TColumnType columnType = CSongTable::getColumnTypeFromInteger(section);

        if (columnType != CSongTable::ColInvalid)
        {
            return CSongTable::getColumnTypeName(columnType);
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
            case CSongTable::ColPosition        : qSort(m_data.begin(), m_data.end(), cmpSongPositionAsc        ); break;
            case CSongTable::ColTitle           : qSort(m_data.begin(), m_data.end(), cmpSongTitleAsc           ); break;
            case CSongTable::ColArtist          : qSort(m_data.begin(), m_data.end(), cmpSongArtistAsc          ); break;
            case CSongTable::ColAlbum           : qSort(m_data.begin(), m_data.end(), cmpSongAlbumAsc           ); break;
            case CSongTable::ColAlbumArtist     : qSort(m_data.begin(), m_data.end(), cmpSongAlbumArtistAsc     ); break;
            case CSongTable::ColComposer        : qSort(m_data.begin(), m_data.end(), cmpSongComposerAsc        ); break;
            case CSongTable::ColYear            : qSort(m_data.begin(), m_data.end(), cmpSongYearAsc            ); break;
            case CSongTable::ColTrackNumber     : qSort(m_data.begin(), m_data.end(), cmpSongTrackAsc           ); break;
            case CSongTable::ColDiscNumber      : qSort(m_data.begin(), m_data.end(), cmpSongDiscAsc            ); break;
            case CSongTable::ColGenre           : qSort(m_data.begin(), m_data.end(), cmpSongGenreAsc           ); break;
            case CSongTable::ColRating          : qSort(m_data.begin(), m_data.end(), cmpSongRatingAsc          ); break;
            case CSongTable::ColComments        : qSort(m_data.begin(), m_data.end(), cmpSongCommentsAsc        ); break;
            case CSongTable::ColPlayCount       : qSort(m_data.begin(), m_data.end(), cmpSongPlayCountAsc       ); break;
            case CSongTable::ColLastPlayTime    : qSort(m_data.begin(), m_data.end(), cmpSongLastPlayedAsc      ); break;
            case CSongTable::ColFileName        : qSort(m_data.begin(), m_data.end(), cmpSongFileNameAsc        ); break;
            case CSongTable::ColBitRate         : qSort(m_data.begin(), m_data.end(), cmpSongBitRateAsc         ); break;
            case CSongTable::ColFormat          : qSort(m_data.begin(), m_data.end(), cmpSongFormatAsc          ); break;
            case CSongTable::ColDuration        : qSort(m_data.begin(), m_data.end(), cmpSongDurationAsc        ); break;
            case CSongTable::ColSampleRate      : qSort(m_data.begin(), m_data.end(), cmpSongSampleRateAsc      ); break;
            case CSongTable::ColCreationDate    : qSort(m_data.begin(), m_data.end(), cmpSongCreationDateAsc    ); break;
            case CSongTable::ColModificationDate: qSort(m_data.begin(), m_data.end(), cmpSongModificationDateAsc); break;
            case CSongTable::ColChannels        : qSort(m_data.begin(), m_data.end(), cmpSongChannelsAsc        ); break;
            case CSongTable::ColFileSize        : qSort(m_data.begin(), m_data.end(), cmpSongFileSizeAsc        ); break;
        }
    }
    else
    {
        switch(column)
        {
            case CSongTable::ColPosition        : qSort(m_data.begin(), m_data.end(), cmpSongPositionDesc        ); break;
            case CSongTable::ColTitle           : qSort(m_data.begin(), m_data.end(), cmpSongTitleDesc           ); break;
            case CSongTable::ColArtist          : qSort(m_data.begin(), m_data.end(), cmpSongArtistDesc          ); break;
            case CSongTable::ColAlbum           : qSort(m_data.begin(), m_data.end(), cmpSongAlbumDesc           ); break;
            case CSongTable::ColAlbumArtist     : qSort(m_data.begin(), m_data.end(), cmpSongAlbumArtistDesc     ); break;
            case CSongTable::ColComposer        : qSort(m_data.begin(), m_data.end(), cmpSongComposerDesc        ); break;
            case CSongTable::ColYear            : qSort(m_data.begin(), m_data.end(), cmpSongYearDesc            ); break;
            case CSongTable::ColTrackNumber     : qSort(m_data.begin(), m_data.end(), cmpSongTrackDesc           ); break;
            case CSongTable::ColDiscNumber      : qSort(m_data.begin(), m_data.end(), cmpSongDiscDesc            ); break;
            case CSongTable::ColGenre           : qSort(m_data.begin(), m_data.end(), cmpSongGenreDesc           ); break;
            case CSongTable::ColRating          : qSort(m_data.begin(), m_data.end(), cmpSongRatingDesc          ); break;
            case CSongTable::ColComments        : qSort(m_data.begin(), m_data.end(), cmpSongCommentsDesc        ); break;
            case CSongTable::ColPlayCount       : qSort(m_data.begin(), m_data.end(), cmpSongPlayCountDesc       ); break;
            case CSongTable::ColLastPlayTime    : qSort(m_data.begin(), m_data.end(), cmpSongLastPlayedDesc      ); break;
            case CSongTable::ColFileName        : qSort(m_data.begin(), m_data.end(), cmpSongFileNameDesc        ); break;
            case CSongTable::ColBitRate         : qSort(m_data.begin(), m_data.end(), cmpSongBitRateDesc         ); break;
            case CSongTable::ColFormat          : qSort(m_data.begin(), m_data.end(), cmpSongFormatDesc          ); break;
            case CSongTable::ColDuration        : qSort(m_data.begin(), m_data.end(), cmpSongDurationDesc        ); break;
            case CSongTable::ColSampleRate      : qSort(m_data.begin(), m_data.end(), cmpSongSampleRateDesc      ); break;
            case CSongTable::ColCreationDate    : qSort(m_data.begin(), m_data.end(), cmpSongCreationDateDesc    ); break;
            case CSongTable::ColModificationDate: qSort(m_data.begin(), m_data.end(), cmpSongModificationDateDesc); break;
            case CSongTable::ColChannels        : qSort(m_data.begin(), m_data.end(), cmpSongChannelsDesc        ); break;
            case CSongTable::ColFileSize        : qSort(m_data.begin(), m_data.end(), cmpSongFileSizeDesc        ); break;
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


/**
 * Cherche le morceau situé avant un autre dans la liste.
 *
 * \todo Implémenter la lecture aléatoire.
 *
 * \param songItem Item actuel.
 * \param shuffle  Indique si la lecture aléatoire est activée.
 * \return Item précédent.
 */

CSongTableItem * CSongTableModel::getPreviousSong(CSongTableItem * songItem, bool shuffle) const
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
            return (row <= 0 ? NULL : m_data.at(row - 1));
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
}


/**
 * Cherche le morceau situé après un autre dans la liste.
 *
 * \todo Implémenter la lecture aléatoire.
 *
 * \param songItem Item actuel.
 * \param shuffle  Indique si la lecture aléatoire est activée.
 * \return Item suivant.
 */

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

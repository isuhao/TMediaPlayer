
#include "CSongTableModel.hpp"
#include <QMouseEvent>

#include <QtDebug>


CSongTableModel::CSongTableModel(const QList<CSong *>& data, QObject * parent) :
    QAbstractTableModel (parent),
    m_canDrop           (false)
{
    int i = 0;

    foreach (CSong * song, data)
    {
        m_data.append(new TSongItem(i++, song));
    }
}


void CSongTableModel::setCanDrop(bool canDrop)
{
    m_canDrop = canDrop;
}


void CSongTableModel::setData(const QList<CSong *>& data)
{
    m_data.clear();
    int i = 0;

    foreach (CSong * song, data)
    {
        m_data.append(new TSongItem(i++, song));
    }
}


QList<CSong *> CSongTableModel::getData() const
{
    QList<CSong *> songList;
    songList.reserve(m_data.size());

    foreach (TSongItem * item, m_data)
    {
        songList.append(item->song);
    }

    return songList;
}


int CSongTableModel::rowCount(const QModelIndex& parent) const
{
    return m_data.size();
}


int CSongTableModel::columnCount(const QModelIndex& parent) const
{
    return 18;
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
            case 0: return m_data.at(index.row())->pos;
            case 1: return m_data.at(index.row())->song->getTitle();
            case 2: return m_data.at(index.row())->song->getArtistName();
            case 3: return m_data.at(index.row())->song->getAlbumTitle();
            case 4: return m_data.at(index.row())->song->getAlbumArtist();
            case 5: return m_data.at(index.row())->song->getComposer();

            // Année
            case 6:
            {
                int year = m_data.at(index.row())->song->getYear();
                return (year > 0 ? year : QVariant(QString()));
            }

            // Numéro de piste
            case 7:
            {
                const int trackNumber = m_data.at(index.row())->song->getTrackNumber();

                if (trackNumber <= 0)
                {
                    return QString();
                }

                const int trackTotal = m_data.at(index.row())->song->getTrackTotal();

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
                const int discNumber = m_data.at(index.row())->song->getDiscNumber();

                if (discNumber <= 0)
                {
                    return QString();
                }

                const int discTotal = m_data.at(index.row())->song->getDiscTotal();

                if (discTotal >= discNumber)
                {
                    return QString("%1/%1").arg(discNumber).arg(discTotal);
                }
                else
                {
                    return discNumber;
                }
            }

            case  9: return m_data.at(index.row())->song->getGenre();
            case 10: return m_data.at(index.row())->song->getRating();
            case 11: return m_data.at(index.row())->song->getComments();
            case 12: return m_data.at(index.row())->song->getNumPlays();
            case 13: return m_data.at(index.row())->song->getLastPlay();
            case 14: return m_data.at(index.row())->song->getFileName();
            case 15: return m_data.at(index.row())->song->getBitRate();

            // Format
            case 16:
            {
                switch (m_data.at(index.row())->song->getFileType())
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
                int duration = m_data.at(index.row())->song->getDuration();

                QTime durationTime(0, 0);
                durationTime = durationTime.addMSecs(duration);
                return durationTime.toString("m:ss"); /// \todo Stocker le format dans les paramètres.
            }
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
        }
    }

    return QVariant::Invalid;
}


void CSongTableModel::sort(int column, Qt::SortOrder order)
{
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
        }
    }

    emit layoutChanged();
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


bool CSongTableModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    if (!data->hasFormat("application/ted.media.song"))
    {
        return false;
    }
}


void CSongTableModel::insertRow(CSong * song, int pos)
{
    Q_CHECK_PTR(song);

    if (pos < 0)
    {
        m_data.append(new TSongItem(pos, song));
    }
    else
    {
        m_data.insert(pos, new TSongItem(pos, song));
    }

    emit layoutChanged();
}


void CSongTableModel::removeRow(int pos)
{
    Q_ASSERT(pos >= 0 && pos < m_data.size());

    delete m_data.takeAt(pos);
}


void CSongTableModel::clear(void)
{
    foreach (TSongItem * songItem, m_data)
    {
        delete songItem;
    }

    m_data.clear();
    emit layoutChanged();
}


CSongTableModel::TSongItem * CSongTableModel::getSongItem(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return getSongItem(index.row());
    }

    return NULL;
}


CSongTableModel::TSongItem * CSongTableModel::getSongItem(int row) const
{
    return m_data.at(row);
}

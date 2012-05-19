
#include "CSongTableModel.hpp"
#include "CSong.hpp"
#include <QMouseEvent>
#include <QtDebug>


CSongTableModel::CSongTableModel(const QList<CSong *>& data, QObject * parent) :
    QAbstractTableModel (parent),
    m_data              (data)
{

}


void CSongTableModel::setData(const QList<CSong *>& data)
{
    m_data = data;
}


int CSongTableModel::rowCount(const QModelIndex& parent) const
{
    return m_data.size();
}


int CSongTableModel::columnCount(const QModelIndex& parent) const
{
    return 17;
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
            case 0:
            {
                QString title = m_data.at(index.row())->getTitleSort();
                return (title.isEmpty() ? m_data.at(index.row())->getTitle() : title);
            }

            case 1:
            {
                QString name = m_data.at(index.row())->getArtistNameSort();
                return (name.isEmpty() ? m_data.at(index.row())->getArtistName() : name);
            }

            case 2:
            {
                QString title = m_data.at(index.row())->getAlbumTitleSort();
                return (title.isEmpty() ? m_data.at(index.row())->getAlbumTitle() : title);
            }

            case 3:
            {
                QString name = m_data.at(index.row())->getAlbumArtistSort();
                return (name.isEmpty() ? m_data.at(index.row())->getAlbumArtist() : name);
            }

            // Compositeur
            case 4:
            {
                QString composer = m_data.at(index.row())->getComposerSort();
                return (composer.isEmpty() ? m_data.at(index.row())->getComposer() : composer);
            }

            // Année
            case 5:
            {
                int year = m_data.at(index.row())->getYear();
                return (year > 0 ? m_data.at(index.row())->getYear() : QVariant(QString()));
            }

            // Numéro de piste
            case 6:
            {
                const int trackNumber = m_data.at(index.row())->getTrackNumber();

                if (trackNumber <= 0)
                {
                    return QString();
                }

                const int trackTotal = m_data.at(index.row())->getTrackTotal();

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
            case 7:
            {
                const int discNumber = m_data.at(index.row())->getDiscNumber();

                if (discNumber <= 0)
                {
                    return QString();
                }

                const int discTotal = m_data.at(index.row())->getDiscTotal();

                if (discTotal >= discNumber)
                {
                    return QString("%1/%1").arg(discNumber).arg(discTotal);
                }
                else
                {
                    return discNumber;
                }
            }

            case 8: return m_data.at(index.row())->getGenre();
            case 9: return m_data.at(index.row())->getRating();
            case 10: return m_data.at(index.row())->getComments();
            case 11: return m_data.at(index.row())->getNumPlays();
            case 12: return m_data.at(index.row())->getLastPlay();
            case 13: return m_data.at(index.row())->getFileName();
            case 14: return m_data.at(index.row())->getBitRate();

            // Format
            case 15:
            {
                switch (m_data.at(index.row())->getFileType())
                {
                    default:
                    case CSong::TypeUnknown: return tr("Unknown");
                    case CSong::TypeMP3:     return tr("MP3");
                    case CSong::TypeOGG:     return tr("OGG Vorbis");
                    case CSong::TypeFlac:    return tr("FLAC");
                }
            }

            // Durée
            case 16:
            {
                int duration = m_data.at(index.row())->getDuration();

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
            case  0: return QString(tr("Title"));
            case  1: return QString(tr("Artist"));
            case  2: return QString(tr("Album"));
            case  3: return QString(tr("Album artist"));
            case  4: return QString(tr("Composer"));
            case  5: return QString(tr("Year"));
            case  6: return QString(tr("Track"));
            case  7: return QString(tr("Disc"));
            case  8: return QString(tr("Genre"));
            case  9: return QString(tr("Rating"));
            case 10: return QString(tr("Comments"));
            case 11: return QString(tr("Plays"));
            case 12: return QString(tr("Last played"));
            case 13: return QString(tr("File name"));
            case 14: return QString(tr("Bit rate"));
            case 15: return QString(tr("Format"));
            case 16: return QString(tr("Duration"));
        }
    }

    return QVariant::Invalid;
}

/*
void CSongTableModel::sort(int column, Qt::SortOrder order)
{
    if(order == Qt::AscendingOrder)
    {
        switch(column)
        {
            case 0: qSort(m_data.begin(), m_data.end(), cmpMessageLevelAsc   ); break;
            case 1: qSort(m_data.begin(), m_data.end(), cmpMessageDateTimeAsc); break;
            case 2: qSort(m_data.begin(), m_data.end(), cmpMessageTextAsc    ); break;
        }
    }
    else
    {
        switch(column)
        {
            case 0: qSort(m_data.begin(), m_data.end(), cmpMessageLevelDesc   ); break;
            case 1: qSort(m_data.begin(), m_data.end(), cmpMessageDateTimeDesc); break;
            case 2: qSort(m_data.begin(), m_data.end(), cmpMessageTextDesc    ); break;
        }
    }

    layoutChanged();
}
*/


void CSongTableModel::insertRow(CSong * song, int pos)
{
    Q_CHECK_PTR(song);

    if (pos < 0)
    {
        m_data.append(song);
    }
    else
    {
        m_data.insert(pos, song);
    }

    emit layoutChanged();
}


void CSongTableModel::removeRow(int pos)
{
    Q_ASSERT(pos >= 0 && pos < m_data.size());

    m_data.removeAt(pos);
}


void CSongTableModel::clear(void)
{
    m_data.clear();
}


CSong * CSongTableModel::getSong(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return m_data.at(index.row());
    }

    return NULL;
}

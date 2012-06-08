
#include "CSongTableModel.hpp"
#include "CSongTable.hpp"
#include "CApplication.hpp"
#include "CRating.hpp"
#include <QMouseEvent>
#include <QUrl>

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


CSongTableModel::CSongTableModel(CApplication * application, const QList<CSong *>& data, QWidget * parent) :
    QAbstractTableModel (parent),
    m_application       (application),
    m_canDrop           (false),
    m_columnSort        (-1),
    m_currentSongItem   (NULL)
{
    Q_CHECK_PTR(application);

    int i = 0;

    foreach (CSong * song, data)
    {
        m_data.append(new CSongTableItem(++i, song));
    }
}


CSongTableModel::CSongTableModel(CApplication * application, QWidget * parent) :
    QAbstractTableModel (parent),
    m_application       (application),
    m_canDrop           (false),
    m_columnSort        (-1),
    m_currentSongItem   (NULL)
{
    Q_CHECK_PTR(application);
}


/**
 * Active ou désactive le glisser-déposer vers la liste.
 * Doit être actif pour les listes statiques uniquement.
 *
 * \param canDrop Indique si on peut déposer ou déplacer des morceaux vers la liste.
 */

void CSongTableModel::setCanDrop(bool canDrop)
{
    m_canDrop = canDrop;
}


/**
 * Modifie la liste des morceaux utilisée par le modèle.
 *
 * \param data Liste de morceaux.
 */

void CSongTableModel::setSongs(const QList<CSong *>& data)
{
    emit layoutAboutToBeChanged();

    m_data.clear();
    m_dataShuffle.clear();
    int i = 0;

    for (QList<CSong *>::const_iterator it = data.begin(); it != data.end(); ++it)
    {
        m_data.append(new CSongTableItem(++i, *it));
    }

    initShuffle();

    emit layoutChanged();
}


/**
 * Retourne la liste des morceaux de la liste, triée selon leur position croissante.
 *
 * \return Liste des morceaux.
 */

QList<CSong *> CSongTableModel::getSongs(void) const
{
    QList<CSongTableItem *> dataCopy = m_data;
    qSort(dataCopy.begin(), dataCopy.end(), cmpSongPositionAsc);

    QList<CSong *> songList;
    songList.reserve(m_data.size());

    for (QList<CSongTableItem *>::const_iterator it = dataCopy.begin(); it != dataCopy.end(); ++it)
    {
        songList.append((*it)->getSong());
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


/**
 * Retourne les données d'un item.
 *
 * \param index Index de l'item.
 * \param role  Rôle demandé.
 * \return Données de l'item.
 */

QVariant CSongTableModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= m_data.size())
    {
        return QVariant::Invalid;
    }

    if (role == Qt::FontRole)
    {
        return QFont("Segoe UI", 8);
    }

    if (role == Qt::DecorationRole)
    {
        if (index.column() == CSongTable::ColPosition)
        {
            if (m_data.at(index.row()) == m_currentSongItem)
            {
                if (m_application->isPlaying())
                {
                    return QPixmap(":/icons/song_playing");
                }
                else if (m_application->isPaused())
                {
                    return QPixmap(":/icons/song_paused");
                }
            }
        }
    }
    else if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case CSongTable::ColPosition:
            {
                if (m_canDrop)
                {
                    return m_data.at(index.row())->getPosition();
                    //return index.row();
                }

                break;
            }

          //case CSongTable::ColShufflePos : return m_dataShuffle.indexOf(m_data.at(index.row()));

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

                const int trackCount = m_data.at(index.row())->getSong()->getTrackCount();

                if (trackCount >= trackNumber)
                {
                    return QString("%1/%2").arg(trackNumber).arg(trackCount);
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

                const int discCount = m_data.at(index.row())->getSong()->getDiscCount();

                if (discCount >= discNumber)
                {
                    return QString("%1/%2").arg(discNumber).arg(discCount);
                }
                else
                {
                    return discNumber;
                }
            }

            case CSongTable::ColGenre       : return m_data.at(index.row())->getSong()->getGenre();
            case CSongTable::ColRating      : return QVariant::fromValue<CRating>(m_data.at(index.row())->getSong()->getRating());
            case CSongTable::ColComments    : return m_data.at(index.row())->getSong()->getComments();
            case CSongTable::ColPlayCount   : return m_data.at(index.row())->getSong()->getNumPlays();
            case CSongTable::ColLastPlayTime: return m_data.at(index.row())->getSong()->getLastPlay();
            case CSongTable::ColFileName    : return m_data.at(index.row())->getSong()->getFileName();

            // Débit
            case CSongTable::ColBitRate:
                return tr("%1 kbit/s").arg(m_data.at(index.row())->getSong()->getBitRate());

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

            // Fréquence d'échantillonnage
            case CSongTable::ColSampleRate:
                return tr("%1 Hz").arg(m_data.at(index.row())->getSong()->getSampleRate());

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

            // Paroles
            case CSongTable::ColLyrics:
                return m_data.at(index.row())->getSong()->getLyrics();

            // Langue
            case CSongTable::ColLanguage:
                return CSong::getLanguageName(m_data.at(index.row())->getSong()->getLanguage());

            // Parolier
            case CSongTable::ColLyricist:
                return m_data.at(index.row())->getSong()->getLyricist();

            // Regroupement
            case CSongTable::ColGrouping:
                return m_data.at(index.row())->getSong()->getGrouping();

            // Sous-titre
            case CSongTable::ColSubTitle:
                return m_data.at(index.row())->getSong()->getSubTitle();
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        switch (index.column())
        {
            // Alignement à gauche
            default:
                return QVariant(Qt::AlignVCenter | Qt::AlignLeft);

            // Alignement à droite
            case CSongTable::ColPosition:
            case CSongTable::ColTrackNumber:
            case CSongTable::ColDiscNumber:
            case CSongTable::ColPlayCount:
            case CSongTable::ColBitRate:
            case CSongTable::ColDuration:
            case CSongTable::ColSampleRate:
            case CSongTable::ColFileSize:
                return QVariant(Qt::AlignVCenter | Qt::AlignRight);
        }
    }
    else if (role == Qt::CheckStateRole)
    {
        if (index.column() == CSongTable::ColTitle)
        {
            return (m_data.at(index.row())->getSong()->isEnabled() ? Qt::Checked : Qt::Unchecked);
        }
    }

    return QVariant::Invalid;
}


/**
 * Modifie les données d'un item.
 * Cette méthode n'est utilisée que pour la case à cocher d'un morceau.
 *
 * \todo Implémentation.
 *
 * \param index Index de l'item.
 * \param value Nouvelle valeur de la donnée.
 * \param role  Rôle de la donnée.
 * \return Booléen indiquant si les données ont été modifiées.
 */

bool CSongTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == CSongTable::ColTitle)
    {
        m_data.at(index.row())->getSong()->setEnabled(value.toInt() == Qt::Checked);
        emit dataChanged(index, index);
        return true;
    }
    else if (role == Qt::EditRole && index.column() == CSongTable::ColRating)
    {
        m_data.at(index.row())->getSong()->setRating(value.value<CRating>().getRating());
        emit dataChanged(index, index);
        return true;
    }

    return QAbstractTableModel::setData(index, value, role);
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

    m_columnSort = column;

    if (order == Qt::AscendingOrder)
    {
        switch (column)
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
            case CSongTable::ColLyrics          : qSort(m_data.begin(), m_data.end(), cmpSongLyricsAsc          ); break;
            case CSongTable::ColLanguage        : qSort(m_data.begin(), m_data.end(), cmpSongLanguageAsc        ); break;
            case CSongTable::ColLyricist        : qSort(m_data.begin(), m_data.end(), cmpSongLyricistAsc        ); break;
            case CSongTable::ColGrouping        : qSort(m_data.begin(), m_data.end(), cmpSongGroupingAsc        ); break;
            case CSongTable::ColSubTitle        : qSort(m_data.begin(), m_data.end(), cmpSongSubTitleAsc        ); break;
        }
    }
    else
    {
        switch (column)
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
            case CSongTable::ColLyrics          : qSort(m_data.begin(), m_data.end(), cmpSongLyricsDesc          ); break;
            case CSongTable::ColLanguage        : qSort(m_data.begin(), m_data.end(), cmpSongLanguageDesc        ); break;
            case CSongTable::ColLyricist        : qSort(m_data.begin(), m_data.end(), cmpSongLyricistDesc        ); break;
            case CSongTable::ColGrouping        : qSort(m_data.begin(), m_data.end(), cmpSongGroupingDesc        ); break;
            case CSongTable::ColSubTitle        : qSort(m_data.begin(), m_data.end(), cmpSongSubTitleDesc        ); break;
        }
    }

    emit layoutChanged();
    emit columnSorted(column, order);
}


/**
 * Retourne les flags d'un item.
 *
 * \param index Index de l'item.
 * \return Flags de l'item (ItemIsEnabled, ItemIsSelectable, ItemIsDragEnabled, et
 *         éventuellement ItemIsDropEnabled et ItemIsUserCheckable).
 */

Qt::ItemFlags CSongTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index) | Qt::ItemIsDragEnabled;

    if (index.isValid())
    {
        if (index.column() == CSongTable::ColTitle)
        {
            flags |= Qt::ItemIsUserCheckable;
        }
        else if (index.column() == CSongTable::ColRating)
        {
            flags |= Qt::ItemIsEditable;
        }
    }

    if (m_canDrop)
    {
        flags |= Qt::ItemIsDropEnabled;
    }

    return flags;
}


/**
 * Retourne la liste des types MIME supportés par le modèle.
 *
 * \return Liste types.
 */

QStringList CSongTableModel::mimeTypes(void) const
{
    QStringList types;

    if (m_canDrop)
    {
        types << "application/x-ted-media-items";
    }

    //types << "application/x-ted-media-songs";
    //types << "text/uri-list";

    return types;
}


QMimeData * CSongTableModel::mimeData(const QModelIndexList& indexes) const
{
    // Liste des lignes et des fichiers
    QStringList fileList;
    QList<int> rows;

    foreach (QModelIndex index, indexes)
    {
        if (index.isValid() && !rows.contains(index.row()))
        {
            rows.append(index.row());

            const QString fileName = m_data.at(index.row())->getSong()->getFileName();

            if (!fileList.contains(fileName))
            {
                fileList.append(fileName);
            }
        }
    }

    qSort(rows);

    QByteArray songListData;
    QDataStream streamSongs(&songListData, QIODevice::WriteOnly);
    streamSongs << rows.size();

    QByteArray rowListData;
    QDataStream streamRows(&rowListData, QIODevice::WriteOnly);
    streamRows << rows.size();

    for (QList<int>::const_iterator it = rows.begin(); it != rows.end(); ++it)
    {
        streamSongs << m_data[*it]->getSong()->getId();
        streamRows << *it;
    }

    QString fileListString;

    for (QStringList::const_iterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        fileListString.append(QUrl::fromLocalFile(*it).toString());
        fileListString.append("\r\n");
    }

    QByteArray fileListData(fileListString.toUtf8());

    QMimeData * mimeData = new QMimeData();
    mimeData->setData("application/x-ted-media-songs", songListData);
    mimeData->setData("text/uri-list", fileListData);

    if (m_canDrop && m_columnSort == CSongTable::ColPosition)
    {
        mimeData->setData("application/x-ted-media-items", rowListData);
    }

    return mimeData;
}


/**
 * Méthode appelée lorsqu'on déposer des données dans le modèle.
 *
 * \param data   Données déposées.
 * \param action Action de glisser-déposer.
 * \param row    Ligne où les données sont déposées.
 * \param column Colonne om les données sont déposées.
 * \param parent Index parent.
 */

bool CSongTableModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    Q_CHECK_PTR(data);

    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    // Fichiers à ajouter à la médiathèque
    if (data->hasFormat("text/uri-list"))
    {
        //...
    }

    // Déplacement des morceaux dans la liste
    if (!m_canDrop || !data->hasFormat("application/x-ted-media-items") || m_columnSort != CSongTable::ColPosition)
    {
        return false;
    }

    return true;
}


/**
 * Déplace un ensemble de lignes vers une autre position.
 * La liste \a rows doit être triée et ne doit pas contenir de doublons.
 *
 * \todo Implémentation.
 *
 * \param rows    Liste de lignes (les numéros de ligne doivent être compris entre 0 et rowCount() - 1).
 * \param rowDest Ligne de destination (entre 0 et rowCount()).
 */

void CSongTableModel::moveRows(const QList<int>& rows, int rowDest)
{
    qDebug() << "M" << rows << " -> " << rowDest;

    if (rows.isEmpty() || rowDest < 0 || rowDest > m_data.size())
    {
        return;
    }

    QList<CSongTableItem *> dataCopy = m_data;
    qSort(dataCopy.begin(), dataCopy.end(), cmpSongPositionAsc);

    QList<int>::const_iterator it2 = rows.begin() - 1;
    int numMoved = 0;

    for (QList<int>::const_iterator it = rows.begin(); it != rows.end(); ++it)
    {
        if (*it < rowDest)
        {
            it2 = it;
            continue;
        }

        dataCopy.move(*it, rowDest + numMoved);
        //qDebug() << "A" << *it << "->" << (rowDest + numMoved);
        ++numMoved;
    }

    numMoved = 0;

    for (; it2 >= rows.begin(); --it2)
    {
        dataCopy.move(*it2, rowDest - 1 - numMoved);
        //qDebug() << "B" << *it2 << "->" << (rowDest - 1 - numMoved);
        ++numMoved;
    }

    for (int pos = 0; pos < dataCopy.size(); ++pos)
    {
        dataCopy[pos]->m_position = pos + 1;
    }

    emit layoutAboutToBeChanged();
    m_data = dataCopy;
    emit layoutChanged();
}


void CSongTableModel::insertRow(CSong * song, int pos)
{
    Q_CHECK_PTR(song);

    emit layoutAboutToBeChanged();

    CSongTableItem * songItem = new CSongTableItem(pos, song);

    if (pos < 0 || pos >= m_data.size())
    {
        m_data.append(songItem);
    }
    else
    {
        m_data.insert(pos, songItem);
    }

    m_dataShuffle.append(songItem);

    emit layoutChanged();
}


void CSongTableModel::removeRow(int row)
{
    Q_ASSERT(row >= 0 && row < m_data.size());

    emit layoutAboutToBeChanged();
    CSongTableItem * songItem = m_data.takeAt(row);
    m_dataShuffle.removeOne(songItem);
    delete songItem;
    emit layoutChanged();
}


void CSongTableModel::removeSongs(const QList<CSong *>& songs)
{
    if (songs.isEmpty())
    {
        return;
    }

    emit layoutAboutToBeChanged();

    QList<CSongTableItem *> dataCopy = m_data;

    for (QList<CSongTableItem *>::const_iterator it = dataCopy.begin(); it != dataCopy.end(); ++it)
    {
        if (songs.contains((*it)->getSong()))
        {
            m_data.removeOne(*it);
            m_dataShuffle.removeOne(*it);
            delete *it;
        }
    }

    emit layoutChanged();
}


/**
 * Supprime toutes les données du modèle.
 */

void CSongTableModel::clear(void)
{
    emit layoutAboutToBeChanged();

    foreach (CSongTableItem * songItem, m_data)
    {
        delete songItem;
    }

    m_data.clear();
    m_dataShuffle.clear();
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
    qDebug() << "Morceau précédant" << songItem;

    if (songItem && !m_data.contains(songItem))
    {
        qWarning() << "CSongTableModel::getPreviousSong() : l'item demandé n'est pas dans la table";
        songItem = NULL;
    }

    if (songItem)
    {
        if (shuffle)
        {
            if (m_data.size() != m_dataShuffle.size())
            {
                qWarning() << "CSongTableModel::getPreviousSong() : la liste des morceaux aléatoires est incorrecte";
            }

            const int row = m_dataShuffle.indexOf(songItem);

            if (row < 0)
            {
                qWarning() << "CSongTableModel::getPreviousSong() : l'item demandé n'est pas dans la liste des morceaux aléatoires";
                return NULL;
            }

            return (row == 0 ? NULL : m_dataShuffle.at(row - 1));
        }
        else
        {
            const int row = getRowForSongItem(songItem);
            return (row <= 0 ? NULL : m_data.at(row - 1));
        }
    }
    else
    {
        return NULL;
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
    qDebug() << "Morceau suivant" << songItem;

    if (songItem && !m_data.contains(songItem))
    {
        qWarning() << "CSongTableModel::getNextSong() : l'item demandé n'est pas dans la table";
        songItem = NULL;
    }

    if (songItem)
    {
        if (shuffle)
        {
            if (m_data.size() != m_dataShuffle.size())
            {
                qWarning() << "CSongTableModel::getNextSong() : la liste des morceaux aléatoires est incorrecte";
            }

            const int row = m_dataShuffle.indexOf(songItem);

            if (row < 0)
            {
                qWarning() << "CSongTableModel::getNextSong() : l'item demandé n'est pas dans la liste des morceaux aléatoires";
                return NULL;
            }

            return (row == m_dataShuffle.size() - 1 ? NULL : m_dataShuffle.at(row + 1));
        }
        else
        {
            const int row = getRowForSongItem(songItem);
            return (row == m_data.size() - 1 ? NULL : m_data.at(row + 1));
        }
    }
    else
    {
        if (shuffle)
        {
            if (m_data.size() != m_dataShuffle.size())
            {
                qWarning() << "CSongTableModel::getNextSong() : la liste des morceaux aléatoires est incorrecte";
                return NULL;
            }

            return (m_dataShuffle.isEmpty() ? NULL : m_dataShuffle.at(0));
        }
        else
        {
            return (m_data.isEmpty() ? NULL : m_data.at(0));
        }
    }
}


void CSongTableModel::setCurrentSong(CSongTableItem * songItem)
{
    m_currentSongItem = songItem;
    emit layoutChanged();
}


/**
 * Initialise la liste des morceaux aléatoires.
 *
 * La liste des morceaux aléatoires contient chaque morceau de la liste de morceaux,
 * sans doublons, avec une position calculée aléatoirement.
 *
 * Cette méthode doit être appelée lorsqu'on lance la lecture d'un morceau dans une liste (en
 * appuyant sur le bouton "Play" ou en demandant la lecture d'un morceau particulier alors
 * qu'aucun morceau n'est en cours de lecture).
 *
 * Une fois cette initialisation effectuée, on peut naviguer dans la liste aléatoire avec
 * les boutons précédent et suivant, ou en lançant la lecture d'un morceau particulier.
 *
 * Le fonctionnement de la lecture aléatoire est identique à celui de iTunes.
 *
 * \param firstSong Morceau à placer au début de la liste.
 */

void CSongTableModel::initShuffle(CSongTableItem * firstSong)
{
    //qDebug() << "CSongTableModel::initShuffle()";

    m_dataShuffle = m_data;
    const int numSongs = rowCount();

    if (numSongs > 1)
    {
        for (int index = 0; index < numSongs; ++index)
        {
            int newIndex = static_cast<int>((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) * (numSongs - index - 1) + index);
            m_dataShuffle.swap(index, newIndex); // Permutation
        }

        if (firstSong)
        {
            m_dataShuffle.swap(m_dataShuffle.indexOf(firstSong), 0);
        }
    }
}

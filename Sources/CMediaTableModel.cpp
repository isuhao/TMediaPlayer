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

#include "CMediaTableModel.hpp"
#include "CMediaTableView.hpp"
#include "CMainWindow.hpp"
#include "CMediaManager.hpp"
#include "CRating.hpp"
//#include "CSongTitle.hpp"
#include "Utils.hpp"

#include <QMouseEvent>
#include <QUrl>
#include <QMimeData>

#include <QtDebug>


/**
 * Constructeur du modèle avec une liste de morceaux.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 * \param data       Liste de morceaux.
 * \param parent     Pointeur sur le widget parent.
 */

CMediaTableModel::CMediaTableModel(CMainWindow * mainWindow, const QList<CSong *>& data, QWidget * parent) :
QAbstractTableModel (parent),
m_mainWindow        (mainWindow),
m_canDrop           (false),
m_columnSort        (-1),
m_currentSongItem   (nullptr)
{
    Q_CHECK_PTR(m_mainWindow);

    int i = 0;

    // Remplissage de la liste des morceaux
    m_data.reserve(data.size());

    for (QList<CSong *>::ConstIterator it = data.begin(); it != data.end(); ++it)
    {
        m_data.append(new CMediaTableItem(++i, *it));
    }

    // TODO: récupérer le filtre courant
    m_dataFiltered = m_data;
}


/**
 * Constructeur du modèle sans liste de morceaux.
 *
 * \param mainWindow Pointeur sur la fenêtre principale de l'application.
 * \param parent     Pointeur sur le widget parent.
 */

CMediaTableModel::CMediaTableModel(CMainWindow * mainWindow, QWidget * parent) :
QAbstractTableModel (parent),
m_mainWindow        (mainWindow),
m_canDrop           (false),
m_columnSort        (-1),
m_currentSongItem   (nullptr)
{
    Q_CHECK_PTR(m_mainWindow);
}


/**
 * Active ou désactive le glisser-déposer vers la liste.
 * Doit être actif pour les listes statiques uniquement.
 *
 * \param canDrop Indique si on peut déposer ou déplacer des morceaux vers la liste.
 */

void CMediaTableModel::setCanDrop(bool canDrop)
{
    m_canDrop = canDrop;
}


/**
 * Modifie la liste des morceaux utilisée par le modèle.
 *
 * \param data Liste de morceaux.
 */

void CMediaTableModel::setSongs(const QList<CSong *>& data)
{
    emit layoutAboutToBeChanged();

    m_data.clear();
    m_dataShuffle.clear();

    m_dataFiltered.clear();

    int i = 0;

    // Remplissage de la liste des morceaux
    m_data.reserve(data.size());

    for (QList<CSong *>::ConstIterator it = data.begin(); it != data.end(); ++it)
    {
        m_data.append(new CMediaTableItem(++i, *it));
    }

    // Application du filtre de recherche
    applyFilter(m_mainWindow->getFilter());

    initShuffle();

    emit layoutChanged();
}


/**
 * Retourne la liste des morceaux de la liste, triée selon leur position croissante.
 *
 * \return Liste des morceaux.
 */

QList<CSong *> CMediaTableModel::getSongs() const
{
    QList<CMediaTableItem *> dataCopy = m_data;
    qSort(dataCopy.begin(), dataCopy.end(), cmpSongPositionAsc);

    QList<CSong *> songList;
    songList.reserve(m_data.size());

    for (QList<CMediaTableItem *>::ConstIterator it = dataCopy.begin(); it != dataCopy.end(); ++it)
    {
        songList.append((*it)->getSong());
    }

    return songList;
}


/**
 * Indique si un morceau est présent dans le modèle.
 *
 * \param song Pointeur sur le morceau à rechercher.
 * \return Booléen.
 */

bool CMediaTableModel::hasSong(CSong * song) const
{
    for (QList<CMediaTableItem *>::ConstIterator it = m_data.begin(); it != m_data.end(); ++it)
    {
        if ((*it)->getSong() == song)
        {
            return true;
        }
    }

    return false;
}


/**
 * Donne le nombre de lignes du modèle.
 *
 * \param parent Index parent (inutile).
 * \return Nombre de lignes.
 */

int CMediaTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_dataFiltered.size();
}


/**
 * Donne le nombre de colonnes du modèle.
 *
 * \param parent Index parent (inutile).
 * \return Nombre de colonnes.
 */

int CMediaTableModel::columnCount(const QModelIndex& parent) const
{
    if(parent.isValid())
    {
        return 0;
    }

    return CMediaTableView::ColNumber;
}


/**
 * Retourne les données d'un item.
 *
 * \param index Index de l'item.
 * \param role  Rôle demandé.
 * \return Données de l'item.
 */

QVariant CMediaTableModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= rowCount(index.parent()))
    {
        return QVariant::Invalid;
    }

    // Police de caractère
    if (role == Qt::FontRole)
    {
        return QFont("Segoe UI", 8);
    }

    QList<CMediaTableItem *> data = m_dataFiltered;
    CSong * song = data.at(index.row())->getSong();

    // Image
    if (role == Qt::DecorationRole)
    {
        if (index.column() == CMediaTableView::ColPosition)
        {
            if (data.at(index.row()) == m_currentSongItem)
            {
                if (m_mainWindow->isPlaying())
                {
                    return QPixmap(":/icons/song_playing");
                }
                else if (m_mainWindow->isPaused())
                {
                    return QPixmap(":/icons/song_paused");
                }
            }
        }
    }
    // Texte pour l'affichage
    else if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case CMediaTableView::ColPosition:
            {
                //if (m_canDrop)
                {
                    //return data.at(index.row())->getPosition();
                    return (index.row() + 1);
                }

                break;
            }

          //case CMediaTableView::ColShufflePos : return m_dataShuffleFiltered.indexOf(data.at(index.row()));

            case CMediaTableView::ColTitle      : return song->getTitle();
            //case CMediaTableView::ColTitle      : return QVariant::fromValue<CSongTitle>(CSongTitle(song->getTitle(), song->getSubTitle()));
            case CMediaTableView::ColArtist     : return song->getArtistName();
            case CMediaTableView::ColAlbum      : return song->getAlbumTitle();
            case CMediaTableView::ColAlbumArtist: return song->getAlbumArtist();
            case CMediaTableView::ColComposer   : return song->getComposer();

            // Année
            case CMediaTableView::ColYear:
            {
                int year = song->getYear();
                return (year > 0 ? year : QVariant(QString()));
            }

            // Numéro de piste
            case CMediaTableView::ColTrackNumber:
            {
                const int trackNumber = song->getTrackNumber();

                if (trackNumber <= 0)
                    return QString();

                const int trackCount = song->getTrackCount();

                if (trackCount >= trackNumber)
                    return QString("%1/%2").arg(trackNumber).arg(trackCount);
                else
                    return trackNumber;
            }

            // Numéro de disque
            case CMediaTableView::ColDiscNumber:
            {
                const int discNumber = song->getDiscNumber();

                if (discNumber <= 0)
                    return QString();

                const int discCount = song->getDiscCount();

                if (discCount >= discNumber)
                    return QString("%1/%2").arg(discNumber).arg(discCount);
                else
                    return discNumber;
            }

            case CMediaTableView::ColGenre       : return song->getGenre();
            case CMediaTableView::ColRating      : return QVariant::fromValue<CRating>(song->getRating());
            case CMediaTableView::ColComments    : return song->getComments();
            case CMediaTableView::ColPlayCount   : return song->getNumPlays();
            case CMediaTableView::ColLastPlayTime: return song->getLastPlay().toLocalTime();
            case CMediaTableView::ColPathName    : return song->getFileName();

            case CMediaTableView::ColFileName:
            {
                const QString fileName = song->getFileName();
                return fileName.mid(fileName.lastIndexOf('/') + 1);
            }

            // Débit
            case CMediaTableView::ColBitRate:
                return tr("%1 kbit/s").arg(song->getBitRate());

            // Format
            case CMediaTableView::ColFormat:
                return CSong::getFormatName(song->getFormat());

            // Durée
            case CMediaTableView::ColDuration:
            {
                int duration = song->getDuration();

                QTime durationTime(0, 0);
                durationTime = durationTime.addMSecs(duration);
                return durationTime.toString(tr("m:ss", "Duration format")); /// \todo Stocker le format dans les paramètres.
            }

            // Fréquence d'échantillonnage
            case CMediaTableView::ColSampleRate:
                return tr("%1 Hz").arg(song->getSampleRate());

            // Date de création
            case CMediaTableView::ColCreationDate:
                return song->getCreationDate();

            // Date de modification
            case CMediaTableView::ColModificationDate:
                return song->getModificationDate();

            // Nombre de canaux
            case CMediaTableView::ColChannels:
                return song->getNumChannels();

            // Taille du fichier
            case CMediaTableView::ColFileSize:
                return getFileSize(song->getFileSize());

            // Paroles
            case CMediaTableView::ColLyrics:
                return song->getLyrics();

            // Langue
            case CMediaTableView::ColLanguage:
                return getLanguageName(song->getLanguage());

            // Parolier
            case CMediaTableView::ColLyricist:
                return song->getLyricist();

            // Regroupement
            case CMediaTableView::ColGrouping:
                return song->getGrouping();

            // Sous-titre
            case CMediaTableView::ColSubTitle:
                return song->getSubTitle();

            // BPM
            case CMediaTableView::ColBPM:
                return song->getBPM();

            // Replay Gain
            case CMediaTableView::ColTrackGain:
            {
                float gain = song->getTrackGain();

                if (gain == std::numeric_limits<float>::infinity())
                    return QVariant::Invalid;
                else
                    return tr("%1 dB").arg(gain);
            }

            case CMediaTableView::ColTrackPeak:
            {
                float peak = song->getTrackPeak();

                if (peak == std::numeric_limits<float>::infinity())
                    return QVariant::Invalid;
                else
                    return peak;
            }

            case CMediaTableView::ColAlbumGain:
            {
                float gain = song->getAlbumGain();

                if (gain == std::numeric_limits<float>::infinity())
                    return QVariant::Invalid;
                else
                    return tr("%1 dB").arg(gain);
            }

            case CMediaTableView::ColAlbumPeak:
            {
                float peak = song->getAlbumPeak();

                if (peak == std::numeric_limits<float>::infinity())
                    return QVariant::Invalid;
                else
                    return peak;
            }

            case CMediaTableView::ColTitleSort      : return song->getTitleSort();
            case CMediaTableView::ColArtistSort     : return song->getArtistNameSort();
            case CMediaTableView::ColAlbumSort      : return song->getAlbumTitleSort();
            case CMediaTableView::ColAlbumArtistSort: return song->getAlbumArtistSort();
            case CMediaTableView::ColComposerSort   : return song->getComposerSort();
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
            case CMediaTableView::ColPosition:
            case CMediaTableView::ColTrackNumber:
            case CMediaTableView::ColDiscNumber:
            case CMediaTableView::ColPlayCount:
            case CMediaTableView::ColBitRate:
            case CMediaTableView::ColDuration:
            case CMediaTableView::ColSampleRate:
            case CMediaTableView::ColFileSize:
            case CMediaTableView::ColBPM:
                return QVariant(Qt::AlignVCenter | Qt::AlignRight);
        }
    }
    // Case à cocher
    else if (role == Qt::CheckStateRole)
    {
        if (index.column() == CMediaTableView::ColTitle)
        {
            return (song->isEnabled() ? Qt::Checked : Qt::Unchecked);
        }
    }
    // Couleur du texte
    else if (role == Qt::ForegroundRole)
    {
        if (song->getFileStatus() == false)
        {
            return QBrush(Qt::darkGray);
        }
    }

    return QVariant::Invalid;
}


/**
 * Modifie les données d'un item.
 * Cette méthode n'est utilisée que pour la case à cocher et la note d'un morceau.
 *
 * \param index Index de l'item.
 * \param value Nouvelle valeur de la donnée.
 * \param role  Rôle de la donnée.
 * \return Booléen indiquant si les données ont été modifiées.
 */

bool CMediaTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    QList<CMediaTableItem *> data = m_dataFiltered;

    if (role == Qt::CheckStateRole && index.column() == CMediaTableView::ColTitle)
    {
        data.at(index.row())->getSong()->setEnabled(value.toInt() == Qt::Checked);
        emit dataChanged(index, index);
        return true;
    }
    else if (role == Qt::EditRole && index.column() == CMediaTableView::ColRating)
    {
        data.at(index.row())->getSong()->setRating(value.value<CRating>().getRating());
        emit dataChanged(index, index);
        return true;
    }

    return QAbstractTableModel::setData(index, value, role);
}


QVariant CMediaTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant::Invalid;
    }

    if (orientation == Qt::Horizontal)
    {
        CMediaTableView::TColumnType columnType = CMediaTableView::getColumnTypeFromInteger(section);

        if (columnType != CMediaTableView::ColInvalid)
        {
            return CMediaTableView::getColumnTypeName(columnType);
        }
    }

    return QVariant::Invalid;
}


/**
 * Trie les morceaux de la liste.
 *
 * \param column Colonne à utiliser comme critère de tri.
 * \param order  Ordre de tri (ascendant ou descendant).
 */

void CMediaTableModel::sort(int column, Qt::SortOrder order)
{
    emit columnAboutToBeSorted(column, order);
    emit layoutAboutToBeChanged();

    m_columnSort = column;

    if (order == Qt::AscendingOrder)
    {
        switch (column)
        {
            case CMediaTableView::ColPosition        : std::sort(m_data.begin(), m_data.end(), cmpSongPositionAsc        ); break;
            case CMediaTableView::ColTitle           : std::sort(m_data.begin(), m_data.end(), cmpSongTitleAsc           ); break;
            case CMediaTableView::ColArtist          : std::sort(m_data.begin(), m_data.end(), cmpSongArtistAsc          ); break;
            case CMediaTableView::ColAlbum           : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumAsc           ); break;
            case CMediaTableView::ColAlbumArtist     : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumArtistAsc     ); break;
            case CMediaTableView::ColComposer        : std::sort(m_data.begin(), m_data.end(), cmpSongComposerAsc        ); break;
            case CMediaTableView::ColYear            : std::sort(m_data.begin(), m_data.end(), cmpSongYearAsc            ); break;
            case CMediaTableView::ColTrackNumber     : std::sort(m_data.begin(), m_data.end(), cmpSongTrackAsc           ); break;
            case CMediaTableView::ColDiscNumber      : std::sort(m_data.begin(), m_data.end(), cmpSongDiscAsc            ); break;
            case CMediaTableView::ColGenre           : std::sort(m_data.begin(), m_data.end(), cmpSongGenreAsc           ); break;
            case CMediaTableView::ColRating          : std::sort(m_data.begin(), m_data.end(), cmpSongRatingAsc          ); break;
            case CMediaTableView::ColComments        : std::sort(m_data.begin(), m_data.end(), cmpSongCommentsAsc        ); break;
            case CMediaTableView::ColPlayCount       : std::sort(m_data.begin(), m_data.end(), cmpSongPlayCountAsc       ); break;
            case CMediaTableView::ColLastPlayTime    : std::sort(m_data.begin(), m_data.end(), cmpSongLastPlayedAsc      ); break;
            case CMediaTableView::ColPathName        : std::sort(m_data.begin(), m_data.end(), cmpSongPathNameAsc        ); break;
            case CMediaTableView::ColBitRate         : std::sort(m_data.begin(), m_data.end(), cmpSongBitRateAsc         ); break;
            case CMediaTableView::ColFormat          : std::sort(m_data.begin(), m_data.end(), cmpSongFormatAsc          ); break;
            case CMediaTableView::ColDuration        : std::sort(m_data.begin(), m_data.end(), cmpSongDurationAsc        ); break;
            case CMediaTableView::ColSampleRate      : std::sort(m_data.begin(), m_data.end(), cmpSongSampleRateAsc      ); break;
            case CMediaTableView::ColCreationDate    : std::sort(m_data.begin(), m_data.end(), cmpSongCreationDateAsc    ); break;
            case CMediaTableView::ColModificationDate: std::sort(m_data.begin(), m_data.end(), cmpSongModificationDateAsc); break;
            case CMediaTableView::ColChannels        : std::sort(m_data.begin(), m_data.end(), cmpSongChannelsAsc        ); break;
            case CMediaTableView::ColFileSize        : std::sort(m_data.begin(), m_data.end(), cmpSongFileSizeAsc        ); break;
            case CMediaTableView::ColLyrics          : std::sort(m_data.begin(), m_data.end(), cmpSongLyricsAsc          ); break;
            case CMediaTableView::ColLanguage        : std::sort(m_data.begin(), m_data.end(), cmpSongLanguageAsc        ); break;
            case CMediaTableView::ColLyricist        : std::sort(m_data.begin(), m_data.end(), cmpSongLyricistAsc        ); break;
            case CMediaTableView::ColGrouping        : std::sort(m_data.begin(), m_data.end(), cmpSongGroupingAsc        ); break;
            case CMediaTableView::ColSubTitle        : std::sort(m_data.begin(), m_data.end(), cmpSongSubTitleAsc        ); break;
            case CMediaTableView::ColTrackGain       : std::sort(m_data.begin(), m_data.end(), cmpSongTrackGainAsc       ); break;
            case CMediaTableView::ColTrackPeak       : std::sort(m_data.begin(), m_data.end(), cmpSongTrackPeakAsc       ); break;
            case CMediaTableView::ColAlbumGain       : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumGainAsc       ); break;
            case CMediaTableView::ColAlbumPeak       : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumPeakAsc       ); break;
            case CMediaTableView::ColBPM             : std::sort(m_data.begin(), m_data.end(), cmpSongBPMAsc             ); break;
            case CMediaTableView::ColTitleSort       : std::sort(m_data.begin(), m_data.end(), cmpSongTitleSortAsc       ); break;
            case CMediaTableView::ColArtistSort      : std::sort(m_data.begin(), m_data.end(), cmpSongArtistSortAsc      ); break;
            case CMediaTableView::ColAlbumSort       : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumSortAsc       ); break;
            case CMediaTableView::ColAlbumArtistSort : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumArtistSortAsc ); break;
            case CMediaTableView::ColComposerSort    : std::sort(m_data.begin(), m_data.end(), cmpSongComposerSortAsc    ); break;
            case CMediaTableView::ColFileName        : std::sort(m_data.begin(), m_data.end(), cmpSongFileNameAsc        ); break;
        }
    }
    else
    {
        switch (column)
        {
            case CMediaTableView::ColPosition        : std::sort(m_data.begin(), m_data.end(), cmpSongPositionDesc        ); break;
            case CMediaTableView::ColTitle           : std::sort(m_data.begin(), m_data.end(), cmpSongTitleDesc           ); break;
            case CMediaTableView::ColArtist          : std::sort(m_data.begin(), m_data.end(), cmpSongArtistDesc          ); break;
            case CMediaTableView::ColAlbum           : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumDesc           ); break;
            case CMediaTableView::ColAlbumArtist     : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumArtistDesc     ); break;
            case CMediaTableView::ColComposer        : std::sort(m_data.begin(), m_data.end(), cmpSongComposerDesc        ); break;
            case CMediaTableView::ColYear            : std::sort(m_data.begin(), m_data.end(), cmpSongYearDesc            ); break;
            case CMediaTableView::ColTrackNumber     : std::sort(m_data.begin(), m_data.end(), cmpSongTrackDesc           ); break;
            case CMediaTableView::ColDiscNumber      : std::sort(m_data.begin(), m_data.end(), cmpSongDiscDesc            ); break;
            case CMediaTableView::ColGenre           : std::sort(m_data.begin(), m_data.end(), cmpSongGenreDesc           ); break;
            case CMediaTableView::ColRating          : std::sort(m_data.begin(), m_data.end(), cmpSongRatingDesc          ); break;
            case CMediaTableView::ColComments        : std::sort(m_data.begin(), m_data.end(), cmpSongCommentsDesc        ); break;
            case CMediaTableView::ColPlayCount       : std::sort(m_data.begin(), m_data.end(), cmpSongPlayCountDesc       ); break;
            case CMediaTableView::ColLastPlayTime    : std::sort(m_data.begin(), m_data.end(), cmpSongLastPlayedDesc      ); break;
            case CMediaTableView::ColPathName        : std::sort(m_data.begin(), m_data.end(), cmpSongPathNameDesc        ); break;
            case CMediaTableView::ColBitRate         : std::sort(m_data.begin(), m_data.end(), cmpSongBitRateDesc         ); break;
            case CMediaTableView::ColFormat          : std::sort(m_data.begin(), m_data.end(), cmpSongFormatDesc          ); break;
            case CMediaTableView::ColDuration        : std::sort(m_data.begin(), m_data.end(), cmpSongDurationDesc        ); break;
            case CMediaTableView::ColSampleRate      : std::sort(m_data.begin(), m_data.end(), cmpSongSampleRateDesc      ); break;
            case CMediaTableView::ColCreationDate    : std::sort(m_data.begin(), m_data.end(), cmpSongCreationDateDesc    ); break;
            case CMediaTableView::ColModificationDate: std::sort(m_data.begin(), m_data.end(), cmpSongModificationDateDesc); break;
            case CMediaTableView::ColChannels        : std::sort(m_data.begin(), m_data.end(), cmpSongChannelsDesc        ); break;
            case CMediaTableView::ColFileSize        : std::sort(m_data.begin(), m_data.end(), cmpSongFileSizeDesc        ); break;
            case CMediaTableView::ColLyrics          : std::sort(m_data.begin(), m_data.end(), cmpSongLyricsDesc          ); break;
            case CMediaTableView::ColLanguage        : std::sort(m_data.begin(), m_data.end(), cmpSongLanguageDesc        ); break;
            case CMediaTableView::ColLyricist        : std::sort(m_data.begin(), m_data.end(), cmpSongLyricistDesc        ); break;
            case CMediaTableView::ColGrouping        : std::sort(m_data.begin(), m_data.end(), cmpSongGroupingDesc        ); break;
            case CMediaTableView::ColSubTitle        : std::sort(m_data.begin(), m_data.end(), cmpSongSubTitleDesc        ); break;
            case CMediaTableView::ColTrackGain       : std::sort(m_data.begin(), m_data.end(), cmpSongTrackGainDesc       ); break;
            case CMediaTableView::ColTrackPeak       : std::sort(m_data.begin(), m_data.end(), cmpSongTrackPeakDesc       ); break;
            case CMediaTableView::ColAlbumGain       : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumGainDesc       ); break;
            case CMediaTableView::ColAlbumPeak       : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumPeakDesc       ); break;
            case CMediaTableView::ColBPM             : std::sort(m_data.begin(), m_data.end(), cmpSongBPMDesc             ); break;
            case CMediaTableView::ColTitleSort       : std::sort(m_data.begin(), m_data.end(), cmpSongTitleSortDesc       ); break;
            case CMediaTableView::ColArtistSort      : std::sort(m_data.begin(), m_data.end(), cmpSongArtistSortDesc      ); break;
            case CMediaTableView::ColAlbumSort       : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumSortDesc       ); break;
            case CMediaTableView::ColAlbumArtistSort : std::sort(m_data.begin(), m_data.end(), cmpSongAlbumArtistSortDesc ); break;
            case CMediaTableView::ColComposerSort    : std::sort(m_data.begin(), m_data.end(), cmpSongComposerSortDesc    ); break;
            case CMediaTableView::ColFileName        : std::sort(m_data.begin(), m_data.end(), cmpSongFileNameDesc        ); break;
        }
    }

    // Morceaux filtrés
    if (order == Qt::AscendingOrder)
    {
        switch (column)
        {
            case CMediaTableView::ColPosition        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongPositionAsc        ); break;
            case CMediaTableView::ColTitle           : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongTitleAsc           ); break;
            case CMediaTableView::ColArtist          : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongArtistAsc          ); break;
            case CMediaTableView::ColAlbum           : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumAsc           ); break;
            case CMediaTableView::ColAlbumArtist     : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumArtistAsc     ); break;
            case CMediaTableView::ColComposer        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongComposerAsc        ); break;
            case CMediaTableView::ColYear            : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongYearAsc            ); break;
            case CMediaTableView::ColTrackNumber     : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongTrackAsc           ); break;
            case CMediaTableView::ColDiscNumber      : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongDiscAsc            ); break;
            case CMediaTableView::ColGenre           : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongGenreAsc           ); break;
            case CMediaTableView::ColRating          : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongRatingAsc          ); break;
            case CMediaTableView::ColComments        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongCommentsAsc        ); break;
            case CMediaTableView::ColPlayCount       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongPlayCountAsc       ); break;
            case CMediaTableView::ColLastPlayTime    : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongLastPlayedAsc      ); break;
            case CMediaTableView::ColPathName        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongFileNameAsc        ); break;
            case CMediaTableView::ColBitRate         : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongBitRateAsc         ); break;
            case CMediaTableView::ColFormat          : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongFormatAsc          ); break;
            case CMediaTableView::ColDuration        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongDurationAsc        ); break;
            case CMediaTableView::ColSampleRate      : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongSampleRateAsc      ); break;
            case CMediaTableView::ColCreationDate    : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongCreationDateAsc    ); break;
            case CMediaTableView::ColModificationDate: std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongModificationDateAsc); break;
            case CMediaTableView::ColChannels        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongChannelsAsc        ); break;
            case CMediaTableView::ColFileSize        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongFileSizeAsc        ); break;
            case CMediaTableView::ColLyrics          : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongLyricsAsc          ); break;
            case CMediaTableView::ColLanguage        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongLanguageAsc        ); break;
            case CMediaTableView::ColLyricist        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongLyricistAsc        ); break;
            case CMediaTableView::ColGrouping        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongGroupingAsc        ); break;
            case CMediaTableView::ColSubTitle        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongSubTitleAsc        ); break;
            case CMediaTableView::ColTrackGain       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongTrackGainAsc       ); break;
            case CMediaTableView::ColTrackPeak       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongTrackPeakAsc       ); break;
            case CMediaTableView::ColAlbumGain       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumGainAsc       ); break;
            case CMediaTableView::ColAlbumPeak       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumPeakAsc       ); break;
            case CMediaTableView::ColBPM             : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongBPMAsc             ); break;
            case CMediaTableView::ColTitleSort       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongTitleSortAsc       ); break;
            case CMediaTableView::ColArtistSort      : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongArtistSortAsc      ); break;
            case CMediaTableView::ColAlbumSort       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumSortAsc       ); break;
            case CMediaTableView::ColAlbumArtistSort : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumArtistSortAsc ); break;
            case CMediaTableView::ColComposerSort    : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongComposerSortAsc    ); break;
            case CMediaTableView::ColFileName        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongFileNameAsc        ); break;
        }
    }
    else
    {
        switch (column)
        {
            case CMediaTableView::ColPosition        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongPositionDesc        ); break;
            case CMediaTableView::ColTitle           : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongTitleDesc           ); break;
            case CMediaTableView::ColArtist          : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongArtistDesc          ); break;
            case CMediaTableView::ColAlbum           : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumDesc           ); break;
            case CMediaTableView::ColAlbumArtist     : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumArtistDesc     ); break;
            case CMediaTableView::ColComposer        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongComposerDesc        ); break;
            case CMediaTableView::ColYear            : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongYearDesc            ); break;
            case CMediaTableView::ColTrackNumber     : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongTrackDesc           ); break;
            case CMediaTableView::ColDiscNumber      : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongDiscDesc            ); break;
            case CMediaTableView::ColGenre           : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongGenreDesc           ); break;
            case CMediaTableView::ColRating          : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongRatingDesc          ); break;
            case CMediaTableView::ColComments        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongCommentsDesc        ); break;
            case CMediaTableView::ColPlayCount       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongPlayCountDesc       ); break;
            case CMediaTableView::ColLastPlayTime    : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongLastPlayedDesc      ); break;
            case CMediaTableView::ColPathName        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongFileNameDesc        ); break;
            case CMediaTableView::ColBitRate         : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongBitRateDesc         ); break;
            case CMediaTableView::ColFormat          : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongFormatDesc          ); break;
            case CMediaTableView::ColDuration        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongDurationDesc        ); break;
            case CMediaTableView::ColSampleRate      : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongSampleRateDesc      ); break;
            case CMediaTableView::ColCreationDate    : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongCreationDateDesc    ); break;
            case CMediaTableView::ColModificationDate: std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongModificationDateDesc); break;
            case CMediaTableView::ColChannels        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongChannelsDesc        ); break;
            case CMediaTableView::ColFileSize        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongFileSizeDesc        ); break;
            case CMediaTableView::ColLyrics          : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongLyricsDesc          ); break;
            case CMediaTableView::ColLanguage        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongLanguageDesc        ); break;
            case CMediaTableView::ColLyricist        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongLyricistDesc        ); break;
            case CMediaTableView::ColGrouping        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongGroupingDesc        ); break;
            case CMediaTableView::ColSubTitle        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongSubTitleDesc        ); break;
            case CMediaTableView::ColTrackGain       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongTrackGainDesc       ); break;
            case CMediaTableView::ColTrackPeak       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongTrackPeakDesc       ); break;
            case CMediaTableView::ColAlbumGain       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumGainDesc       ); break;
            case CMediaTableView::ColAlbumPeak       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumPeakDesc       ); break;
            case CMediaTableView::ColBPM             : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongBPMDesc             ); break;
            case CMediaTableView::ColTitleSort       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongTitleSortDesc       ); break;
            case CMediaTableView::ColArtistSort      : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongArtistSortDesc      ); break;
            case CMediaTableView::ColAlbumSort       : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumSortDesc       ); break;
            case CMediaTableView::ColAlbumArtistSort : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongAlbumArtistSortDesc ); break;
            case CMediaTableView::ColComposerSort    : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongComposerSortDesc    ); break;
            case CMediaTableView::ColFileName        : std::sort(m_dataFiltered.begin(), m_dataFiltered.end(), cmpSongFileNameDesc        ); break;
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

Qt::ItemFlags CMediaTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index) | Qt::ItemIsDragEnabled;

    if (index.isValid())
    {
        if (index.column() == CMediaTableView::ColTitle)
        {
            flags |= Qt::ItemIsUserCheckable;
        }
        else if (index.column() == CMediaTableView::ColRating)
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

QStringList CMediaTableModel::mimeTypes() const
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


QMimeData * CMediaTableModel::mimeData(const QModelIndexList& indexes) const
{
    // Liste des lignes et des fichiers
    QStringList fileList;
    QList<int> rows;

    for (QModelIndexList::ConstIterator it = indexes.begin(); it != indexes.end(); ++it)
    {
        if (it->isValid() && !rows.contains(it->row()))
        {
            rows.append(it->row());

            const QString fileName = m_dataFiltered.at(it->row())->getSong()->getFileName();

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

    for (QList<int>::ConstIterator it = rows.begin(); it != rows.end(); ++it)
    {
        streamSongs << m_dataFiltered[*it]->getSong()->getId();
        streamRows << *it;
    }

    QString fileListString;

    for (QStringList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        fileListString.append(QUrl::fromLocalFile(*it).toString());
        fileListString.append("\r\n");
    }

    QByteArray fileListData(fileListString.toUtf8());

    QMimeData * mimeData = new QMimeData();
    mimeData->setData("application/x-ted-media-songs", songListData);
    mimeData->setData("text/uri-list", fileListData);

    if (m_canDrop && m_columnSort == CMediaTableView::ColPosition)
    {
        mimeData->setData("application/x-ted-media-items", rowListData);
    }

    return mimeData;
}


/**
 * Méthode appelée lorsqu'on dépose des données dans le modèle.
 *
 * \todo Pouvoir ajouter des morceaux par glisser-déposer depuis une autre application.
 *
 * \param data   Données déposées.
 * \param action Action de glisser-déposer.
 * \param row    Ligne où les données sont déposées.
 * \param column Colonne om les données sont déposées.
 * \param parent Index parent.
 */

bool CMediaTableModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    Q_CHECK_PTR(data);

    if (action == Qt::IgnoreAction)
        return true;

    // Fichiers à ajouter à la médiathèque
    if (data->hasFormat("text/uri-list"))
    {
        //...
    }

    // Déplacement des morceaux dans la liste
    if (!m_canDrop || !data->hasFormat("application/x-ted-media-items") || m_columnSort != CMediaTableView::ColPosition)
    {
        return false;
    }

    return true;
}


/**
 * Déplace un ensemble de lignes vers une autre position.
 * La liste \a rows doit être triée et ne doit pas contenir de doublons.
 *
 * \param rows    Liste de lignes (les numéros de ligne doivent être compris entre 0 et rowCount() - 1).
 * \param rowDest Ligne de destination (entre 0 et rowCount()).
 */

void CMediaTableModel::moveRows(const QList<int>& rows, int rowDest)
{
    if (rows.isEmpty() || rowDest < 0 || rowDest > m_data.size())
    {
        return;
    }

    QList<CMediaTableItem *> dataCopy = m_data;
    qSort(dataCopy.begin(), dataCopy.end(), cmpSongPositionAsc);

    QList<int>::ConstIterator it2 = rows.begin() - 1;
    int numMoved = 0;

    for (QList<int>::ConstIterator it = rows.begin(); it != rows.end(); ++it)
    {
        if (*it < rowDest)
        {
            it2 = it;
            continue;
        }

        dataCopy.move(*it, rowDest + numMoved);
        ++numMoved;
    }

    numMoved = 0;

    for (; it2 >= rows.begin(); --it2)
    {
        dataCopy.move(*it2, rowDest - 1 - numMoved);
        ++numMoved;
    }

    for (int pos = 0; pos < dataCopy.size(); ++pos)
    {
        dataCopy[pos]->m_position = pos + 1;
    }

    emit layoutAboutToBeChanged();
    m_data = dataCopy;
    emit layoutChanged();
/*
    emit rowsAboutToBeMoved(QModelIndex(), int sourceStart, int sourceEnd, QModelIndex(), int destinationRow);
    emit rowsMoved(QModelIndex(), int start, int end, QModelIndex(), int row);
*/
}


/**
 * Ajoute un morceau au modèle.
 *
 * \param song Pointeur sur le morceau à ajouter.
 * \param pos  Numéro de ligne.
 */

void CMediaTableModel::insertRow(CSong * song, int pos)
{
    Q_CHECK_PTR(song);

    emit layoutAboutToBeChanged();

    CMediaTableItem * songItem = new CMediaTableItem(pos, song);

    if (pos < 0 || pos >= m_data.size())
    {
        m_data.append(songItem);
        songItem->m_position = m_data.size();
    }
    else
    {
        m_data.insert(pos, songItem);

        // Réorganisation des éléments
        int itemPos = 1;

        for (QList<CMediaTableItem *>::ConstIterator it = m_data.begin() /*+ pos*/; it != m_data.end(); ++it, ++itemPos)
        {
            (*it)->m_position = itemPos;
        }
    }

    m_dataShuffle.append(songItem);

    if (song->matchFilter(m_mainWindow->getFilter()))
    {
        m_dataFiltered.append(songItem);
        m_dataShuffleFiltered.append(songItem);
    }

    emit layoutChanged();
}


/**
 * Supprime une ligne du modèle.
 *
 * \param row Numéro de la ligne à supprimer.
 */

void CMediaTableModel::removeRow(int row)
{
    if (row < 0 || row >= m_data.size())
    {
        return;
    }

    emit layoutAboutToBeChanged();

    CMediaTableItem * songItem = m_data.takeAt(row);
    m_dataShuffle.removeOne(songItem);

    m_dataFiltered.removeOne(songItem);
    m_dataShuffleFiltered.removeOne(songItem);

    delete songItem;

    emit layoutChanged();
}


/**
 * Supprime une liste de morceaux du modèle.
 *
 * \param songs Liste de morceaux à supprimer.
 */

void CMediaTableModel::removeSongs(const QList<CSong *>& songs)
{
    if (songs.isEmpty())
    {
        return;
    }

    emit layoutAboutToBeChanged();

    QList<CMediaTableItem *> dataCopy = m_data;

    for (QList<CMediaTableItem *>::ConstIterator it = dataCopy.begin(); it != dataCopy.end(); ++it)
    {
        if (songs.contains((*it)->getSong()))
        {
            m_data.removeOne(*it);
            m_dataShuffle.removeOne(*it);

            m_dataFiltered.removeOne(*it);
            m_dataShuffleFiltered.removeOne(*it);

            delete *it;
        }
    }

    emit layoutChanged();
}


/**
 * Supprime toutes les données du modèle.
 */

void CMediaTableModel::clear()
{
    emit layoutAboutToBeChanged();

    // Suppression des morceaux
    for (QList<CMediaTableItem *>::ConstIterator songItem = m_data.begin(); songItem != m_data.end(); ++songItem)
    {
        delete *songItem;
    }

    m_data.clear();
    m_dataShuffle.clear();

    m_dataFiltered.clear();
    m_dataShuffleFiltered.clear();

    emit layoutChanged();
}


/**
 * Retourne le morceau correspondant à un index.
 *
 * \param index Index de modèle.
 * \return Pointeur sur le morceau, ou nullptr si l'index est incorrect.
 */

CMediaTableItem * CMediaTableModel::getSongItem(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return getSongItem(index.row());
    }

    return nullptr;
}


/**
 * Retourne le morceau correspondant à une ligne.
 *
 * \param row Numéro de ligne.
 * \return Pointeur sur le morceau, ou nullptr si le numéro de ligne est incorrect.
 */

CMediaTableItem * CMediaTableModel::getSongItem(int row) const
{
    QList<CMediaTableItem *> data = m_dataFiltered;
    return (row < data.size() && row >= 0 ? data.at(row) : nullptr);
}


/**
 * Retourne la ligne correspondant à un morceau.
 *
 * \param songItem Morceau à rechercher.
 * \return Numéro de ligne, ou -1 si le morceau n'est pas dans la liste.
 */

int CMediaTableModel::getRowForSongItem(CMediaTableItem * songItem) const
{
    if (songItem == nullptr)
    {
        return -1;
    }

    QList<CMediaTableItem *> data = m_dataFiltered;
    return data.indexOf(songItem);
}


/**
 * Cherche le morceau situé avant un autre dans la liste.
 *
 * \param songItem Item actuel.
 * \param shuffle  Indique si la lecture aléatoire est activée.
 * \return Item précédent.
 */

CMediaTableItem * CMediaTableModel::getPreviousSong(CMediaTableItem * songItem, bool shuffle) const
{
    QList<CMediaTableItem *> data = m_dataFiltered;
    QList<CMediaTableItem *> dataShuffle = m_dataShuffleFiltered;

    if (songItem && !data.contains(songItem))
    {
        m_mainWindow->getMediaManager()->logError(tr("the requested item is not in the table"), __FUNCTION__, __FILE__, __LINE__);
        songItem = nullptr;
    }

    if (songItem)
    {
        if (shuffle)
        {
            if (data.size() != dataShuffle.size())
            {
                m_mainWindow->getMediaManager()->logError(tr("the shuffle list is incorrect"), __FUNCTION__, __FILE__, __LINE__);
            }

            const int row = dataShuffle.indexOf(songItem);

            if (row < 0)
            {
                m_mainWindow->getMediaManager()->logError(tr("the requested item is not in the shuffle list"), __FUNCTION__, __FILE__, __LINE__);
                return nullptr;
            }

            if (row == 0)
            {
                return nullptr;
            }

            CMediaTableItem * previousItem = dataShuffle.at(row - 1);
            return (previousItem->getSong()->isSkipShuffle() ? getNextSong(previousItem, true) : previousItem);
        }
        else
        {
            const int row = getRowForSongItem(songItem);
            return (row <= 0 ? nullptr : data.at(row - 1));
        }
    }
    else
    {
        return nullptr;
    }
}


/**
 * Cherche le morceau situé après un autre dans la liste.
 *
 * \param songItem Item actuel.
 * \param shuffle  Indique si la lecture aléatoire est activée.
 * \return Item suivant.
 */

CMediaTableItem * CMediaTableModel::getNextSong(CMediaTableItem * songItem, bool shuffle) const
{
    QList<CMediaTableItem *> data = m_dataFiltered;
    QList<CMediaTableItem *> dataShuffle = m_dataShuffleFiltered;

    if (songItem && !data.contains(songItem))
    {
        m_mainWindow->getMediaManager()->logError(tr("the requested item is not in the table"), __FUNCTION__, __FILE__, __LINE__);
        songItem = nullptr;
    }

    if (songItem)
    {
        if (shuffle)
        {
            if (data.size() != dataShuffle.size())
            {
                m_mainWindow->getMediaManager()->logError(tr("the shuffle list is incorrect"), __FUNCTION__, __FILE__, __LINE__);
            }

            const int row = dataShuffle.indexOf(songItem);

            if (row < 0)
            {
                m_mainWindow->getMediaManager()->logError(tr("the requested item is not in the shuffle list"), __FUNCTION__, __FILE__, __LINE__);
                return nullptr;
            }

            if (row == dataShuffle.size() - 1)
            {
                return nullptr;
            }

            CMediaTableItem * nextItem = dataShuffle.at(row + 1);
            return (nextItem->getSong()->isSkipShuffle() ? getNextSong(nextItem, true) : nextItem);
        }
        else
        {
            const int row = getRowForSongItem(songItem);
            return (row == data.size() - 1 ? nullptr : data.at(row + 1));
        }
    }
    // Premier élément de la liste
    else
    {
        if (shuffle)
        {
            if (data.size() != dataShuffle.size())
            {
                m_mainWindow->getMediaManager()->logError(tr("the shuffle list is incorrect"), __FUNCTION__, __FILE__, __LINE__);
                return nullptr;
            }

            if (dataShuffle.isEmpty())
            {
                return nullptr;
            }

            CMediaTableItem * nextItem = dataShuffle.at(0);
            return (nextItem->getSong()->isSkipShuffle() ? getNextSong(nextItem, true) : nextItem);
        }
        else
        {
            return (data.isEmpty() ? nullptr : data.at(0));
        }
    }
}


/**
 * Retourne le dernier morceau du modèle.
 *
 * \param shuffle Indique si la lecture aléatoire est activée.
 * \return Pointeur sur le dernier morceau, ou nullptr si la liste est vide.
 */

CMediaTableItem * CMediaTableModel::getLastSong(bool shuffle) const
{
    QList<CMediaTableItem *> data = m_dataFiltered;
    QList<CMediaTableItem *> dataShuffle = m_dataShuffleFiltered;

    // Aucun morceau dans le modèle
    if (data.isEmpty())
    {
        return nullptr;
    }

    if (!shuffle)
    {
        return data.last();
    }

    // Aucun morceau dans le modèle
    if (dataShuffle.isEmpty())
    {
        return nullptr;
    }

    QListIterator<CMediaTableItem *> it(dataShuffle);
    it.toBack();

    while (it.hasPrevious())
    {
        CMediaTableItem * item = it.previous();
        Q_CHECK_PTR(item);

        if (!item->getSong()->isSkipShuffle())
        {
            return item;
        }
    }

    return nullptr;
}


void CMediaTableModel::setCurrentSong(CMediaTableItem * songItem)
{
    emit layoutAboutToBeChanged();
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

void CMediaTableModel::initShuffle(CMediaTableItem * firstSong)
{
    m_dataShuffle = m_data;
    const int numSongs = m_data.size();

    if (numSongs > 1)
    {
        // Fisher-Yates algorithm
        for (int index = numSongs - 1; index > 0; --index)
        {
            const int newIndex = static_cast<int>((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) * (index + 1));
            m_dataShuffle.swap(index, newIndex); // Permutation
        }

        if (firstSong)
        {
            m_dataShuffle.swap(m_dataShuffle.indexOf(firstSong), 0);
        }
    }

    // Modification de la liste filtrée
    applyFilterForShuffleList(m_mainWindow->getFilter());
}


/**
 * Remplace un morceau par un autre.
 *
 * \param oldSong Pointeur sur le morceau à remplacer.
 * \param newSong Pointeur sur le nouveau morceau.
 */

void CMediaTableModel::replaceSong(CSong * oldSong, CSong * newSong)
{
    // On n'a pas besoin de parcourir les autres listes, car les pointeurs sont partagés entre ces listes
    for (QList<CMediaTableItem *>::ConstIterator item = m_data.begin(); item != m_data.end(); ++item)
    {
        if ((*item)->m_song == oldSong)
        {
            (*item)->m_song = newSong;
        }
    }
}


/**
 * Applique un filtre de recherche aux morceaux de la liste.
 *
 * \param filter Filtre de recherche.
 */

void CMediaTableModel::applyFilter(const QString& filter)
{
    emit layoutAboutToBeChanged();

    if (filter.isEmpty())
    {
        m_dataFiltered = m_data;
    }
    else
    {
        m_dataFiltered.clear();

        // Liste normale
        for (QList<CMediaTableItem *>::ConstIterator item = m_data.begin(); item != m_data.end(); ++item)
        {
            if ((*item)->m_song->matchFilter(filter))
            {
                m_dataFiltered.append(*item);
            }
        }
    }

    applyFilterForShuffleList(filter);
    emit layoutChanged();
}


/**
 * Applique le filtre de recherche aux morceaux de la liste aléatoire.
 *
 * \param filter Filtre de recherche.
 */

void CMediaTableModel::applyFilterForShuffleList(const QString& filter)
{
    if (filter.isEmpty())
    {
        m_dataShuffleFiltered = m_dataShuffle;
        return;
    }

    m_dataShuffleFiltered.clear();

    for (QList<CMediaTableItem *>::ConstIterator item = m_dataShuffle.begin(); item != m_dataShuffle.end(); ++item)
    {
        if ((*item)->m_song->matchFilter(filter))
        {
            m_dataShuffleFiltered.append(*item);
        }
    }
}

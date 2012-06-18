
#include "CPlayListModel.hpp"
#include "CSongTable.hpp"
#include "CStaticPlayList.hpp"
#include "CApplication.hpp"
#include <QMimeData>

#include <QtDebug>


/**
 * Constructeur du modèle.
 *
 * \param application Pointeur sur l'application.
 */

CPlayListModel::CPlayListModel(CApplication * application) :
    QStandardItemModel (application),
    m_application      (application)
{
    Q_CHECK_PTR(application);
}


/**
 * Destructeur du modèle.
 */

CPlayListModel::~CPlayListModel()
{
    //qDebug() << "CPlayListModel::~CPlayListModel()";
}


/**
 * \todo Pouvoir créer une liste de lecture statique.
 */

bool CPlayListModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    Q_CHECK_PTR(data);

    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    if (!data->hasFormat("application/x-ted-media-songs"))
    {
        return false;
    }

    if (parent.isValid())
    {
        //qDebug() << "CPlayListModel::dropMimeData() : ajout à une liste...";

        CSongTable * songTable = this->data(parent, Qt::UserRole + 1).value<CSongTable *>();
        CStaticPlayList * playList = qobject_cast<CStaticPlayList *>(songTable);

        if (songTable && playList)
        {
            //qDebug() << "playlist " << playList->getName();

            QByteArray encodedData = data->data("application/x-ted-media-songs");
            QList<CSong *> songList = decodeData(encodedData);
/*
            QDataStream stream(&encodedData, QIODevice::ReadOnly);

            int numSongs;
            stream >> numSongs;

            QList<CSong *> songList;

            for (int i = 0; i < numSongs; ++i)
            {
                int songId;
                stream >> songId;
                songList << m_application->getSongFromId(songId);
            }
*/

            playList->addSongs(songList);

            return true;
        }

        //qDebug() << "library ou dynamic list";
        return false;
    }

    // Création d'une liste statique
    qDebug() << "CPlayListModel::dropMimeData() : création d'une nouvelle liste...";

    QByteArray encodedData = data->data("application/x-ted-media-songs");
    QList<CSong *> songs = decodeData(encodedData);

    m_application->openDialogCreateStaticList(NULL, songs);

    return true;
}


QList<CSong *> CPlayListModel::decodeData(const QByteArray& encodedData) const
{
    QDataStream stream(encodedData);

    int numSongs;
    stream >> numSongs;

    QList<CSong *> songList;

    for (int i = 0; i < numSongs; ++i)
    {
        int songId;
        stream >> songId;
        songList << m_application->getSongFromId(songId);
    }

    return songList;
}


/**
 * Retourne la liste des types MIME acceptés pour le drop.
 *
 * \return Liste de types.
 */

QStringList CPlayListModel::mimeTypes(void) const
{
    QStringList types;
    types << "application/x-ted-media-songs"; // Liste de morceaux
    //types << "application/x-ted-media-list";  // Liste de lecture ou dossier
    return types;
}


/**
 * Retourne la liste des actions acceptées pour le drop.
 *
 * \return Actions acceptées.
 */

Qt::DropActions CPlayListModel::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::MoveAction);
}


#include "CPlayListModel.hpp"
#include "CSongTable.hpp"
#include "CStaticPlayList.hpp"
#include "CApplication.hpp"
#include <QMimeData>

#include <QtDebug>


CPlayListModel::CPlayListModel(CApplication * application) :
    QStandardItemModel (application),
    m_application      (application)
{
    Q_CHECK_PTR(application);
}


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

            playList->addSongs(songList);

            return true;
        }

        //qDebug() << "library ou dynamic list";
        return false;
    }

    qDebug() << "CPlayListModel::dropMimeData() : création d'une nouvelle liste...";
    return true;
}


QStringList CPlayListModel::mimeTypes(void) const
{
    QStringList types;
    types << "application/x-ted-media-songs";
    return types;
}


Qt::DropActions CPlayListModel::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::MoveAction);
}

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

#include "CMusicBrainzLookup.hpp"
#include "../CMainWindow.hpp"
#include "../CMediaManager.hpp"
#include "../CCDRomDrive.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QtXml>

#include <QtDebug>


const QString CMusicBrainzLookup::m_lookupUrl = "http://musicbrainz.org/ws/2/discid/%1?inc=artist-credits+recordings";


CMusicBrainzLookup::CMusicBrainzLookup(CCDRomDrive * cdRomDrive, CMainWindow * mainWindow) :
QObject          (mainWindow),
m_mainWindow     (mainWindow),
m_cdRomDrive     (cdRomDrive),
m_musicBrainzId  (cdRomDrive->getMusicBrainzDiscId()),
m_networkManager (nullptr)
{
    Q_CHECK_PTR(m_mainWindow);
    Q_CHECK_PTR(m_cdRomDrive);

    m_networkManager = new QNetworkAccessManager(this);
    //m_networkManager->setUserAgent();
    connect(m_networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));

    QNetworkRequest request(QUrl(m_lookupUrl.arg(cdRomDrive->getMusicBrainzDiscId())));
    request.setRawHeader("User-Agent", QString("TMediaPlayer/%1").arg(CMediaManager::getAppVersion()).toLatin1()); 
    m_networkManager->get(request);
}


CMusicBrainzLookup::~CMusicBrainzLookup()
{

}


void CMusicBrainzLookup::replyFinished(QNetworkReply * reply)
{
    Q_CHECK_PTR(reply);

    QByteArray data = reply->readAll();

    QDomDocument doc;
    QString error;

    if (!doc.setContent(data, &error))
    {
        m_mainWindow->getMediaManager()->logError(tr("Invalid XML document (%1)").arg(error), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    QDomElement node = doc.documentElement();

    if (node.tagName() != "metadata")
    {
        m_mainWindow->getMediaManager()->logError(tr("Réponse XML incorrecte (élément 'metadata' attendu)"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    node = node.firstChildElement();

    if (node.tagName() != "disc")
    {
        m_mainWindow->getMediaManager()->logError(tr("Réponse XML incorrecte (élément 'disc' attendu)"), __FUNCTION__, __FILE__, __LINE__);
        return;
    }

    for (node = node.firstChildElement(); !node.isNull(); node = node.nextSibling().toElement())
    {
        if (node.tagName() != "release-list")
        {
            continue;
        }

        if (node.attribute("count", "1") != "1")
        {
            m_mainWindow->getMediaManager()->logError(tr("MusicBrainz: invalid response (release-list count != 1)"), __FUNCTION__, __FILE__, __LINE__);
            return;
        }

        node = node.firstChildElement();

        if (node.tagName() != "release")
        {
            m_mainWindow->getMediaManager()->logError(tr("Réponse XML incorrecte (élément 'release' attendu)"), __FUNCTION__, __FILE__, __LINE__);
            return;
        }

        //node.attribute("id", "");

        // Informations sur l'album
        QString albumTitle;  ///< Titre de l'album.
        QString albumArtist; ///< Artiste de l'album.
        int albumYear = 0;   ///< Année de sortie de l'album.

        // Informations sur les pistes
        struct TTrackInfos
        {
            QString title;  ///< Titre de la piste.
            QString artist; ///< Nom de l'artiste.
            int duration;   ///< Durée de la piste, en millisecondes.

            inline TTrackInfos() : duration(0) { }
        };

        TTrackInfos trackInfos[100];
            
        // Parcours de la liste des informations de l'enregistrement
        for (QDomElement nodeList = node.firstChildElement(); !nodeList.isNull(); nodeList = nodeList.nextSibling().toElement())
        {
            // Titre de l'album
            if (nodeList.tagName() == "title")
            {
                albumTitle = nodeList.text();
            }
            // Artiste de l'album
            else if (nodeList.tagName() == "artist-credit")
            {
                albumArtist = getArtistName(nodeList);
            }
            // Année
            else if (nodeList.tagName() == "date")
            {
                albumYear = nodeList.text().toInt();
            }
            // Liste des médias
            else if (nodeList.tagName() == "medium-list")
            {
                if (nodeList.attribute("count", "1") != "1")
                {
                    m_mainWindow->getMediaManager()->logError(tr("MusicBrainz: invalid response (medium-list count != 1)"), __FUNCTION__, __FILE__, __LINE__);
                    return;
                }

                nodeList = nodeList.firstChildElement();

                if (nodeList.tagName() != "medium")
                {
                    m_mainWindow->getMediaManager()->logError(tr("invalid XML response (expected element '%1')").arg("medium"), __FUNCTION__, __FILE__, __LINE__);
                    return;
                }
            
                // Parcours de la liste des informations du média
                for (QDomElement nodeMedium = nodeList.firstChildElement(); !nodeMedium.isNull(); nodeMedium = nodeMedium.nextSibling().toElement())
                {
                    if (nodeMedium.tagName() == "track-list")
                    {
                        int trackCount = nodeMedium.attribute("count", "0").toInt();   //TODO: voir comment utiliser cette variable.
                        int trackOffset = nodeMedium.attribute("offset", "0").toInt(); //TODO: voir comment utiliser cette variable.
            
                        // Parcours de la liste des pistes
                        for (QDomElement nodeTrack = nodeMedium.firstChildElement(); !nodeTrack.isNull(); nodeTrack = nodeTrack.nextSibling().toElement())
                        {
                            if (nodeTrack.tagName() != "track")
                            {
                                m_mainWindow->getMediaManager()->logError(tr("invalid XML response (expected element '%1')").arg("track"), __FUNCTION__, __FILE__, __LINE__);
                                return;
                            }

                            int trackNumber = 0;
            
                            // Parcours de la liste des informations de la piste
                            for (QDomElement nodeTrackInfos = nodeTrack.firstChildElement(); !nodeTrackInfos.isNull(); nodeTrackInfos = nodeTrackInfos.nextSibling().toElement())
                            {
                                if (nodeTrackInfos.tagName() == "position")
                                {
                                    //TODO: voir comment utiliser cette variable.
                                }
                                else if (nodeTrackInfos.tagName() == "number")
                                {
                                    trackNumber = nodeTrackInfos.text().toInt();
                                }
                                // Durée de la piste
                                else if (nodeTrackInfos.tagName() == "length")
                                {
                                    trackInfos[trackNumber].duration = nodeTrackInfos.text().toInt();
                                }
                                else if (nodeTrackInfos.tagName() == "recording")
                                {
                                    // Parcours de la liste des informations de la piste
                                    for (QDomElement nodeTrackInfos2 = nodeTrackInfos.firstChildElement(); !nodeTrackInfos2.isNull(); nodeTrackInfos2 = nodeTrackInfos2.nextSibling().toElement())
                                    {
                                        // Titre de la piste
                                        if (nodeTrackInfos2.tagName() == "title")
                                        {
                                            trackInfos[trackNumber].title = nodeTrackInfos2.text();
                                        }
                                        // Artiste
                                        else if (nodeTrackInfos2.tagName() == "artist-credit")
                                        {
                                            trackInfos[trackNumber].artist = getArtistName(nodeTrackInfos2);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // On vérifie que le disque dans le lecteur n'a pas changé
        if (m_musicBrainzId == m_cdRomDrive->getMusicBrainzDiscId())
        {
            // Modification des informations de chaque piste du disque
            for (int track = 0; track < 100; ++track)
            {
                CSong * song = m_cdRomDrive->getSong(track);

                if (!song)
                    continue;

                song->setTitle(trackInfos[track].title);
                song->setArtistName(trackInfos[track].artist);
                song->setAlbumTitle(albumTitle);
                song->setAlbumArtist(albumArtist);
                song->setYear(albumYear);

                if (song->getDuration() != trackInfos[track].duration)
                    qDebug() << "Duration incorrecte...";
            }
        }
    }

    reply->deleteLater();
}


/**
 * Extrait un nom d'artiste depuis un nœud XML.
 *
 * \return Nom de l'artiste.
 */

QString CMusicBrainzLookup::getArtistName(const QDomElement& node) const
{
    // Argument incorrect
    if (node.tagName() != "artist-credit")
        return QString();

    QString artist;
            
    // Parcours de la liste des éléments enfants
    for (QDomElement nodeName = node.firstChildElement(); !nodeName.isNull(); nodeName = nodeName.nextSibling().toElement())
    {
        if (nodeName.tagName() != "name-credit")
            continue;

        QString joinPhrase = nodeName.attribute("joinphrase", QString());

        QDomElement nodeNameSub = nodeName.firstChildElement();

        if (nodeNameSub.tagName() == "name")
        {
            artist += nodeNameSub.text();
        }
        else if (nodeNameSub.tagName() == "artist")
        {
            nodeNameSub = nodeNameSub.firstChildElement();

            if (nodeNameSub.tagName() == "name")
            {
                artist += nodeNameSub.text();
            }
        }

        artist += joinPhrase;
    }

    return artist;
}

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

#ifndef FILE_C_DIALOG_EDIT_METADATA
#define FILE_C_DIALOG_EDIT_METADATA

#include <QDialog>
#include "ui_DialogEditMetadata.h"
#include <attachedpictureframe.h>


class CSong;
class CMainWindow;
class QStandardItemModel;

namespace TagLib
{
    namespace ID3v1 { class Tag; }
    namespace ID3v2 { class Tag; }
    namespace APE   { class Tag; }
    namespace Ogg   { class XiphComment; }
}


/**
 * Boite de dialogue pour afficher et modifier les métadonnées d'un morceau.
 */

class CDialogEditMetadata : public QDialog
{
    Q_OBJECT

public:

    CDialogEditMetadata(CMainWindow * application, CSong * song);
    virtual ~CDialogEditMetadata();

protected slots:

    void apply();
    void save();
    void reset();

    void enableTagID3v1(bool enable);
    void enableTagID3v2(bool enable);
    void enableTagAPE(bool enable);
    void enableTagXiphComment(bool enable);

protected:

    void initTagID3v1(TagLib::ID3v1::Tag * tags);
    void initTagID3v2(TagLib::ID3v2::Tag * tags);
    void initTagAPE(TagLib::APE::Tag * tags);
    void initTagXiphComment(TagLib::Ogg::XiphComment * tags);
    QString pictureType(TagLib::ID3v2::AttachedPictureFrame::Type type) const;

private:

    Ui::DialogEditMetadata * m_uiWidget;
    QStandardItemModel * m_modelID3v2Text;
    QStandardItemModel * m_modelID3v2URL;
    QStandardItemModel * m_modelID3v2Lyrics;
    QStandardItemModel * m_modelID3v2Comments;
    QStandardItemModel * m_modelID3v2Pictures;
    QStandardItemModel * m_modelAPE;
    QStandardItemModel * m_modelXiphComment;
    CMainWindow * m_mainWindow; ///< Pointeur sur l'application.
    CSong * m_song;               ///< Pointeur sur le morceau.
};

#endif // FILE_C_DIALOG_EDIT_METADATA

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

#ifndef FILE_C_DIALOG_EDIT_SONGS
#define FILE_C_DIALOG_EDIT_SONGS

#include <QDialog>
#include <QDateTime>
//#include "CMediaTableModel.hpp"
#include "ui_DialogEditSongs.h"


class CSong;
class CMainWindow;
class CSpecialSpinBox;
class CMediaTableItem;


/**
 * Boite de dialogue pour modifier les informations de plusieurs morceaux.
 */

class CDialogEditSongs : public QDialog
{
    Q_OBJECT

public:

    explicit CDialogEditSongs(QList<CMediaTableItem *>& songItemList, CMainWindow * mainWindow);
    virtual ~CDialogEditSongs();

protected slots:

    void apply();
    void save();

    void onTitleChange(const QString& title);
    void onTitleSortChange(const QString& title);
    void onArtistChange(const QString& artistName);
    void onArtistSortChange(const QString& artistName);
    void onAlbumChange(const QString& albumTitle);
    void onAlbumSortChange(const QString& albumTitle);
    void onAlbumArtistChange(const QString& albumArtist);
    void onAlbumArtistSortChange(const QString& albumArtist);
    void onComposerChange(const QString& composer);
    void onComposerSortChange(const QString& composer);
    void onSubTitleChange(const QString& subTitle);
    void onGroupingChange(const QString& grouping);
    void onCommentsChange();
    void onYearChange(const QString& year);
    void onTrackNumberChange(const QString& trackNumber);
    void onTrackCountChange(const QString& trackCount);
    void onDiscNumberChange(const QString& discNumber);
    void onDiscCountChange(const QString& discCount);
    void onBPMChange(const QString& bpm);
    void onGenreChange(const QString& genre);
    void onRatingChange(int rating);
    void onLyricsChange();
    void onLyricistChange(const QString& lyricist);
    void onLanguageChange(const QString& language);

    void onTitleChecked(bool checked);
    void onTitleSortChecked(bool checked);
    void onArtistChecked(bool checked);
    void onArtistSortChecked(bool checked);
    void onAlbumChecked(bool checked);
    void onAlbumSortChecked(bool checked);
    void onAlbumArtistChecked(bool checked);
    void onAlbumArtistSortChecked(bool checked);
    void onComposerChecked(bool checked);
    void onComposerSortChecked(bool checked);
    void onSubTitleChecked(bool checked);
    void onGroupingChecked(bool checked);
    void onCommentsChecked(bool checked);
    void onYearChecked(bool checked);
    void onTrackNumberChecked(bool checked);
    void onTrackCountChecked(bool checked);
    void onDiscNumberChecked(bool checked);
    void onDiscCountChecked(bool checked);
    void onBPMChecked(bool checked);
    void onGenreChecked(bool checked);
    void onRatingChecked(bool checked);
    void onLyricsChecked(bool checked);
    void onLyricistChecked(bool checked);
    void onLanguageChecked(bool checked);

    void onFocusChange(QWidget * old, QWidget * now);

private:

    struct TPlay
    {
        CSong * song;
        QDateTime time;
        QDateTime timeUTC;
    };

    inline static bool comparePlay(const TPlay& p1, const TPlay& p2)
    {
        if (p1.timeUTC.isNull())
            return false;

        if (p2.timeUTC.isNull())
            return true;

        return (p1.timeUTC > p2.timeUTC);
    }

    Ui::DialogEditSongs * m_uiWidget;
    CSpecialSpinBox * m_editRating;
    bool m_differentComments;
    bool m_differentLyrics;
    QList<CMediaTableItem *> m_songItemList;

    // Widgets
    QLineEdit * m_editTitleLE;
    QLineEdit * m_editTitleLE_2;
    QComboBox * m_editTitleCB;
    QComboBox * m_editTitleCB_2;
    QLineEdit * m_editTitleSortLE;
    QComboBox * m_editTitleSortCB;
    QLineEdit * m_editArtistLE;
    QLineEdit * m_editArtistLE_2;
    QComboBox * m_editArtistCB;
    QComboBox * m_editArtistCB_2;
    QLineEdit * m_editArtistSortLE;
    QComboBox * m_editArtistSortCB;
    QLineEdit * m_editAlbumLE;
    QLineEdit * m_editAlbumLE_2;
    QComboBox * m_editAlbumCB;
    QComboBox * m_editAlbumCB_2;
    QLineEdit * m_editAlbumSortLE;
    QComboBox * m_editAlbumSortCB;
    QLineEdit * m_editAlbumArtistLE;
    QLineEdit * m_editAlbumArtistLE_2;
    QComboBox * m_editAlbumArtistCB;
    QComboBox * m_editAlbumArtistCB_2;
    QLineEdit * m_editAlbumArtistSortLE;
    QComboBox * m_editAlbumArtistSortCB;
    QLineEdit * m_editComposerLE;
    QLineEdit * m_editComposerLE_2;
    QComboBox * m_editComposerCB;
    QComboBox * m_editComposerCB_2;
    QLineEdit * m_editComposerSortLE;
    QComboBox * m_editComposerSortCB;
};

#endif // FILE_C_DIALOG_EDIT_SONGS

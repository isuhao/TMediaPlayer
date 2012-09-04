/*
Copyright (C) 2012 Teddy Michel

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
#include "CSongTableModel.hpp"
#include "ui_DialogEditSongs.h"


class CApplication;
class CSpecialSpinBox;


/**
 * Boite de dialogue pour modifier les informations de plusieurs morceaux.
 */

class CDialogEditSongs : public QDialog
{
    Q_OBJECT

public:

    explicit CDialogEditSongs(QList<CSongTableItem *>& songItemList, CApplication * application);
    virtual ~CDialogEditSongs();

protected slots:

    void apply(void);
    void save(void);

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
    void onCommentsChange(void);
    void onYearChange(const QString& year);
    void onTrackNumberChange(const QString& trackNumber);
    void onTrackCountChange(const QString& trackCount);
    void onDiscNumberChange(const QString& discNumber);
    void onDiscCountChange(const QString& discCount);
    void onBPMChange(const QString& bpm);
    void onGenreChange(const QString& genre);
    void onRatingChange(int rating);
    void onLyricsChange(void);
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
    
    Ui::DialogEditSongs * m_uiWidget;
    CSpecialSpinBox * m_editRating;
    bool m_differentComments;
    bool m_differentLyrics;
    QList<CSongTableItem *> m_songItemList;
};

#endif // FILE_C_DIALOG_EDIT_SONGS

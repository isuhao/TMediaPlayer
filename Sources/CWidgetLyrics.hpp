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

#ifndef FILE_C_WIDGET_LYRICS
#define FILE_C_WIDGET_LYRICS


#include <QWidget>


class CApplication;
class CSong;
class QTextEdit;
class QPushButton;
class QGridLayout;


class CWidgetLyrics : public QWidget
{
    Q_OBJECT

public:

    explicit CWidgetLyrics(CApplication * application);

    void setSong(CSong * song);

protected slots:

    void findLyrics();
    void editLyrics();
    void saveLyrics();
    void cancelEdit();
    void onLyricsFound(const QString& lyrics);

private:

    CApplication * m_application;
    CSong * m_song;
    QGridLayout * m_layout;
    QTextEdit * m_textEdit;
    QPushButton * m_buttonFind;
    QPushButton * m_buttonEdit;
    QPushButton * m_buttonSave;
    QPushButton * m_buttonCancel;
};

#endif // FILE_C_WIDGET_LYRICS

/*
Copyright (C) 2012-2015 Teddy Michel

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

#ifndef FILE_C_UTILS_HPP_
#define FILE_C_UTILS_HPP_

#include <QString>
#include <QStringList>


class QMenu;
class QPoint;


QPoint getCorrectMenuPosition(QMenu * menu, const QPoint& menuPosition);
QString durationToString(qlonglong durationMS);
QString getFileSize(qlonglong fileSize);
QStringList listFilesInFolder(const QString& pathName);

#endif // FILE_C_UTILS_HPP_

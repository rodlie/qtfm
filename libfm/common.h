/*
 * This file is part of QtFM <https://qtfm.eu>
 *
 * Copyright (C) 2013-2019 QtFM developers (see AUTHORS)
 * Copyright (C) 2010-2012 Wittfella <wittfella@qtfm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QString>
#include <QVariant>

#ifndef APP
#define APP "qtfm"
#endif
#define FM_MAJOR 6

#define FM_SERVICE "eu.qtfm.qtfm"
#define FM_PATH "/qtfm"

#define BOOKMARK_PATH Qt::UserRole+1
#define BOOKMARK_ICON Qt::UserRole+2
#define BOOKMARKS_AUTO Qt::UserRole+3

#define MIME_APPS "/.local/share/applications/mimeapps.list"

#define COPY_X_OF "Copy (%1) of %2"
#define COPY_X_TS "yyyyMMddHHmmss"

class Common
{
public:
    enum DragMode {
      DM_UNKNOWN = 0,
      DM_COPY,
      DM_MOVE,
      DM_LINK
    };
    static QString configDir();
    static QString configFile();
    static QString trashDir();
    static QStringList iconLocations(QString appPath);
    static QStringList pixmapLocations(QString appPath);
    static QStringList applicationLocations(QString appPath);
    static QStringList mimeGlobLocations(QString appPath);
    static QStringList mimeGenericLocations(QString appPath);
    static QStringList mimeTypeLocations(QString appPath);
    static QString getDesktopIcon(QString desktop);
    static QString findIconInDir(QString appPath,
                                 QString theme,
                                 QString dir,
                                 QString icon);
    static QString findIcon(QString appPath,
                            QString theme,
                            QString fileIcon);
    static QString findApplication(QString appPath,
                                   QString desktopFile);
    static QStringList findApplications(QString filename);
    static QString findApplicationIcon(QString appPath,
                                       QString theme,
                                       QString app);
    static QMap<QString, QString> readGlobMimesFromFile(QString filename);
    static QMap<QString, QString> getMimesGlobs(QString appPath);
    static QMap<QString, QString> readGenericMimesFromFile(QString filename);
    static QMap<QString, QString> getMimesGeneric(QString appPath);
    static QStringList getPixmaps(QString appPath);
    static QStringList getMimeTypes(QString appPath);
    static QStringList getIconThemes(QString appPath);
    static bool removeFileCache();
    static bool removeFolderCache();
    static bool removeThumbsCache();
    static void setupIconTheme(QString appFilePath);
    static DragMode int2dad(int value);
    static QVariant readSetting(QString key,
                                QString fallback = QString());
    static void writeSetting(QString key,
                             QVariant value);
    static DragMode getDADaltMod();
    static DragMode getDADctrlMod();
    static DragMode getDADshiftMod();
    static DragMode getDefaultDragAndDrop();
    static QString getDeviceForDir(QString dir);
    static QPalette darkTheme();
    static QStringList iconPaths(QString appPath);
    static QVector<QStringList> getDefaultActions();
    static QString formatSize(qint64 num);
    static QString getDriveInfo(QString path);
};

#endif // COMMON_H

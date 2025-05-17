/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#include "common.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QIcon>
#include <QDirIterator>
#include <QRegularExpression>
#include <QTextStream>
#include <QMap>
#include <QMapIterator>
#include <QDirIterator>
#include <QSettings>
#include <QPalette>
#include <QVector>
#include <QCryptographicHash>
#include <QUrl>
#include <QStandardPaths>

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif
#include <sys/stat.h>
#ifdef __NetBSD__
#include <sys/statvfs.h>
#endif

QString Common::configDir()
{
    QString dir = QString("%1/.config/%2%3")
                  .arg(QDir::homePath())
                  .arg(APP)
                  .arg(FM_MAJOR);
    if (!QFile::exists(dir)) {
        QDir makedir(dir);
        if (!makedir.mkpath(dir)) { dir.clear(); }
    }
    return dir;
}

QString Common::configFile()
{
    return QString("%1/%2%3.conf")
           .arg(configDir())
           .arg(APP)
           .arg(FM_MAJOR);
}

QString Common::trashDir()
{
    QString dir = QString("%1/.local/share/Trash").arg(QDir::homePath());
    if (!QFile::exists(dir)) {
        QDir makedir(dir);
        if (!makedir.mkpath(dir)) { dir.clear(); }
    }
    return dir;
}

QStringList Common::iconLocations(QString appPath)
{
    QStringList result;
    result << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                        "icons",
                                        QStandardPaths::LocateDirectory);
    result << QString("%1/../share/icons").arg(appPath);
    return result;
}

QStringList Common::pixmapLocations(QString appPath)
{
    QStringList result;
    result << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                        "pixmaps",
                                        QStandardPaths::LocateDirectory);
    result << QString("%1/../share/pixmaps").arg(appPath);
    return result;
}

QStringList Common::applicationLocations(QString appPath)
{
    QStringList result;
    result << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                        "applications",
                                        QStandardPaths::LocateDirectory);
    result << QString("%1/../share/applications").arg(appPath);
    return result;
}

QStringList Common::mimeGlobLocations(QString appPath)
{
    QStringList result;
    result << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                        "mime/globs",
                                        QStandardPaths::LocateFile);
    result << QString("%1/../share/mime/globs").arg(appPath);
    return result;
}

QStringList Common::mimeGenericLocations(QString appPath)
{
    QStringList result;
    result << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                        "mime/generic-icons",
                                        QStandardPaths::LocateFile);
    result << QString("%1/../share/mime/generic-icons").arg(appPath);
    return result;
}

QStringList Common::mimeTypeLocations(QString appPath)
{
    QStringList result;
    result << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                        "mime/types",
                                        QStandardPaths::LocateFile);
    result << QString("%1/../share/mime/types").arg(appPath);
    return result;
}

QString Common::getDesktopIcon(QString desktop)
{
    QString result;
    if (desktop.isEmpty()) { return result; }
    QFile file(desktop);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { return result; }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) { continue; }
        if (line.trimmed().startsWith("Icon=")) {
            result = line.trimmed().replace("Icon=", "");
            break;
        }
    }
    file.close();
    return result;
}

QString Common::findIconInDir(QString appPath,
                              QString theme,
                              QString dir,
                              QString icon)
{
    QString result;
    if (dir.isEmpty() || icon.isEmpty()) { return result; }

    if (theme.isEmpty()) { theme = "hicolor"; }

    QStringList iconSizes;
    iconSizes << "128" << "64" << "48" << "32" << "22" << "16";

    // theme
    QDirIterator it(QString("%1/%2").arg(dir).arg(theme),
                    QStringList() << "*.png" << "*.jpg" << "*.xpm",
                    QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString found = it.next();
        QString iconName = QFileInfo(found).completeBaseName();
        if (iconName == icon) {
            for (int i=0;i<iconSizes.size();++i) {
                QString hasFile = found.replace(QRegularExpression("/\\d+x\\d+/"),
                                                QString("/%1x%1/").arg(iconSizes.at(i)));
                if (QFile::exists(hasFile)) { return hasFile; }
            }
            return found;
        }
    }
    // hicolor
    if (theme != "hicolor") {
        QDirIterator hicolor(QString("%1/%2").arg(dir).arg("hicolor"),
                             QStringList() << "*.png" << "*.jpg" << "*.xpm",
                             QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (hicolor.hasNext()) {
            QString found = hicolor.next();
            QString iconName = QFileInfo(found).completeBaseName();
            if (iconName == icon) {
                for (int i=0;i<iconSizes.size();++i) {
                    QString hasFile = found.replace(QRegularExpression("/\\d+x\\d+/"),
                                                    QString("/%1x%1/").arg(iconSizes.at(i)));
                    if (QFile::exists(hasFile)) { return hasFile; }
                }
                return found;
            }
        }
    }
    // pixmaps
    QStringList pixs = pixmapLocations(appPath);
    for (int i=0;i<pixs.size();++i) {
        QDirIterator pixmaps(pixs.at(i),
                             QStringList() << "*.png" << "*.jpg" << "*.xpm",
                             QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (pixmaps.hasNext()) {
            QString found = pixmaps.next();
            QString iconName = QFileInfo(found).completeBaseName();
            if (iconName == icon) { return found; }
        }
    }
    return result;
}

QString Common::findIcon(QString appPath,
                         QString theme,
                         QString fileIcon)
{
    QString result;
    if (fileIcon.isEmpty()) { return result; }
    QStringList icons = iconLocations(appPath);
    for (int i=0;i<icons.size();++i) {
        QString icon = findIconInDir(appPath,
                                     theme,
                                     icons.at(i),
                                     fileIcon);
        if (!icon.isEmpty()) { return icon; }
    }
    return result;
}

QString Common::findApplication(QString appPath,
                                QString desktopFile)
{
    QString result;
    if (desktopFile.isEmpty()) { return result; }
    QStringList apps = applicationLocations(appPath);
    for (int i=0;i<apps.size();++i) {
        QDirIterator it(apps.at(i), QStringList("*.desktop"), QDir::Files|QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            QString found = it.next();
            if (found.split("/").takeLast()==desktopFile) {
                //qDebug() << "found app" << found;
                return found;
            }
        }
    }
    return result;
}

QStringList Common::findApplications(QString filename)
{
    QStringList result;
    if (filename.isEmpty()) { return result; }
    QString path = qgetenv("PATH");
    QStringList paths = path.split(":", Qt::SkipEmptyParts);
    for (int i=0;i<paths.size();++i) {
        QDirIterator it(paths.at(i),
                        QStringList("*"),
                        QDir::Files|QDir::Executable|QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            QString found = it.next();
            if (found.split("/").takeLast().startsWith(filename)) {
                result << found;
            }
        }
    }
    return result;
}

QString Common::findApplicationIcon(QString appPath,
                                    QString theme,
                                    QString app)
{
    QString result;
    QString desktop = findApplication(appPath, app);
    if (desktop.isEmpty()) { return result; }
    QString icon = getDesktopIcon(desktop);
    if (icon.isEmpty()) { return result; }
    result = findIcon(appPath, theme, icon);
    return result;
}

QMap<QString, QString> Common::readGlobMimesFromFile(QString filename)
{
    QMap<QString, QString> map;
    if (filename.isEmpty()) { return map; }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) { return map; }
    QTextStream s(&file);
    while (!s.atEnd()) {
        QStringList line = s.readLine().split(":");
        if (line.count() == 2) {
            QString suffix = line.at(1);
            if (!suffix.startsWith("*.")) { continue; }
            suffix.remove("*.");
            QString mime = line.at(0);
            mime.replace("/", "-");
            if (!suffix.isEmpty() && !mime.isEmpty()) { map[mime] = suffix; }
        }
    }
    file.close();
    return map;
}

QMap<QString, QString> Common::getMimesGlobs(QString appPath)
{
    QMap<QString, QString> map;
    QStringList mimes = mimeGlobLocations(appPath);
    for (int i=0;i<mimes.size();++i) {
        QMapIterator<QString, QString> globs(readGlobMimesFromFile(mimes.at(i)));
        while (globs.hasNext()) {
            globs.next();
            map[globs.key()] = globs.value();
        }
    }
    return map;
}

QMap<QString, QString> Common::readGenericMimesFromFile(QString filename)
{
    QMap<QString, QString> map;
    if (filename.isEmpty()) { return map; }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) { return map; }
    QTextStream s(&file);
    while (!s.atEnd()) {
        QStringList line = s.readLine().split(":");
        if (line.count() == 2) {
            QString mimeName = line.at(0);
            mimeName.replace("/","-");
            QString mimeIcon = line.at(1);
            if (!mimeName.isEmpty() && !mimeIcon.isEmpty()) { map[mimeName] = mimeIcon; }
        }
    }
    file.close();
    return map;
}

QMap<QString, QString> Common::getMimesGeneric(QString appPath)
{
    QMap<QString, QString> map;
    QStringList mimes = mimeGenericLocations(appPath);
    for (int i=0;i<mimes.size();++i) {
        QMapIterator<QString, QString> generic(readGenericMimesFromFile(mimes.at(i)));
        while (generic.hasNext()) {
            generic.next();
            map[generic.key()] = generic.value();
        }
    }
    return map;
}

QStringList Common::getPixmaps(QString appPath)
{
    QStringList result;
    QStringList pixs = pixmapLocations(appPath);
    for (int i=0;i<pixs.size();++i) {
        QDir pixmaps(pixs.at(i), "", QDir::NoSort,
                     static_cast<QDir::Filters>(QDir::Files | QDir::NoDotAndDotDot));
        for (int i=0;i<pixmaps.entryList().size();++i) {
            result << QString("%1/%2").arg(pixmaps.absolutePath()).arg(pixmaps.entryList().at(i));
        }
    }
    return result;
}

QStringList Common::getMimeTypes(QString appPath)
{
    QStringList result;
    QStringList mimes = mimeTypeLocations(appPath);
    for (int i=0;i<mimes.size();++i) {
        QFile file(mimes.at(i));
        if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) { continue; }
        QTextStream s(&file);
        while (!s.atEnd()) {
            QString line = s.readLine();
            if (!line.isEmpty()) { result.append(line); }
        }
        file.close();
    }
    return result;
}

QStringList Common::getIconThemes(QString appPath)
{
    QStringList result;
    QStringList icons = iconLocations(appPath);
    for (int i=0;i<icons.size();++i) {
        QDirIterator it(icons.at(i), QDir::Dirs | QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            it.next();
            //qDebug() << it.fileName() << it.filePath();
            if (QFile::exists(it.filePath()+"/index.theme")) { result.append(it.fileName()); }
        }
    }
    return result;
}

bool Common::removeFileCache()
{
    QFile cache(QString("%1/file.cache").arg(Common::configDir()));
    if (cache.exists()) {
        return cache.remove();
    }
    return false;
}

bool Common::removeFolderCache()
{
    QFile cache(QString("%1/folder.cache").arg(Common::configDir()));
    if (cache.exists()) {
        return cache.remove();
    }
    return false;
}

bool Common::removeThumbsCache()
{
    QFile cache(QString("%1/thumbs.cache").arg(Common::configDir()));
    if (cache.exists()) {
        return cache.remove();
    }
    return false;
}

void Common::setupIconTheme(QString appFilePath)
{
    QString temp = QIcon::themeName();
    if (temp.isEmpty()  || temp == "hicolor") {
        qDebug() << "checking for icon theme in settings" << Common::configFile();
        QSettings settings(Common::configFile(), QSettings::IniFormat);
        temp = settings.value("fallbackTheme").toString();
    }
    if (temp.isEmpty() || temp == "hicolor") {
        if (QFile::exists(QDir::homePath() + "/" + ".gtkrc-2.0")) { // try gtk-2.0
            qDebug() << "checking for icon theme in gtkrc-2.0";
            QSettings gtkFile(QDir::homePath() + "/.gtkrc-2.0",QSettings::IniFormat/*,this*/);
            temp = gtkFile.value("gtk-icon-theme-name").toString().remove("\"");
        }
        else { //try gtk-3.0
            qDebug() << "checking for icon theme in gtk-3.0";
            QSettings gtkFile(QDir::homePath() + "/.config/gtk-3.0/settings.ini",QSettings::IniFormat/*,this*/);
            temp = gtkFile.value("gtk-fallback-icon-theme").toString().remove("\"");
        }
        //fallback
        if (temp.isEmpty()) {
#ifdef Q_OS_MACX
            Q_UNUSED(appFilePath)
            temp = "Adwaita";
#else
            qDebug() << "checking for icon theme in static fallback";
            QStringList themes;
            themes << QString("%1/../share/icons/Humanity").arg(appFilePath);
            themes << "/usr/share/icons/Humanity" << "/usr/local/share/icons/Humanity";
            themes << QString("%1/../share/icons/Adwaita").arg(appFilePath);
            themes << "/usr/share/icons/Adwaita" << "/usr/local/share/icons/Adwaita";
            themes << QString("%1/../share/icons/Tango").arg(appFilePath);
            themes << "/usr/share/icons/Tango" << "/usr/local/share/icons/Tango";
            themes << QString("%1/../share/icons/gnome").arg(appFilePath);
            themes << "/usr/share/icons/gnome" << "/usr/local/share/icons/gnome";
            themes << QString("%1/../share/icons/oxygen").arg(appFilePath);
            themes << "/usr/share/icons/oxygen" << "/usr/local/share/icons/oxygen";
            themes << QString("%1/../share/icons/hicolor").arg(appFilePath);
            themes << "/usr/share/icons/hicolor" << "/usr/local/share/icons/hicolor";
            for (int i=0;i<themes.size();++i) {
                if (QFile::exists(themes.at(i))) {
                    temp = QString(themes.at(i)).split("/").takeLast();
                    break;
                }
            }
#endif
        }
        if (temp!="hicolor" && !temp.isEmpty()) {
            qDebug() << "save icon theme for later use";
            QSettings settings(Common::configFile(), QSettings::IniFormat);
            settings.setValue("fallbackTheme", temp);
        }
    }
    qDebug() << "setting icon theme" << temp;
    QIcon::setThemeName(temp);
}

Common::DragMode Common::int2dad(int value)
{
    switch (value) {
    case 0:
        return DM_UNKNOWN;
    case 1:
        return DM_COPY;
    case 2:
        return DM_MOVE;
    case 3:
        return DM_LINK;
    default:
        return DM_MOVE;
    }
}

QVariant Common::readSetting(QString key, QString fallback) {
    QSettings settings(Common::configFile(), QSettings::IniFormat);
    return settings.value(key, fallback);
}

void Common::writeSetting(QString key, QVariant value) {
    QSettings settings(Common::configFile(), QSettings::IniFormat);
    settings.setValue(key, value);
}

Common::DragMode Common::getDADaltMod()
{
    QSettings settings(Common::configFile(), QSettings::IniFormat);
    return int2dad(settings.value("dad_alt", DM_UNKNOWN).toInt());
}

Common::DragMode Common::getDADctrlMod()
{
    QSettings settings(Common::configFile(), QSettings::IniFormat);
    return int2dad(settings.value("dad_ctrl", DM_COPY).toInt());
}

Common::DragMode Common::getDADshiftMod()
{
    QSettings settings(Common::configFile(), QSettings::IniFormat);
    return int2dad(settings.value("dad_shift", DM_MOVE).toInt());
}

Common::DragMode Common::getDefaultDragAndDrop()
{
    QSettings settings(Common::configFile(), QSettings::IniFormat);
    return int2dad(settings.value("dad", DM_MOVE).toInt());
}

QString Common::getDeviceForDir(QString dir)
{
    QFile mtab("/etc/mtab");
    if (!mtab.open(QIODevice::ReadOnly)) { return QString(); }
    QTextStream ts(&mtab);
    QString root;
    QVector<QStringList> result;
    QStringList entries = ts.readAll().split("\n", Qt::SkipEmptyParts);
    for (int i=0;i<entries.length();++i) {
        QString line = entries.at(i);
        QStringList info = line.split(" ", Qt::SkipEmptyParts);
        if (info.size()>=2) {
            QString dev = info.at(0);
            QString mnt = info.at(1);
            if (mnt == "/") {
                root = dev;
                continue;
            }
            if (dir.startsWith(mnt)) { result.append(QStringList() << dev << mnt); }
        }
    }
    mtab.close();

    if (result.size()==0) { return root; }
    if (result.size()==1) { return result.at(0).at(0); }
    if (result.size()>1) {
        int lastDevCount = 0;
        QString lastDevice;
        for (int i=0;i<result.size();++i) {
            QStringList device = result.at(i);
            QStringList devCount = device.at(1).split("/");
            if (devCount.size()>lastDevCount) {
                lastDevCount = devCount.size();
                lastDevice = device.at(0);
            }
        }
        return lastDevice;
    }
    return QString();
}

QPalette Common::darkTheme()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(64,66,68));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(46,47,48));
    palette.setColor(QPalette::AlternateBase, QColor(64,66,68));
    //palette.setColor(QPalette::ToolTipBase, Qt::white);
    //palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Link, Qt::white);
    palette.setColor(QPalette::LinkVisited, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::black);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(64,66,68));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Highlight, QColor(28,28,29));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
    return palette;
}

QStringList Common::iconPaths(QString appPath)
{
    QStringList iconsPath = QIcon::themeSearchPaths();
    iconsPath << iconLocations(appPath);
    /*QString iconsHomeLocal = QString("%1/.local/share/icons").arg(QDir::homePath());
    QString iconsHome = QString("%1/.icons").arg(QDir::homePath());
    if (QFile::exists(iconsHomeLocal) && !iconsPath.contains(iconsHomeLocal)) { iconsPath.prepend(iconsHomeLocal); }
    if (QFile::exists(iconsHome) && !iconsPath.contains(iconsHome)) { iconsPath.prepend(iconsHome); }
    iconsPath << QString("%1/../share/icons").arg(appPath);*/
    iconsPath.removeDuplicates();
    return  iconsPath;
}

QVector<QStringList> Common::getDefaultActions()
{
    QVector<QStringList> result;
    result.append(QStringList()<< "tar.gz,tar.bz2,tar.xz,tar,tgz,tbz,tbz2,txz" << "Extract tar here ..." << "package-x-generic" << "tar xvf %F");
    result.append(QStringList()<< "7z" << "Extract 7z here ..." << "package-x-generic" << "7za x %F");
    result.append(QStringList()<< "rar" << "Extract rar here ..." << "package-x-generic" << "unrar x %F");
    result.append(QStringList()<< "zip" << "Extract zip here ..." << "package-x-generic" << "unzip %F");
    result.append(QStringList()<< "gz" << "Extract gz here ..." << "package-x-generic" << "gunzip --keep %F");
    result.append(QStringList()<< "bz2" << "Extract bz2 here ..." << "package-x-generic" << "bunzip2 --keep %F");
    result.append(QStringList()<< "xz" << "Extract xz here ..." << "package-x-generic" << "xz -d --keep %F");
    result.append(QStringList()<< "*" << "Compress to tar.gz" << "package-x-generic" << "tar cvvzf %n.tar.gz %F");
    result.append(QStringList()<< "*" << "Compress to tar.bz2" << "package-x-generic" << "tar cvvjf %n.tar.bz2 %F");
    result.append(QStringList()<< "*" << "Compress to tar.xz" << "package-x-generic" << "tar cvvJf %n.tar.xz %F");
    result.append(QStringList()<< "*" << "Compress to zip" << "package-x-generic" << "zip -r %n.zip %F");
    return result;
}

QString Common::formatSize(qint64 num)
{
    QString total;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if (num >= tb) { total = QString("%1TB").arg(QString::number(qreal(num) / tb, 'f', 2)); }
    else if (num >= gb) { total = QString("%1GB").arg(QString::number(qreal(num) / gb, 'f', 2)); }
    else if (num >= mb) { total = QString("%1MB").arg(QString::number(qreal(num) / mb, 'f', 1)); }
    else if (num >= kb) { total = QString("%1KB").arg(QString::number(qreal(num) / kb,'f', 1)); }
    else { total = QString("%1 bytes").arg(num); }

    return total;
}

QString Common::getDriveInfo(QString path)
{
#ifdef __NetBSD__
    struct statvfs info;
    statvfs(path.toLocal8Bit(), &info);
#else
    struct statfs info;
    statfs(path.toLocal8Bit(), &info);
#endif
    if(info.f_blocks == 0) return "";

    return QString("%1  /  %2  (%3%)").arg(formatSize((qint64) (info.f_blocks - info.f_bavail)*info.f_bsize))
            .arg(formatSize((qint64) info.f_blocks*info.f_bsize))
            .arg((info.f_blocks - info.f_bavail)*100/info.f_blocks);
}

QString Common::getXdgCacheHome()
{
    QString result = qgetenv("XDG_CACHE_HOME");
    if (result.isEmpty()) {
        result = QString("%1/.cache").arg(QDir::homePath());
    }
    return result;
}

QString Common::getThumbnailHash(const QString &filename)
{
    if (!filename.isEmpty()) {
        QString filenameString = QUrl::fromUserInput(filename).toString();
        QString hash = QString(QCryptographicHash::hash(filenameString.toUtf8(),
                                                        QCryptographicHash::Md5).toHex());
        return hash;
    }
    return QString();
}

QString Common::hasThumbnail(const QString &filename)
{
    if (QFile::exists(filename)) {
        QString imagePath = QString("%1/thumbnails/normal/%2.png")
                            .arg(getXdgCacheHome())
                            .arg(getThumbnailHash(filename));
        if (QFile::exists(imagePath)) { return imagePath; }
    }
    return QString();
}

QString Common::getTempPath()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    qDebug() << "Temp folder" << path;
    QDir dir(path);
    if (!dir.exists()) {
        if (!dir.mkpath(path)) { return QString(); }
    }
    return path;
}

QString Common::getTempClipboardFile()
{
    const QString path = getTempPath();
    if (path.isEmpty()) { return QString(); }
    return QString("%1/clipboard.tmp").arg(path);
}

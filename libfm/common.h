#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QIcon>
#include <QDirIterator>
#include <QRegExp>
#include <QTextStream>
#include <QMap>
#include <QMapIterator>
#include <QDirIterator>

#define FM_MAJOR 6

#define BOOKMARK_PATH Qt::UserRole+1
#define BOOKMARK_ICON Qt::UserRole+2
#define BOOKMARKS_AUTO Qt::UserRole+3

class Common
{
public:
    static QString configDir()
    {
        QString dir = QString("%1/.config/%2%3").arg(QDir::homePath()).arg(APP).arg(FM_MAJOR);
        if (!QFile::exists(dir)) {
            QDir makedir(dir);
            if (!makedir.mkpath(dir)) { dir.clear(); }
        }
        return dir;
    }
    static QString trashDir()
    {
        QString dir = QString("%1/.local/share/Trash").arg(QDir::homePath());
        if (!QFile::exists(dir)) {
            QDir makedir(dir);
            if (!makedir.mkpath(dir)) { dir.clear(); }
        }
        return dir;
    }
    static QStringList iconLocations()
    {
        QStringList result;
        result << QString("%1/.local/share/icons").arg(QDir::homePath());
        result << QString("%1/../share/icons").arg(qApp->applicationFilePath());
        result << "/usr/share/icons" << "/usr/local/share/icons";
        return result;
    }
    static QStringList pixmapLocations()
    {
        QStringList result;
        result << QString("%1/.local/share/pixmaps").arg(QDir::homePath());
        result << QString("%1/../share/pixmaps").arg(qApp->applicationFilePath());
        result << "/usr/share/pixmaps" << "/usr/local/share/pixmaps";
        return result;
    }
    static QStringList applicationLocations()
    {
        QStringList result;
        result << QString("%1/.local/share/applications").arg(QDir::homePath());
        result << QString("%1/../share/applications").arg(qApp->applicationFilePath());
        result << "/usr/share/applications" << "/usr/local/share/applications";
        return result;
    }
    static QStringList mimeGlobLocations()
    {
        QStringList result;
        result << QString("%1/.local/share/mime/globs").arg(QDir::homePath());
        result << QString("%1/../share/mime/globs").arg(qApp->applicationFilePath());
        result << "/usr/share/mime/globs" << "/usr/local/share/mime/globs";
        return result;
    }
    static QStringList mimeGenericLocations()
    {
        QStringList result;
        result << QString("%1/.local/share/mime/generic-icons").arg(QDir::homePath());
        result << QString("%1/../share/mime/generic-icons").arg(qApp->applicationFilePath());
        result << "/usr/share/mime/generic-icons" << "/usr/local/share/mime/generic-icons";
        return result;
    }
    static QStringList mimeTypeLocations()
    {
        QStringList result;
        result << QString("%1/.local/share/mime/types").arg(QDir::homePath());
        result << QString("%1/../share/mime/types").arg(qApp->applicationFilePath());
        result << "/usr/share/mime/types" << "/usr/local/share/mime/types";
        return result;
    }
    static QString getDesktopIcon(QString desktop)
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
    static QString findIconInDir(QString dir, QString icon)
    {
        QString result;
        if (dir.isEmpty() || icon.isEmpty()) { return result; }

        QString theme = QIcon::themeName();
        if (theme.isEmpty()) { theme = "hicolor"; }

        QStringList iconSizes;
        iconSizes << "128" << "64" << "48" << "32" << "22" << "16";

        // theme
        QDirIterator it(QString("%1/%2").arg(dir).arg(theme), QStringList() << "*.png" << "*.jpg" << "*.xpm", QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString found = it.next();
            if (found.split("/").takeLast().split(".").takeFirst()==icon) {
                for (int i=0;i<iconSizes.size();++i) {
                    QString hasFile = found.replace(QRegExp("/[.*]x[.*]/"),QString("/%1x%1/").arg(iconSizes.at(i)));
                    if (QFile::exists(hasFile)) { return hasFile; }
                }
                return found;
            }
        }
        // hicolor
        if (theme!="hicolor") {
            QDirIterator hicolor(QString("%1/%2").arg(dir).arg("hicolor"), QStringList() << "*.png" << "*.jpg" << "*.xpm", QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (hicolor.hasNext()) {
                QString found = hicolor.next();
                if (found.split("/").takeLast().split(".").takeFirst()==icon) {
                    for (int i=0;i<iconSizes.size();++i) {
                        QString hasFile = found.replace(QRegExp("/[.*]x[.*]/"),QString("/%1x%1/").arg(iconSizes.at(i)));
                        if (QFile::exists(hasFile)) { return hasFile; }
                    }
                    return found;
                }
            }
        }
        // pixmaps
        for (int i=0;i<pixmapLocations().size();++i) {
            QDirIterator pixmaps(pixmapLocations().at(i), QStringList() << "*.png" << "*.jpg" << "*.xpm", QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (pixmaps.hasNext()) {
                QString found = pixmaps.next();
                if (found.split("/").takeLast().split(".").takeFirst()==icon) { return found; }
            }
        }
        return result;
    }
    static QString findIcon(QString fileIcon)
    {
        QString result;
        if (fileIcon.isEmpty()) { return result; }
        for (int i=0;i<iconLocations().size();++i) {
            QString icon = findIconInDir(iconLocations().at(i), fileIcon);
            if (!icon.isEmpty()) { return icon; }
        }
        return result;
    }
    static QString findApplication(QString desktopFile)
    {
        QString result;
        if (desktopFile.isEmpty()) { return result; }
        for (int i=0;i<applicationLocations().size();++i) {
            QDirIterator it(applicationLocations().at(i), QStringList("*.desktop"), QDir::Files|QDir::NoDotAndDotDot);
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
    static QString findApplicationIcon(QString app)
    {
        QString result;
        QString desktop = findApplication(app);
        if (desktop.isEmpty()) { return result; }
        QString icon = getDesktopIcon(desktop);
        if (icon.isEmpty()) { return result; }
        result = findIcon(icon);
        return result;
    }
    static QMap<QString, QString> readGlobMimesFromFile(QString filename)
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
    static QMap<QString, QString> getMimesGlobs()
    {
        QMap<QString, QString> map;
        for (int i=0;i<mimeGlobLocations().size();++i) {
            QMapIterator<QString, QString> globs(readGlobMimesFromFile(mimeGlobLocations().at(i)));
            while (globs.hasNext()) {
                globs.next();
                map[globs.key()] = globs.value();
            }

        }
        return map;
    }
    static QMap<QString, QString> readGenericMimesFromFile(QString filename)
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
    static QMap<QString, QString> getMimesGeneric()
    {
        QMap<QString, QString> map;
        for (int i=0;i<mimeGenericLocations().size();++i) {
            QMapIterator<QString, QString> generic(readGenericMimesFromFile(mimeGenericLocations().at(i)));
            while (generic.hasNext()) {
                generic.next();
                map[generic.key()] = generic.value();
            }

        }
        return map;
    }
    static QStringList getPixmaps()
    {
        QStringList result;
        for (int i=0;i<pixmapLocations().size();++i) {
            QDir pixmaps(pixmapLocations().at(i), "",  0, QDir::Files | QDir::NoDotAndDotDot);
            for (int i=0;i<pixmaps.entryList().size();++i) {
                result << QString("%1/%2").arg(pixmaps.absolutePath()).arg(pixmaps.entryList().at(i));
            }
        }
        return result;
    }
    static QStringList getMimeTypes()
    {
        QStringList result;
        for (int i=0;i<mimeTypeLocations().size();++i) {
            QFile file(mimeTypeLocations().at(i));
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
    static QStringList getIconThemes()
    {
        QStringList result;
        for (int i=0;i<iconLocations().size();++i) {
            QDirIterator it(iconLocations().at(i), QDir::Dirs | QDir::NoDotAndDotDot);
            while (it.hasNext()) {
                it.next();
                //qDebug() << it.fileName() << it.filePath();
                if (QFile::exists(it.filePath()+"/index.theme")) { result.append(it.fileName()); }
            }
        }
        return result;
    }
    static bool removeFileCache()
    {
        QFile cache(QString("%1/file.cache").arg(Common::configDir()));
        if (cache.exists()) {
            return cache.remove();
        }
        return false;
    }
    static bool removeFolderCache()
    {
        QFile cache(QString("%1/folder.cache").arg(Common::configDir()));
        if (cache.exists()) {
            return cache.remove();
        }
        return false;
    }
    static bool removeThumbsCache()
    {
        QFile cache(QString("%1/thumbs.cache").arg(Common::configDir()));
        if (cache.exists()) {
            return cache.remove();
        }
        return false;
    }
};

#endif // COMMON_H

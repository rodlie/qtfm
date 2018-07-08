#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QIcon>
#include <QDirIterator>
#include <QRegExp>
#include <QTextStream>
#include <QMap>
#include <QMapIterator>
#include <QDirIterator>
#include <QSettings>
#include <QPalette>

#ifndef APP
#define APP "qtfm"
#endif
#define FM_MAJOR 6

#define FM_SERVICE "org.dracolinux.qtfm"
#define FM_PATH "/qtfm"

#define BOOKMARK_PATH Qt::UserRole+1
#define BOOKMARK_ICON Qt::UserRole+2
#define BOOKMARKS_AUTO Qt::UserRole+3

class Common
{
public:
    enum DragMode {
      DM_UNKNOWN = 0,
      DM_COPY,
      DM_MOVE,
      DM_LINK
    };
    static QString configDir()
    {
        QString dir = QString("%1/.config/%2%3").arg(QDir::homePath()).arg(APP).arg(FM_MAJOR);
        if (!QFile::exists(dir)) {
            QDir makedir(dir);
            if (!makedir.mkpath(dir)) { dir.clear(); }
        }
        return dir;
    }
    static QString configFile()
    {
        return QString("%1/%2%3.conf").arg(configDir()).arg(APP).arg(FM_MAJOR);
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
    static QStringList iconLocations(QString appPath)
    {
        QStringList result;
        result << QString("%1/.local/share/icons").arg(QDir::homePath());
        result << QString("%1/../share/icons").arg(appPath);
        result << "/usr/share/icons" << "/usr/local/share/icons";
        return result;
    }
    static QStringList pixmapLocations(QString appPath)
    {
        QStringList result;
        result << QString("%1/.local/share/pixmaps").arg(QDir::homePath());
        result << QString("%1/../share/pixmaps").arg(appPath);
        result << "/usr/share/pixmaps" << "/usr/local/share/pixmaps";
        return result;
    }
    static QStringList applicationLocations(QString appPath)
    {
        QStringList result;
        result << QString("%1/.local/share/applications").arg(QDir::homePath());
        result << QString("%1/../share/applications").arg(appPath);
        result << "/usr/share/applications" << "/usr/local/share/applications";
        return result;
    }
    static QStringList mimeGlobLocations(QString appPath)
    {
        QStringList result;
        result << QString("%1/.local/share/mime/globs").arg(QDir::homePath());
        result << QString("%1/../share/mime/globs").arg(appPath);
        result << "/usr/share/mime/globs" << "/usr/local/share/mime/globs";
        return result;
    }
    static QStringList mimeGenericLocations(QString appPath)
    {
        QStringList result;
        result << QString("%1/.local/share/mime/generic-icons").arg(QDir::homePath());
        result << QString("%1/../share/mime/generic-icons").arg(appPath);
        result << "/usr/share/mime/generic-icons" << "/usr/local/share/mime/generic-icons";
        return result;
    }
    static QStringList mimeTypeLocations(QString appPath)
    {
        QStringList result;
        result << QString("%1/.local/share/mime/types").arg(QDir::homePath());
        result << QString("%1/../share/mime/types").arg(appPath);
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
    static QString findIconInDir(QString appPath, QString theme, QString dir, QString icon)
    {
        QString result;
        if (dir.isEmpty() || icon.isEmpty()) { return result; }

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
        QStringList pixs = pixmapLocations(appPath);
        for (int i=0;i<pixs.size();++i) {
            QDirIterator pixmaps(pixs.at(i), QStringList() << "*.png" << "*.jpg" << "*.xpm", QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (pixmaps.hasNext()) {
                QString found = pixmaps.next();
                if (found.split("/").takeLast().split(".").takeFirst()==icon) { return found; }
            }
        }
        return result;
    }
    static QString findIcon(QString appPath, QString theme, QString fileIcon)
    {
        QString result;
        if (fileIcon.isEmpty()) { return result; }
        QStringList icons = iconLocations(appPath);
        for (int i=0;i<icons.size();++i) {
            QString icon = findIconInDir(appPath, theme, icons.at(i), fileIcon);
            if (!icon.isEmpty()) { return icon; }
        }
        return result;
    }
    static QString findApplication(QString appPath, QString desktopFile)
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
    static QStringList findApplications(QString filename)
    {
        QStringList result;
        if (filename.isEmpty()) { return result; }
        QString path = qgetenv("PATH");
        QStringList paths = path.split(":", QString::SkipEmptyParts);
        for (int i=0;i<paths.size();++i) {
            QDirIterator it(paths.at(i), QStringList("*"), QDir::Files|QDir::Executable|QDir::NoDotAndDotDot);
            while (it.hasNext()) {
                QString found = it.next();
                if (found.split("/").takeLast().startsWith(filename)) {
                    result << found;
                }
            }
        }
        return result;
    }
    static QString findApplicationIcon(QString appPath, QString theme, QString app)
    {
        QString result;
        QString desktop = findApplication(appPath, app);
        if (desktop.isEmpty()) { return result; }
        QString icon = getDesktopIcon(desktop);
        if (icon.isEmpty()) { return result; }
        result = findIcon(appPath, theme, icon);
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
    static QMap<QString, QString> getMimesGlobs(QString appPath)
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
    static QMap<QString, QString> getMimesGeneric(QString appPath)
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
    static QStringList getPixmaps(QString appPath)
    {
        QStringList result;
        QStringList pixs = pixmapLocations(appPath);
        for (int i=0;i<pixs.size();++i) {
            QDir pixmaps(pixs.at(i), "",  0, QDir::Files | QDir::NoDotAndDotDot);
            for (int i=0;i<pixmaps.entryList().size();++i) {
                result << QString("%1/%2").arg(pixmaps.absolutePath()).arg(pixmaps.entryList().at(i));
            }
        }
        return result;
    }
    static QStringList getMimeTypes(QString appPath)
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
    static QStringList getIconThemes(QString appPath)
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
    static void setupIconTheme(QString appFilePath)
    {
        QString temp = QIcon::themeName();
        if (temp.isEmpty()  || temp == "hicolor") {
            qDebug() << "checking for icon theme in settings" << Common::configFile();
            QSettings settings(Common::configFile(), QSettings::IniFormat);
            temp = settings.value("fallbackTheme").toString();
        }
        if(temp.isEmpty() || temp == "hicolor") {
            if(QFile::exists(QDir::homePath() + "/" + ".gtkrc-2.0")) { // try gtk-2.0
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
            if(temp.isNull()) {
                qDebug() << "checking for icon theme in static fallback";
                QStringList themes;
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
    static DragMode int2dad(int value)
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
    static QVariant readSetting(QString key) {
        QSettings settings(Common::configFile(), QSettings::IniFormat);
        return settings.value(key);
    }
    static void writeSetting(QString key, QVariant value) {
        QSettings settings(Common::configFile(), QSettings::IniFormat);
        settings.setValue(key, value);
    }
    static DragMode getDADaltMod()
    {
        QSettings settings(Common::configFile(), QSettings::IniFormat);
        return int2dad(settings.value("dad_alt").toInt());
    }
    static DragMode getDADctrlMod()
    {
        QSettings settings(Common::configFile(), QSettings::IniFormat);
        return int2dad(settings.value("dad_ctrl").toInt());
    }
    static DragMode getDADshiftMod()
    {
        QSettings settings(Common::configFile(), QSettings::IniFormat);
        return int2dad(settings.value("dad_shift").toInt());
    }
    static DragMode getDefaultDragAndDrop()
    {
        QSettings settings(Common::configFile(), QSettings::IniFormat);
        return int2dad(settings.value("dad").toInt());
    }
    static QString getDeviceForDir(QString dir)
    {
        QFile mtab("/etc/mtab");
        if (!mtab.open(QIODevice::ReadOnly)) { return QString(); }
        QTextStream ts(&mtab);
        QString root;
        QVector<QStringList> result;
        while(!ts.atEnd()) {
            QString line = ts.readLine();
            QStringList info = line.split(" ", QString::SkipEmptyParts);
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
    static QPalette darkTheme()
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
};

#endif // COMMON_H

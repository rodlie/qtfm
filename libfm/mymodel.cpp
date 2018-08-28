/****************************************************************************
* This file is part of qtFM, a simple, fast file manager.
* Copyright (C) 2010,2011,2012 Wittfella
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
* Contact e-mail: wittfella@qtfm.org
*
****************************************************************************/

//#include <mainwindow.h>
#include "mymodel.h"
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <QApplication>
#include <QMessageBox>
#include "fileutils.h"

/**
 * @brief Creates file system model
 * @param realMime
 * @param mimeUtils
 */
myModel::myModel(bool realMime, MimeUtils *mimeUtils) {

  // thumbs generator
  thumbGenerator = new Thumbs();
  connect(thumbGenerator, SIGNAL(generatedIcon(QString,QByteArray)),
          this, SLOT(handleNewThumb(QString,QByteArray)));

  // Stores mime utils
  mimeUtilsPtr = mimeUtils;

  // Initialization
  mimeGeneric = new QHash<QString,QString>;
  mimeGlob = new QHash<QString,QString>;
  mimeIcons = new QHash<QString,QIcon>;
  folderIcons = new QHash<QString,QIcon>;
  thumbs = new QHash<QString,QByteArray>;
  icons = new QCache<QString,QIcon>;
  icons->setMaxCost(500);

  // Loads cached mime icons
  QFile fileIcons(QString("%1/file.cache").arg(Common::configDir()));
  if (fileIcons.open(QIODevice::ReadOnly)) {
      QDataStream out(&fileIcons);
      out >> *mimeIcons;
      fileIcons.close();
  }

  // Loads folder cache
  fileIcons.setFileName(QString("%1/folder.cache").arg(Common::configDir()));
  if (fileIcons.open(QIODevice::ReadOnly)) {
      QDataStream out(&fileIcons);
      out.setDevice(&fileIcons);
      out >> *folderIcons;
      fileIcons.close();
  }

  // Create root item
  rootItem = new myModelItem(QFileInfo("/"), new myModelItem(QFileInfo(), 0));
  currentRootPath = "/";
  QDir root("/");
  QFileInfoList drives = root.entryInfoList( QDir::AllEntries | QDir::Files
                                            | QDir::Hidden | QDir::System
                                            | QDir::NoDotAndDotDot);

  // Create item per each drive
  foreach (QFileInfo drive, drives) {
    new myModelItem(drive, rootItem);
  }

  rootItem->walked = true;
  rootItem = rootItem->parent();

  iconFactory = new QFileIconProvider();

  inotifyFD = inotify_init();
  notifier = new QSocketNotifier(inotifyFD, QSocketNotifier::Read, this);
  connect(notifier, SIGNAL(activated(int)), this, SLOT(notifyChange()));
  connect(&eventTimer,SIGNAL(timeout()),this,SLOT(eventTimeout()));

  realMimeTypes = realMime;
}
//---------------------------------------------------------------------------

/**
 * @brief Deletes model of file system
 */
myModel::~myModel() {
  delete rootItem;
  delete iconFactory;
  thumbGenerator->deleteLater();
}
//---------------------------------------------------------------------------

/**
 * @brief Deletes icon cache
 */
void myModel::clearIconCache() {
  folderIcons->clear();
  mimeIcons->clear();
  QFile(QString("%1/folder.cache").arg(Common::configDir())).remove();
  QFile(QString("%1/file.cache").arg(Common::configDir())).remove();
}
//---------------------------------------------------------------------------

void myModel::handleNewThumb(QString item, QByteArray icon)
{
    //qDebug() << "handle new thumb" << item << icon.size();
    if (icon.isEmpty()) { return; }
    thumbs->insert(item, icon);
    emit thumbUpdate(index(item));

    if (item.split("/").takeLast() == lastEventFilename) {
        lastEventFilename.clear();
        QFile fileIcons(QString("%1/thumbs.cache").arg(Common::configDir()));
        if (fileIcons.size() > 10000000) { fileIcons.remove(); }
        else {
            if (fileIcons.open(QIODevice::WriteOnly)) {
                QDataStream out(&fileIcons);
                out.setDevice(&fileIcons);
                out << *thumbs;
                fileIcons.close();
            }
        }
    }
}
//---------------------------------------------------------------------------

/**
 * @brief Sets whether use real mime types or not
 * @param realMimeTypes
 */
void myModel::setRealMimeTypes(bool realMimeTypes) {
  this->realMimeTypes = realMimeTypes;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns true if real mime types are used
 * @return true if real mime types are used
 */
bool myModel::isRealMimeTypes() const {
  return realMimeTypes;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns mime utils
 * @return mime utils
 */
MimeUtils* myModel::getMimeUtils() const {
  return mimeUtilsPtr;
}
//---------------------------------------------------------------------------

QModelIndex myModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid() && parent.column() != 0) { return QModelIndex(); }

    myModelItem *parentItem = static_cast<myModelItem*>(parent.internalPointer());
    if(!parentItem) { parentItem = rootItem; }

    myModelItem *childItem = parentItem->childAt(row);
    if(childItem) { return createIndex(row, column, childItem); }

    return QModelIndex();
}

//---------------------------------------------------------------------------------------
QModelIndex myModel::index(const QString& path) const
{
    myModelItem *item = rootItem->matchPath(path.split(SEPARATOR),0);
    if(item) { return createIndex(item->childNumber(),0,item); }
    return QModelIndex();
}

//---------------------------------------------------------------------------------------
QModelIndex myModel::parent(const QModelIndex &index) const
{
    if(!index.isValid()) { return QModelIndex(); }

    myModelItem *childItem = static_cast<myModelItem*>(index.internalPointer());

    if(!childItem) { return QModelIndex(); }

    myModelItem *parentItem = childItem->parent();

    if (!parentItem || parentItem == rootItem) { return QModelIndex(); }

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

//---------------------------------------------------------------------------------------
bool myModel::isDir(const QModelIndex &index)
{
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item && item != rootItem) { return item->fileInfo().isDir(); }
    return false;
}

//---------------------------------------------------------------------------------------
QFileInfo myModel::fileInfo(const QModelIndex &index)
{
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item) { return item->fileInfo(); }
    return QFileInfo();
}

//---------------------------------------------------------------------------------------
qint64 myModel::size(const QModelIndex &index)
{
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item) { return item->fileInfo().size(); }
    return 0;
}

//---------------------------------------------------------------------------------------
QString myModel::fileName(const QModelIndex &index)
{
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item) { return item->fileName(); }
    return "";
}

//---------------------------------------------------------------------------------------
QString myModel::filePath(const QModelIndex &index)
{
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item) { return item->absoluteFilePath(); }
    return QString();
}

//---------------------------------------------------------------------------------------
QString myModel::getMimeType(const QModelIndex &index)
{
    qDebug() << "myModel getMimeType";
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if (item->mMimeType.isNull()) {
        if (realMimeTypes) { item->mMimeType = mimeUtilsPtr->getMimeType(item->absoluteFilePath()); }
        else {
            if (item->fileInfo().isDir()) item->mMimeType = "folder";
            else item->mMimeType = item->fileInfo().suffix();
            if (item->mMimeType.isNull()) item->mMimeType = "file";
        }
    }
    return item->mMimeType;
}

//---------------------------------------------------------------------------------------
void myModel::notifyChange()
{
    notifier->setEnabled(0);

    int buffSize = 0;
    ioctl(inotifyFD, FIONREAD, (char *) &buffSize);

    QByteArray buffer;
    buffer.resize(buffSize);
    read(inotifyFD,buffer.data(), (size_t) buffSize);
    const char *at = buffer.data();
    const char * const end = at + buffSize;

    while (at < end) {
        const inotify_event *event = reinterpret_cast<const inotify_event *>(at);
        int w = event->wd;
        lastEventFilename = event->name;
        if (eventTimer.isActive()) {
            if (w == lastEventID) {
                eventTimer.start(40);
            } else {
                eventTimer.stop();
                notifyProcess(lastEventID, lastEventFilename);
                lastEventID = w;
                eventTimer.start(40);
            }
        } else {
            lastEventID = w;
            eventTimer.start(40);
        }
        at += sizeof(inotify_event) + event->len;
    }

    notifier->setEnabled(1);
    emit reloadDir();
}

//---------------------------------------------------------------------------------------
void myModel::eventTimeout()
{
    notifyProcess(lastEventID, lastEventFilename);
    eventTimer.stop();
}

//---------------------------------------------------------------------------------------
void myModel::notifyProcess(int eventID, QString fileName)
{
    qDebug() << "notifyProcess" << eventID << fileName;
    if(watchers.contains(eventID)) {
        myModelItem *parent = rootItem->matchPath(watchers.value(eventID).split(SEPARATOR));
        if(parent) {
            parent->dirty = 1;
            QDir dir(parent->absoluteFilePath());
            QFileInfoList all = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
            foreach(myModelItem * child, parent->children()) {
                if(all.contains(child->fileInfo())) {
                    //just remove known items
                    all.removeOne(child->fileInfo());
                } else {
                    //must of been deleted, remove from model
                    if(child->fileInfo().isDir()) {
                        int wd = watchers.key(child->absoluteFilePath());
                        inotify_rm_watch(inotifyFD,wd);
                        watchers.remove(wd);
                    }
                    beginRemoveRows(index(parent->absoluteFilePath()),child->childNumber(),child->childNumber());
                    parent->removeChild(child);
                    endRemoveRows();
                }
            }
            foreach(QFileInfo one, all) { //only new items left in list
                beginInsertRows(index(parent->absoluteFilePath()),parent->childCount(),parent->childCount());
                new myModelItem(one,parent);
                endInsertRows();
            }
        }
    } else {
        inotify_rm_watch(inotifyFD,eventID);
        watchers.remove(eventID);
    }
    if (!fileName.isEmpty() && showThumbs) {
        lastEventFilename = fileName;
    }
    emit reloadDir();
}

//---------------------------------------------------------------------------------
void myModel::addWatcher(myModelItem *item)
{
    qDebug() << "addWatcher" << item->absoluteFilePath();
    while(item != rootItem) {
        watchers.insert(inotify_add_watch(inotifyFD, item->absoluteFilePath().toLocal8Bit(), IN_CREATE | IN_MODIFY | IN_MOVE | IN_DELETE),item->absoluteFilePath()); //IN_ONESHOT | IN_ALL_EVENTS)
        item->watched = 1;
        item = item->parent();
    }
}

//---------------------------------------------------------------------------------
bool myModel::setRootPath(const QString& path)
{
    currentRootPath = path;

    myModelItem *item = rootItem->matchPath(path.split(SEPARATOR));

    if (item == NULL) {
        QMessageBox::warning(NULL, tr("No such directory"), tr("Directory requested does not exists."));
        return false;
    }

    if(item->watched == 0) { addWatcher(item); }

    if(item->walked == 0) {
        populateItem(item);
        return false;
    } else {
        if(item->dirty) { //model is up to date, but view needs to be invalidated
            item->dirty = 0;
            return true;
        }
    }
    return false;
}

QString myModel::getRootPath()
{
    return currentRootPath;
}

//---------------------------------------------------------------------------------------
bool myModel::canFetchMore (const QModelIndex & parent) const
{
    myModelItem *item = static_cast<myModelItem*>(parent.internalPointer());

    if(item) {
        if(item->walked) { return false; }
    }

    return true;

}

//---------------------------------------------------------------------------------------
void myModel::fetchMore (const QModelIndex & parent)
{
    myModelItem *item = static_cast<myModelItem*>(parent.internalPointer());

    if(item) {
        populateItem(item);
        emit dataChanged(parent,parent);
    }

    return;
}

//---------------------------------------------------------------------------------------
void myModel::populateItem(myModelItem *item)
{
    if (item == NULL) { return; }
    item->walked = 1;

    QDir dir(item->absoluteFilePath());
    QFileInfoList all = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);

    foreach(QFileInfo one, all) { new myModelItem(one,item); }
}

//---------------------------------------------------------------------------------
int myModel::columnCount(const QModelIndex &parent) const
{
    return (parent.column() > 0) ? 0 : 5;
}

//---------------------------------------------------------------------------------------
int myModel::rowCount(const QModelIndex &parent) const
{
    myModelItem *item = static_cast<myModelItem*>(parent.internalPointer());
    if(item) { return item->childCount(); }
    return rootItem->childCount();
}

//---------------------------------------------------------------------------------
void myModel::refresh()
{
    myModelItem *item = rootItem->matchPath(QStringList("/"));

    //free all inotify watches
    foreach(int w, watchers.keys()) { inotify_rm_watch(inotifyFD,w); }
    watchers.clear();

    beginResetModel();
    item->clearAll();
    endResetModel();
}

//---------------------------------------------------------------------------------
void myModel::update()
{
    myModelItem *item = rootItem->matchPath(currentRootPath.split(SEPARATOR));
    foreach(myModelItem *child, item->children()) { child->refreshFileInfo(); }
}

//---------------------------------------------------------------------------------
void myModel::refreshItems()
{
    myModelItem *item = rootItem->matchPath(currentRootPath.split(SEPARATOR));
    if (item == NULL) { return; }
    item->clearAll();
    populateItem(item);
}

//---------------------------------------------------------------------------------
QModelIndex myModel::insertFolder(QModelIndex parent)
{
    myModelItem *item = static_cast<myModelItem*>(parent.internalPointer());

    int num = 0;
    QString name;

    do {
        num++;
        name = QString("new_folder%1").arg(num);
    }
    while(item->hasChild(name));


    QDir temp(currentRootPath);
    if(!temp.mkdir(name)) { return QModelIndex(); }

    beginInsertRows(parent,item->childCount(),item->childCount());
    new myModelItem(QFileInfo(currentRootPath + "/" + name),item);
    endInsertRows();

    return index(item->childCount() - 1,0,parent);
}

//---------------------------------------------------------------------------------
QModelIndex myModel::insertFile(QModelIndex parent)
{
    myModelItem *item = static_cast<myModelItem*>(parent.internalPointer());

    int num = 0;
    QString name;

    do {
        num++;
        name = QString("new_file%1").arg(num);
    }
    while (item->hasChild(name));


    QFile temp(currentRootPath + "/" + name);
    if(!temp.open(QIODevice::WriteOnly)) { return QModelIndex(); }
    temp.close();

    beginInsertRows(parent,item->childCount(),item->childCount());
    new myModelItem(QFileInfo(temp),item);
    endInsertRows();

    return index(item->childCount()-1,0,parent);
}

//---------------------------------------------------------------------------------
Qt::DropActions myModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

//---------------------------------------------------------------------------------
QStringList myModel::mimeTypes() const
{
    return QStringList("text/uri-list");
}

//---------------------------------------------------------------------------------
QMimeData * myModel::mimeData(const QModelIndexList & indexes) const
{
    QMimeData *data = new QMimeData();

    QList<QUrl> files;

    foreach(QModelIndex index, indexes) {
        myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
        QUrl url = QUrl::fromLocalFile(item->absoluteFilePath());
        if (!files.contains(url)) { files.append(url); }
    }

    data->setUrls(files);
    return data;
}


//---------------------------------------------------------------------------------
void myModel::cacheInfo()
{
    QFile fileIcons(QString("%1/file.cache").arg(Common::configDir()));
    if (fileIcons.open(QIODevice::WriteOnly)) {
        QDataStream out(&fileIcons);
        out << *mimeIcons;
        fileIcons.close();
    }

    fileIcons.setFileName(QString("%1/folder.cache").arg(Common::configDir()));
    if (fileIcons.open(QIODevice::WriteOnly)) {
        QDataStream out(&fileIcons);
        out.setDevice(&fileIcons);
        out << *folderIcons;
        fileIcons.close();
    }

    if(thumbs->count() > thumbCount) {
        fileIcons.setFileName(QString("%1/thumbs.cache").arg(Common::configDir()));
        if(fileIcons.size() > 10000000) { fileIcons.remove(); }
        else {
            if (fileIcons.open(QIODevice::WriteOnly)) {
                QDataStream out(&fileIcons);
                out.setDevice(&fileIcons);
                out << *thumbs;
                fileIcons.close();
            }
        }
    }
}

//---------------------------------------------------------------------------

/**
 * @brief Sets indicator wthether show thumbnails of pictures
 * @param icons
 */
void myModel::setMode(bool icons) {
  showThumbs = icons;
}
//---------------------------------------------------------------------------

/**
 * @brief Loads mime types
 */
void myModel::loadMimeTypes() const {
    QMapIterator<QString, QString> globs(Common::getMimesGlobs(qApp->applicationFilePath()));
    while(globs.hasNext()) {
        globs.next();
        mimeGlob->insert(globs.value(), globs.key());
    }
    QMapIterator<QString, QString> generic(Common::getMimesGeneric(qApp->applicationFilePath()));
    while(generic.hasNext()) {
        generic.next();
        mimeGeneric->insert(generic.key(), generic.value());
    }
}
//---------------------------------------------------------------------------

/**
 * @brief Loads thumbnails
 * @param indexes
 */
void myModel::loadThumbs(QModelIndexList indexes) {

  // Files that should be thumbnailed
  QMap<QString, QString> files;

  // Add files we want thumbnails from
  // TODO: should be a setting!
  foreach (QModelIndex item, indexes) {
      QString mimetype = mimeUtilsPtr->getMimeType(fileName(item));
      if (mimetype.startsWith("image/") ||
          mimetype.startsWith("video/") ||
          mimetype == "application/pdf")
      {
          files[filePath(item)] = mimetype;
      }
  }

  // Loads thumbnails from cache
  if (files.count()>0) {
    if (thumbs->count() == 0) {
      qDebug() << "try to load thumb cache ...";
      QFile fileIcons(QString("%1/thumbs.cache").arg(Common::configDir()));
      if (fileIcons.open(QIODevice::ReadOnly)) {
          QDataStream out(&fileIcons);
          out >> *thumbs;
          fileIcons.close();
      }
      thumbCount = thumbs->count();
    }
    // generate thumbs
    QMapIterator<QString, QString> i(files);
    while (i.hasNext()) {
        i.next();
        QString item = i.key();
        if (!thumbs->contains(item)) {
            qDebug() << "GENERATE thumb for" << item;
            thumbGenerator->generateIcon(item, i.value());
        } else if (item.split("/").takeLast() == lastEventFilename &&
                   !lastEventFilename.isEmpty()) {
            qDebug() << "UPDATE thumb for" << item;
            thumbGenerator->generateIcon(item, i.value());
        } else {
            emit thumbUpdate(index(item));
        }
    }
  }
}

//---------------------------------------------------------------------------

/**
 * @brief Creates thumbnail for given item
 * @param item
 * @return thumbnail
 */
/*QByteArray myModel::getThumb(QString item)
#ifndef NO_MAGICK
{
    QByteArray result;
    qDebug() << "generate thumbnail for" << item;
    try {
        Magick::Image background(Magick::Geometry(128, 128), Magick::Color("black"));
#ifdef MAGICK7
        background.alpha(true);
#else
        background.matte(true);
#endif
        background.backgroundColor(background.pixelColor(0,0));
        background.transparent(background.pixelColor(0,0));

        Magick::Image thumb;
        QString filename = item;
        QStringList videos = Common::videoFormats();
        QFileInfo fileinfo(filename);
        if (videos.contains(fileinfo.suffix(), Qt::CaseInsensitive)) {
            thumb.read(QString("%1[100]").arg(filename).toUtf8().data());
        } else {
            thumb.read(filename.toUtf8().data());
        }
        thumb.scale(Magick::Geometry(128, 128));
        if (thumb.depth()>8) { thumb.depth(8); }
        int offsetX = 0;
        int offsetY = 0;
        if (thumb.columns()<background.columns()) {
            offsetX = (background.columns()-thumb.columns())/2;
        }
        if (thumb.rows()<background.rows()) {
            offsetY = (background.rows()-thumb.rows())/2;
        }
        background.composite(thumb, offsetX, offsetY, MagickCore::OverCompositeOp);
        background.magick("BMP");
        Magick::Blob buffer;
        background.write(&buffer);
        result = QByteArray((char*)buffer.data(), buffer.length());
    }
    catch(Magick::Error &error_ ) {
        qWarning() << error_.what();
    }
    catch(Magick::Warning &warn_ ) {
        qWarning() << warn_.what();
    }
    return result;
}
#else
{
  qDebug() << "generate thumbnail for" << item;

  // Thumbnail image
  QImage theThumb, background;
  QImageReader pic(item);
  int w = pic.size().width();
  int h = pic.size().height();

  // Background
  background = QImage(128, 128, QImage::Format_RGB32);
  background.fill(QApplication::palette().color(QPalette::Base).rgb());

  // Scale image and create its shadow template (background.png)
  if (w > 128 || h > 128) {
    pic.setScaledSize(QSize(123, 93));
    QImage temp = pic.read();
    theThumb.load(":/fm/images/background.png");
    QPainter painter(&theThumb);
    painter.drawImage(QPoint(0, 0), temp);
  } else {
    pic.setScaledSize(QSize(64, 64));
    theThumb = pic.read();
  }

  // Draw thumbnail picture
  QPainter painter(&background);
  painter.drawImage(QPoint((123 - theThumb.width()) / 2,
                           (115 - theThumb.height()) / 2), theThumb);

  // Write it to buffer
  QBuffer buffer;
  QImageWriter writer(&buffer, "jpg");
  writer.setQuality(50);
  writer.write(background);
  return buffer.buffer();
}
#endif*/
//---------------------------------------------------------------------------

/**
 * @brief Returns model data (information about directories and files)
 * @param index
 * @param role
 * @return model data
 */
QVariant myModel::data(const QModelIndex & index, int role) const {
  // Retrieve model item
  myModelItem *item = static_cast<myModelItem*>(index.internalPointer());

  // Color of filename (depends on file type)
  if (role == Qt::ForegroundRole) {
    if (!Common::readSetting("fileColor").toBool()) { return colors.windowText(); }
    QFileInfo type(item->fileInfo());
    if (cutItems.contains(type.filePath())) {
      return colors.windowText();
    } else if (type.isHidden()) {
      return colors.dark();
    } else if (type.isSymLink()) {
      return colors.link();
    } else if (type.isDir()) {
      return colors.windowText();
    } else if (type.isExecutable()) {
      return QBrush(QColor(Qt::darkGreen));
    }
  }
  // Alignment of filename
  else if (role == Qt::TextAlignmentRole) {
    if (index.column() == 1) {
      return Qt::AlignRight + Qt::AlignVCenter;
    }
  }
  // Display information about file
  else if (role == Qt::DisplayRole) {
    QVariant data;
    switch (index.column()) {
      case 0 :
        data = item->fileName();
        break;
      case 1 :
        data = item->fileInfo().isDir() ? "" : Common::formatSize(
               item->fileInfo().size());
        break;
      case 2 :
        if (item->mMimeType.isNull()) {
          if (realMimeTypes) {
            item->mMimeType = mimeUtilsPtr->getMimeType(item->absoluteFilePath());
          } else {
            item->mMimeType = item->fileInfo().isDir() ? "folder" :
                              item->fileInfo().suffix();
            if (item->mMimeType.isNull()) { item->mMimeType = "file"; }
          }
        }
        data = item->mMimeType;
        break;
      case 3 :
        data = item->fileInfo().lastModified().toString(Qt::LocalDate);
        break;
      case 4 : {
        if (item->mPermissions.isNull()) {
          QString str;
          QFile::Permissions perms = item->fileInfo().permissions();
          str.append(perms.testFlag(QFileDevice::ReadOwner) ? "r" : "-" );
          str.append(perms.testFlag(QFileDevice::WriteOwner) ? "w" : "-" );
          str.append(perms.testFlag(QFileDevice::ExeOwner) ? "x" : "-" );
          str.append(perms.testFlag(QFileDevice::ReadGroup) ? "r" : "-" );
          str.append(perms.testFlag(QFileDevice::WriteGroup) ? "w" : "-" );
          str.append(perms.testFlag(QFileDevice::ExeGroup) ? "x" : "-" );
          str.append(perms.testFlag(QFileDevice::ReadOther) ? "r" : "-" );
          str.append(perms.testFlag(QFileDevice::WriteOther) ? "w" : "-" );
          str.append(perms.testFlag(QFileDevice::ExeOther) ? "x" : "-" );
          str.append(" " + item->fileInfo().owner() + " " +
                     item->fileInfo().group());
          item->mPermissions = str;
        }
        return item->mPermissions;
      }
      default :
        data = "";
        break;
    }
    return data;
  }
  // Display file icon
  else if (role == Qt::DecorationRole) {
    if (index.column() != 0) {
      return QVariant();
    }
    return findIcon(item);
  }
  // Display file name
  else if(role == Qt::EditRole) {
    return item->fileName();
  }

  if (role == Qt::StatusTipRole) {
    return item->fileName();
  }
  return QVariant();
}
//---------------------------------------------------------------------------

/**
 * @brief Finds icon of a file
 * @param item
 * @return icon
 */
QVariant myModel::findIcon(myModelItem *item) const {

  //qDebug() << "findicon" << item->absoluteFilePath();
  // If type of file is directory, return icon of directory
  QFileInfo type(item->fileInfo());
  if (type.isDir()) {
    if (folderIcons->contains(type.fileName())) {
      return folderIcons->value(type.fileName());
    }
    return iconFactory->icon(type);
  }

  // If thumbnails are allowed and current file has it, show it
  if (showThumbs) {
    /*if (icons->contains(item->absoluteFilePath())) {
      return *icons->object(item->absoluteFilePath());
    } else*/ if (thumbs->contains(item->absoluteFilePath())) {
      QPixmap pic;
      pic.loadFromData(thumbs->value(item->absoluteFilePath()));
      icons->insert(item->absoluteFilePath(), new QIcon(pic), 1);
      return *icons->object(item->absoluteFilePath());
    }
  }

  // NOTE: Suffix is resolved using method getRealSuffix instead of suffix()
  // method. It is because files can contain version suffix e.g. .so.1.0.0

  // If there is icon for current suffix then return it
  QString suffix = FileUtils::getRealSuffix(type.fileName()); /*type.suffix();*/
  if (mimeIcons->contains(suffix)) { return mimeIcons->value(suffix); }

  // The icon
  QIcon theIcon;

  // If file has not suffix
  if (suffix.isEmpty()) {

    // If file is not executable, read mime type info from the system and create
    // an icon for it
    // NOTE: the icon cannot be cached because this file has not any suffix,
    // however operation 'getMimeType' could cause slowdown
    if (!type.isExecutable()) {
      QString mime = mimeUtilsPtr->getMimeType(type.absoluteFilePath());
      return FileUtils::searchMimeIcon(mime);
    }

    // If file is executable, set suffix to exec and find/create icon for it
    suffix = "exec";
    if (mimeIcons->contains(suffix)) {
      theIcon = mimeIcons->value(suffix);
    } else {
      theIcon = QIcon::fromTheme("application-x-executable", theIcon);
    }
  }
  // If file has unknown suffix (icon hasn't been assigned)
  else {

    // Load mime/suffix associations if they aren't loaded yet
    if (mimeGlob->count() == 0) loadMimeTypes();

    // Retrieve mime type for current suffix, if suffix is not present in list
    // from '/usr/share/mime/globs', its mime has to be detected manually
    QString mimeType = mimeGlob->value(suffix.toLower(), "");
    if (mimeType.isEmpty()) {
      mimeType = mimeUtilsPtr->getMimeType(type.absoluteFilePath());
      mimeGlob->insert(suffix.toLower(), mimeType);
    }

    // Load the icon
    theIcon = FileUtils::searchMimeIcon(mimeType);
  }

  // Insert icon to the list of icons
  mimeIcons->insert(suffix, theIcon);
  return theIcon;
}
//---------------------------------------------------------------------------

/**
 * @brief Finds icon of a file, based on file mime type
 * @param item
 * @return icon
 */
QVariant myModel::findMimeIcon(myModelItem *item) const {

  // Retrieve mime and search cache for it
  QString mime = mimeUtilsPtr->getMimeType(item->absoluteFilePath());
  if (mimeIcons->contains(mime)) {
    return mimeIcons->value(mime);
  }

  // Search file system for icon
  QIcon theIcon = FileUtils::searchMimeIcon(mime);
  mimeIcons->insert(mime, theIcon);
  return theIcon;
}
//---------------------------------------------------------------------------

bool myModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    Q_UNUSED(role)

    //can only set the filename
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());

    //physically change the name on disk
    bool ok = QFile::rename(item->absoluteFilePath(),item->parent()->absoluteFilePath() + SEPARATOR + value.toString());

    //change the details in the modelItem
    if(ok) {
        item->mMimeType.clear();                //clear the suffix/mimetype in case the user changes type
        item->changeName(value.toString());
        emit dataChanged(index,index);
    }

    return ok;
}

//---------------------------------------------------------------------------------
bool myModel::remove(const QModelIndex & theIndex)
{
    myModelItem *item = static_cast<myModelItem*>(theIndex.internalPointer());

    QString path = item->absoluteFilePath();

    //physically remove files from disk
    QDirIterator it(path,QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories);
    QStringList children;

    while (it.hasNext()) { children.prepend(it.next()); }
    children.append(path);

    children.removeDuplicates();

    bool error = false;
    for (int i = 0; i < children.count(); i++) {
        QFileInfo info(children.at(i));
        if(info.isDir()) {
            int wd = watchers.key(info.filePath());
            inotify_rm_watch(inotifyFD,wd);
            watchers.remove(wd);
            error |= QDir().rmdir(info.filePath());
        }
        else { error |= QFile::remove(info.filePath()); }
    }

    //remove from model
    beginRemoveRows(index(item->parent()->absoluteFilePath()),item->childNumber(),item->childNumber());
    item->parent()->removeChild(item);
    endRemoveRows();
    return error;
}

//---------------------------------------------------------------------------------

/**
 * @brief Drag drop to current datamodel
 * @param data
 * @param action
 * @param row
 * @param column
 * @param parent
 * @return true if successfull
 */
bool myModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                           int row, int column, const QModelIndex &parent) {

  // Unused
  Q_UNUSED(action);
  Q_UNUSED(row);
  Q_UNUSED(column);

  // If parent is not a directory, exit
  if (!isDir(parent)) {
    return false;
  }

  // Get urls of files
  QList<QUrl> files = data->urls();
  //QStringList cutList;

  // Don't do anything if drag and drop in same folder
  if (QFileInfo(files.at(0).path()).canonicalPath() == filePath(parent)) {
    return false;
  }

  // Holding ctrl is copy, holding shift is move, holding alt is ask
  Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
  Common::DragMode mode = Common::getDefaultDragAndDrop();
  if (mods == Qt::ControlModifier) {
    mode = Common::getDADctrlMod();
  } else if (mods == Qt::ShiftModifier) {
    mode = Common::getDADshiftMod();
  } else if (mods == Qt::AltModifier) {
    mode = Common::getDADaltMod();
  }


    /*foreach (QUrl item, files) {
      cutList.append(item.path());
    }*/


  // Emit drag drop paste
  emit dragDropPaste(data, filePath(parent), mode);
  return true;
}
//------------------------------------------------------------------------------

QVariant myModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)

    if(role == Qt::DisplayRole) {
        switch(section) {
        case 0: return tr("Name");
        case 1: return tr("Size");
        case 2: return tr("Type");
        case 4: return tr("Owner");
        case 3: return tr("Date Modified");
        default: return QVariant();
        }
    }

    return QVariant();
}

//---------------------------------------------------------------------------------------
Qt::ItemFlags myModel::flags(const QModelIndex &index) const
{
    if(!index.isValid()) return 0;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

//---------------------------------------------------------------------------------
void myModel::addCutItems(QStringList files)
{
    cutItems = files;
}

//---------------------------------------------------------------------------------
void myModel::clearCutItems()
{
    cutItems.clear();
    QFile(QDir::tempPath() + QString("/%1.temp").arg(APP)).remove();
}


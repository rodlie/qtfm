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

#include "mymodel.h"
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <QApplication>
#include <QMessageBox>
#include "fileutils.h"

#ifdef WITH_MAGICK
#include <Magick++.h>
#endif

#ifdef WITH_FFMPEG
extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
}
#endif

/**
 * @brief Creates file system model
 * @param realMime
 * @param mimeUtils
 */
myModel::myModel(bool realMime, MimeUtils *mimeUtils) {

#ifdef WITH_MAGICK
  Magick::InitializeMagick(Q_NULLPTR);
#endif
#ifdef WITH_FFMPEG
    av_register_all();
    avdevice_register_all();
    avcodec_register_all();
    avformat_network_init();
#ifdef QT_NO_DEBUG
    av_log_set_level(AV_LOG_QUIET);
#endif
#endif

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
  rootItem = new myModelItem(QFileInfo("/"), new myModelItem(QFileInfo(), Q_NULLPTR));
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

void myModel::forceRefresh()
{
    qDebug() << "force refresh model view";
    beginResetModel();
    endResetModel();
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
    if (!index.isValid()) { return false; }
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item && item != rootItem) { return item->fileInfo().isDir(); }
    return false;
}

//---------------------------------------------------------------------------------------
QFileInfo myModel::fileInfo(const QModelIndex &index)
{
    if (!index.isValid()) { return QFileInfo(); }
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item) { return item->fileInfo(); }
    return QFileInfo();
}

//---------------------------------------------------------------------------------------
qint64 myModel::size(const QModelIndex &index)
{
    if (!index.isValid()) { return 0; }
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item) { return item->fileInfo().size(); }
    return 0;
}

//---------------------------------------------------------------------------------------
QString myModel::fileName(const QModelIndex &index)
{
    if (!index.isValid()) { return QString(); }
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item) { return item->fileName(); }
    return QString();
}

//---------------------------------------------------------------------------------------
QString myModel::filePath(const QModelIndex &index)
{
    if (!index.isValid()) { return QString(); }
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item) { return item->absoluteFilePath(); }
    return QString();
}

//---------------------------------------------------------------------------------------
QString myModel::getMimeType(const QModelIndex &index)
{
    if (!index.isValid()) { return QString(); }
    qDebug() << "myModel getMimeType";
    myModelItem *item = static_cast<myModelItem*>(index.internalPointer());
    if(item->mMimeType.isNull()) {
        if(realMimeTypes) { item->mMimeType = mimeUtilsPtr->getMimeType(item->absoluteFilePath()); }
        else {
            if(item->fileInfo().isDir()) item->mMimeType = "folder";
            else item->mMimeType = item->fileInfo().suffix();
            if(item->mMimeType.isNull()) item->mMimeType = "file";
        }
    }
    //qDebug() << "item mime" << item->absoluteFilePath() << item->mMimeType;
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
    read(inotifyFD,buffer.data(),buffSize);
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
    //if (!lastEventFilename.isEmpty()) { emit reloadDir(); }
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
    QString folderChanged;
    if (watchers.contains(eventID)) {
        myModelItem *parent = rootItem->matchPath(watchers.value(eventID).split(SEPARATOR));
        if (parent) {
            parent->dirty = 1;
            QDir dir(parent->absoluteFilePath());
            folderChanged = dir.absolutePath();
            QFileInfoList all = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
            foreach(myModelItem * child, parent->children()) {
                if (all.contains(child->fileInfo())) {
                    //just remove known items
                    all.removeOne(child->fileInfo());
                } else {
                    //must have been deleted, remove from model
                    if (child->fileInfo().isDir()) {
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
    if (!folderChanged.isEmpty()) {
        qDebug() << "folder modified" << folderChanged;
        emit reloadDir(folderChanged);
    }
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

    if (item == Q_NULLPTR) {
        QMessageBox::warning(Q_NULLPTR, tr("No such directory"), tr("Directory requested does not exists."));
        return false;
    }

    if (!item->watched) { addWatcher(item); }
    if (!item->walked || !item->watched) {
        populateItem(item);
        return false;
    } else {
        if(item->dirty) { //model is up to date, but view needs to be invalidated
            item->dirty = false;
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

bool myModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) { return true; }
    myModelItem *item = static_cast<myModelItem*>(parent.internalPointer());
    if (item && item->fileInfo().isDir()) {
        if (QDir(item->fileInfo()
                 .absoluteFilePath())
                .entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries|QDir::System)
                .count() > 0)
        {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------
void myModel::fetchMore (const QModelIndex & parent)
{
    myModelItem *item = static_cast<myModelItem*>(parent.internalPointer());

    if (item) {
        populateItem(item);
        emit dataChanged(parent,parent);
    }
}

//---------------------------------------------------------------------------------------
void myModel::populateItem(myModelItem *item)
{
    if (item == Q_NULLPTR) { return; }
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
    if (item) { item->clearAll(); }
    endResetModel();
}

//---------------------------------------------------------------------------------
void myModel::update()
{
    myModelItem *item = rootItem->matchPath(currentRootPath.split(SEPARATOR));
    if (item == Q_NULLPTR) { return; }
    foreach(myModelItem *child, item->children()) { child->refreshFileInfo(); }
}

//---------------------------------------------------------------------------------
void myModel::refreshItems()
{
    myModelItem *item = rootItem->matchPath(currentRootPath.split(SEPARATOR));
    if (item == Q_NULLPTR) { return; }
    qDebug() << "refresh items";
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

  // Types that should be thumbnailed
  QStringList files;

  // Remember files with valid mime
  foreach (QModelIndex item, indexes) {
    QString filename = filePath(item);
    QString mimetype = mimeUtilsPtr->getMimeType(filename);
    //qDebug() << "mime for file" << filename << mimetype;
    if (mimetype.startsWith(QString("image"))
#ifdef WITH_MAGICK
        || mimetype == QString("application/pdf")
#ifdef WITH_FFMPEG
        || mimetype.startsWith(QString("video"))
        || mimetype == QString("audio/mpeg")
#endif
#endif
        || filename.endsWith(".desktop")
        )
    { files.append(filename); }
  }

  // Loads thumbnails from cache
  if (files.count()) {
    QFileInfo pathInfo (files.at(0));
    if (thumbs->count() == 0) {
        qDebug() << "thumbs are empty, try to load cache ...";
      QFile fileIcons(QString("%1/thumbs.cache").arg(Common::configDir()));
      if (fileIcons.open(QIODevice::ReadOnly)) {
          qDebug() << "load thumbs from cache ...";
          QDataStream out(&fileIcons);
          out >> *thumbs;
          fileIcons.close();
      }
      thumbCount = thumbs->count();
      qDebug() << "thumbcount" << thumbCount;
    }

    foreach (QString item, files) {
      if (!thumbs->contains(item) ||
          (item.split("/").takeLast() == lastEventFilename && !lastEventFilename.isEmpty()))
      {
          qDebug() << "gen new thumb" << item;
          QByteArray thumb = getThumb(item);
          if (thumb.size()>0) {
              thumbs->insert(item, thumb);
              if (item.split("/").takeLast() == lastEventFilename) {
                  qDebug() << "save new thumb cache";
                  lastEventFilename.clear();
                  QFile fileIcons(QString("%1/thumbs.cache").arg(Common::configDir()));
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
      }
    }
    emit thumbUpdate(pathInfo.absolutePath());
  }
}

//---------------------------------------------------------------------------

/**
 * @brief Creates thumbnail for given item
 * @param item
 * @return thumbnail
 */
QByteArray myModel::getThumb(QString item) {

  if (item.isEmpty()) { return QByteArray(); }
  if (item.endsWith(".desktop")) {
      QString iconFile = Common::findIcon("", QIcon::themeName(), Common::getDesktopIcon(item));
      if (!iconFile.isEmpty()) {
          QPixmap pix = QPixmap::fromImage(QImage(iconFile));
          if (!pix.isNull()) {
              QByteArray raw;
              QBuffer buffer(&raw);
              buffer.open(QIODevice::WriteOnly);
              pix.save(&buffer, "PNG");
              return raw;
          }
      }
      return QByteArray();
  }
#ifdef WITH_MAGICK
#ifdef WITH_FFMPEG
  QString itemMime = mimeUtilsPtr->getMimeType(item);
  if (itemMime.startsWith(QString("video"))) {
      QByteArray coverart =  getVideoFrame(item, true);
      if (coverart.size()>0) { return coverart; }
      return getVideoFrame(item, false);
  } else if (itemMime == QString("audio/mpeg")) {
      return getVideoFrame(item, true);
  }
#endif
  QByteArray result;
  qDebug() << "generate thumbnail for" << item;
  try {
      Magick::Image background(Magick::Geometry(128, 128), Magick::ColorRGB(0, 0, 0));
#ifndef OLDMAGICK
      background.quiet(true);
#endif
#if MagickLibVersion >= 0x700
      background.alpha(true);
#else
      background.matte(true);
#endif
      background.backgroundColor(background.pixelColor(0,0));
      background.transparent(background.pixelColor(0,0));

      Magick::Image thumb;
#ifndef OLDMAGICK
      thumb.quiet(true);
#endif
      QString filename = item;
      thumb.read(filename.toUtf8().data());
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
      background.composite(thumb, offsetX, offsetY, Magick::OverCompositeOp);
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
#else
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
    theThumb.load(":/images/background.png");
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
#endif
}

#ifdef WITH_FFMPEG
QByteArray myModel::getVideoFrame(QString file, bool getEmbedded, int videoFrame, int pixSize)
{
    qDebug() << "getVideoFrame" << file << getEmbedded << videoFrame << pixSize;
    QByteArray result;
    if (file.isEmpty()) { return result; }

    AVCodecContext  *pCodecCtx;
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVCodec * pCodec;
    AVFrame *pFrame, *pFrameRGB;

    qDebug() << "open media file";
    if (avformat_open_input(&pFormatCtx,file.toUtf8().data(),
                            Q_NULLPTR,
                            Q_NULLPTR) != 0) { return result; }
    if (avformat_find_stream_info(pFormatCtx,
                                  Q_NULLPTR) < 0) { return result; }

    av_dump_format(pFormatCtx, 0, file.toUtf8().data(), 0);
    int videoStream = -1;
    int possibleVideoCover = -1;

    qDebug() << "get video stream";
    for (int i=0; i < (int)pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (pFormatCtx->streams[i]->codecpar->codec_id == AV_CODEC_ID_MJPEG ||
                pFormatCtx->streams[i]->codecpar->codec_id == AV_CODEC_ID_PNG ||
                pFormatCtx->streams[i]->codecpar->codec_id == AV_CODEC_ID_GIF ||
                pFormatCtx->streams[i]->codecpar->codec_id == AV_CODEC_ID_TIFF ||
                pFormatCtx->streams[i]->codecpar->codec_id == AV_CODEC_ID_BMP) {
                possibleVideoCover = i;
            }
            if (videoStream<0) { videoStream = i; }
            //break;
        }
    }
    if (possibleVideoCover>-1 && getEmbedded) {
        qDebug() << "FOUND COVER?";
        videoStream = possibleVideoCover;
    }
    if (videoStream == -1) { return result; }

    qDebug() << "find decoder";
    pCodec =avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
    pCodecCtx = avcodec_alloc_context3(Q_NULLPTR);
    if (pCodec == Q_NULLPTR || pCodecCtx == Q_NULLPTR) { return result; }
    if (avcodec_parameters_to_context(pCodecCtx,
                                      pFormatCtx->streams[videoStream]->codecpar)<0)
    { return result; }
    if (avcodec_open2(pCodecCtx,
                     pCodec,
                     Q_NULLPTR) < 0) { return result; }

    qDebug() << "check for embedded?" << getEmbedded;
    if (getEmbedded) {
        if (pFormatCtx->streams[videoStream]->disposition == AV_DISPOSITION_ATTACHED_PIC) {
            AVPacket pkt = pFormatCtx->streams[videoStream]->attached_pic;
            if (pkt.size>0) {
                QByteArray attachedPix = QByteArray(reinterpret_cast<const char*>(pkt.data),
                                                    pkt.size);
                if (attachedPix.length()>0) {
                    qDebug() << "got embedded picture";
                    avcodec_close(pCodecCtx);
                    avformat_close_input(&pFormatCtx);
                    //return attachedPix;


                    qDebug() << "prepare thumbnail for" << file;
                    try {
                        Magick::Image thumb(Magick::Blob(attachedPix, attachedPix.length()));

                        Magick::Image background(Magick::Geometry((size_t)pixSize,
                                                                  (size_t)pixSize),
                                                 Magick::Color("black"));
#if MagickLibVersion >= 0x700
                        background.alpha(true);
#else
                        background.matte(true);
#endif
                        background.backgroundColor(background.pixelColor(0,0));
                        background.transparent(background.pixelColor(0,0));

                        thumb.scale(Magick::Geometry((size_t)pixSize,
                                                     (size_t)pixSize));
                        int offsetX = 0;
                        int offsetY = 0;
                        if (thumb.columns()<background.columns()) {
                            offsetX = (int)(background.columns()-thumb.columns())/2;
                        }
                        if (thumb.rows()<background.rows()) {
                            offsetY = (int)(background.rows()-thumb.rows())/2;
                        }
                        background.composite(thumb,
                                             offsetX,
                                             offsetY,
                                             MagickCore::OverCompositeOp);
                        background.magick("BMP");
                        Magick::Blob buffer;
                        background.write(&buffer);
                        result = QByteArray((char*)buffer.data(),
                                            (int)buffer.length());
                        qDebug() << "thumbnail is done" << file;
                    }
                    catch(Magick::Error &error_ ) {
                        qWarning() << error_.what();
                    }
                    catch(Magick::Warning &warn_ ) {
                        qWarning() << warn_.what();
                    }

                }
            }
        }
        avcodec_close(pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return  result;
    }

    qDebug() << "setup frame";
    pFrame    = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    uint8_t *buffer;
    int numBytes;

    AVPixelFormat  pFormat = AV_PIX_FMT_BGR24;
    numBytes = av_image_get_buffer_size(pFormat,
                                        pCodecCtx->width,
                                        pCodecCtx->height,
                                        16);
    buffer = (uint8_t *) av_malloc(numBytes*sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGB->data,
                         pFrameRGB->linesize,
                         buffer,
                         pFormat,
                         pCodecCtx->width,
                         pCodecCtx->height,
                         1);

    qDebug() << "calculate frame to get";
    int res;
    int frameFinished;
    AVPacket packet;
    int currentFrame = 0;
    bool fetchFrame = false;
    double fps = av_q2d(pFormatCtx->streams[videoStream]->r_frame_rate);
    double dur = static_cast<double>(pFormatCtx->duration)/AV_TIME_BASE;
    int maxFrame = qRound((dur*fps)/2);
    if (videoFrame>=0) { maxFrame = videoFrame; }

    qDebug() << "we need to get frame" << maxFrame;
    int64_t seekT = (int64_t(maxFrame) *
                     pFormatCtx->streams[videoStream]->r_frame_rate.den *
                     pFormatCtx->streams[videoStream]->time_base.den) /
                     (int64_t(pFormatCtx->streams[videoStream]->r_frame_rate.num) *
                     pFormatCtx->streams[videoStream]->time_base.num);
    if (av_seek_frame(pFormatCtx, videoStream, seekT, AVSEEK_FLAG_FRAME) >= 0) {
              av_init_packet(&packet);
              currentFrame = maxFrame;
    }
    while((res = av_read_frame(pFormatCtx,&packet)>=0)) {
        qDebug() << "at current frame" << currentFrame;
        if (currentFrame>=maxFrame) { fetchFrame = true; }
        if (packet.stream_index == videoStream){
            if (!fetchFrame) {
                currentFrame++;
                continue;
            }

            qDebug() << "get frame" << currentFrame;
            int ret = avcodec_send_packet(pCodecCtx, &packet);
            if (ret<0) { continue; }
            ret = avcodec_receive_frame(pCodecCtx, pFrame);
            if (ret>=0) { frameFinished = true; }
            else { continue; }

            if (frameFinished) {
                qDebug() << "extract image from frame" << currentFrame;
                struct SwsContext * img_convert_ctx;
                img_convert_ctx = sws_getCachedContext(Q_NULLPTR,
                                                       pCodecCtx->width,
                                                       pCodecCtx->height,
                                                       pCodecCtx->pix_fmt,
                                                       pCodecCtx->width,
                                                       pCodecCtx->height,
                                                       AV_PIX_FMT_BGR24,
                                                       SWS_BICUBIC,
                                                       Q_NULLPTR,
                                                       Q_NULLPTR,
                                                       Q_NULLPTR);
                sws_scale(img_convert_ctx,
                          ((AVFrame*)pFrame)->data,
                          ((AVFrame*)pFrame)->linesize,
                          0,
                          pCodecCtx->height,
                          ((AVFrame*)pFrameRGB)->data,
                          ((AVFrame*)pFrameRGB)->linesize);

                qDebug() << "prepare thumbnail for" << file;
                try {
                    Magick::Image background(Magick::Geometry((size_t)pixSize,
                                                              (size_t)pixSize),
                                             Magick::Color("black"));
#if MagickLibVersion >= 0x700
                    background.alpha(true);
#else
                    background.matte(true);
#endif
                    background.backgroundColor(background.pixelColor(0,0));
                    background.transparent(background.pixelColor(0,0));

                    Magick::Image thumb((size_t)pFrame->width,
                                        (size_t)pFrame->height,
                                        "BGR",
                                        Magick::CharPixel,
                                        pFrameRGB->data[0]);
                    thumb.scale(Magick::Geometry((size_t)pixSize,
                                                 (size_t)pixSize));
                    int offsetX = 0;
                    int offsetY = 0;
                    if (thumb.columns()<background.columns()) {
                        offsetX = (int)(background.columns()-thumb.columns())/2;
                    }
                    if (thumb.rows()<background.rows()) {
                        offsetY = (int)(background.rows()-thumb.rows())/2;
                    }
                    background.composite(thumb,
                                         offsetX,
                                         offsetY,
                                         MagickCore::OverCompositeOp);
                    background.magick("BMP");
                    Magick::Blob buffer;
                    background.write(&buffer);
                    result = QByteArray((char*)buffer.data(),
                                        (int)buffer.length());
                    qDebug() << "thumbnail is done" << file;
                }
                catch(Magick::Error &error_ ) {
                    qWarning() << error_.what();
                }
                catch(Magick::Warning &warn_ ) {
                    qWarning() << warn_.what();
                }

                av_packet_unref(&packet);
                sws_freeContext(img_convert_ctx);
                break;
            }
            currentFrame++;
        }
    }

    av_packet_unref(&packet);
    avcodec_close(pCodecCtx);
    av_free(pFrame);
    av_free(pFrameRGB);
    avformat_close_input(&pFormatCtx);

    return result;
}
#endif
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

  if (item == Q_NULLPTR) { return  QIcon(); }

  //qDebug() << "findicon" << item->absoluteFilePath();
  // If type of file is directory, return icon of directory
  QFileInfo type(item->fileInfo());
  if (type.isDir()) {
    if (folderIcons->contains(type.fileName())) {
      return folderIcons->value(type.fileName());
    }
#ifdef Q_OS_MAC
    if (type.fileName().endsWith(".app")) {
        return iconFactory->icon(type);
    }
    return QIcon::fromTheme("folder", iconFactory->icon(type));
#else
    return iconFactory->icon(type);
#endif
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

  if (item == Q_NULLPTR) { return QIcon(); }

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
    if(!index.isValid()) return Q_NULLPTR;
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


#ifndef QTFM_H
#define QTFM_H

#include <QMainWindow>
#include <QMdiArea>
#include "mimeutils.h"

class QtFM : public QMainWindow
{
    Q_OBJECT

public:
    QtFM(QWidget *parent = nullptr);
    ~QtFM();

private:
    QMdiArea *mdi;
    MimeUtils *mimes;
    bool mime;
    QString startPath;
public slots:
    void newSubWindow(QString path);
};

#endif // QTFM_H

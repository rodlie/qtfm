/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#ifndef THUMBS_H
#define THUMBS_H

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QString>

class Thumbs : public QObject
{
    Q_OBJECT
public:
    explicit Thumbs(QObject *parent = 0);
    ~Thumbs();

signals:
    void generatedIcon(QString file, QByteArray icon);

public slots:
    void generateIcon(QString file, QString mimetype = QString());

private slots:
    void procIcon(QString file, QString mimetype);

private:
    QThread t;
};

#endif // THUMBS_H

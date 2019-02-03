#include "completer.h"

#include <QDir>

QStringList myCompleter::splitPath(const QString& path) const
{
    QString temp = path;
    if (temp.startsWith('~')){
        temp.replace("~", QDir::homePath());
    }
    QStringList parts = temp.split("/");
    parts[0] = "/";

    return parts;
}

QString myCompleter::pathFromIndex(const QModelIndex& index) const
{
    if (!index.isValid()) { return QString(); }

    QModelIndex idx = index;
    QStringList list;
    do {
        QString t = model()->data(idx, Qt::EditRole).toString();
        list.prepend(t);
        QModelIndex parent = idx.parent();
        idx = parent.sibling(parent.row(), index.column());
    } while (idx.isValid());

    list[0].clear() ; // the join below will provide the separator

    return list.join("/");
}

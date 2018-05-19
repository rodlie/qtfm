#include "systray.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // needed to get org.freedesktop as prefix in service
    QCoreApplication::setApplicationName("freedesktop");
    QCoreApplication::setOrganizationDomain("org");

    SysTray tray(a.parent());
    return a.exec();
}

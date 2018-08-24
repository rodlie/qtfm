#include "service.h"

void qtfm::openPath(QString path)
{
    emit pathRequested(path);
}

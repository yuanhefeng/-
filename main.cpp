#include "interlock.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    InterLock w;
    w.show();

    return a.exec();
}

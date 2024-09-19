#include "mcuupgrade.h"
#include <qdesktopwidget.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    McuUpgrade w;

    w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    QDesktopWidget *desktop = QApplication::desktop();

    w.move((desktop->width() - w.width())/ 2, (desktop->height() - w.height()) /2);
    w.show();
    return a.exec();
}

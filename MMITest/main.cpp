#include "mmitest.h"
#include <qdesktopwidget.h>
#include <QApplication>
#include "libraries/VirtualKeyboard.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MMITest w;

    INSTALL_GLOBAL_EVENT_FILTER(a, &w);

    QDesktopWidget *desktop = QApplication::desktop();

    w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    w.move((desktop->width() - w.width())/ 2, (desktop->height() - w.height()) /2);

    w.show();
    return a.exec();


//    QApplication a(argc, argv);
//    QWidgt window;
//    window.show();
//    window.setWindowState(Qt::WindowMaximized);
//    window.setAttribute(Qt::WA_TranslucentBackground);
//    window.setAttribute(Qt::WA_TransparentForMouseEvents);
//    window.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
//    QRect r = QApplication::desktop()-&gt;
//    avlableGeometry(window);
//    window.setGeometry(r.x(),r.y(),r.height(),r.width());
//    //设置旋转方向为竖屏
}

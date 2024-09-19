#include "XXXXXX.h"
#include <qdesktopwidget.h>
#include <QApplication>
#include "libraries/VirtualKeyboard.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    XXXXXX w;								/*应用类*/

    INSTALL_GLOBAL_EVENT_FILTER(a, &w);	/*注册软键盘即可*/

    QDesktopWidget *desktop = QApplication::desktop();

    w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    w.move((desktop->width() - w.width())/ 2, (desktop->height() - w.height()) /2);

    w.show();
    return a.exec();
}

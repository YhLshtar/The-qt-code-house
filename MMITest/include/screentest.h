#ifndef SCREENTEST_H
#define SCREENTEST_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include "ui_screentest.h"

namespace Ui {
class ScreenTest;
}

class ScreenTest : public QWidget
{
    Q_OBJECT

public:
    ScreenTest(QWidget *parent = nullptr);
    virtual ~ScreenTest();

protected:
    QFont ft;

private slots:
    void on_pushButton2_clicked();
    void on_pushButton1_clicked();

private:
    Ui::ScreenTest *ui;

    QList<QPoint> plist;
    QPoint coursePoint; //coursePoint 变量在头文件中定义，获得鼠标位置

    void paintEvent(QPaintEvent *e)            override;
    void mousePressEvent(QMouseEvent *event)   override;
    void mouseMoveEvent(QMouseEvent *event)    override;
    void mouseReleaseEvent(QMouseEvent *event) override;

};

#endif // SCREENTEST_H

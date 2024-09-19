#include <QDebug>
#include "screentest.h"
#pragma execution_character_set("utf-8")

ScreenTest::ScreenTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScreenTest)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    //this->setWindowFlags(windowFlags()|Qt::FramelessWindowHint);

    this->setAutoFillBackground(true);
    this->showFullScreen();

    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(QPixmap(":/new/images/images/yellow.png")));
    this->setPalette(palette);

    qDebug("Screen Test Init");
}

ScreenTest::~ScreenTest()
{
    delete ui;
    qDebug("Screen Test Quit");
}

void ScreenTest::on_pushButton1_clicked()
{
    static unsigned char i = 0;
    i++;
    switch(i)
    {
        case 1:
        {
            QPalette pal = this->palette();
            pal.setBrush(QPalette::Background, QBrush(QPixmap(":/new/images/images/green.png")));
            this-> setPalette(pal);
        }break;
        case 2:
        {
            QPalette pal = this->palette();
            pal.setBrush(QPalette::Background,QBrush(QPixmap(":/new/images/images/blue.png")));
            this-> setPalette(pal);
        }break;
        case 3:
        {
            QPalette pal = this->palette();
            pal.setBrush(QPalette::Background,QBrush(QPixmap(":/new/images/images/white.png")));
            this-> setPalette(pal);
        }break;
        case 4:
        {
            QPalette pal = this->palette();
            pal.setBrush(QPalette::Background,QBrush(QPixmap(":/new/images/images/black.png")));
            this-> setPalette(pal);
        }break;
        case 5:
        {
            QPalette pal = this->palette();
            pal.setBrush(QPalette::Background,QBrush(QPixmap(":/new/images/images/yellow.png")));
            this-> setPalette(pal);
            i = 0;
        }break;
    }
}

void ScreenTest::on_pushButton2_clicked()
{
    this->close();
}

void ScreenTest::paintEvent(QPaintEvent *e)
{
    //qDebug("sss");
    QPainter painters(this);
    painters.setPen(QColor(255, 0, 0));//设置画笔颜色
    ft.setPointSize(20);
    painters.setFont(ft);
    QRect rect(this->width()/2-125, this->height()/2-25, 400, 80);
    painters.drawText(rect, "mousePosition:"+QString::number(coursePoint.x())+","+
                            QString::number(coursePoint.y()));
}

void ScreenTest::mousePressEvent(QMouseEvent *event)
{
    coursePoint = event->pos();
    this->update();	// 强制更新绘图
}

void ScreenTest::mouseMoveEvent(QMouseEvent *event)
{
    coursePoint = event->pos();
    this->update();	// 强制更新绘图
}

void ScreenTest::mouseReleaseEvent(QMouseEvent *event)
{

}

#include "VirtualKeyboard.h"
#include "ui_VirtualKeyboard.h"

#include <QDebug>
#include <QApplication>
#include <qdesktopwidget.h>
#include <QRegularExpression>

VirtualKeyboard::VirtualKeyboard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VirtualKeyboard)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

    this->setWindowOpacity(0.7);

    backspaceTimer = new QTimer(this);

    backspaceTimer->setInterval(100);

    ui->btnBackSpace->setText(QStringLiteral("\u2190"));
    ui->btnEnter->setText(QStringLiteral("\u21A9"));
    ui->btnToggle->setText(QStringLiteral("\u2191"));

    btnHighLight(ui->btnToggle);

    connect(backspaceTimer, &QTimer::timeout, this, &VirtualKeyboard::backspace);
}

VirtualKeyboard::~VirtualKeyboard()
{
    delete ui;
}

void VirtualKeyboard::showEvent(QShowEvent *)
{
    QDesktopWidget *desktop = QApplication::desktop();

    int DesktopW = desktop->width();
    int DesktopH = desktop->height();
    int WindowsW = this->parentWidget()->width();
    int WindowsH = this->parentWidget()->height();
    int WindowMy = this->parentWidget()->geometry().y() + WindowsH / 2;
    int EditY    = -(mLineEdit->mapFromGlobal(QPoint(0 ,0)).y()) + mLineEdit->height();
//    qDebug()<<EditY;
    this->move((DesktopW - WindowsW) / 2,
                this->parentWidget()->geometry().y() + WindowsH - this->height());
//    qDebug()<<(DesktopH - WindowsH) / 2 - EditY + WindowMy;
//    if(EditY > WindowMy)
//    {
//        this->parentWidget()->move((DesktopW - WindowsW) / 2,
//                                   (DesktopH - WindowsH) / 2 - (EditY - WindowMy));
//    }

    ui->stackedWidget->setCurrentWidget(ui->number);
    // 按钮初始
    btnInit();

    QList<QPushButton*> btnList = findChildren<QPushButton*>(QRegularExpression("^btn"));

    for(const auto& btn : btnList)
    {
        // 文本按钮
        if(btn->text().length() == 1 && (btn->text().at(0).isNumber() || btn->text().at(0).isLetter()))
        {
            mList.append(connect(btn, &QPushButton::pressed, this, [=]()
            {
                this->mLineEdit->setText(this->mLineEdit->text() + btn->text());
                btnHighLight(btn);
            }));

            mList.append(connect(btn, &QPushButton::released, this, [=](){ btnLowLight(btn); }));
            continue;
        }
        // 清除按钮
        if(btn->text() == "clear")
        {
            mList.append(connect(btn, &QPushButton::pressed, this, [=]()
            {
                this->mLineEdit->clear();
                btnHighLight(btn);
            }));

            mList.append(connect(btn, &QPushButton::released, this, [=](){ btnLowLight(btn); }));
            continue;
        }
        // 字母按钮
        if(btn->text() == "ABC")
        {
            mList.append(connect(btn, &QPushButton::pressed, this, [=](){ ui->stackedWidget->setCurrentWidget(ui->letter); }));
            continue;
        }
        // 数字按钮
        if(btn->text() == "123")
        {
            mList.append(connect(btn, &QPushButton::pressed, this, [=](){ ui->stackedWidget->setCurrentWidget(ui->number); }));
            continue;
        }
        // 大小写切换的按钮
        if(btn->text() == QStringLiteral("\u2191"))
        {
            mList.append(connect(btn, &QPushButton::clicked, this, [=](){ toggle(btn); }));
            continue;
        }
        // 退格按钮
        if(btn->text() == QStringLiteral("\u2190"))
        {
            mList.append(connect(btn, &QPushButton::pressed, this, [=]()
            {
                btnHighLight(btn);
                backspaceTimer->start();
            }));

            mList.append(connect(btn, &QPushButton::released, this, [=]()
            {
                btnLowLight(btn);
                backspaceTimer->stop();
            }));

            continue;
        }
        // 完成按钮
        if(btn->text() == QStringLiteral("\u21A9"))
        {
            mList.append(connect(btn, &QPushButton::clicked, this, &QWidget::hide));
        }
    }
}

void VirtualKeyboard::hideEvent(QHideEvent *)
{
    QDesktopWidget *desktop = QApplication::desktop();

    for(const auto& conn : mList)
    {
        disconnect(conn);
    }

    mList.clear();

    int DesktopW = desktop->width();
    int DesktopH = desktop->height();
    int WindowsW = this->parentWidget()->width();
    int WindowsH = this->parentWidget()->height();
    this->parentWidget()->move((DesktopW - WindowsW) / 2, (DesktopH - WindowsH) / 2);
}

void VirtualKeyboard::setLineEdit(QLineEdit *lineEdit)
{
    mLineEdit = lineEdit;
}

void VirtualKeyboard::backspace()
{
    QString text = this->mLineEdit->text();

    if(text.isEmpty()) return;

    text.chop(1);

    this->mLineEdit->setText(text);
}

void VirtualKeyboard::toggle(QPushButton* btn)
{
    enum
    {
        NOT_A_LETTER,
        UPPER_LETTER,
        LOWER_LETTER,
    } btnText = NOT_A_LETTER;

    QList<QPushButton*> btnList = findChildren<QPushButton*>(QRegularExpression("^btn"));

    for(const auto& btn : btnList)
    {
        QString text = btn->text();

        if(text.length() != 1) continue;

        QChar ch = text.at(0);

        if(ch.isLetter())
        {
           if(ch.isUpper())
           {
               btn->setText(ch.toLower());
               btnText = LOWER_LETTER;
           }
           else
           {
               btn->setText(ch.toUpper());
               btnText = UPPER_LETTER;
           }
        }
    }

    if(btnText ==  LOWER_LETTER)
    {
        btnLowLight(btn);
        return;
    }

    if(btnText == UPPER_LETTER)
    {
        btnHighLight(btn);
        return;
    }
}

void VirtualKeyboard::btnHighLight(QPushButton* btn)
{
    btn->setStyleSheet("background-color: rgba(255, 255, 255);"
                       "color: rgb(0, 0, 0);"
                       "border-radius: 15px;"
                       "padding: 10px;"
                       "font-size: 24px;");
}

void VirtualKeyboard::btnLowLight(QPushButton *btn)
{
    btn->setStyleSheet("background-color: rgba(255, 255, 255, 50);"
                       "color: rgb(255, 255, 255);"
                       "border-radius: 15px;"
                       "padding: 10px;"
                       "font-size: 24px;");
}

void VirtualKeyboard::btnInit()
{
    QList<QPushButton*> btnList = findChildren<QPushButton*>(QRegularExpression("^btn"));

    for(const auto& btn : btnList)
    {
        btnLowLight(btn);
    }
}

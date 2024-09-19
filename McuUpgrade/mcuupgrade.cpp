#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QFileDialog>
#include <QSerialPort>
#include <QtConcurrent>
#include <qdesktopwidget.h>
#include "include/YmodemMaster.h"
#include "mcuupgrade.h"
#include "ui_mcuupgrade.h"

#pragma  execution_character_set("utf-8")

McuUpgrade::McuUpgrade(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::McuUpgrade)
{
    ui->setupUi(this);
//    setWindowTitle("McuUpgarde_V01.A.002.240827");
    ui->labelVersion->setText("McuUpgarde_V01.A.003.240911");
    qDebug()<<"Main Thread"<<QThread::currentThread();

    ymodem_master = new YmodemMaster();

    signed char status = ymodem_master->SlaveConnect("/dev/ttyS3", 460800, QSerialPort::Data8,
                                                                           QSerialPort::NoParity,
                                                                           QSerialPort::OneStop);
    switch(status)
    {
        case -1:
        {

        }break;
        case  0:
        {
            ui->plainTextEdit->appendPlainText("串口打开失败!");
        }break;
        case  1:
        {

        }break;
    }

    connect(ymodem_master, &YmodemMaster::SerialPortData, this, [=](QString str)
    {
        ui->plainTextEdit->appendPlainText(str.split("\n")[0]);
    });
    connect(ymodem_master, &YmodemMaster::UpgradeStart, this, [=]()
    {
        ui->plainTextEdit->appendPlainText("Start");
    });
    connect(ymodem_master, &YmodemMaster::UpgradeProcess, this, [=](int process_value)
    {
        QTextCursor cursor = ui->plainTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.removeSelectedText();
        cursor.insertText(QString::number((float)(process_value * 100 / ymodem_master->TotalPackets())) + "%");
        ui->plainTextEdit->setTextCursor(cursor);
    });
    connect(ymodem_master, &YmodemMaster::UpgradeFinish, this, [=]()
    {
        ui->plainTextEdit->appendPlainText("烧录完成");
    });
    //点击选取文件按钮，弹出文件对话框
    connect(ui->btn_openFile, &QPushButton::clicked, [=]()
    {
        QFileDialog m_filedialog;
//        //显示打开文件窗口
//        m_filedialog.setFixedSize(480, 320);
//        m_filedialog.setWindowTitle("路径");
//        m_filedialog.setNameFilter("Mcu固件(*.bin)");
//        QDesktopWidget *desktop = QApplication::desktop();
//        m_filedialog.move((desktop->width()  - m_filedialog.width())  / 2,
//                          (desktop->height() - m_filedialog.height()) / 2);
//        m_filedialog.exec();
//        m_FileName = m_filedialog.selectedFiles().value(0);
        m_FileName = m_filedialog.getOpenFileName(this, "打开文件", "/home/tyzc/Mcu_test/bin/","Mcu固件(*.bin)");//返回一个文件的路径
        qDebug()<<"file path   : "<<m_FileName;
        //将路径放入到lineEdit中
        ui->label->setText(m_FileName);
        ymodem_master->SetUpgradeFile(m_FileName);
    });

    qDebug("McuUpgrade Main Thread Start");
}

McuUpgrade::~McuUpgrade()
{
    delete ymodem_master;
    delete ui;
    qDebug("McuUpgrade Main Thread Quit");
}

void McuUpgrade::on_btn_close_clicked()
{
    ymodem_master->StopUpgrade();
    close();
}

void McuUpgrade::on_btn_upgrade_clicked()
{
    if(m_FileName != "")
    {
        ui->btn_openFile->setEnabled(false);
        ui->btn_upgrade->setEnabled(false);
        ui->btn_close->setEnabled(false);
        ui->plainTextEdit->appendPlainText("开始升级");
        signed char status = ymodem_master->SlaveCheck();
        switch(status)
        {
            case  0:
            {
                ui->plainTextEdit->appendPlainText("连接MCU失败, 请联系工作人员!");//升级失败
                return;
            }break;
            case  1:
            {
                ui->plainTextEdit->appendPlainText("Mode 1...");
                if(ymodem_master->StartUpgrade())
                {
                    ui->plainTextEdit->appendPlainText("完成升级");
                }
                else
                {
                    ui->plainTextEdit->appendPlainText("升级失败, 请重试!");
                }
            }break;
            case  2:
            {
                ui->plainTextEdit->appendPlainText("Mode 2...");
                signed char status = ymodem_master->SlaveLoadBoot();
                switch(status)
                {
                    case -3:
                    case -2:
                    case -1:
                    {
                        ui->plainTextEdit->appendPlainText("升级失败, 请重试!");
                    }break;
                    case  0:
                    {
                        ui->plainTextEdit->appendPlainText("串口未打开!");
                    }break;
                    default:
                    case  1:
                    {
                        if(ymodem_master->StartUpgrade())
                        {
                            ui->plainTextEdit->appendPlainText("完成升级");
                        }
                        else
                        {
                            ui->plainTextEdit->appendPlainText("升级失败, 请重试!");
                        }
                    }break;
                }
            }break;
        }
    }
    else
    {
        ui->plainTextEdit->appendPlainText("请选择固件路径!");
    }
    ui->btn_close->setEnabled(true);
    ui->btn_upgrade->setEnabled(true);
    ui->btn_openFile->setEnabled(true);
}

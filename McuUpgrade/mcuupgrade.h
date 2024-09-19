#ifndef MCUUPGRADE_H
#define MCUUPGRADE_H

#include <QWidget>
#include "include/YmodemMaster.h"

QT_BEGIN_NAMESPACE
namespace Ui { class McuUpgrade; }
QT_END_NAMESPACE

class McuUpgrade : public QWidget
{
    Q_OBJECT

public:
    McuUpgrade(QWidget *parent = nullptr);
    ~McuUpgrade();

protected:
    bool isAutoLoadBoot = false;

private:
    Ui::McuUpgrade *ui;
    YmodemMaster   *ymodem_master = nullptr;

    QString         m_FileName;

private slots:
    void on_btn_close_clicked();
    void on_btn_upgrade_clicked();

};
#endif // MCUUPGRADE_H

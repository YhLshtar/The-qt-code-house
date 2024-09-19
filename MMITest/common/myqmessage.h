#ifndef MYQMESSAGE_H
#define MYQMESSAGE_H

#include <QWidget>
#include <QMessageBox>
#include <QResizeEvent>

class MyQmessage : public QMessageBox
{
    Q_OBJECT
public:
    explicit MyQmessage(QWidget *parent = 0);
    virtual ~MyQmessage();

    void setMySize(int width, int height);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    int width;
    int height;
};

#endif // MYQMESSAGE_H

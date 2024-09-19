#include "myqmessage.h"

MyQmessage::MyQmessage(QWidget *parent) :
    QMessageBox(parent),
    width(0),
    height(0)
{

}

MyQmessage::~MyQmessage()
{

}

void MyQmessage::setMySize(int m_width, int m_height)
{
    width = m_width;
    height = m_height;
}

void MyQmessage::resizeEvent(QResizeEvent *event)
{
    setFixedSize(width, height);
}


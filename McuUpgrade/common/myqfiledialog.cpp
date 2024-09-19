#include "myqfiledialog.h"

MYQFileDialog::MYQFileDialog(QWidget *parent) :
    QFileDialog(parent),
    width(0),
    height(0)
{

}

MYQFileDialog::~MYQFileDialog()
{

}

void MYQFileDialog::setSize(int m_width, int m_height)
{
    width = m_width;
    height = m_height;
}

void MYQFileDialog::resizeEvent(QResizeEvent *event)
{
    setFixedSize(width, height);
}


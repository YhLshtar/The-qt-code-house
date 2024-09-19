#ifndef MYQFILEDIALOG_H
#define MYQFILEDIALOG_H

#include <QWidget>
#include <QFileDialog>
#include <QResizeEvent>

class MYQFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    explicit MYQFileDialog(QWidget *parent = 0);
    virtual ~MYQFileDialog();

    void setSize(int width, int height);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    int width;
    int height;
};

#endif // MYQFILEDIALOG_H

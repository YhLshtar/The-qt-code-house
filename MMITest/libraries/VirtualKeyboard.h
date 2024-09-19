#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include <QTimer>
#include <QEvent>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

namespace Ui {
class VirtualKeyboard;
}

class VirtualKeyboard : public QWidget
{
    Q_OBJECT

public:
    explicit VirtualKeyboard(QWidget *parent = nullptr);
    ~VirtualKeyboard();

    void showEvent(QShowEvent *) override;

    void hideEvent(QHideEvent *) override;

    void setLineEdit(QLineEdit* lineEdit);

private slots:
    void backspace();

private:
    Ui::VirtualKeyboard *ui;

    QLineEdit* mLineEdit = nullptr;

    QList<QMetaObject::Connection> mList;

    void toggle(QPushButton* btn);

    void btnHighLight(QPushButton* btn);

    void btnLowLight(QPushButton* btn);

    void btnInit();

    QTimer* backspaceTimer;
};

class VirtualKeyboardEventFilter : public QObject
{
    Q_OBJECT

public:
    VirtualKeyboardEventFilter(VirtualKeyboard* virtualKeyboard, QObject *parent = nullptr)
        :QObject(parent), mVirtualKeyboard(virtualKeyboard){}
    ~VirtualKeyboardEventFilter(){ delete mVirtualKeyboard; mVirtualKeyboard = nullptr; }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QLineEdit* lineEdit = qobject_cast<QLineEdit*>(obj);

            if(lineEdit)
            {
                mVirtualKeyboard->setLineEdit(lineEdit);

                mVirtualKeyboard->show();
            }
        }

        return QObject::eventFilter(obj, event);
    }

private:
    VirtualKeyboard* mVirtualKeyboard;
};

#define INSTALL_GLOBAL_EVENT_FILTER(app, w)\
    do{\
        VirtualKeyboard* virtualKeyboard   = new VirtualKeyboard(w);\
        VirtualKeyboardEventFilter* filter = new VirtualKeyboardEventFilter(virtualKeyboard);\
        app.installEventFilter(filter);\
    }while(0)


#endif // VIRTUALKEYBOARD_H

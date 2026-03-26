#ifndef IO_DIALOG_H
#define IO_DIALOG_H

#include <QDialog>
#include <QTimer>
#include "status_led.h"

namespace Ui {
class IODialog;
}

class IODialog : public QDialog
{
    Q_OBJECT

public:
    explicit IODialog(QWidget *parent = nullptr);
    ~IODialog();
    void setLedIn(int index, bool on);
    void setLedOut(int index, bool on);

private slots:
    void on_btnOutput_clicked();
    void do_timer_output();

signals:
    void ledOutSet(int index, bool on);

private:
    QList<StatusLed*> led_out;  //true: 亮（导通）， false: 灭 (断开)
    QList<StatusLed*> led_in;   //数组，方便调用
    int curr_index;             //当前亮灯位置
    QTimer* m_timer;            //定时器，用于处理流水灯
private:
    Ui::IODialog *ui;
};

#endif // IO_DIALOG_H

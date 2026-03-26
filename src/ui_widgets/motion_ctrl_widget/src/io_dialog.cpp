#include "io_dialog.h"
#include "ui_io_dialog.h"

IODialog::IODialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::IODialog)
{
    ui->setupUi(this);
    led_in.reserve(12);
    led_out.reserve(12);
    curr_index = 0;                 //流水灯当前位置
    for(int i = 0; i < 12; i++)
    {
        QString InName = QString("labIn%1").arg(i);
        QString OutName = QString("labOut%1").arg(i);
        StatusLed* ledIn= findChild<StatusLed*>(InName);
        StatusLed* ledOut= findChild<StatusLed*>(OutName);
        led_in.append(ledIn);
        led_out.append(ledOut);
    }

    m_timer = new QTimer(this);         //内存管理交给Qt
    m_timer->setInterval(500);          //500ms
    m_timer->setSingleShot(false);
    m_timer->stop();
    //绑定定时器超时函数
    connect(m_timer, &QTimer::timeout, this, &IODialog::do_timer_output);
}

IODialog::~IODialog()
{
    delete ui;
}

void IODialog::on_btnOutput_clicked()
{   //输出LED控制
    m_timer->stop();        //按下按钮时，先关闭定时器
    curr_index = 0;         //位置清零

    int ch = ui->comboMode->currentIndex();
    int index = ui->comboIndex->currentIndex();
    switch (ch)
    {
        case 0:                 //导通
        {
            led_out[index]->setState(true);
            emit ledOutSet(index, true);    //发送信号通知控制器
            break;
        }
        case 1:                 //关断
        {
            led_out[index]->setState(false);
            emit ledOutSet(index, false);
            break;
        }
        case 3:                 //全亮
        {
            for(int i = 0; i < 12; i++)
                led_out[i]->setState(true);
            emit ledOutSet(12, true);
            break;
        }
        case 2:
        case 4:                 //全灭
        {
            for(int i = 0; i < 12; i++)
                led_out[i]->setState(false);
            emit ledOutSet(12, false);
            if(ch == 2)         //流水灯
            {
                m_timer->start();   //启动定时器
                led_out[0]->setState(true);
                emit ledOutSet(0, true);    //0号LED亮
            }
            break;
        }
        default : break;
    }
}

void IODialog::do_timer_output()
{
    led_out[curr_index]->setState(false);
    emit ledOutSet(curr_index, false);    //关闭上一个LED
    //开启下一个LED
    curr_index = (curr_index + 1) % 12;
    led_out[curr_index]->setState(true);
    emit ledOutSet(curr_index, true);
}

void IODialog::setLedIn(int index, bool on) {
    if(index >= 0 && index < led_in.size())
        led_in[index]->setState(on);
    else
    {   //否则全亮或全灭
        for(int i = 0; i < 12; i++)
            led_in[i]->setState(on);
    }
}

void IODialog::setLedOut(int index, bool on) {
    if(index >= 0 && index < led_out.size())
        led_out[index]->setState(on);
    else
    {   //否则全亮或全灭
        for(int i = 0; i < 12; i++)
            led_out[i]->setState(on);
    }
}

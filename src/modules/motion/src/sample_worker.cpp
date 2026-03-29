#include "sample_worker.h"
#include "motion_controller.h"
#include "motion_manager.h"
#include <QDateTime>
#include <QElapsedTimer>

#define SAMPLE_COUNT 6
SampleWorker::SampleWorker(QObject* parent):QObject(parent)
{
    m_nodeID = 1002;                      //默认值
    data_num.resize(SAMPLE_COUNT);        //120ms发送一次数据
    m_timer = new QTimer(this);
    m_timer->setInterval(20);             //20ms采样一次, 这个采样程序的执行时间有点长
    m_timer->setTimerType(Qt::PreciseTimer);
    //m_timer->setInterval(10);
    m_timer->setSingleShot(false);
    m_timer->stop();
    connect(m_timer, &QTimer::timeout, this, &SampleWorker::do_timer_timeout);
}

SampleWorker::~SampleWorker(){}

void SampleWorker::do_timer_timeout()
{
    if(!m_controller)       return;         //判空
    static int64_t count = 0;
    // //测试程序执行时间
    // static QElapsedTimer timer;
    // static double total = 0;
    // timer.restart();

    float volt;
    int value;
    qint64 now = QDateTime::currentMSecsSinceEpoch();       //获取时间戳
    int index = count % SAMPLE_COUNT;
    
    //读三个模拟量通道就好了，一个5.4ms左右（笔者电脑性能一般， USB转网口有额外延迟）
    for(int i = 0; i < 3; i++) 
    {   //直接调用控制器接口
        m_controller->getADValue(m_nodeID, i, &value, &volt);
        data_num[index].volt[i] = volt;
        data_num[index].value[i] = value;  
    }


    data_num[index].timestamp = now;             //记录时间戳
    if(count % SAMPLE_COUNT == 0 && count != 0)  //一次发送100ms的数据到数据处理线程
    {
        if(m_controller->getCtrlStatus() == 1)   //检查控制器连接状态,控制器断连就不发送数据
            emit sensorDataUpdated(data_num);    //发送信号到数据处理线程
    }
    
    if(count % 10 == 0)                          //250ms 调用一次
    {
        emit getLedIn(12, &on);                  //读取控制器所有输入IO口
    }
    count++;


    // total += timer.elapsed();
    // if(count % 50 == 0)
    // {
    //     qDebug() << "total time (ms) : " << total/50;
    //     total = 0;      //清零
    // }
}

void SampleWorker::connectController(MotionController* controller)
{   //绑定信号与槽
    if(!controller)     return;          //自动判空
    connect(this, &SampleWorker::getCtrlStatus, controller, &MotionController::getCtrlStatus);
    connect(this, &SampleWorker::getLedIn, controller, &MotionController::getLedIn);
    m_controller = controller;
}

void SampleWorker::start()        //启动工作对象
{
    m_timer->start();
}
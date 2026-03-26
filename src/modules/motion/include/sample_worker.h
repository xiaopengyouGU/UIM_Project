#ifndef SAMPLE_WORKER_H
#define SAMPLE_WORKER_H

#include <QObject>
#include <QTimer>


class MotionController;                         //前向声明      
class SensorData;
class MotionTimer;                              //高精度运动定时器
#if defined(MOTION_LIBRARY)
#  define MOTION_EXPORT Q_DECL_EXPORT
#else
#  define MOTION_EXPORT Q_DECL_IMPORT
#endif

//采样工作类，以100Hz的频率采样，运行在数据采集线程中
class MOTION_EXPORT SampleWorker:public QObject
{
    Q_OBJECT
public:
    SampleWorker(QObject* parent = nullptr);
    ~SampleWorker();
    void connectController(MotionController* controller);
public slots:
    void do_timer_timeout();   
    void setNodeID(int nodeID)              { m_nodeID = nodeID; }   //设置从站节点号
    void start();                                                    //启动工作对象                             
signals:
    int getCtrlStatus();                       //读取控制器状态
    //数据获取接口,对应EM06AX-E4 模拟量模块
    void sensorDataUpdated(QList<SensorData> dataNum);
    void getLedIn(int index, bool *on);        //共12个In,  0-11

private:
    QTimer              *m_timer;                  //25ms， 40Hz
    int                 m_nodeID;                  //从站节点号，默认为1002
    QList<SensorData>   data_num;                  //采集的传感器数据（采集60ms的数据，统一发送）
    MotionController    *m_controller;             //直接访问controller接口。
    bool            on;
};

#endif // SAMPLE_WORKER_H
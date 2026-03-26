#ifndef MOTION_CONTROLLER_H
#define MOTION_CONTROLLER_H

#include <QObject>

#if defined(MOTION_LIBRARY)
#  define MOTION_EXPORT Q_DECL_EXPORT
#else
#  define MOTION_EXPORT Q_DECL_IMPORT
#endif

class MOTION_EXPORT MotionController:public QObject
{
    Q_OBJECT
public:
    MotionController(QObject* parent = nullptr);
    ~MotionController();
public:                                             //这部分函数既可直接调用，又可作为槽函数绑定信号
    //函数的返回值为状态变量
    void connect(const QString& ip_addr);           //连接控制器
    void disConnect();                              //断开控制器
    int getCtrlStatus();                            //读取控制器状态
    //数据获取接口,对应EM06AX-E4 模拟量模块
    void getADValue(int nodeID, int ch, int *value, float *volt);
    void setDAValue(int nodeID, int ch, float volt);
    //轴控制接口
    int getAxisNum();                              //获取控制器配置的轴数
    void setVel(int axis, float vel);              //设置速度和位置时，会先停机
    void setPos(int axis, float pos);
    float getVel(int axis);                        //单位：unit/s
    float getPos(int axis);                        //单位：unit
    int getStatus(int axis);                       //获取轴状态
    void enable(int axis);                         //使能轴
    void disable(int axis);                        //使能轴
    void stop(int axis);                           //停止轴
    void emerStop();                               //急停，停止所有轴
    void setMode();                              //切换控制模式
    //IO控制接口
    void setLedOut(int index, bool on);            //共12个Out, 0-11
    void getLedIn(int index, bool *on);            //共12个In,  0-11
signals:
    //这些信号都是不是实时高频发送的，也不应该这么做。
    void ledInSet(int index, bool on);            //共12个In,  0-11,通知UI切换IO口状态
    void DAValueUpdated(int ch, int value, float volt);
    //控制器和轴状态信号
    void controllerConnected(bool success);                      //控制器连接信号
    void controllerDisConnected();
    void axisEnabled(int axis, bool success);                   //轴使能信号
    void axisDisabled(int axis);
    void axisStop(int axis);                                    //轴停止信号
    void axisNumChanged(int num);                               //更新控制的轴数量
private:
    int ctrlStatus;                                 //控制器状态
};

#endif // MOTION_CONTROLLER_H

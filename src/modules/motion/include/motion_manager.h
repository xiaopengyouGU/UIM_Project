#ifndef MOTION_MANAGER_H
#define MOTION_MANAGER_H

#include <QObject>
#include <QString>

#if defined(MOTION_LIBRARY)
#  define MOTION_EXPORT Q_DECL_EXPORT
#else
#  define MOTION_EXPORT Q_DECL_IMPORT
#endif

class MotionSolver;   // 前向声明，避免暴露定义
class MotionController;
class MotionWorker;
class SampleWorker;

class SensorData{
public:
    qint64 timestamp; //时间戳，一次采样四个通道，可认为时间戳相同
    int value[4];     //四个通道，原始数值
    float volt[4];      //对应的电压值
    SensorData(){}
};

//MotionManager对象运行在主线程中
class MOTION_EXPORT MotionManager : public QObject
{
    Q_OBJECT
public:
    typedef enum{
        Mode_Pos = 0,
        Mode_Vel,
        Mode_Torque,
        Mode_Unknown
    }CtrlMode;
    
    typedef enum{
        Default_Solver = 0,
        MPC_Solver,
    }Solver_Type;                                           //系统支持的求解器类型

public:
    explicit MotionManager(QObject *parent = nullptr);
    ~MotionManager();

    // 公共接口
    void setSolver(Solver_Type type);
    void setSolver(MotionSolver* solver);                   //设置运动求解器
    void setNodeID(int nodeID);                             //设置传感器从站节点号
    MotionSolver* getSolver() const;                        //获取当前求解器
    void start();                                           //启动管理器
    void stop();                                            //停止管理器
    void process();                                         //处理计算
    //控制器相关操作接口
    void connectController(const QString& ip_addr);         //连接控制器
    void disConnectController();                            //断开控制器
    void setAxisVel(int axis, float vel);                   //设置速度和位置时，会先停机
    void setAxisPos(int axis, float pos);
    void enableAxis(int axis);                              //使能轴
    void disableAxis(int axis);                             //使能轴
    void stopAxis(int axis);                                //停止轴
    void setAxisMode(int axis, int mode);                   //切换控制模式
    //IO与传感器操作接口
    void setLedOut(int index, bool on);                     //设置LED输出
    void setDAValue(int nodeID, int ch, float volt);        //设置DA通道输出

signals:
    //发送给用户的信号
    void axisEnabled(int axis, bool success);
    void axisDisabled(int axis);
    void axisNumChanged(int num);
    void controllerConnected(bool success);
    void controllerDisConnected();
    //传感器与IO信号
    void ledInSet(int index, bool on);            //共12个In,  0-11,通知UI切换IO口状态
    void DAValueUpdated(int ch, int value, float volt);
    void sensorDataUpdated(QList<SensorData>);              //发送一批采样数据（默认60ms）
private:
    class Private;                                          //前向声明                   
    Private *pimpl;                                         //私有实现指针
};

#endif // MOTION_MANAGER_H



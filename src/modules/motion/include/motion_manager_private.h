#ifndef MOTION_MANAGER_PRIVATE_H__
#define MOTION_MANAGER_PRIVATE_H__

#include "motion_manager.h"

#if defined(MOTION_LIBRARY)
#  define MOTION_EXPORT Q_DECL_EXPORT
#else
#  define MOTION_EXPORT Q_DECL_IMPORT
#endif

// ========== 私有实现类 ==========
class MOTION_EXPORT MotionManager::Private : public QObject
{
    Q_OBJECT
public:
    Private(MotionManager *parent);
    ~Private();

    void start();
    void stop();

    // 内部成员（原私有成员）
    MotionSolver        *curr_solver = nullptr;               //当前所用求解器
    MotionSolver        *m_solver = nullptr;                  //默认求解器
    MotionController    *m_controller = nullptr;
    MotionWorker        *m_motion = nullptr;                  //运动控制
    SampleWorker        *m_sample = nullptr;                  //采样Worker,定时采样
    //线程控制器
    QThread             *motion_thread = nullptr;             //运动控制线程，负责运行运动控制算法
    QThread             *sample_thread = nullptr;             //采样线程，负责定时采样
    QThread             *controller_thread = nullptr;         //控制器线程，负责操作控制器硬件

private slots:
    // 转发来自控制器信号的槽（原私有槽函数）
    void do_controllerConnected(bool success);
    void do_controllerDisconnected();
    void do_axisEnabled(int axis, bool success);
    void do_axisDisabled(int axis);
    void do_axisNumChanged(int num);
    void do_ledInSet(int index, bool on);
    void do_DAValueUpdated(int ch, int value, float volt);
    void do_sensorDataUpdated(QList<SensorData> dataNum);  //一次发送信号
signals:
    //与运动控制相关的信号（原私有信号）
    void controllerConnect(const QString& ip_addr);
    void controllerDisConnect();
    void axisEnable(int axis);
    void axisDisable(int axis);
    void axisStop(int axis);
    void axisVelSet(int axis, float vel);
    void axisPosSet(int axis, float pos);
    void axisModeSet(int axis, int mode);
    void changeSolver(MotionSolver* solver);                //切换求解器
    void changeNodeID(int nodeID);                          //切换传感器从站节点号
    //IO与传感器操作接口
    void setLedOut(int index, bool on);                     //设置LED输出
    void setDAValue(int nodeID, int ch, float volt);        //设置DA通道输出

private:
    MotionManager *m_manager;                               //指向外部类，用于发射公开信号
    void setupConnections();                                //设置内部信号与槽连接
};

#endif
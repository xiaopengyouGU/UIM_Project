#include "motion_manager.h"
#include "motion_controller.h"
#include "motion_solver.h"
#include "motion_worker.h"
#include "sample_worker.h"
#include "motion_manager_private.h"
#include <QThread>

// ========== 私有实现类 ==========
// Private 构造函数
MotionManager::Private::Private(MotionManager *parent)
    : QObject(parent), m_manager(parent)
{
    // 创建对象, 内存管理交给Qt实现
    m_solver = new MotionSolver(nullptr);
    curr_solver = m_solver;             //更新当前求解器
    m_controller = new MotionController;
    m_motion    = new MotionWorker;
    m_sample = new SampleWorker;
    //线程控制器，工控PC为多核系统，采用默认优先级即可
    motion_thread = new QThread(this);
    sample_thread = new QThread(this);
    controller_thread = new QThread(this);

    //运动控制线程，移入线程的对象不能有parent
    m_motion->setSolver(m_solver);
    m_motion->moveToThread(motion_thread);
    m_solver->moveToThread(motion_thread);
    //采样线程
    m_sample->connectController(m_controller);
    m_sample->moveToThread(sample_thread);
    //控制器线程
    m_controller->moveToThread(controller_thread);

    //绑定信号与槽
    setupConnections();
}

void MotionManager::Private::start()
{
    controller_thread->start();
    sample_thread->start();
    motion_thread->start();
}

void MotionManager::Private::stop()
{
    // 停止所有线程的事件循环(关闭内部定时器等)
    motion_thread->quit();
    sample_thread->quit();
    controller_thread->quit();
    // 等待线程安全结束
    motion_thread->wait();
    sample_thread->wait();
    controller_thread->wait();
}

MotionManager::Private::~Private()
{
    // 首先停止所有线程，对象管理交给Qt实现
    stop();
    //移动到线程中的对象还是得手动析构
    delete m_controller;
    delete m_solver;
    delete m_motion;
    delete m_sample;
}

void MotionManager::Private::setupConnections()
{
    // 连接私有信号到控制器槽
    connect(this, &Private::controllerConnect, m_controller, &MotionController::connect);
    connect(this, &Private::controllerDisConnect, m_controller, &MotionController::disConnect);
    connect(this, &Private::axisEnable, m_controller, &MotionController::enable);
    connect(this, &Private::axisDisable, m_controller, &MotionController::disable);
    connect(this, &Private::axisStop, m_controller, &MotionController::stop);
    connect(this, &Private::axisVelSet, m_controller, &MotionController::setVel);
    connect(this, &Private::axisPosSet, m_controller, &MotionController::setPos);
    connect(this, &Private::axisModeSet, m_controller, &MotionController::setMode); // 假设存在
    connect(this, &Private::changeSolver, m_motion, &MotionWorker::setSolver);
    connect(this, &Private::changeNodeID, m_sample, &SampleWorker::setNodeID);
    //IO与传感器信号
    connect(this, &Private::setLedOut, m_controller, &MotionController::setLedOut);
    connect(this, &Private::setDAValue, m_controller, &MotionController::setDAValue);
    // 线程启动信号
    connect(motion_thread, &QThread::started, m_motion, &MotionWorker::start);
    connect(sample_thread, &QThread::started, m_sample, &SampleWorker::start);
    // 传感器采集数据
    connect(m_sample, &SampleWorker::sensorDataUpdated, this, &Private::do_sensorDataUpdated);

    // 控制器内部信号转发到 Private 的槽
    connect(m_controller, &MotionController::axisEnabled, this, &Private::do_axisEnabled);
    connect(m_controller, &MotionController::axisDisabled, this, &Private::do_axisDisabled);
    connect(m_controller, &MotionController::axisNumChanged, this, &Private::do_axisNumChanged);
    connect(m_controller, &MotionController::controllerConnected, this, &Private::do_controllerConnected);
    connect(m_controller, &MotionController::controllerDisConnected, this, &Private::do_controllerDisconnected);
    connect(m_controller, &MotionController::DAValueUpdated, this, &Private::do_DAValueUpdated);
    connect(m_controller, &MotionController::ledInSet, this, &Private::do_ledInSet);
}

// 转发控制器信号到外部类（发射公开信号）
void MotionManager::Private::do_controllerConnected(bool success)
{
    emit m_manager->controllerConnected(success);
}
void MotionManager::Private::do_controllerDisconnected()
{
    emit m_manager->controllerDisConnected();
}
void MotionManager::Private::do_axisEnabled(int axis, bool success)
{
    emit m_manager->axisEnabled(axis, success);
}
void MotionManager::Private::do_axisDisabled(int axis)
{
    emit m_manager->axisDisabled(axis);
}
void MotionManager::Private::do_axisNumChanged(int num)
{
    emit m_manager->axisNumChanged(num);
}
void MotionManager::Private::do_ledInSet(int index, bool on)
{
    emit m_manager->ledInSet(index, on);
}

void MotionManager::Private::do_DAValueUpdated(int ch, int value, float volt)
{
    emit m_manager->DAValueUpdated(ch, value, volt);
}

void MotionManager::Private::do_sensorDataUpdated(QList<SensorData> dataNum)
{
    emit m_manager->sensorDataUpdated(dataNum);
}

// ========== MotionManager 公共接口实现 ==========
MotionManager::MotionManager(QObject *parent)
    : QObject(parent), pimpl(new Private(this))
{
    // 所有初始化已在 Private 构造函数中完成
}

MotionManager::~MotionManager() {}

//切换默认求解器
void MotionManager::setSolver(Solver_Type type)
{
    pimpl->curr_solver = pimpl->m_solver;
    // 原实现无额外操作
}

//支持动态切换用户自定义求解器
void MotionManager::setSolver(MotionSolver* solver)   //设置运动求解器
{
    if(!solver)     return;         //判空
    pimpl->curr_solver = solver;           //更新当前所用求解器
    emit pimpl->changeSolver(solver);
}

void MotionManager::setNodeID(int nodeID)                             //设置传感器从站节点号
{
    if(nodeID <= 1000) return;         //错误节点号，直接返回
    emit pimpl->changeNodeID(nodeID);
}

//IO与传感器操作接口
void MotionManager::setLedOut(int index, bool on)                     //设置LED输出
{
    emit pimpl->setLedOut(index, on);
}

void MotionManager::setDAValue(int nodeID, int ch, float volt)        //设置DA通道输出
{
    emit pimpl->setDAValue(nodeID, ch, volt);
}

void MotionManager::start()                                           //启动处理器
{
    pimpl->start();
}

void MotionManager::stop()                                            //停止管理器
{
    pimpl->stop();
}

void MotionManager::process()                                         //处理计算
{
    // 空实现，可由用户扩展
}

//运动控制器相关接口
void MotionManager::connectController(const QString& ip_addr)
{
    emit pimpl->controllerConnect(ip_addr);
}

void MotionManager::disConnectController()
{
    emit pimpl->controllerDisConnect();
}

void MotionManager::setAxisVel(int axis, float vel)
{
    emit pimpl->axisVelSet(axis, vel);
}

void MotionManager::setAxisPos(int axis, float pos)
{
    emit pimpl->axisPosSet(axis, pos);
}

void MotionManager::enableAxis(int axis)
{
    emit pimpl->axisEnable(axis);
}

void MotionManager::disableAxis(int axis)
{
    emit pimpl->axisDisable(axis);
}

void MotionManager::stopAxis(int axis)
{
    emit pimpl->axisStop(axis);
}

void MotionManager::setAxisMode(int axis, int mode)
{
    emit pimpl->axisModeSet(axis, mode);
}

MotionSolver* MotionManager::getSolver() const
{
    return pimpl->curr_solver;
}


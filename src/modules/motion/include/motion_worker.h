#ifndef MOTION_WORKER_H
#define MOTION_WORKER_H

#include <QObject>


class MotionSolver;                                         //前向声明，加快编译速度
#if defined(MOTION_LIBRARY)
#  define MOTION_EXPORT Q_DECL_EXPORT
#else
#  define MOTION_EXPORT Q_DECL_IMPORT
#endif

class MOTION_EXPORT MotionWorker:public QObject
{
    Q_OBJECT
public:
    MotionWorker(QObject* parent = nullptr);
    ~MotionWorker();
public slots:
    void setSolver(MotionSolver* solver);                   //设置运动求解器
    void process();                                         //处理计算
    void start();                                           //启动工作对象
public slots:                              

private:
    MotionSolver        *m_solver;
};

#endif // MOTION_WORKER_H

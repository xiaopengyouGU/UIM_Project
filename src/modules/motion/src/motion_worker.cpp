#include "motion_worker.h"
#include "motion_solver.h"

MotionWorker::MotionWorker(QObject* parent):QObject(parent){}

MotionWorker::~MotionWorker(){}

void MotionWorker::setSolver(MotionSolver* solver)                   //设置运动求解器
{
    if(!solver)         return;
    m_solver = solver;
}

void MotionWorker::process()                                         //处理计算
{
    //
}

void MotionWorker::start()                                          //启动工作对象
{
    //
}
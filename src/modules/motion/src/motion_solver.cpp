#include "motion_solver.h"
#include <QtMath>

MotionSolver::MotionSolver(QObject *parent):QObject(parent)
{
    m_length = 0;
    m_pos = nullptr;
    m_time = nullptr;
}

MotionSolver::~MotionSolver()
{
    if(m_pos != nullptr)    delete m_pos;
    if(m_time != nullptr)   delete m_time;
}

void MotionSolver::calc_ptt(double params)                                  //单轴梯形速度规划
{
    emit calcFinished();                                                    //发送计算完毕信号
}

void MotionSolver::calc_pts(double params)                                  //单轴S形速度规划
{
    emit calcFinished();                                                    //发送计算完毕信号
}

void MotionSolver::getPosTime(double *pos_num, double *t_num, int *length) //获取位置和时间序列
{
    pos_num = m_pos;
    t_num = m_time;
    length = &m_length;
}
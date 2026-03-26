#ifndef MOTION_SOLVER_H
#define MOTION_SOLVER_H

#include <QObject>

// motion_manager.h
#if defined(MOTION_LIBRARY)
#  define MOTION_EXPORT Q_DECL_EXPORT
#else
#  define MOTION_EXPORT Q_DECL_IMPORT
#endif

class MOTION_EXPORT MotionSolver:public QObject
{
    Q_OBJECT
public:
    MotionSolver(QObject* parent);
    ~MotionSolver();
public:
    void calc_ptt(double params);                                 //单轴梯形速度规划
    void calc_pts(double params);                                 //单轴S形速度规划
    void getPosTime(double *pos_num, double *t_num, int *length); //获取位置和时间序列
signals:
    void calcFinished();                    //计算完毕 
private:
    double *m_pos;                          //位置数组
    double *m_time;                         //时间数组
    int m_length;                           //数组长度
};

#endif // MOTION_SOLVER_H

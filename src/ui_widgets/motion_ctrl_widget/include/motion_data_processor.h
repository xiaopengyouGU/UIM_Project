#ifndef MOTION_DATA_PROCESSOR_H__
#define MOTION_DATA_PROCESSOR_H__

#include <QObject>

class ChartManager;
class SensorData;
struct axis_data_t;             //前向声明

#include "motion_manager.h"   // 包含 SensorData 的完整定义
// 这个对象负责处理数据采样线程发送的高频数据，运行在独立线程中，
// 减轻UI主线程的处理工作量，避免出现丢指令或界面卡顿

class MotionDataProcessor : public QObject
{
    Q_OBJECT
public:
    explicit MotionDataProcessor(QObject *parent = nullptr):QObject(parent){}
    void setManager(ChartManager *manager);                     //设置图表管理器
    void setAxisNum(QList<axis_data_t> *num);                   //设置轴数据数组
public slots:  
    void do_sensorDataUpdated(QList<SensorData>);         //通道数据更新，已处理60ms的数据
signals:
    void axisDataChanged(int ch, const QString& pos, const QString& vel);
private:
    ChartManager    *m_manager;
    QList<axis_data_t> *m_axes;
};



#endif
#include "motion_data_processor.h"
#include "motion_ctrl_widget.h"
#include "chart_manager.h"
#include "motion_manager.h"

void MotionDataProcessor::setManager(ChartManager *manager)                     //设置图表管理器
{
    if(!manager)        return;
    m_manager = manager;
}

void MotionDataProcessor::setAxisNum(QList<axis_data_t> *num)                   //设置轴数据数组
{
    if(!num)            return;
    m_axes = num;
}

void MotionDataProcessor::do_sensorDataUpdated(QList<SensorData> dataNum)
{
    if (!m_manager || !m_axes) return;

    const int cnt = dataNum.size();
    QList<ChannelData> cdNum;
    cdNum.resize(5);
    for (int i = 0; i < 5; ++i) {
        cdNum[i].actual.resize(cnt);
        cdNum[i].target.resize(cnt);
        cdNum[i].timestamp.resize(cnt);
    }

    // 静态变量：跨批次保持状态（使用实际时间戳）
    static float s_pos_last = 0.0f;
    static float s_vel_last = 0.0f;
    static qint64 s_last_timestamp = 0;

    // 首次调用时，从 m_axes 获取初始值
    static bool first_call = true;
    if (first_call && !m_axes->isEmpty()) {
        s_pos_last = m_axes->at(0).pos;
        s_vel_last = m_axes->at(0).vel;
        // 如果 m_axes 中没有时间戳，则使用第一个数据点的时间戳作为初始值
        if (cnt > 0) {
            s_last_timestamp = dataNum[0].timestamp;
        }
        first_call = false;
    }

    if (!m_axes->isEmpty()) {
        int mode = m_axes->at(0).mode;
        float axis_target = m_axes->at(0).target;

        float pos, vel, volt;
        for (int i = 0; i < cnt; ++i) {
            qint64 cur_timestamp = dataNum[i].timestamp;

            // 计算实际时间差（秒）
            float dt_sec = (cur_timestamp - s_last_timestamp) / 1000.0f;

            // 电压转换为位置
            volt = dataNum[i].volt[0];
            pos = (10.0f - volt) * 325.0f / 10.0f;

            // 速度计算（使用实际时间差）
            if (dt_sec > 0.001f) {  // 有效时间间隔（大于1ms）
                float vel_inst = (pos - s_pos_last) / dt_sec;

                // 可选：限制异常速度跳变（根据物理极限设置）
                const float MAX_VEL_CHANGE = 200.0f;  // mm/s，根据实际调整
                if (vel_inst > s_vel_last + MAX_VEL_CHANGE)
                    vel_inst = s_vel_last + MAX_VEL_CHANGE;
                if (vel_inst < s_vel_last - MAX_VEL_CHANGE)
                    vel_inst = s_vel_last - MAX_VEL_CHANGE;

                // 一阶低通滤波
                const float alpha = 0.2f;
                vel = alpha * vel_inst + (1.0f - alpha) * s_vel_last;
            } else {
                // 时间间隔无效（首次或异常），使用上一速度
                vel = s_vel_last;
            }

            // 更新静态状态
            s_pos_last = pos;
            s_vel_last = vel;
            s_last_timestamp = cur_timestamp;

            // 填充通道数据
            if (mode == MotionManager::Mode_Pos) {
                cdNum[0].target[i] = axis_target;
                cdNum[0].actual[i] = pos;
                cdNum[1].target[i] = 0;
                cdNum[1].actual[i] = vel;
            } else {
                cdNum[0].target[i] = 0;
                cdNum[0].actual[i] = pos;
                cdNum[1].target[i] = axis_target;
                cdNum[1].actual[i] = vel;
            }
        }

        // 更新 m_axes 中的状态（供外部查询）
        (*m_axes)[0].pos = s_pos_last;
        (*m_axes)[0].vel = s_vel_last;

        // 更新 UI（使用最后一个点的数据）
        QString str_pos = QString::number(pos, 'f', 2);
        QString str_vel = QString::number(vel, 'f', 2);
        emit axisDataChanged(0, str_pos, str_vel);
    }

    // 统一设置所有通道的时间戳，并填充通道2-4的实际值
    for (int i = 0; i < cnt; ++i) {
        for (int j = 0; j < 5; ++j) {
            cdNum[j].timestamp[i] = dataNum[i].timestamp;
        }
        for (int j = 0; j < 3; ++j) {
            cdNum[j + 2].actual[i] = dataNum[i].volt[j + 1];
        }
    }

    // 发送一批数据
    m_manager->addData(cdNum);
}

#include <QtMath>
#include "motion_controller.h"
#include "LTSMC.h"

MotionController::MotionController(QObject* parent):QObject(parent)
{   //控制器状态为未连接
    ctrlStatus = 0;
}

MotionController::~MotionController()
{
    if(!ctrlStatus)         return;     //控制器断连，直接返回
    emerStop();                         //先调用急停
    disConnect();                       //再关闭控制器
    ctrlStatus = 0;                     //更新状态
}

void MotionController::connect(const QString& ip_addr)
{
    if(ctrlStatus == 1) return;                 //避免重复初始化
    short ret;  //错误码返回值
    WORD ConnectNo = 0; //连接号，默认为0
    WORD type = 2;      //网口类型
    char* pconnectstring = ip_addr.toUtf8().data();
    DWORD baud = 0;
    ret = smc_board_init(ConnectNo,type,pconnectstring,baud);   //初始化运动控制器

    if(ret == 0)
    {
        ctrlStatus = 1;                 //控制器初始化完毕
        emit controllerConnected(true);
        DWORD num = 0;
        nmcs_get_total_axes(0, &num);
        emit axisNumChanged(num);       //发送控制的轴数量
        
        //配置从站传感器
        WORD nodeID = 1002;
        for(int i = 0; i < 4; i++)
        {
            WORD subIndex = i + 1;      //子索引
            nmcs_set_node_od(0, 2, nodeID, 0x3008, subIndex, 8, 0);     //设置，+-10V输入
        }
    }
    else
    {
        ctrlStatus = 0;                 //控制器初始化失败
        emit controllerConnected(false);
    }
}

void MotionController::disConnect()
{
    if(!ctrlStatus) return;             //控制器断连，则直接返回
    smc_board_close(0);                 //关闭控制器，释放资源
    ctrlStatus = 0;                     //更新状态
    emit controllerDisConnected();
}

int MotionController::getCtrlStatus()
{
    if(!ctrlStatus) return 0;         //控制器断连，直接返回
    //控制器已连接
    short ret = smc_get_connect_status(0);   //1 正常， 其他结果：丢失连接
    if(ret == 1)        return 1;         //控制器连接正常，返回1
    else                                  //控制器意外断连
    {
        ctrlStatus = 0;                   //手动设置断连
        //emit controllerDisConnected();     //发送断连信号
        return -1;
    }
}

//数据获取接口,对应EM06AX-E4 模拟量模块,输出0-10V, 输入0-10V
void MotionController::getADValue(int nodeID, int ch, int *value, float *volt)
{
    if(!ctrlStatus)         return;         //控制器断连，直接返回
    if(ch < 0 || ch >= 4)   return;         //通道不对，直接返回
    WORD index = ch + 0x3002;               //AD0采样值索引， TxPDO
    DWORD val;
    nmcs_get_node_od(0, 2, nodeID, index, 0x2, 32, &val);       //直接读取浮点数
    memcpy(volt, &val, sizeof(float));
    *value = (*volt) * 32767 / 10.0 + 32768;
}

void MotionController::setDAValue(int nodeID, int ch, float volt)
{
    if(!ctrlStatus)         return;         //控制器断连，直接返回
    if(ch < 0 || ch >= 2)   return;         //通道不对，直接返回
    WORD subIndex = (ch == 0)? 0x1 : 0x2;
    nmcs_set_node_od(0, 2, nodeID, 0x3010, subIndex, 8, 1);      //DA输出使能
    nmcs_set_node_od(0, 2, nodeID, 0x3009, subIndex, 8, 0);      //输出模式 2#10, +-10V输出
    WORD index = (ch == 0)? 0x3006 : 0x3007;
    //数值切换
    if(volt < -10.0)        volt = -10.0;
    else if(volt > 10.0)    volt = 10.0;

    DWORD value = 32767 * volt / 10.0 + 32768;                  //转换为16位无符号数
    DWORD volt_int = 0;
    memcpy(&volt_int, &volt, sizeof(volt));                     //数值转换
    //nmcs_set_node_od(0, 2, nodeID, index, 0x1, 16, value);    //此处得写拓展PDO
    nmcs_write_rxpdo_extra(0, 2, ch*2, 2, volt_int);
    emit DAValueUpdated(ch, value, volt);                       //发送信号
}

//轴控制相关接口
int MotionController::getAxisNum()      //获取EtherCAT轴数量
{
    if(!ctrlStatus)     return 0;         //控制器断连，直接返回
    DWORD num = 0;
    nmcs_get_total_axes(0, &num);
    emit axisNumChanged(num);
    return (int)num;
}

void MotionController::setVel(int axis, float vel)
{
    if (!ctrlStatus)    return;         //控制器断连，直接返回
    //如果函数调用没反应，可能是轴暂时没有使能
    smc_stop(0, axis, 0);               //先停机，避免状态切换时电机卡死
    //配置参数，默认
    WORD dir = (vel >= 0) ? 0 : 1;      //1:正方向，0:负方向。这是雷赛的，实际情况要反向
    vel = qAbs(vel);
    double Max_vel = vel/2.0 * 10000 ;  //最大速度：unit/s
    double Tacc = 0.2;                  //加速时间：s
    double Tdec = 0.2;                  //减速时间
    double Min_vel = 0;                 //起始速度
    double Stop_vel = 0;                //停止速度
    double s_para = 0.1;                //S形平滑参数
    WORD posi_mode = 0;                 //0：相对模式，1：绝对模式
    //调用该函数时默认轴已使能
    //设置单轴运动速度曲线
    smc_set_profile_unit(0, axis, Min_vel, Max_vel, Tacc, Tdec, Stop_vel);
    //设置单轴速度平滑S段参数
    smc_set_s_profile(0, axis, 0, s_para);
    //启动定速运动
    smc_vmove(0, axis, dir);
}

void MotionController::setPos(int axis, float pos)
{
    if (!ctrlStatus)    return;         //控制器断连，直接返回
    smc_stop(0, axis, 0);               //先停机，避免状态切换时电机卡死
    //如果函数调用没反应，可能是轴暂时没有使能
    //伺服设置的电子齿轮比：1048756/10000 ， 分子：电机编码器分辨率， 分母：电机转1圈所需脉冲数
    //配置参数，默认，此处的unit就是给伺服发送的脉冲数
    double Max_vel = 200000;            //最大速度：unit/s，此时对应5r/s,1200rpm ==> 240mm/s
    double Tacc = 0.1;                  //加速时间：s
    double Tdec = 0.2;                  //减速时间
    double Min_vel = 0;                 //起始速度
    double Stop_vel = 0;                //停止速度
    double s_para = 0.1;                //S形平滑参数
    //double gear_ratio = 1048756.0/10000.0;  //电子齿轮比
    double lead = 2.0;                  //导程 2mm
    double Dist = - 2* pos / lead * 10000.0; //运动距离：unit, 单位换算，实际方向相反

    WORD posi_mode = 1;                 //0：相对模式，1：绝对模式
    //调用该函数时默认轴已使能
    //设置单轴运动速度曲线
    smc_set_profile_unit(0, axis, Min_vel, Max_vel, Tacc, Tdec, Stop_vel);
    //设置单轴速度平滑S段参数
    smc_set_s_profile(0, axis, 0, s_para);
    //启动定长运动
    smc_pmove_unit(0, axis, Dist, posi_mode);
}

float MotionController::getVel(int axis)       //单位：unit/s
{
    if (!ctrlStatus)    return 0;              //控制器断连，直接返回0
    //如果函数调用没反应，可能是轴暂时没有使能
    return 0;
}

float MotionController::getPos(int axis)                         //单位：unit
{
    if (!ctrlStatus)    return 0;              //控制器断连，直接返回0
    return 0;
}

int MotionController::getStatus(int axis)                       //获取轴状态
{
    if (!ctrlStatus)    return 0;              //控制器断连，直接返回0
    WORD stateMachine = 0;
    nmcs_get_axis_state_machine(0,axis,&stateMachine);
    return (int)stateMachine;
}

void MotionController::enable(int axis)
{
    if (!ctrlStatus)    return;         //控制器断连，直接返回
    nmcs_set_axis_enable(0, axis);      //轴
    WORD stateMachine = 0;              //轴状态机
    //传感器采集模块在拓展TxPDO中
    //等待轴状态机切换为轴使能状态
    int count = 0;                      //等待轴使能
    while(stateMachine != 4)
    {
        nmcs_get_axis_state_machine(0,axis,&stateMachine);
        count++;
        if (count > 500)    break;      //长时间未使能，直接退出
    }
    if(stateMachine == 4)
        emit axisEnabled(axis, true);
    else
        emit axisEnabled(axis, false);
}

void MotionController::disable(int axis)
{
    if (!ctrlStatus)    return;         //控制器断连，直接返回
    nmcs_set_axis_disable(0, axis);     //轴0
    emit axisDisabled(axis);            //发送信号
}

void MotionController::stop(int axis)
{
    if(!ctrlStatus)     return;         //控制器断连，直接返回
    smc_stop(0,axis,0);                 //停止轴
    emit axisStop(axis);
}

void MotionController::emerStop()
{
    if(!ctrlStatus)     return;         //控制器断连，直接返回
    smc_emg_stop(0);
}

void MotionController::setMode()
{
    if(!ctrlStatus)     return;         //控制器断连，直接返回
}

//IO控制相关函数
void MotionController::getLedIn(int index, bool *on)
{
    if (!ctrlStatus)    return;         //控制器断连，直接返回
    if((index < 0) || (index > 12))     index = 12;
    DWORD IoValue = 0;
    if(index == 12)
    {
        IoValue = smc_read_inport(0, 0);
        for(int i = 0; i < 12; i++)
        {
            bool state = (IoValue & (1 << i))? false: true;
            emit ledInSet(i, state);
        }
    }
    else
    {
        short value = smc_read_inbit(0, index);
        *on = (value == 0)? true : false;
        emit ledInSet(index, *on);
    }
}

void MotionController::setLedOut(int index, bool on)           //共12个In,  0-11
{
    if (!ctrlStatus)    return;         //控制器断连，直接返回
    if((index < 0) || (index > 12)) index = 12;
    WORD IoValue = (on == true) ? 0:1;  // 1:高电平，0：低电平， IO口为NPN类型，导通为0， 关断为1
    if(index == 12)                     //全亮或全灭
    {
        IoValue = (IoValue == 0) ? 0 : 4095;    //2^12 -1
        smc_write_outport(0, 0, IoValue);       //全亮或全灭
    }
    else
    {
        smc_write_outbit(0, index, IoValue);
    }
}


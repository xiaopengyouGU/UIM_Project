#ifndef MOTION_CTRL_WIDGET_H
#define MOTION_CTRL_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QThread>

class SensorDialog;                     //前向声明，提高编译速度
class IODialog;
class MotionManager;
class ChartManager;
class ChartDialog;
class MotionDataProcessor;
//data definition
struct axis_data_t{
    int axis;
    float te;                           //转矩，单位为额定值的千分之一
    float vel;                          //速度, RPM
    float pos;                          //位置，
    float target;
    int mode;                           //运动模式
    int status;                         //轴状态参数
};


QT_BEGIN_NAMESPACE
namespace Ui {
class MotionCtrlWidget;
}
QT_END_NAMESPACE

class MotionCtrlWidget : public QWidget
{
    Q_OBJECT

public:
    MotionCtrlWidget(QWidget *parent = nullptr);
    ~MotionCtrlWidget();
    void buildController();           //控制器相关对象创建
    void buildUI_Chart();             //图表创建
    void buildUI_Others();            //其余UI部分创建
protected:
    bool eventFilter(QObject *obj, QEvent *event) override; //采用事件过滤器
public slots:
    //控制器与轴
    void do_controllerConnected(bool success);              //控制器连接成功
    void do_controllerDisConnected();                       //控制器断连成功
    void do_axisEnabled(int axis, bool success);            //轴使能成功
    void do_axisDisabled(int axis);                         //轴失能成功
    void do_axisNumChanged(int num);                        //轴的数量更新
    //图表控制
    void do_exportFinished(bool success, const QString& msg);   //数据导出完毕
    void do_axisDataChanged(int ch, const QString& pos, const QString& vel);//数据更新
private slots:
    void on_btnSensor_clicked();                            //传感器配置
    void on_btnIO_clicked();                                //IO配置
    void on_comboAxis_currentIndexChanged(int index);
    void on_btnStop_clicked();                              //停机
    void on_btnStart_clicked();                             //启动
    void on_btnTarget_clicked();                            //发送目标值
    void on_btnMode_clicked();                              //模式切换
    void on_btnEnableAxis_clicked();                        //轴使能
    void on_btnConnect_clicked();                           //连接控制器
    void on_btnDisCon_clicked();                            //断开控制器
    void on_btnDataExport_clicked();                        //CSV数据格式导出
    void on_btnChart_clicked();                             //图表显示
private:
    QList<axis_data_t> axis_num;
    //与控制器操作相关的对象
    SensorDialog        *sensor;
    IODialog            *io;
    MotionManager       *motion_manager;            //运动管理器，负责motion库模块对象管理
    //与图表相关的对象
    ChartManager        *chart_manager;             //图表管理器，负责chart模块对象管理
    ChartDialog         *m_dialog;                  //图表控制对话框
    //
    MotionDataProcessor *processor;                 //负责处理高频接收数据
    QThread             *data_thread;               //数据线程，
private:
    Ui::MotionCtrlWidget *ui;
};
#endif // MOTION_CTRL_WIDGET_H

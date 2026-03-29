#include "motion_ctrl_widget.h"
#include "ui_motion_ctrl_widget.h"

#include <QMessageBox>
#include <QFileDialog>
#include "sensor_dialog.h"
#include "io_dialog.h"
#include "chart.h"                      //图表显示模块
#include "chart_dialog.h"               //图表控制对话框，配合图表模块使用
#include "motion.h"                     //运动控制模块

#include "motion_data_processor.h"

MotionCtrlWidget::MotionCtrlWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MotionCtrlWidget)
{
    ui->setupUi(this);

    buildUI_Chart();
    buildController();
    buildUI_Others();
}

MotionCtrlWidget::~MotionCtrlWidget()
{
    data_thread->quit();
    data_thread->wait();
    delete processor;
    
    delete ui;
}

void MotionCtrlWidget::buildUI_Chart()
{
    //设置5个通道
    chart_manager = new ChartManager(5, this);              //创建图表管理器，负责图表模块对象管理
    ui->chartView->installEventFilter(this);                //安装事件管理器，启动鼠标双击事件
    ui->chartView->setChartManager(chart_manager);          //配置图表管理器
    m_dialog = new ChartDialog(this);                       //图表控制对话框
    m_dialog->connectManager(chart_manager);                //信号与槽连接
    //修改图表图例名
    chart_manager->setLegendName(0, "位置");
    chart_manager->setLegendName(1, "速度");
    chart_manager->setLegendName(2, "AD CH0 Volt");
    chart_manager->setLegendName(3, "AD CH1 Volt");
    chart_manager->setLegendName(4, "AD CH2 Volt");
    //绑定信号与槽
    connect(chart_manager, &ChartManager::exportDataFinished, this, &MotionCtrlWidget::do_exportFinished);
    //启动图表管理器
    chart_manager->setPeriod(120);                           //设置图表刷新周期：150ms
    //chart_manager->setPeriod(200);
    chart_manager->start();
}

void MotionCtrlWidget::buildUI_Others()
{
    
}

void MotionCtrlWidget::buildController()
{
    ui->btnDisCon->setDisabled(true);
    sensor = new SensorDialog(this);
    io = new IODialog(this);
    motion_manager = new MotionManager(this);      //创建运动管理器
    //数据接收处理线程，减轻UI主线程处理压力
    processor = new MotionDataProcessor;
    data_thread = new QThread(this);                //数据处理线程
    processor->setManager(chart_manager);           //绑定对象
    processor->setAxisNum(&axis_num);               
    processor->moveToThread(data_thread);           //对象移动到线程中
    data_thread->start();

    connect(motion_manager, &MotionManager::sensorDataUpdated, processor, &MotionDataProcessor::do_sensorDataUpdated);
    connect(processor, &MotionDataProcessor::axisDataChanged, this, &MotionCtrlWidget::do_axisDataChanged);
    //绑定信号与槽
    connect(motion_manager, &MotionManager::axisEnabled, this, &MotionCtrlWidget::do_axisEnabled);
    connect(motion_manager, &MotionManager::axisDisabled, this, &MotionCtrlWidget::do_axisDisabled);
    connect(motion_manager, &MotionManager::controllerConnected, this, &MotionCtrlWidget::do_controllerConnected);
    connect(motion_manager, &MotionManager::controllerDisConnected, this, &MotionCtrlWidget::do_controllerDisConnected);
    connect(motion_manager, &MotionManager::axisNumChanged, this, &MotionCtrlWidget::do_axisNumChanged);
    //IO口配置
    connect(io, &IODialog::ledOutSet, motion_manager, &MotionManager::setLedOut);
    connect(motion_manager, &MotionManager::ledInSet, io, &IODialog::setLedIn);
    //传感器配置
    connect(sensor, &SensorDialog::DAValueSet, motion_manager, &MotionManager::setDAValue);
    connect(motion_manager, &MotionManager::sensorDataUpdated, sensor, &SensorDialog::do_sensorDataUpdated);
    connect(motion_manager, &MotionManager::DAValueUpdated, sensor, &SensorDialog::do_DAValueUpdated);
    //启动运动管理器
    motion_manager->setNodeID(1002);        //设置传感器从站节点号
    motion_manager->start();            
}

void MotionCtrlWidget::on_btnSensor_clicked()
{
    sensor->show();
}

void MotionCtrlWidget::on_btnIO_clicked()
{
    io->show();
}

void MotionCtrlWidget::on_comboAxis_currentIndexChanged(int index)
{
    if(index == -1) return;                       //没有任何轴时
    ui->editPos->setText(QString("%1").arg(axis_num[index].pos));
    ui->editVel->setText(QString("%1").arg(axis_num[index].vel));
    ui->editTorque->setText(QString("%1").arg(axis_num[index].te));
    ui->editTarget->setText(QString("%1").arg(axis_num[index].target));
    ui->comboMode->setCurrentIndex(axis_num[index].mode);
    QString str = (axis_num[index].status == 1)? "轴失能":"轴使能";
}

void MotionCtrlWidget::on_btnStop_clicked()
{
    int index = ui->comboAxis->currentIndex();
    if(index == -1) return;                 //此时没有任何轴的数据
    motion_manager->stopAxis(index);
    QMessageBox::information(this, "信息", QString("停止轴 %1").arg(index+1));
}

void MotionCtrlWidget::on_btnStart_clicked()
{
    int index = ui->comboAxis->currentIndex();
    if(index == -1) return;                 //此时没有任何轴的数据
    int mode = axis_num[index].mode;        //配置好的模式
    switch(mode)
    {
        case MotionManager::Mode_Vel:
        {
            float vel = axis_num[index].target;
            motion_manager->setAxisVel(index, vel);
            break;
        }
        case MotionManager::Mode_Pos:
        {
            float pos = axis_num[index].target;
            motion_manager->setAxisPos(index, pos);
            break;
        }
        defalt: break;
    }
    QMessageBox::information(this, "信息", QString("启动轴 %1").arg(index+1));
}

void MotionCtrlWidget::on_btnTarget_clicked()
{
    int index = ui->comboAxis->currentIndex();
    if(index == -1) return;                 //此时没有任何轴的数据
    float target = ui->editTarget->text().toFloat();
    axis_num[index].target = target;
}

void MotionCtrlWidget::on_btnMode_clicked()
{
    int index = ui->comboAxis->currentIndex();
    if(index == -1) return;                 //此时没有任何轴的数据
    int mode = ui->comboMode->currentIndex();
    axis_num[index].mode = mode;            //保存设置的模式
    motion_manager->setAxisMode(index, mode);
}

void MotionCtrlWidget::on_btnEnableAxis_clicked()
{
    int index = ui->comboAxis->currentIndex();
    if(index == -1) return;                 //此时没有任何轴的数据
    if(ui->btnEnableAxis->text() == "轴使能")
        motion_manager->enableAxis(index);  //轴使能
    else
        motion_manager->disableAxis(index); //轴失能
}

void MotionCtrlWidget::on_btnConnect_clicked()
{
    //发送信号给控制器
    QString ip_addr = ui->editIP->text();
    motion_manager->connectController(ip_addr);
}

void MotionCtrlWidget::on_btnDisCon_clicked()
{
    motion_manager->disConnectController();
}

void MotionCtrlWidget::on_btnDataExport_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出CSV数据", "", "CSV文件(*.csv)");
    if(fileName.isEmpty()) return;
    chart_manager->exportData(fileName, 0, 0);  //0,0 表示导出全数据
}

void MotionCtrlWidget::on_btnChart_clicked()
{
    m_dialog->show();
}

void MotionCtrlWidget::do_controllerConnected(bool success)
{
    if(success)
    {
        QMessageBox::information(this, "信息", "控制器连接成功");
        ui->btnDisCon->setEnabled(true);
        ui->btnConnect->setEnabled(false);
    }
    else
        QMessageBox::critical(this, "错误", "控制器连接失败");
}

void MotionCtrlWidget::do_controllerDisConnected()
{
    QMessageBox::information(this, "信息", "控制器断开连接");
    axis_num.clear();                       //清空轴数据
    ui->comboAxis->clear();                 //清空轴列表
    ui->btnConnect->setEnabled(true);
    ui->btnDisCon->setEnabled(false);
}

void MotionCtrlWidget::do_axisEnabled(int axis, bool success)
{
    if(axis_num.size() < axis)    return;

    if(success)
    {
        axis_num[axis].status = 1;         //使能
        ui->btnEnableAxis->setText("轴失能");
        QMessageBox::information(this, "信息", QString("轴%1使能成功").arg(axis+1));
    }
    else
    {
        axis_num[axis].status = 0;         //失能
        ui->btnEnableAxis->setText("轴使能");
        QMessageBox::critical(this, "错误", QString("轴%1使能失败").arg(axis+1));
    }
}

void MotionCtrlWidget::do_axisDisabled(int axis)
{
    if(axis_num.size() < axis)    return;
    axis_num[axis].status = 0;         //失能
    ui->btnEnableAxis->setText("轴使能");
    QMessageBox::information(this, "信息", QString("轴%1失能成功").arg(axis+1));
}

void MotionCtrlWidget::do_axisNumChanged(int num)
{
    axis_num.clear();
    axis_num.reserve(num);
    ui->comboAxis->clear();
    for(int i = 0; i < num; i++)
    {
        axis_data_t axis_data = {0};
        axis_data.axis = i;
        axis_num.append(axis_data);         //先添加数据，再增加Item
        ui->comboAxis->addItem(QString("轴%1").arg(i+1));
    }
}

void MotionCtrlWidget::do_exportFinished(bool success, const QString& msg)   //数据导出完毕
{
    if(success)
        QMessageBox::information(this, "导出完成", msg);
    else
        QMessageBox::warning(this, "导出失败", msg);
}

void MotionCtrlWidget::do_axisDataChanged(int ch, const QString& pos, const QString& vel)//数据更新         //数据更新
{
   if(ch == ui->comboAxis->currentIndex())
   {
        ui->editPos->setText(pos);          //更新轴数据
        ui->editVel->setText(vel);
   }
}

bool MotionCtrlWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->chartView)
    {
        if(event->type() == QEvent::MouseButtonDblClick) 
        {
            m_dialog->do_modeChanged(0);  //双击恢复自动模式
            return true;             //其他事件继续交由主窗口对象处理
        }
        if (event->type() == QEvent::MouseButtonPress) 
        {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton && chart_manager->getMode() == 0)
            {
               m_dialog->do_modeChanged(1);            //切换为手动模式
                // 不返回 true，让事件继续传递给 ChartView 以便开始平移
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

/***********************************************************************************/
//内部函数与静态函数
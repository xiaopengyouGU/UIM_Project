#include "sensor_dialog.h"
#include "ui_sensor_dialog.h"
#include "motion_manager.h"

SensorDialog::SensorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SensorDialog)
{
    ui->setupUi(this);
}

SensorDialog::~SensorDialog()
{
    delete ui;
}

void SensorDialog::do_sensorDataUpdated(QList<SensorData> dataNum)
{   //60ms更新一次数据
    //此时通道和值都已经检查过了，不必重复检查
    int size = dataNum.size() - 1;
    if(size < 0) return;
    for(int ch = 0; ch < 4; ch++)
    {
        QString valName = QString("editADValCH%1").arg(ch);
        QString volName = QString("editADVolCH%1").arg(ch);
        QLineEdit* editVal= findChild<QLineEdit*>(valName);
        QLineEdit* editVol= findChild<QLineEdit*>(volName);
        editVal->setText(QString("%1").arg(dataNum[size].value[ch]));
        editVol->setText(QString::asprintf("%.2f",dataNum[size].volt[ch]));
    }
}

void SensorDialog::do_DAValueUpdated(int ch, int value, float volt)
{
    //此时通道和值都已经检查过了，不必重复检查
    QString valName = QString("editDAValCH%1").arg(ch);
    QString volName = QString("editDAVolCH%1").arg(ch);
    QLineEdit* editVal= findChild<QLineEdit*>(valName);
    QLineEdit* editVol= findChild<QLineEdit*>(volName);
    editVal->setText(QString("%1").arg(value));
    editVol->setText(QString::asprintf("%.2f",volt));
}

void SensorDialog::on_btnSetDA_clicked()
{
    int ch = ui->comboDA->currentIndex();
    float volt = ui->editSetDA->text().toFloat();
    emit DAValueSet(1002, ch, volt);    //发送信号，触发控制器输出
}


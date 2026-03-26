#ifndef SENSOR_DIALOG_H
#define SENSOR_DIALOG_H

#include <QDialog>

namespace Ui {
class SensorDialog;
}

class SensorData;

class SensorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorDialog(QWidget *parent = nullptr);
    ~SensorDialog();
public slots:
    void do_sensorDataUpdated(QList<SensorData> dataNum);
    void do_DAValueUpdated(int ch, int value, float volt);
signals:
    void DAValueSet(int nodeID,int ch, float volt);
private slots:
    void on_btnSetDA_clicked();

private:
    Ui::SensorDialog *ui;
};

#endif // SENSOR_DIALOG_H

#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <QLabel>

class StatusLed : public QLabel
{
    Q_OBJECT
public:
    explicit StatusLed(QWidget *parent = nullptr, int size = 16);
    void setState(bool on);  // true = 高电平/亮，false = 低电平/暗
    bool getState() const       { return m_state; }
private:
    int m_size;
    bool m_state;
    QString m_onColor;
    QString m_offColor;
};

#endif // STATU_SLED_H

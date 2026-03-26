#include "status_led.h"

StatusLed::StatusLed(QWidget *parent, int size)
    : QLabel(parent), m_size(size), m_onColor("green"), m_offColor("lightgray")
{
    setFixedSize(m_size, m_size);
    // 初始为关闭状态
    setState(false);
    m_state = false;
}

void StatusLed::setState(bool on)
{
    QString color = on ? m_onColor : m_offColor;
    setStyleSheet(QString(
                      "QLabel {"
                      "   background-color: %1;"
                      "   border-radius: %2px;"
                      "}"
                      ).arg(color).arg(m_size / 2));
    m_state = on;
}

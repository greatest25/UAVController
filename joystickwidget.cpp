#include "joystickwidget.h"
#include <QDebug>

/*
摇杆组件
成员变量
m_joystickPos:摇杆位置
m_dragging:是否拖动
m_lastAngle:上一次角度
*/
JoystickWidget::JoystickWidget(QWidget *parent)
    : QWidget(parent)
    , m_joystickPos(0, 0)
    , m_dragging(false)
    , m_lastAngle(-1)
{
    setMinimumSize(100, 100);
    updateGeometry();
}

//重置事件
void JoystickWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_center = rect().center();
    m_radius = qMin(width(), height()) / 2 - 5; // 半径
}

//绘制事件
void JoystickWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制外圈阴影效果
    painter.setPen(QPen(QColor(0, 0, 0, 100), 2));
    painter.setBrush(QBrush(QColor(0, 0, 0, 50)));
    painter.drawEllipse(m_center.x() - m_radius + 2, m_center.y() - m_radius + 2,
                       m_radius * 2, m_radius * 2);

    // 绘制外圈（摇杆可移动范围）- 手柄风格
    QRadialGradient outerGradient(m_center, m_radius);
    outerGradient.setColorAt(0, QColor(70, 70, 70));
    outerGradient.setColorAt(0.8, QColor(50, 50, 50));
    outerGradient.setColorAt(1, QColor(30, 30, 30));

    painter.setPen(QPen(QColor(155, 89, 182), 3));
    painter.setBrush(QBrush(outerGradient));
    painter.drawEllipse(m_center.x() - m_radius, m_center.y() - m_radius,
                       m_radius * 2, m_radius * 2);

    // 绘制内圈纹理
    painter.setPen(QPen(QColor(100, 100, 100), 1));
    for (int i = 1; i <= 3; i++) {
        int innerRadius = m_radius * 0.2 * i;
        painter.drawEllipse(m_center.x() - innerRadius, m_center.y() - innerRadius,
                           innerRadius * 2, innerRadius * 2);
    }

    // 绘制摇杆指针 - 3D效果
    QPointF knobPos = m_center + m_joystickPos;

    // 摇杆阴影
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(0, 0, 0, 80)));
    painter.drawEllipse(knobPos.x() - 10, knobPos.y() - 8, 20, 20);

    // 摇杆主体 - 渐变效果
    QRadialGradient knobGradient(knobPos, 12);
    knobGradient.setColorAt(0, QColor(220, 220, 220));
    knobGradient.setColorAt(0.6, QColor(180, 180, 180));
    knobGradient.setColorAt(1, QColor(120, 120, 120));

    painter.setPen(QPen(QColor(155, 89, 182), 2));
    painter.setBrush(QBrush(knobGradient));
    painter.drawEllipse(knobPos.x() - 12, knobPos.y() - 12, 24, 24);

    // 摇杆高光
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(255, 255, 255, 150)));
    painter.drawEllipse(knobPos.x() - 8, knobPos.y() - 10, 8, 6);

    // 绘制中心十字线
    painter.setPen(QPen(QColor(155, 89, 182, 150), 1));
    painter.drawLine(m_center.x() - 10, m_center.y(), m_center.x() + 10, m_center.y());
    painter.drawLine(m_center.x(), m_center.y() - 10, m_center.x(), m_center.y() + 10);

    // 绘制中心点
    painter.setPen(QPen(QColor(155, 89, 182), 2));
    painter.setBrush(QBrush(QColor(155, 89, 182)));
    painter.drawEllipse(m_center.x() - 2, m_center.y() - 2, 4, 4);
}

//鼠标按下事件
void JoystickWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint localPos = event->pos();
        
        // 检查是否点击在摇杆活动范围内
        if (QLineF(m_center, localPos).length() <= m_radius) {
            m_dragging = true;
            updateJoystickPosition(localPos);
        }
    }
}

//鼠标移动事件
void JoystickWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        updateJoystickPosition(event->pos());
    }
}

//鼠标释放事件
void JoystickWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        resetJoystick();
    }
}

//更新摇杆位置
void JoystickWidget::updateJoystickPosition(const QPoint &pos)
{
    // 计算相对于中心的位置
    m_joystickPos = pos - m_center;
    
    // 限制在圆内
    double distance = QLineF(QPointF(0, 0), m_joystickPos).length();
    if (distance > m_radius) {
        QLineF line(QPointF(0, 0), m_joystickPos);
        line.setLength(m_radius);
        m_joystickPos = line.p2();
    }
    
    update();
    
    // 发射信号
    int angle = getAngle();
    if (angle != m_lastAngle) {
        m_lastAngle = angle;
        emit angleChanged(angle);
    }
    emit positionChanged(m_joystickPos);
}

//重置摇杆
void JoystickWidget::resetJoystick()
{
    m_joystickPos = QPointF(0, 0);
    m_lastAngle = -1;
    update();
    
    emit angleChanged(-1);
    emit positionChanged(m_joystickPos);
    //qDebug() << "摇杆返回中心！";
}

//获取角度
int JoystickWidget::getAngle() const
{
    if (m_joystickPos.x() == 0 && m_joystickPos.y() == 0) {
        return -1; // 在中心位置
    }
    
    // 计算角度（0度为正北方向）
    double angle = qAtan2(m_joystickPos.x(), -m_joystickPos.y()) * 180.0 / M_PI;
    if (angle < 0) {
        angle += 360.0;
    }
    
    return static_cast<int>(angle);
}

//获取距离
double JoystickWidget::getDistance() const
{
    if (m_radius == 0) return 0.0;
    
    double distance = QLineF(QPointF(0, 0), m_joystickPos).length();
    return qMin(distance / m_radius, 1.0);
}

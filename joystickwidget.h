#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPointF>
#include <QtMath>

class JoystickWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JoystickWidget(QWidget *parent = nullptr);
    
    // 获取摇杆角度（0-359度，-1表示在中心）
    int getAngle() const;
    
    // 获取摇杆偏移量（相对于中心的距离，0-1之间）
    double getDistance() const;

    // 重置摇杆到中心位置
    void resetJoystick();

signals:
    void angleChanged(int angle);//角度改变信号
    void positionChanged(QPointF position);//位置改变信号

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateJoystickPosition(const QPoint &pos);//更新摇杆位置
    
    QPointF m_joystickPos;      // 摇杆相对于中心的位置
    QPoint m_center;            // 摇杆中心点
    int m_radius;               // 摇杆活动半径
    bool m_dragging;            // 是否正在拖拽
    int m_lastAngle;            // 上次的角度
};

#endif // JOYSTICKWIDGET_H

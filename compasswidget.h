#ifndef COMPASSWIDGET_H
#define COMPASSWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QtMath>
#include <QFont>
#include <QColor>

class CompassWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CompassWidget(QWidget *parent = nullptr);
    
    // 设置当前无人机的前进方向（角度，0度为正北，顺时针）
    void setDirection(double angle);
    
    // 设置当前无人机的速度（用于动画效果）
    void setSpeed(double speed);
    
    // 获取当前方向
    double getDirection() const { return m_currentDirection; }

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateAnimation();

private:
    // 绘制方法
    void drawCompassBackground(QPainter &painter);
    void drawCompassRose(QPainter &painter);
    void drawDirectionIndicator(QPainter &painter);
    void drawSpeedIndicator(QPainter &painter);
    void drawDirectionText(QPainter &painter);
    
    // 辅助方法
    QString getDirectionText(double angle) const;
    QColor getSpeedColor(double speed) const;
    
    // 动态计算半径，让指南针自适应控件大小
    int getOuterRadius() const { return qMin(width(), height()) / 2 - 6; }
    int getInnerRadius() const { return getOuterRadius() * 0.8; }
    
    // 成员变量
    double m_currentDirection;                // 当前方向（度）
    double m_targetDirection;                 // 目标方向（度）
    double m_currentSpeed;                    // 当前速度
    
    QTimer *m_animationTimer;                 // 动画定时器
    
    // 颜色定义
    QColor m_backgroundColor;
    QColor m_borderColor;
    QColor m_roseColor;
    QColor m_needleColor;
    QColor m_textColor;
    QColor m_speedLowColor;
    QColor m_speedMediumColor;
    QColor m_speedHighColor;
};

#endif // COMPASSWIDGET_H

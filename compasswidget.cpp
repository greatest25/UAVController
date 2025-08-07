#include "compasswidget.h"

CompassWidget::CompassWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentDirection(0.0)
    , m_targetDirection(0.0)
    , m_currentSpeed(0.0)
{
    // 设置最小大小，允许缩放
    setMinimumSize(80, 80); 
    setMaximumSize(512, 512); 
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);    
    
    // 初始化颜色
    m_backgroundColor = QColor(240, 240, 240);    // 浅灰色背景
    m_borderColor = QColor(100, 100, 100);        // 深灰色边框
    m_roseColor = QColor(50, 50, 50);             // 深灰色罗盘刻度
    m_needleColor = QColor(255, 0, 0);            // 红色指针
    m_textColor = QColor(0, 0, 0);                // 黑色文字
    m_speedLowColor = QColor(0, 255, 0);          // 绿色（低速）
    m_speedMediumColor = QColor(255, 255, 0);     // 黄色（中速）
    m_speedHighColor = QColor(255, 0, 0);         // 红色（高速）
    
    // 创建动画定时器
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &CompassWidget::updateAnimation);
    m_animationTimer->start(50); // 50ms更新一次，平滑动画
}

//设定方向
void CompassWidget::setDirection(double angle)
{
    // 规范化角度到0-360度
    while (angle < 0) angle += 360;
    while (angle >= 360) angle -= 360;
    
    m_targetDirection = angle;
}

//设定速度
void CompassWidget::setSpeed(double speed)
{
    m_currentSpeed = qMax(0.0, speed); // 确保速度非负
    update();
}

//绘制指南针
void CompassWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制指南针背景
    drawCompassBackground(painter);

    // 绘制罗盘刻度（固定不旋转）
    drawCompassRose(painter);

    // 绘制方向指针（指向当前无人机方向）
    drawDirectionIndicator(painter);

    // 绘制速度指示器
    drawSpeedIndicator(painter);

    // 注释掉方向文字绘制，让指南针更简洁
    // drawDirectionText(painter);
}

//更新动画
void CompassWidget::updateAnimation()
{
    // 平滑过渡到目标方向
    double diff = m_targetDirection - m_currentDirection;
    
    // 处理角度跨越0度的情况
    if (diff > 180) {
        diff -= 360;
    } else if (diff < -180) {
        diff += 360;
    }
    
    // 平滑插值
    if (qAbs(diff) > 1.0) {
        m_currentDirection += diff * 0.1; // 10%的插值速度
        
        // 规范化角度
        while (m_currentDirection < 0) m_currentDirection += 360;
        while (m_currentDirection >= 360) m_currentDirection -= 360;
        
        update();
    }
}

//绘制指南针背景
void CompassWidget::drawCompassBackground(QPainter &painter)
{
    QPoint center(width() / 2, height() / 2);//中心点
    
    // 动态计算半径，让指南针自适应控件大小
    int outerRadius = getOuterRadius();
    int innerRadius = getInnerRadius();

    // 绘制外圆阴影
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(0, 0, 0, 80)));
    painter.drawEllipse(center.x() - outerRadius + 2, center.y() - outerRadius + 2,
                       outerRadius * 2, outerRadius * 2);

    // 绘制外圆背景 - 金属质感
    QRadialGradient outerGradient(center, outerRadius);
    outerGradient.setColorAt(0, QColor(200, 200, 200));
    outerGradient.setColorAt(0.7, QColor(150, 150, 150));
    outerGradient.setColorAt(1, QColor(100, 100, 100));

    painter.setPen(QPen(QColor(22, 160, 133), 3));
    painter.setBrush(QBrush(outerGradient));
    painter.drawEllipse(center.x() - outerRadius, center.y() - outerRadius,
                       outerRadius * 2, outerRadius * 2);

    // 绘制内圆 - 白色背景带渐变
    QRadialGradient innerGradient(center, innerRadius);
    innerGradient.setColorAt(0, QColor(255, 255, 255));
    innerGradient.setColorAt(0.8, QColor(240, 240, 240));
    innerGradient.setColorAt(1, QColor(220, 220, 220));

    painter.setPen(QPen(QColor(22, 160, 133), 2));
    painter.setBrush(QBrush(innerGradient));
    painter.drawEllipse(center.x() - innerRadius, center.y() - innerRadius,
                       innerRadius * 2, innerRadius * 2);

    // 绘制装饰圆环
    painter.setPen(QPen(QColor(22, 160, 133, 100), 1));
    painter.setBrush(Qt::NoBrush);
    for (int i = 1; i <= 3; i++) {
        int ringRadius = innerRadius + (outerRadius - innerRadius) * i / 4;
        painter.drawEllipse(center.x() - ringRadius, center.y() - ringRadius,
                           ringRadius * 2, ringRadius * 2);
    }
}

//绘制罗盘刻度
void CompassWidget::drawCompassRose(QPainter &painter)
{
    QPoint center(width() / 2, height() / 2);
    painter.setPen(QPen(m_roseColor, 2));

    // 动态计算半径
    int outerRadius = getOuterRadius();
    int innerRadius = getInnerRadius();

    // 设置字体，确保使用系统默认字体避免乱码
    QFont font;
    font.setFamily("Microsoft YaHei"); // 使用微软雅黑字体，支持中英文
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);

    // 绘制主要方向刻度和标签
    for (int angle = 0; angle < 360; angle += 30) {
        double angleRad = qDegreesToRadians((double)angle);

        // 计算刻度线的起点和终点
        QPoint innerPoint(center.x() + (innerRadius - 10) * qSin(angleRad),
                         center.y() - (innerRadius - 10) * qCos(angleRad));
        QPoint outerPoint(center.x() + (innerRadius - 2) * qSin(angleRad),
                         center.y() - (innerRadius - 2) * qCos(angleRad));

        painter.drawLine(innerPoint, outerPoint);

        // 绘制方向标签
        if (angle % 90 == 0) {
            QString label;
            switch (angle) {
                case 0: label = "N"; break;
                case 90: label = "E"; break;
                case 180: label = "S"; break;
                case 270: label = "W"; break;
            }

            // 计算文字位置，确保居中显示
            QFontMetrics fm(font);
            QRect textRect = fm.boundingRect(label);
            QPoint textPoint(center.x() + (innerRadius - 25) * qSin(angleRad) - textRect.width()/2,
                           center.y() - (innerRadius - 25) * qCos(angleRad) + textRect.height()/2);

            painter.setPen(QPen(m_textColor, 2));
            painter.drawText(textPoint, label);
            painter.setPen(QPen(m_roseColor, 2));
        }
    }
    
    // 绘制次要刻度
    painter.setPen(QPen(m_roseColor, 1));
    for (int angle = 0; angle < 360; angle += 10) {
        if (angle % 30 != 0) { // 跳过主要刻度
            double angleRad = qDegreesToRadians((double)angle);
            
            QPoint innerPoint(center.x() + (innerRadius - 5) * qSin(angleRad),
                             center.y() - (innerRadius - 5) * qCos(angleRad));
            QPoint outerPoint(center.x() + (innerRadius - 2) * qSin(angleRad),
                             center.y() - (innerRadius - 2) * qCos(angleRad));
            
            painter.drawLine(innerPoint, outerPoint);
        }
    }
}

//绘制方向指针
void CompassWidget::drawDirectionIndicator(QPainter &painter)
{
    QPoint center(width() / 2, height() / 2);

    // 动态计算指针长度
    int needleLength = getInnerRadius() * 0.7;

    // 指针指向当前无人机的方向
    double angleRad = qDegreesToRadians(m_currentDirection);
    QPoint needleEnd(center.x() + needleLength * qSin(angleRad),
                     center.y() - needleLength * qCos(angleRad));
    QPoint needleTail(center.x() - 15 * qSin(angleRad),
                      center.y() + 15 * qCos(angleRad));

    // 绘制指针主体 - 加粗红色指针
    painter.setPen(QPen(m_needleColor, 4));
    painter.drawLine(needleTail, needleEnd);

    // 绘制中心点
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(Qt::white));
    painter.drawEllipse(center, 6, 6);
}

//绘制速度指示器
void CompassWidget::drawSpeedIndicator(QPainter &painter)
{
    QPoint center(width() / 2, height() / 2);
    
    // 动态计算半径
    int outerRadius = getOuterRadius();
    
    // 根据速度选择颜色
    QColor speedColor = getSpeedColor(m_currentSpeed);
    
    // 绘制速度环
    painter.setPen(QPen(speedColor, 4));
    painter.setBrush(Qt::NoBrush);
    
    // 计算速度环的角度（0-360度对应0-最大速度）
    double maxSpeed = 50.0; // 假设最大速度为50
    double speedRatio = qMin(m_currentSpeed / maxSpeed, 1.0);
    int spanAngle = (int)(speedRatio * 360 * 16); // Qt使用1/16度为单位
    
    painter.drawArc(center.x() - outerRadius + 5, center.y() - outerRadius + 5,
                    (outerRadius - 5) * 2, (outerRadius - 5) * 2,
                    90 * 16, -spanAngle); // 从顶部开始，逆时针
}

//绘制方向文字
void CompassWidget::drawDirectionText(QPainter &painter)
{
    QPoint center(width() / 2, height() / 2);

    // 绘制当前方向角度
    painter.setPen(QPen(m_textColor, 1));
    painter.setFont(QFont("Arial", 12, QFont::Bold));

    QString angleText = QString("%1°").arg((int)m_currentDirection);
    QRect textRect(center.x() - 30, center.y() + 20, 60, 20);
    painter.drawText(textRect, Qt::AlignCenter, angleText);

    // 绘制方向描述
    painter.setFont(QFont("Arial", 10));
    QString directionText = getDirectionText(m_currentDirection);
    QRect dirTextRect(center.x() - 30, center.y() + 35, 60, 15);
    painter.drawText(dirTextRect, Qt::AlignCenter, directionText);
}

//获取方向文字，根据角度返回方向文字
QString CompassWidget::getDirectionText(double angle) const
{
    if (angle >= 337.5 || angle < 22.5) return "N";
    else if (angle >= 22.5 && angle < 67.5) return "NE";
    else if (angle >= 67.5 && angle < 112.5) return "E";
    else if (angle >= 112.5 && angle < 157.5) return "SE";
    else if (angle >= 157.5 && angle < 202.5) return "S";
    else if (angle >= 202.5 && angle < 247.5) return "SW";
    else if (angle >= 247.5 && angle < 292.5) return "W";
    else return "NW";
}

//获取速度颜色，根据速度返回颜色
QColor CompassWidget::getSpeedColor(double speed) const
{
    if (speed < 10.0) {
        return m_speedLowColor;
    } else if (speed < 30.0) {
        return m_speedMediumColor;
    } else {
        return m_speedHighColor;
    }
}

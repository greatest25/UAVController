#include "radarwidget.h"

/*
雷达组件
主要功能：
展示无人机和障碍物
提供鼠标点击事件
提供缩放功能
提供中心点设置功能
提供障碍物添加功能
提供障碍物清除功能
提供障碍物清除功能
成员变量
m_currentDronePos:当前无人机的位置
m_sweepAngle:扫描角度
*/
RadarWidget::RadarWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentDronePos(0, 0)
    , m_sweepAngle(0)
{
    // 设置合适的大小，允许缩放
    setMinimumSize(200, 150); // 最小尺寸
    setMaximumSize(400, 300); // 最大尺寸
    
    // 初始化颜色
    m_backgroundColor = QColor(0, 20, 0);      // 深绿色背景
    m_gridColor = QColor(0, 255, 0, 100);     // 半透明绿色网格
    m_sweepColor = QColor(0, 255, 0, 50);     // 半透明绿色扫描线
    m_mountainColor = QColor(139, 69, 19);     // 棕色山体
    m_radarColor = QColor(255, 0, 255);        // 紫色雷达
    m_cloudColor = QColor(255, 255, 0);        // 黄色雷云
    m_blueTeamColor = QColor(0, 100, 255);     // 蓝色队伍
    m_redTeamColor = QColor(255, 50, 50);      // 红色队伍
    
    // 创建更新定时器
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &RadarWidget::updateRadar);
    m_updateTimer->start(100); // 100ms更新一次，模拟雷达扫描
}

//设置当前无人机位置
void RadarWidget::setCurrentDronePosition(const QPoint &pos)
{
    m_currentDronePos = pos;
    update();
}

//更新无人机信息,包括位置、队伍、id、血量
void RadarWidget::updateDroneInfo(const QString &uid, const QPoint &pos, const QString &team, int hp)
{
    RadarDroneInfo info;
    info.position = pos;
    info.team = team;
    info.uid = uid;
    info.hp = hp;
    info.isVisible = isInDetectionRange(pos);
    
    m_drones[uid] = info;
    update();
}

//添加障碍物
void RadarWidget::addObstacle(const QString &id, const QPoint &pos, int radius, ObstacleType type)
{
    ObstacleInfo info(id, type, pos.x(), pos.y(), radius);
    m_obstacles[id] = info;
    update();
}

//清除过期障碍物
void RadarWidget::clearStaleObstacles(int timeoutMs)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QStringList staleIds;
    
    for (auto it = m_obstacles.begin(); it != m_obstacles.end(); ++it) {
        if (it.value().type == Cloud && 
            it.value().timestamp.msecsTo(currentTime) > timeoutMs) {
            staleIds.append(it.key());
        }
    }
    
    for (const QString &id : staleIds) {
        m_obstacles.remove(id);
    }
    
    if (!staleIds.isEmpty()) {
        update();
    }
}

//清除所有障碍物
void RadarWidget::clearObstacles()
{
    m_obstacles.clear();
    update();
}

//清除所有无人机和障碍物
void RadarWidget::clearAll()
{
    m_drones.clear();
    m_obstacles.clear();
    update();
}

//绘制事件
void RadarWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制雷达背景
    drawRadarBackground(painter);
    
    // 绘制探测范围
    drawDetectionRange(painter);
    
    // 绘制障碍物
    drawObstacles(painter);
    
    // 绘制无人机
    drawDrones(painter);
    
    // 绘制雷达扫描线
    drawRadarSweep(painter);
}

//更新雷达
void RadarWidget::updateRadar()
{
    // 更新扫描角度
    m_sweepAngle = (m_sweepAngle + 6) % 360; // 每次增加6度
    
    // 清除过期的动态障碍物
    clearStaleObstacles();
    
    update();
}

//绘制雷达背景
void RadarWidget::drawRadarBackground(QPainter &painter)
{
    // 计算雷达显示区域
    QPoint center(width() / 2, height() / 2);
    int maxRadius = qMin(width(), height()) / 2 - 8; // 留出8像素边距确保完整显示
    
    // 绘制圆形背景
    painter.setPen(QPen(m_backgroundColor, 1));
    painter.setBrush(QBrush(m_backgroundColor));
    painter.drawEllipse(center, maxRadius, maxRadius);
    
    // 绘制同心圆网格
    painter.setPen(QPen(m_gridColor, 1));
    painter.setBrush(Qt::NoBrush);
    
    // 绘制同心圆（每40像素一个圆，更密集）
    for (int r = 40; r <= maxRadius; r += 40) {
        painter.drawEllipse(center, r, r);
    }
    
    // 绘制十字线
    painter.drawLine(center.x(), center.y() - maxRadius, center.x(), center.y() + maxRadius);
    painter.drawLine(center.x() - maxRadius, center.y(), center.x() + maxRadius, center.y());
    
    // 绘制对角线（缩短长度，避免超出边界）
    int diagonalLength = maxRadius * 0.6;
    painter.drawLine(center.x() - diagonalLength, center.y() - diagonalLength,
                     center.x() + diagonalLength, center.y() + diagonalLength);
    painter.drawLine(center.x() - diagonalLength, center.y() + diagonalLength,
                     center.x() + diagonalLength, center.y() - diagonalLength);
}

//绘制探测范围
void RadarWidget::drawDetectionRange(QPainter &painter)
{
    // 绘制探测范围边界
    painter.setPen(QPen(QColor(0, 255, 0), 2));
    painter.setBrush(Qt::NoBrush);
    
    QPoint center(width() / 2, height() / 2);
    int maxRadius = qMin(width(), height()) / 2 - 8; // 与背景保持一致
    
    painter.drawEllipse(center, maxRadius, maxRadius);
}

//绘制障碍物
void RadarWidget::drawObstacles(QPainter &painter)
{
    QPoint center(width() / 2, height() / 2);
    int maxDisplayRadius = qMin(width(), height()) / 2 - 8; // 与背景保持一致
    
    for (auto it = m_obstacles.begin(); it != m_obstacles.end(); ++it) {
        const ObstacleInfo &obstacle = it.value();
        
        // 检查障碍物是否在探测范围内
        QPoint worldPos(obstacle.x, obstacle.y);
        if (!isInDetectionRange(worldPos)) {
            continue;
        }
        
        // 转换到雷达坐标
        QPoint radarPos = worldToRadar(worldPos);
        
        // 计算显示半径
        double scale = (double)maxDisplayRadius / DETECTION_RADIUS;
        int displayRadius = obstacle.radius * scale;
        
        // 根据类型绘制不同的形状和图标
        switch (obstacle.type) {
            case Mountain://山体
                drawMountainObstacle(painter, radarPos, displayRadius);
                break;
            case Radar://雷达
                drawRadarObstacle(painter, radarPos, displayRadius);
                break;
            case Cloud://雷云
                drawCloudObstacle(painter, radarPos, displayRadius);
                break;
        }
    }
}

//绘制无人机
void RadarWidget::drawDrones(QPainter &painter)
{
    QPoint center(width() / 2, height() / 2);
    
    // 绘制当前控制的无人机（中心点）
    painter.setPen(QPen(Qt::white, 3));
    painter.setBrush(QBrush(Qt::white));
    painter.drawEllipse(center, 5, 5);
    
    // 绘制其他无人机
    for (auto it = m_drones.begin(); it != m_drones.end(); ++it) {
        const RadarDroneInfo &drone = it.value();
        
        // 只显示在探测范围内的无人机
        if (!drone.isVisible) {
            continue;
        }
        
        // 转换到雷达坐标
        QPoint radarPos = worldToRadar(drone.position);
        
        // 根据队伍选择颜色
        QColor color = (drone.team == "B") ? m_blueTeamColor : m_redTeamColor;
        
        painter.setPen(QPen(color, 2));
        painter.setBrush(QBrush(color));
        
        // 绘制无人机图标（三角形）
        QPolygon triangle;
        triangle << QPoint(radarPos.x(), radarPos.y() - 6)
                 << QPoint(radarPos.x() - 5, radarPos.y() + 4)
                 << QPoint(radarPos.x() + 5, radarPos.y() + 4);
        painter.drawPolygon(triangle);
        
        // 绘制ID标签
        painter.setPen(QPen(color, 1));
        painter.setFont(QFont("Arial", 8));
        painter.drawText(radarPos + QPoint(8, -8), drone.uid);
    }
}

//绘制雷达扫描线
void RadarWidget::drawRadarSweep(QPainter &painter)
{
    QPoint center(width() / 2, height() / 2);
    int maxRadius = qMin(width(), height()) / 2 - 8; // 与背景保持一致
    
    // 绘制扫描线
    painter.setPen(QPen(QColor(0, 255, 0, 150), 2));
    
    double angleRad = qDegreesToRadians((double)m_sweepAngle);
    QPoint endPoint(center.x() + maxRadius * qCos(angleRad),
                    center.y() + maxRadius * qSin(angleRad));
    
    painter.drawLine(center, endPoint);
    
    // 绘制扫描扇形（渐变效果）
    painter.setBrush(QBrush(m_sweepColor));
    painter.setPen(Qt::NoPen);
    
    int spanAngle = 30; // 扇形角度
    painter.drawPie(center.x() - maxRadius, center.y() - maxRadius,
                    maxRadius * 2, maxRadius * 2,
                    (m_sweepAngle - spanAngle) * 16, spanAngle * 16);
}

//将世界坐标转换为雷达坐标
QPoint RadarWidget::worldToRadar(const QPoint &worldPos) const
{
    QPoint center(width() / 2, height() / 2);
    int maxDisplayRadius = qMin(width(), height()) / 2 - 8; // 与背景保持一致
    
    // 计算相对于当前无人机的位置
    QPoint relativePos = worldPos - m_currentDronePos;
    
    // 缩放到雷达显示范围
    double scale = (double)maxDisplayRadius / DETECTION_RADIUS;
    QPoint radarPos(center.x() + relativePos.x() * scale,
                    center.y() + relativePos.y() * scale);
    
    return radarPos;
}

//判断是否在探测范围内
bool RadarWidget::isInDetectionRange(const QPoint &worldPos) const
{
    QPoint relativePos = worldPos - m_currentDronePos;
    double distance = qSqrt(relativePos.x() * relativePos.x() + relativePos.y() * relativePos.y());
    return distance <= DETECTION_RADIUS;
}

//绘制山体障碍物
void RadarWidget::drawMountainObstacle(QPainter &painter, const QPoint &pos, int radius)
{
    // 山体障碍物：绘制三角形图标
    painter.setPen(QPen(m_mountainColor, 2));
    painter.setBrush(QBrush(m_mountainColor, Qt::SolidPattern));

    // 绘制背景圆圈
    painter.drawEllipse(pos.x() - radius, pos.y() - radius, radius * 2, radius * 2);

    // 绘制三角形山峰图标
    QPolygon triangle;
    int iconSize = qMax(8, radius / 2);
    triangle << QPoint(pos.x(), pos.y() - iconSize)           // 顶点
             << QPoint(pos.x() - iconSize, pos.y() + iconSize/2)  // 左下
             << QPoint(pos.x() + iconSize, pos.y() + iconSize/2); // 右下

    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
    painter.drawPolygon(triangle);

    // 添加文字标识
    painter.setPen(QPen(Qt::white, 1));
    painter.setFont(QFont("Arial", 8, QFont::Bold));
    painter.drawText(pos.x() - 10, pos.y() + radius + 12, "山体");
}

//绘制雷达障碍物
void RadarWidget::drawRadarObstacle(QPainter &painter, const QPoint &pos, int radius)
{
    // 雷达站障碍物：绘制雷达扫描图标
    painter.setPen(QPen(m_radarColor, 2));
    painter.setBrush(QBrush(m_radarColor, Qt::SolidPattern));

    // 绘制背景圆圈
    painter.drawEllipse(pos.x() - radius, pos.y() - radius, radius * 2, radius * 2);

    // 绘制雷达扫描扇形
    int iconSize = qMax(8, radius / 2);
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));

    // 绘制雷达底座（矩形）
    QRect base(pos.x() - iconSize/2, pos.y() + iconSize/3, iconSize, iconSize/3);
    painter.drawRect(base);

    // 绘制雷达扫描扇形
    QRect scanArea(pos.x() - iconSize, pos.y() - iconSize, iconSize * 2, iconSize * 2);
    painter.setBrush(QBrush(Qt::white, Qt::Dense4Pattern));
    painter.drawPie(scanArea, 45 * 16, 90 * 16); // 45度到135度的扇形

    // 添加文字标识
    painter.setPen(QPen(Qt::white, 1));
    painter.setFont(QFont("Arial", 8, QFont::Bold));
    painter.drawText(pos.x() - 15, pos.y() + radius + 12, "雷达站");
}

//绘制雷云障碍物
void RadarWidget::drawCloudObstacle(QPainter &painter, const QPoint &pos, int radius)
{
    // 雷云障碍物：绘制云朵和闪电图标，带闪烁效果
    QColor cloudColor = m_cloudColor;

    // 动态闪烁效果
    if (m_sweepAngle % 60 < 30) {
        cloudColor.setAlpha(220);
    } else {
        cloudColor.setAlpha(120);
    }

    painter.setPen(QPen(cloudColor, 2));
    painter.setBrush(QBrush(cloudColor, Qt::SolidPattern));

    // 绘制背景圆圈
    painter.drawEllipse(pos.x() - radius, pos.y() - radius, radius * 2, radius * 2);

    // 绘制云朵形状
    int iconSize = qMax(8, radius / 2);
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));

    // 绘制多个重叠的圆形组成云朵
    painter.drawEllipse(pos.x() - iconSize, pos.y() - iconSize/2, iconSize, iconSize);
    painter.drawEllipse(pos.x() - iconSize/2, pos.y() - iconSize, iconSize, iconSize);
    painter.drawEllipse(pos.x(), pos.y() - iconSize/2, iconSize, iconSize);

    // 绘制闪电图标
    painter.setPen(QPen(Qt::yellow, 2));
    painter.setBrush(QBrush(Qt::yellow, Qt::SolidPattern));

    QPolygon lightning;
    lightning << QPoint(pos.x() - 2, pos.y() - iconSize/3)
              << QPoint(pos.x() + 2, pos.y() - iconSize/6)
              << QPoint(pos.x() - 1, pos.y())
              << QPoint(pos.x() + 3, pos.y() + iconSize/3)
              << QPoint(pos.x() - 1, pos.y() + iconSize/6)
              << QPoint(pos.x() + 1, pos.y());
    painter.drawPolygon(lightning);

    // 添加文字标识
    painter.setPen(QPen(Qt::white, 1));
    painter.setFont(QFont("Arial", 8, QFont::Bold));
    painter.drawText(pos.x() - 10, pos.y() + radius + 12, "雷云");
}


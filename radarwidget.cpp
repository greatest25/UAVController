#include "radarwidget.h"

RadarWidget::RadarWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentDronePos(0, 0)
    , m_sweepAngle(0)
{
    // 设置合适的大小，允许缩放
    setMinimumSize(300, 200);
    setMaximumSize(500, 350);
    
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

void RadarWidget::setCurrentDronePosition(const QPoint &pos)
{
    m_currentDronePos = pos;
    update();
}

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

void RadarWidget::addObstacle(const QString &id, const QPoint &pos, int radius, ObstacleType type)
{
    ObstacleInfo info;
    info.position = pos;
    info.radius = radius;
    info.type = type;
    info.lastUpdate = QDateTime::currentDateTime();
    
    m_obstacles[id] = info;
    update();
}

void RadarWidget::clearStaleObstacles(int timeoutMs)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QStringList staleIds;
    
    for (auto it = m_obstacles.begin(); it != m_obstacles.end(); ++it) {
        if (it.value().type == CLOUD_OBS && 
            it.value().lastUpdate.msecsTo(currentTime) > timeoutMs) {
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

void RadarWidget::clearObstacles()
{
    m_obstacles.clear();
    update();
}

void RadarWidget::clearAll()
{
    m_drones.clear();
    m_obstacles.clear();
    update();
}

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

void RadarWidget::updateRadar()
{
    // 更新扫描角度
    m_sweepAngle = (m_sweepAngle + 6) % 360; // 每次增加6度
    
    // 清除过期的动态障碍物
    clearStaleObstacles();
    
    update();
}

void RadarWidget::drawRadarBackground(QPainter &painter)
{
    // 填充背景
    painter.fillRect(rect(), m_backgroundColor);
    
    // 绘制同心圆网格
    painter.setPen(QPen(m_gridColor, 1));
    painter.setBrush(Qt::NoBrush);
    
    QPoint center(width() / 2, height() / 2);
    int maxRadius = qMin(width(), height()) / 2 - 5; // 减少边距
    
    // 绘制同心圆（每50像素一个圆）
    for (int r = 50; r <= maxRadius; r += 50) {
        painter.drawEllipse(center, r, r);
    }
    
    // 绘制十字线
    painter.drawLine(center.x(), 5, center.x(), height() - 5);
    painter.drawLine(5, center.y(), width() - 5, center.y());
    
    // 绘制对角线
    painter.drawLine(center.x() - maxRadius * 0.7, center.y() - maxRadius * 0.7,
                     center.x() + maxRadius * 0.7, center.y() + maxRadius * 0.7);
    painter.drawLine(center.x() - maxRadius * 0.7, center.y() + maxRadius * 0.7,
                     center.x() + maxRadius * 0.7, center.y() - maxRadius * 0.7);
}

void RadarWidget::drawDetectionRange(QPainter &painter)
{
    // 绘制探测范围边界
    painter.setPen(QPen(QColor(0, 255, 0), 2));
    painter.setBrush(Qt::NoBrush);
    
    QPoint center(width() / 2, height() / 2);
    int displayRadius = qMin(width(), height()) / 2 - 10; // 减少边距
    
    painter.drawEllipse(center, displayRadius, displayRadius);
}

void RadarWidget::drawObstacles(QPainter &painter)
{
    QPoint center(width() / 2, height() / 2);
    int maxDisplayRadius = qMin(width(), height()) / 2 - 10; // 减少边距
    
    for (auto it = m_obstacles.begin(); it != m_obstacles.end(); ++it) {
        const ObstacleInfo &obstacle = it.value();
        
        // 检查障碍物是否在探测范围内
        if (!isInDetectionRange(obstacle.position)) {
            continue;
        }
        
        // 转换到雷达坐标
        QPoint radarPos = worldToRadar(obstacle.position);
        
        // 计算显示半径
        double scale = (double)maxDisplayRadius / DETECTION_RADIUS;
        int displayRadius = obstacle.radius * scale;
        
        // 根据类型绘制不同的形状和图标
        switch (obstacle.type) {
            case MOUNTAIN_OBS:
                drawMountainObstacle(painter, radarPos, displayRadius);
                break;
            case RADAR_OBS:
                drawRadarObstacle(painter, radarPos, displayRadius);
                break;
            case CLOUD_OBS:
                drawCloudObstacle(painter, radarPos, displayRadius);
                break;
        }
    }
}

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

void RadarWidget::drawRadarSweep(QPainter &painter)
{
    QPoint center(width() / 2, height() / 2);
    int maxRadius = qMin(width(), height()) / 2 - 10; // 减少边距
    
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

QPoint RadarWidget::worldToRadar(const QPoint &worldPos) const
{
    QPoint center(width() / 2, height() / 2);
    int maxDisplayRadius = qMin(width(), height()) / 2 - 10; // 减少边距
    
    // 计算相对于当前无人机的位置
    QPoint relativePos = worldPos - m_currentDronePos;
    
    // 缩放到雷达显示范围
    double scale = (double)maxDisplayRadius / DETECTION_RADIUS;
    QPoint radarPos(center.x() + relativePos.x() * scale,
                    center.y() + relativePos.y() * scale);
    
    return radarPos;
}

bool RadarWidget::isInDetectionRange(const QPoint &worldPos) const
{
    QPoint relativePos = worldPos - m_currentDronePos;
    double distance = qSqrt(relativePos.x() * relativePos.x() + relativePos.y() * relativePos.y());
    return distance <= DETECTION_RADIUS;
}

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


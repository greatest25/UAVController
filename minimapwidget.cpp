#include "minimapwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QtMath>
#include <QDebug>

MinimapWidget::MinimapWidget(QWidget *parent)
    : QWidget(parent)
    , m_selectedDrone("B1")
    , m_zoomLevel(1.0)
    , m_droneSize(4)
    , m_gridSpacing(50)
    , m_margin(10)
{
    setMinimumSize(120, 120);
    
    // 设置默认地图范围为1280*800像素
    m_mapBounds = QRect(0, 0, 1280, 800);
    
    // 颜色主题
    m_backgroundColor = QColor(30, 34, 42);
    m_gridColor = QColor(60, 64, 72);
    m_borderColor = QColor(70, 130, 180);
    m_blueTeamColor = QColor(0, 191, 255);
    m_redTeamColor = QColor(255, 0, 0);
    m_selectedColor = QColor(255,165,102);
    m_offlineColor = QColor(128, 128, 128);
    
    // 障碍物颜色
    m_mountainColor = QColor(39, 255, 96);  // 绿色
    m_radarColor = QColor(255, 89, 252);    // 紫色
    m_cloudColor = QColor(255, 255, 15);    // 黄色
    
    // 字体
    m_labelFont = QFont("Arial", 8);
    m_centerOffset = QPointF(0, 0);
}

void MinimapWidget::updateDroneInfo(const QString &droneId, const QPoint &pos, const QString &team, int hp, bool online, double heading)
{
    DroneInfo info;
    info.position = pos;
    info.team = team;
    info.hp = hp;
    info.online = online;
    info.heading = heading;
    info.id = droneId;
    
    m_drones[droneId] = info;
    update();
}

void MinimapWidget::setMapBounds(const QRect &bounds)
{
    m_mapBounds = bounds;
    update();
}

void MinimapWidget::setSelectedDrone(const QString &droneId)
{
    m_selectedDrone = droneId;
    update();
}

void MinimapWidget::clearAll()
{
    m_drones.clear();
    m_obstacles.clear();
    update();
}

void MinimapWidget::setZoomLevel(double zoom)
{
    // 调整缩放范围，适应1280*800的地图
    m_zoomLevel = qBound(0.3, zoom, 2.0);
    update();
}

void MinimapWidget::setCenter(const QPointF &center)
{
    m_centerOffset = center;
    update();
}


void MinimapWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter);
    drawGrid(painter);
    drawObstacles(painter);
    drawDrones(painter);    
}

void MinimapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QString droneId = getDroneAtPosition(event->pos());
        if (!droneId.isEmpty()) {
            emit droneClicked(droneId);
        }
    }
}

void MinimapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

void MinimapWidget::drawBackground(QPainter &painter)
{
    // 绘制背景
    painter.fillRect(rect(), m_backgroundColor);
    
    // 绘制边框
    painter.setPen(QPen(m_borderColor, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
}

void MinimapWidget::drawGrid(QPainter &painter)
{
    painter.setPen(QPen(m_gridColor, 1));
    
    QRect mapRect = rect().adjusted(m_margin, m_margin, -m_margin, -m_margin);
    
    // 绘制网格线
    int gridStep = m_gridSpacing * m_zoomLevel;
    if (gridStep < 10) gridStep = 10;
    
    // 垂直线
    for (int x = mapRect.left(); x <= mapRect.right(); x += gridStep) {
        painter.drawLine(x, mapRect.top(), x, mapRect.bottom());
    }
    
    // 水平线
    for (int y = mapRect.top(); y <= mapRect.bottom(); y += gridStep) {
        painter.drawLine(mapRect.left(), y, mapRect.right(), y);
    }
    
    // 绘制中心十字线
    painter.setPen(QPen(m_borderColor, 2));
    QPoint center = mapRect.center();
    painter.drawLine(center.x(), mapRect.top(), center.x(), mapRect.bottom());
    painter.drawLine(mapRect.left(), center.y(), mapRect.right(), center.y());
}

void MinimapWidget::drawDrones(QPainter &painter)
{
    QRect mapRect = rect().adjusted(m_margin, m_margin, -m_margin, -m_margin);
    
    for (auto it = m_drones.begin(); it != m_drones.end(); ++it) {
        const DroneInfo &drone = it.value();
        
        // 转换世界坐标到窗口坐标
        QPoint widgetPos = mapToWidget(drone.position);
        
        // 添加调试输出
        if (drone.id == m_selectedDrone) {
            qDebug() << "[Minimap] Selected Drone" << drone.id << "World Pos:" << drone.position << "Widget Pos:" << widgetPos;
        }
        
        // 调试血量为0的无人机
        if (drone.hp <= 0) {
            qDebug() << "[Minimap] Dead Drone" << drone.id << "HP:" << drone.hp << "Color: White";
        }

        // 检查是否在可见区域内
        if (!mapRect.contains(widgetPos)) continue;
        
        // 选择颜色
        QColor droneColor;
        if (drone.id == m_selectedDrone) {
            droneColor = m_selectedColor;
        } else if (drone.hp <= 0) {
            // 血量为0的无人机使用白色
            droneColor = Qt::white;
        } else if (!drone.online) {
            droneColor = m_offlineColor;
        } else if (drone.team == "B") {
            droneColor = m_blueTeamColor;
        } else {
            droneColor = m_redTeamColor;
        }
        
        // 绘制无人机
        painter.setPen(QPen(droneColor, 2));
        painter.setBrush(QBrush(droneColor));
        
        int size = m_droneSize * m_zoomLevel;
        if (size < 4) size = 4;
        
        // 绘制圆形
        painter.drawEllipse(widgetPos, size, size);
        
        // 绘制方向指示器
        if (drone.online && drone.heading != 0.0) {
            painter.setPen(QPen(Qt::white, 1));
            double angle = drone.heading * M_PI / 180.0;
            QPoint direction(
                widgetPos.x() + size * cos(angle),
                widgetPos.y() - size * sin(angle)
            );
            painter.drawLine(widgetPos, direction);
        }
        
        // 绘制ID标签
        painter.setFont(m_labelFont);
        painter.setPen(Qt::white);
        QRect labelRect(widgetPos.x() - 15, widgetPos.y() + size + 2, 30, 12);
        painter.drawText(labelRect, Qt::AlignCenter, drone.id);
    }
}

// 图例绘制已移至CustomDashboard类中

QPoint MinimapWidget::mapToWidget(const QPoint &worldPos) const
{
    QRectF mapRect = rect().adjusted(m_margin, m_margin, -m_margin, -m_margin);
    
    // 计算缩放比例
    double scaleX = mapRect.width() / m_mapBounds.width();
    double scaleY = mapRect.height() / m_mapBounds.height();
    
    // 计算中心点
    QPointF widgetCenter = mapRect.center();
    
    // 1. 将世界坐标转换为无缩放的窗口坐标
    double initialX = mapRect.left() + worldPos.x() * scaleX;
    double initialY = mapRect.top() + worldPos.y() * scaleY;
    
    // 2. 计算该点相对于窗口中心点的偏移
    double offsetX = initialX - widgetCenter.x();
    double offsetY = initialY - widgetCenter.y();
    
    // 3. 应用缩放
    offsetX *= m_zoomLevel;
    offsetY *= m_zoomLevel;
    
    // 4. 将缩放后的偏移量加回到中心点，得到最终坐标
    return QPoint(qRound(widgetCenter.x() + offsetX), qRound(widgetCenter.y() + offsetY));
}

QPoint MinimapWidget::widgetToMap(const QPoint &widgetPos) const
{
    QRectF mapRect = rect().adjusted(m_margin, m_margin, -m_margin, -m_margin);

    // 计算缩放比例
    double scaleX = mapRect.width() / m_mapBounds.width();
    double scaleY = mapRect.height() / m_mapBounds.height();

    // 计算中心点
    QPointF widgetCenter = mapRect.center();

    // 1. 计算窗口坐标相对于中心点的偏移
    double offsetX = widgetPos.x() - widgetCenter.x();
    double offsetY = widgetPos.y() - widgetCenter.y();

    // 2. 应用逆缩放
    offsetX /= m_zoomLevel;
    offsetY /= m_zoomLevel;

    // 3. 将缩放后的偏移加回中心点，得到无缩放的窗口坐标
    double initialX = widgetCenter.x() + offsetX;
    double initialY = widgetCenter.y() + offsetY;

    // 4. 将无缩放的窗口坐标转换回世界坐标
    int worldX = qRound((initialX - mapRect.left()) / scaleX);
    int worldY = qRound((initialY - mapRect.top()) / scaleY);

    return QPoint(worldX, worldY);
}

QString MinimapWidget::getDroneAtPosition(const QPoint &widgetPos) const
{
    int clickRadius = m_droneSize * m_zoomLevel + 5;
    if (clickRadius < 8) clickRadius = 8;
    
    for (auto it = m_drones.begin(); it != m_drones.end(); ++it) {
        const DroneInfo &drone = it.value();
        QPoint dronePos = mapToWidget(drone.position);
        
        if ((widgetPos - dronePos).manhattanLength() <= clickRadius) {
            return drone.id;
        }
    }
    
    return QString();
}

// 障碍物相关方法实现
void MinimapWidget::addObstacle(const QString &id, double x, double y, double radius, ObstacleType type)
{
    ObstacleInfo info(id, type, x, y, radius);
    m_obstacles[id] = info;
    
    // 添加调试输出
    QString typeStr;
    switch (type) {
        case Mountain: typeStr = "Mountain"; break;
        case Radar: typeStr = "Radar"; break;
        case Cloud: typeStr = "Cloud"; break;
        default: typeStr = "Unknown"; break;
    }
    qDebug() << "[Minimap] Added obstacle:" << id << "at (" << x << "," << y << ") radius" << radius << "type" << typeStr;
    
    update();
}

void MinimapWidget::clearObstacles()
{
    m_obstacles.clear();
    update();
}

void MinimapWidget::clearStaleObstacles(int timeoutMs)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QList<QString> keysToRemove;
    
    for (auto it = m_obstacles.begin(); it != m_obstacles.end(); ++it) {
        if (it.value().timestamp.msecsTo(currentTime) > timeoutMs) {
            keysToRemove.append(it.key());
        }
    }
    
    for (const QString &key : keysToRemove) {
        m_obstacles.remove(key);
    }
    
    if (!keysToRemove.isEmpty()) {
        update();
    }
}

void MinimapWidget::drawObstacles(QPainter &painter)
{
    QRect mapRect = rect().adjusted(m_margin, m_margin, -m_margin, -m_margin);
    
    for (auto it = m_obstacles.begin(); it != m_obstacles.end(); ++it) {
        const ObstacleInfo &obstacle = it.value();
        
        // 将世界坐标转换为窗口坐标
        QPoint widgetPos = mapToWidget(QPoint(obstacle.x, obstacle.y));
        
        // 检查是否在可见区域内
        if (!mapRect.contains(widgetPos)) continue;
        
        // 1. 根据类型选择颜色
        QColor obstacleColor;
        switch (obstacle.type) {
            case Mountain:
                obstacleColor = m_mountainColor;
                break;
            case Radar:
                obstacleColor = m_radarColor;
                break;
            case Cloud:
                obstacleColor = m_cloudColor;
                break;
            default:
                obstacleColor = Qt::gray;
                break;
        }
        
        // 2. 计算无人机和障碍物的显示半径（关键修改）
        const int droneDisplayRadius = qMax(2, static_cast<int>(DRONE_BASE_SIZE * m_zoomLevel / 2)); // 无人机视觉半径
        
        // 3. 计算缩放比例（匹配雷达的6~8倍比例）
        double scale = static_cast<double>(width()) / m_mapBounds.width(); // 地图到窗口的缩放比
        int obstacleDisplayRadius = obstaclePhysicalRadius * scale * 0.7; // 0.7为微调系数

        // 4. 强制外切：障碍物绘制半径 = 物理半径 + 无人机半径
        int finalObstacleRadius = qMax(4, obstacleDisplayRadius + droneDisplayRadius);

        // 5. 绘制障碍物（圆心对齐）
        painter.setPen(QPen(obstacleColor, 1));
        painter.setBrush(QBrush(obstacleColor, Qt::SolidPattern));
        painter.drawEllipse(widgetPos, finalObstacleRadius, finalObstacleRadius);
    }
}


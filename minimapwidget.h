#ifndef MINIMAPWIDGET_H
#define MINIMAPWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QFont>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QMap>
#include <QStringList>
#include <QDateTime>
#include "obstacle_types.h"

// 无人机信息结构
struct DroneInfo {
    QPoint position;
    QString team;  // "B" for Blue, "R" for Red
    int hp;
    bool online;
    double heading;
    QString id;
};

// 全局缩略地图组件
class MinimapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MinimapWidget(QWidget *parent = nullptr);

    // 更新无人机信息
    void updateDroneInfo(const QString &droneId, const QPoint &pos, const QString &team, int hp, bool online, double heading = 0.0);
    
    // 设置地图范围
    void setMapBounds(const QRect &bounds);
    
    // 设置当前选中的无人机
    void setSelectedDrone(const QString &droneId);
    
    // 清除所有数据
    void clearAll();
    
    // 设置缩放级别
    void setZoomLevel(double zoom);
    
    // 设置缩放中心
    void setCenter(const QPointF &center);
    
    // 障碍物相关方法
    void addObstacle(const QString &id, double x, double y, double radius, ObstacleType type);
    void clearObstacles();
    void clearDynamicObstacles();  // 只清除动态障碍物
    void clearStaticObstacles();   // 只清除静态障碍物
    void clearStaleObstacles(int timeoutMs = 1000);
    
    // 获取颜色方法，用于外部绘制图例
    QColor getBlueTeamColor() const { return m_blueTeamColor; }
    QColor getRedTeamColor() const { return m_redTeamColor; }
    QColor getSelectedColor() const { return m_selectedColor; }
    QColor getOfflineColor() const { return m_offlineColor; }
    QColor getMountainColor() const { return m_mountainColor; }
    QColor getRadarColor() const { return m_radarColor; }
    QColor getCloudColor() const { return m_cloudColor; }

signals:
    void droneClicked(const QString &droneId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawBackground(QPainter &painter);
    void drawGrid(QPainter &painter);
    void drawDrones(QPainter &painter);
    void drawObstacles(QPainter &painter);
    void drawMountainObstacle(QPainter &painter, const QPoint &pos, int radius);
    void drawRadarObstacle(QPainter &painter, const QPoint &pos, int radius);
    void drawCloudObstacle(QPainter &painter, const QPoint &pos, int radius);
    void drawLegend(QPainter &painter);
    

    QPoint mapToWidget(const QPoint &worldPos) const;
    QPoint widgetToMap(const QPoint &widgetPos) const;
    QString getDroneAtPosition(const QPoint &widgetPos) const;
    
    // 数据存储
    QMap<QString, DroneInfo> m_drones;
    QMap<QString, ObstacleInfo> m_obstacles;  // 障碍物信息
    QString m_selectedDrone;
    
    // 地图设置
    QRect m_mapBounds;
    double m_zoomLevel;
    QPointF m_centerOffset; // 缩放中心偏移量
    
    // 颜色主题
    QColor m_backgroundColor;
    QColor m_gridColor;
    QColor m_borderColor;
    QColor m_blueTeamColor;
    QColor m_redTeamColor;
    QColor m_selectedColor;
    QColor m_offlineColor;
    
    // 障碍物颜色
    QColor m_mountainColor;
    QColor m_radarColor;
    QColor m_cloudColor;
    
    // 字体
    QFont m_labelFont;
    
    // 绘制参数
    int m_droneSize;
    int m_gridSpacing;
    int m_margin;
    
    // 实际地图中的障碍物半径
    const int obstaclePhysicalRadius = 80; 
    // 无人机基准大小
    const int DRONE_BASE_SIZE = 4; 
};

#endif // MINIMAPWIDGET_H 
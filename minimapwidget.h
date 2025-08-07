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


struct DroneInfo {
    QPoint position;    ///< 无人机在世界坐标系中的位置
    QString team;       ///< 队伍标识("B"表示蓝队，"R"表示红队)
    int hp;            ///< 无人机血量
    bool online;       ///< 无人机是否在线
    double heading;    ///< 无人机航向角度(弧度)
    QString id;        ///< 无人机唯一标识
};

class MinimapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MinimapWidget(QWidget *parent = nullptr);

    // 无人机相关方法
    //更新无人机信息
    void updateDroneInfo(const QString &droneId, const QPoint &pos, const QString &team, int hp, bool online, double heading = 0.0);
    //设置选中无人机
    void setSelectedDrone(const QString &droneId);
    
    // 地图设置方法
    void setMapBounds(const QRect &bounds);//设置地图边界
    void clearAll();//清除所有无人机和障碍物
    void setZoomLevel(double zoom);//设置缩放级别
    void setCenter(const QPointF &center);
    
    // 障碍物相关方法
    //添加障碍物
    void addObstacle(const QString &id, double x, double y, double radius, ObstacleType type);
    //清除所有障碍物
    void clearObstacles();
    //清除动态障碍物
    void clearDynamicObstacles();  
    //清除静态障碍物
    void clearStaticObstacles();   
    //清除过期障碍物
    void clearStaleObstacles(int timeoutMs = 1000);
    
    // 颜色获取方法(用于外部绘制图例)
    QColor getBlueTeamColor() const { return m_blueTeamColor; }//蓝队颜色
    QColor getRedTeamColor() const { return m_redTeamColor; }//红队颜色
    QColor getSelectedColor() const { return m_selectedColor; }//选中无人机颜色
    QColor getOfflineColor() const { return m_offlineColor; }//离线无人机颜色
    QColor getMountainColor() const { return m_mountainColor; }//山体障碍物颜色
    QColor getRadarColor() const { return m_radarColor; }//雷达障碍物颜色
    QColor getCloudColor() const { return m_cloudColor; }//雷云障碍物颜色

signals:
    /// 当用户点击地图上的无人机时发出此信号
    void droneClicked(const QString &droneId);

protected:
    // Qt事件处理函数
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // 绘制函数
    void drawBackground(QPainter &painter);//绘制背景
    void drawGrid(QPainter &painter);//绘制网格
    void drawDrones(QPainter &painter);//绘制无人机
    void drawObstacles(QPainter &painter);//绘制障碍物
    void drawMountainObstacle(QPainter &painter, const QPoint &pos, int radius);//绘制山体障碍物
    void drawRadarObstacle(QPainter &painter, const QPoint &pos, int radius);//绘制雷达障碍物
    void drawCloudObstacle(QPainter &painter, const QPoint &pos, int radius);//绘制雷云障碍物
    void drawLegend(QPainter &painter);//绘制图例
    
    // 坐标转换函数
    QPoint mapToWidget(const QPoint &worldPos) const;//将世界坐标转换为窗口坐标
    QPoint widgetToMap(const QPoint &widgetPos) const;//将窗口坐标转换为世界坐标
    QString getDroneAtPosition(const QPoint &widgetPos) const;//获取点击的无人机
    
    // 数据存储
    QMap<QString, DroneInfo> m_drones;          // 无人机信息映射表
    QMap<QString, ObstacleInfo> m_obstacles;    // 障碍物信息映射表
    QString m_selectedDrone;                    // 当前选中的无人机ID
    
    // 地图设置
    QRect m_mapBounds;          // 地图边界(世界坐标系)
    double m_zoomLevel;         // 当前缩放级别
    QPointF m_centerOffset;     // 缩放中心偏移量
    
    // 颜色主题
    QColor m_backgroundColor;   // 背景颜色
    QColor m_gridColor;         // 网格线颜色
    QColor m_borderColor;       // 边框颜色
    QColor m_blueTeamColor;     // 蓝队无人机颜色
    QColor m_redTeamColor;      // 红队无人机颜色
    QColor m_selectedColor;     // 选中无人机颜色
    QColor m_offlineColor;      // 离线无人机颜色
    
    // 障碍物颜色
    QColor m_mountainColor;     // 山脉障碍物颜色
    QColor m_radarColor;        // 雷达障碍物颜色
    QColor m_cloudColor;        // 云层障碍物颜色
    
    // 字体
    QFont m_labelFont;          // 标签字体
    
    // 绘制参数
    int m_droneSize;            // 无人机绘制大小
    int m_gridSpacing;          // 网格间距
    int m_margin;               // 边距
    
    // 常量
    const int obstaclePhysicalRadius = 80;  // 障碍物实际半径(世界坐标系)
    const int DRONE_BASE_SIZE = 4;          // 无人机基准大小
};

#endif // MINIMAPWIDGET_H
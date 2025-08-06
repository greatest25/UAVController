#ifndef RADARWIDGET_H
#define RADARWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QMap>
#include <QPoint>
#include <QColor>
#include <QtMath>
#include <QDateTime>
#include "obstacle_types.h"

// 无人机信息结构
struct RadarDroneInfo {
    QPoint position;     // 无人机位置
    QString team;        // 队伍（B或R）
    QString uid;         // 无人机ID
    int hp;             // 血量
    bool isVisible;     // 是否在探测范围内
};

class RadarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RadarWidget(QWidget *parent = nullptr);
    
    // 设置当前控制的无人机位置（雷达中心）
    void setCurrentDronePosition(const QPoint &pos);
    
    // 添加/更新无人机信息
    void updateDroneInfo(const QString &uid, const QPoint &pos, const QString &team, int hp);
    
    // 添加/更新障碍物信息
    void addObstacle(const QString &id, const QPoint &pos, int radius, ObstacleType type);

    // 清除过期的动态障碍物
    void clearStaleObstacles(int timeoutMs = 1000);

    // 清除所有障碍物
    void clearObstacles();

    // 清除所有数据
    void clearAll();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateRadar();  // 定时更新雷达显示

private:
    // 绘制方法
    void drawRadarBackground(QPainter &painter);
    void drawDetectionRange(QPainter &painter);
    void drawObstacles(QPainter &painter);
    void drawDrones(QPainter &painter);
    void drawRadarSweep(QPainter &painter);

    // 障碍物绘制方法
    void drawMountainObstacle(QPainter &painter, const QPoint &pos, int radius);
    void drawRadarObstacle(QPainter &painter, const QPoint &pos, int radius);
    void drawCloudObstacle(QPainter &painter, const QPoint &pos, int radius);
    
    // 坐标转换方法
    QPoint worldToRadar(const QPoint &worldPos) const;
    bool isInDetectionRange(const QPoint &worldPos) const;
    
    // 常量定义
    static const int DETECTION_RADIUS = 300;  // 探测半径（像素）
    static const int RADAR_SIZE = 350;        // 雷达显示区域大小
    
    // 成员变量
    QPoint m_currentDronePos;                 // 当前无人机位置（雷达中心）
    QMap<QString, RadarDroneInfo> m_drones;   // 所有无人机信息
    QMap<QString, ObstacleInfo> m_obstacles;  // 所有障碍物信息
    
    QTimer *m_updateTimer;                    // 更新定时器
    int m_sweepAngle;                         // 雷达扫描角度
    
    // 颜色定义
    QColor m_backgroundColor;
    QColor m_gridColor;
    QColor m_sweepColor;
    QColor m_mountainColor;
    QColor m_radarColor;
    QColor m_cloudColor;
    QColor m_blueTeamColor;
    QColor m_redTeamColor;
};

#endif // RADARWIDGET_H

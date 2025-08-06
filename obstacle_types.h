#ifndef OBSTACLE_TYPES_H
#define OBSTACLE_TYPES_H

#include <QString>
#include <QDateTime>

// 障碍物类型枚举
enum ObstacleType {
    Mountain,   // 山体 - 静态障碍物
    Radar,      // 雷达站 - 静态障碍物
    Cloud       // 雷云 - 动态障碍物
};

// 判断障碍物是否为静态障碍物
inline bool isStaticObstacle(ObstacleType type) {
    return type == Mountain || type == Radar;
}

// 判断障碍物是否为动态障碍物
inline bool isDynamicObstacle(ObstacleType type) {
    return type == Cloud;
}

// 障碍物信息结构
struct ObstacleInfo {
    QString id;                 // 障碍物ID
    ObstacleType type;          // 障碍物类型
    double x, y;                // 位置坐标
    double radius;              // 半径
    QDateTime timestamp;        // 时间戳
    
    ObstacleInfo() : x(0), y(0), radius(0), type(Mountain) {}
    ObstacleInfo(const QString& id, ObstacleType type, double x, double y, double radius)
        : id(id), type(type), x(x), y(y), radius(radius), timestamp(QDateTime::currentDateTime()) {}
};

#endif // OBSTACLE_TYPES_H 
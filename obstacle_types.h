#ifndef OBSTACLE_TYPES_H
#define OBSTACLE_TYPES_H

#include <QString>
#include <QDateTime>

// 障碍物类型枚举
enum ObstacleType {
    Mountain,   // 山体
    Radar,      // 雷达站
    Cloud       // 雷云
};

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
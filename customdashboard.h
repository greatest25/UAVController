#ifndef CUSTOMDASHBOARD_H
#define CUSTOMDASHBOARD_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QFont>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QMap>
#include <QStringList>
#include <QComboBox>
#include "dronestatuswidget.h"
#include "radarwidget.h"
#include "compasswidget.h"
#include "joystickwidget.h"
#include "minimapwidget.h"

// 前向声明，避免重复定义
class CompassWidget;

// 飞行控制面板
class FlightControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FlightControlWidget(QWidget *parent = nullptr);

    void setSelectedDrone(const QString &droneId);
    void updateDroneStatus(const QString &droneId, const QPoint &pos, double speed,
                          double heading, double altitude, int hp);

    QString getSelectedDrone() const { return m_selectedDrone; }

signals:
    void droneChanged(const QString &droneId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onDroneSelectionChanged(const QString &droneId);

private:
    void drawBackground(QPainter &painter);
    void drawDroneSelector(QPainter &painter);
    void drawFlightInfo(QPainter &painter);
    void drawHealthIndicator(QPainter &painter);
    void setupDroneComboBox();
    QString getDirectionText(double angle) const; // 添加方向文字转换函数
    
    QString m_selectedDrone;
    QPoint m_position;
    double m_speed;
    double m_heading;
    double m_altitude;
    int m_hp;

    // UI组件
    QComboBox *m_droneComboBox;

    QColor m_backgroundColor;
    QColor m_borderColor;
    QColor m_textColor;
    QColor m_accentColor;
    
    QFont m_titleFont;
    QFont m_dataFont;
};

// 主仪表盘控件
class CustomDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit CustomDashboard(QWidget *parent = nullptr);
    
    // 更新数据接口
    void updateDroneInfo(const QString &droneId, int hp, const QPoint &pos, bool online);
    void updateRadarInfo(const QString &uid, const QPoint &pos, const QString &team, int hp);
    void updateFlightInfo(const QString &droneId, const QPoint &pos, double speed,
                         double heading, double altitude, int hp);
    void setSelectedDrone(const QString &droneId);
    void setCompassHeading(double heading);
    
    // 全局缩略地图相关方法
    void updateMinimapDroneInfo(const QString &droneId, const QPoint &pos, const QString &team, int hp, bool online, double heading = 0.0);
    void setMinimapMapBounds(const QRect &bounds);
    void setMinimapZoomLevel(double zoom);
    
    // 全局缩略地图障碍物相关方法
    void addMinimapObstacle(const QString &id, const QPoint &pos, int radius, ObstacleType type);
    void clearMinimapObstacles();
    void clearMinimapStaleObstacles(int timeoutMs = 1000);
    
    // 雷达相关
    void addObstacle(const QString &id, const QPoint &pos, int radius,
                    ObstacleType type);
    void clearObstacles();
    void setCurrentDronePosition(const QPoint &pos);

    // 重置所有数据
    void resetAllData();

    // 获取子控件
    JoystickWidget* getJoystickWidget() const { return m_joystickWidget; }

signals:
    void droneSelectionChanged(const QString &droneId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupLayout();
    void drawBackground(QPainter &painter);
    void drawSectionTitles(QPainter &painter);
    // 手柄外形绘制
    void drawHandleShape(QPainter &painter);
    void drawHandleGrip(QPainter &painter);
    // 绘制缩略图图例
    void drawMinimapLegend(QPainter &painter);
    
    // 子控件
    DroneStatusWidget *m_blueTeamStatus;
    DroneStatusWidget *m_redTeamStatus;
    RadarWidget *m_radarWidget;
    CompassWidget *m_compassWidget;
    FlightControlWidget *m_flightControlWidget;
    JoystickWidget *m_joystickWidget;
    MinimapWidget *m_minimapWidget;
    
    // 布局参数
    QRect m_blueTeamRect;
    QRect m_redTeamRect;
    QRect m_radarRect;
    QRect m_compassRect;
    QRect m_flightControlRect;
    QRect m_joystickRect;
    QRect m_minimapRect;
    
    // 颜色主题
    QColor m_backgroundColor;
    QColor m_sectionBorderColor;
    QColor m_titleColor;
    
    // 字体
    QFont m_titleFont;
    
    // 标题
    QStringList m_sectionTitles;
    // 手柄区域缓存
    QRect m_handleBodyRect;
    QRect m_handleGripRect;
};

#endif // CUSTOMDASHBOARD_H

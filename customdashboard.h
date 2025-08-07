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
#include "obstacle_types.h"

// 前向声明，避免重复定义
class CompassWidget;

// 飞行控制面板
class FlightControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FlightControlWidget(QWidget *parent = nullptr);

    void setSelectedDrone(const QString &droneId);//设置当前选择无人机
    //更新无人机状态
    void updateDroneStatus(const QString &droneId, const QPoint &pos, double speed,
                          double heading, double altitude, int hp);

    QString getSelectedDrone() const { return m_selectedDrone; }//获取当前选择无人机

signals:
    void droneChanged(const QString &droneId);//无人机改变信号

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onDroneSelectionChanged(const QString &droneId);//无人机选择改变槽函数

private:
    void drawBackground(QPainter &painter);//绘制背景
    void drawDroneSelector(QPainter &painter);//绘制无人机选择器
    void drawFlightInfo(QPainter &painter);//绘制飞行信息
    void drawHealthIndicator(QPainter &painter);//绘制血量
    void setupDroneComboBox();//设置无人机选择器
    QString getDirectionText(double angle) const; // 添加方向文字转换函数
    
    /*
    成员变量
    m_selectedDrone:当前选择无人机
    m_position:无人机位置
    m_speed:无人机速度
    m_heading:无人机方向
    m_altitude:无人机高度（暂未使用）
    m_hp:无人机血量
    */
    QString m_selectedDrone;
    QPoint m_position;
    double m_speed;
    double m_heading;
    double m_altitude;
    int m_hp;

    // UI组件
    QComboBox *m_droneComboBox;

    QColor m_backgroundColor;//背景颜色
    QColor m_borderColor;//边框颜色
    QColor m_textColor;//文字颜色
    QColor m_accentColor;//强调颜色
    
    QFont m_titleFont;//标题字体    
    QFont m_dataFont;//数据字体
};

// 主仪表盘控件
class CustomDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit CustomDashboard(QWidget *parent = nullptr);
    
    // 更新数据接口
    void updateDroneInfo(const QString &droneId, int hp, const QPoint &pos, bool online);//更新无人机状态
    void updateRadarInfo(const QString &uid, const QPoint &pos, const QString &team, int hp);//更新雷达信息
    void updateFlightInfo(const QString &droneId, const QPoint &pos, double speed,
                         double heading, double altitude, int hp);//更新飞行信息
    void setSelectedDrone(const QString &droneId);//设置当前选择无人机
    void setCompassHeading(double heading);//设置指南针方向
    
    // 全局缩略地图相关方法
    //更新全局缩略地图无人机状态
    void updateMinimapDroneInfo(const QString &droneId, const QPoint &pos, const QString &team, int hp, bool online, double heading = 0.0);
    //设置全局缩略地图边界
    void setMinimapMapBounds(const QRect &bounds);
    //设置全局缩略地图缩放级别
    void setMinimapZoomLevel(double zoom);
    
    // 全局缩略地图障碍物相关方法
    //添加全局缩略地图障碍物
    void addMinimapObstacle(const QString &id, const QPoint &pos, int radius, ObstacleType type);
    //清除全局缩略地图障碍物
    void clearMinimapObstacles();
    //清除全局缩略地图动态障碍物
    void clearMinimapDynamicObstacles();  // 只清除动态障碍物
    void clearMinimapStaticObstacles();   // 只清除静态障碍物
    void clearMinimapStaleObstacles(int timeoutMs = 1000);//清除全局缩略地图过期障碍物，1000ms后清除
    
    // 雷达相关
    //添加雷达障碍物
    void addObstacle(const QString &id, const QPoint &pos, int radius,
                    ObstacleType type);
    //清除雷达障碍物
    void clearObstacles();
    //设置当前无人机位置
    void setCurrentDronePosition(const QPoint &pos);

    // 重置所有数据
    void resetAllData();

    // 获取子控件
    JoystickWidget* getJoystickWidget() const { return m_joystickWidget; }

signals:
    void droneSelectionChanged(const QString &droneId);//无人机选择改变信号

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupLayout();//设置布局
    void drawBackground(QPainter &painter);//绘制背景
    void drawSectionTitles(QPainter &painter);//绘制标题
    // 手柄外形绘制
    void drawHandleShape(QPainter &painter);//绘制手柄外形
    void drawHandleGrip(QPainter &painter);//绘制手柄握把
    // 绘制缩略图图例
    void drawMinimapLegend(QPainter &painter);//绘制缩略图图例
    
    // 子控件
    DroneStatusWidget *m_blueTeamStatus;//蓝方无人机状态
    DroneStatusWidget *m_redTeamStatus;//红方无人机状态
    RadarWidget *m_radarWidget;//雷达
    CompassWidget *m_compassWidget;//指南针
    FlightControlWidget *m_flightControlWidget;//飞行控制面板
    JoystickWidget *m_joystickWidget;//摇杆
    MinimapWidget *m_minimapWidget;//全局缩略地图
    
    // 布局参数
    QRect m_blueTeamRect;//蓝方无人机状态位置和大小
    QRect m_redTeamRect;//红方无人机状态位置和大小
    QRect m_radarRect;//雷达位置和大小
    QRect m_compassRect;//指南针位置和大小
    QRect m_flightControlRect;//飞行控制面板位置和大小
    QRect m_joystickRect;//摇杆位置和大小
    QRect m_minimapRect;//全局缩略地图位置和大小
    
    // 颜色主题
    QColor m_backgroundColor;//背景颜色
    QColor m_sectionBorderColor;//边框颜色
    QColor m_titleColor;//标题颜色
    
    // 字体
    QFont m_titleFont;
    
    // 标题
    QStringList m_sectionTitles;
    // 手柄区域缓存，用于绘制手柄外形和握把
    QRect m_handleBodyRect;//手柄外形
    QRect m_handleGripRect;//手柄握把
};

#endif // CUSTOMDASHBOARD_H

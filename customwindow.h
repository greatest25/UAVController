#ifndef CUSTOMWINDOW_H
#define CUSTOMWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QVsoa>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMap>
#include <QString>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>
#include "obstacle_types.h"
#include "customdashboard.h"
#include "radarwidget.h"


// 定义无人机信息结构体
/*
成员变量
hp:无人机血量
status:无人机状态
team:无人机队伍
uid:无人机ID
vx:无人机x速度
vy:无人机y速度
x:无人机x坐标
y:无人机y坐标
isOnline:无人机是否在线
speed:无人机速度
heading:无人机方向
altitude:无人机高度（暂未使用）
*/
struct CustomDroneInfo {
    int hp;
    QString status;
    QString team;
    QString uid;
    double vx;
    double vy;
    double x;
    double y;
    bool isOnline;
    double speed;
    double heading;
    double altitude;
    
    CustomDroneInfo() : hp(100), vx(0), vy(0), x(0), y(0),
                       isOnline(false), speed(0), heading(0),
                       altitude(0) {}
};

class CustomWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CustomWindow(QWidget *parent = nullptr);
    ~CustomWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;//键盘事件
    void mousePressEvent(QMouseEvent *event) override;//鼠标事件
    void closeEvent(QCloseEvent *event) override;//关闭事件

private slots:
    // 发送控制命令到服务器
    void sendControlCommand();

    // 更新UI显示
    void updateUIDisplay();

    // 摇杆控制（改用定时器驱动，不需要事件槽）

private:
    // 主要控件
    CustomDashboard *m_dashboard;
    
    // 通信相关
    QVsoaClient m_client;//VSOA客户端
    QTimer *m_controlTimer;//控制定时器
    QTimer *m_updateTimer;//更新定时器
    
    // 数据存储
    QMap<QString, CustomDroneInfo> m_dronesInfo;//无人机信息
    QString m_selectedDroneId;//选中无人机ID
    int m_currentDroneId;//当前无人机ID
    
    // 游戏状态
    int m_gameLeftTime;//游戏剩余时间
    QString m_gameStage;//游戏状态
    
    // 控制状态（使用定时器驱动，不需要状态变量）
    
    // 初始化函数
    void initializeUI();//初始化UI
    void initializeConnections();//初始化连接
    void initializeCommunication();//初始化通信
    
    // 通信回调函数
    void onConnected(bool ok, QString info);//VSOA连接成功
    void onDisconnected();//VSOA连接断开
    void onDatagram(QVsoaClient *client, QString url, QVsoaPayload payload);//VSOA数据信号
    
    // 数据处理函数
    void processGameData(const QString &jsonData);//处理游戏数据
    void updateDroneData();//更新无人机数据
    void updateRadarData();//更新雷达数据
    void resetAllData();//重置所有数据
    
    // 控制函数
    void handleKeyboardControl(QKeyEvent *event);//键盘控制
    
    // 工具函数
    double calculateDroneDirection(const QString &uid) const;//计算无人机方向
    double calculateDroneSpeed(const QString &uid) const;//计算无人机速度
    double calculateDroneDistance(const QString &uid) const;//计算无人机距离
    double calculateDistanceToNearestTarget(double x, double y, const QString &uid) const;//计算无人机到最近目标的距离
    QString getCurrentDroneUid() const;//获取当前无人机ID
};

#endif // CUSTOMWINDOW_H

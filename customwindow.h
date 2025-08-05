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
#include "customdashboard.h"
#include "radarwidget.h"

// 定义无人机信息结构体
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
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

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
    QVsoaClient m_client;
    QTimer *m_controlTimer;
    QTimer *m_updateTimer;
    
    // 数据存储
    QMap<QString, CustomDroneInfo> m_dronesInfo;
    QString m_selectedDroneId;
    int m_currentDroneId;
    
    // 游戏状态
    int m_gameLeftTime;
    QString m_gameStage;
    
    // 控制状态（使用定时器驱动，不需要状态变量）
    
    // 初始化函数
    void initializeUI();
    void initializeConnections();
    void initializeCommunication();
    
    // 通信回调函数
    void onConnected(bool ok, QString info);
    void onDisconnected();
    void onDatagram(QVsoaClient *client, QString url, QVsoaPayload payload);
    
    // 数据处理函数
    void processGameData(const QString &jsonData);
    void updateDroneData();
    void updateRadarData();
    void resetAllData();
    
    // 控制函数
    void handleKeyboardControl(QKeyEvent *event);
    
    // 工具函数
    double calculateDroneDirection(const QString &uid) const;
    double calculateDroneSpeed(const QString &uid) const;
    double calculateDroneDistance(const QString &uid) const;
    double calculateDistanceToNearestTarget(double x, double y, const QString &uid) const;
    QString getCurrentDroneUid() const;
};

#endif // CUSTOMWINDOW_H

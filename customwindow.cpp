#include "customwindow.h"
#include <QApplication>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QtMath>
#include <QRandomGenerator>
#include <functional>

CustomWindow::CustomWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_dashboard(nullptr)
    , m_controlTimer(nullptr)
    , m_updateTimer(nullptr)
    , m_currentDroneId(1)
    , m_selectedDroneId("B1")  // 初始化选中的无人机
    , m_gameLeftTime(0)
    , m_gameStage("init")
{
    initializeUI();
    initializeConnections();
    initializeCommunication();
}

CustomWindow::~CustomWindow()
{
    if (m_controlTimer) {
        m_controlTimer->stop();
    }
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
}

void CustomWindow::initializeUI()
{
    setWindowTitle("UAV Custom Console");
    // 设置与传统模式相同的固定尺寸，适配嵌入式环境
    setFixedSize(786, 490);
    
    // 创建主仪表盘
    m_dashboard = new CustomDashboard(this);
    setCentralWidget(m_dashboard);
    
    // 设置默认选择的无人机
    m_selectedDroneId = "B1";
    m_dashboard->setSelectedDrone(m_selectedDroneId);
    
    // 设置窗口样式
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #1e2329;"
        "    color: #ffffff;"
        "}"
    );
}

void CustomWindow::initializeConnections()
{
    // 创建定时器
    m_controlTimer = new QTimer(this);
    m_updateTimer = new QTimer(this);

    // 连接定时器信号 - 沿用传统模式的逻辑
    connect(m_controlTimer, &QTimer::timeout, this, &CustomWindow::sendControlCommand);
    connect(m_updateTimer, &QTimer::timeout, this, &CustomWindow::updateUIDisplay);

    // 连接无人机选择信号
    connect(m_dashboard, &CustomDashboard::droneSelectionChanged,
            this, [this](const QString &droneId) {
                m_selectedDroneId = droneId;
                qDebug() << "选中无人机:" << droneId;
            });

    // 启动定时器 - 使用与传统模式相同的频率
    m_controlTimer->start(50);   // 20Hz = 50ms，与传统模式一致
    m_updateTimer->start(50);    // 50ms更新一次UI
}

void CustomWindow::initializeCommunication()
{
    if (m_client.isInvalid()) {
        qDebug() << "Cannot create VSOA client!";
        return;
    }
    
    // 连接VSOA信号
    connect(&m_client, &QVsoaClient::connected, this, &CustomWindow::onConnected);
    connect(&m_client, &QVsoaClient::disconnected, this, &CustomWindow::onDisconnected);
    connect(&m_client, &QVsoaClient::datagram,
            std::bind(&CustomWindow::onDatagram, this, &m_client, std::placeholders::_1, std::placeholders::_2));
    
    // 连接到服务器
    m_client.connect2server("vsoa://127.0.0.1:3005/game_server", "", 1000);
    m_client.autoConnect(1000, 500);
}



void CustomWindow::onConnected(bool ok, QString info)
{
    if (ok) {
        qDebug() << "✅ VSOA连接成功:" << info;

        // 订阅游戏数据
        m_client.subscribe("/ctrl");
        m_client.subscribe("/game");
        m_client.autoConsistent({"/ctrl", "/game"}, 1000);

        qDebug() << "📡 已订阅频道: /ctrl, /game";
    } else {
        qDebug() << "❌ VSOA连接失败:" << info;
    }
}

void CustomWindow::onDisconnected()
{
    qDebug() << "🔌 VSOA连接断开";
}

void CustomWindow::onDatagram(QVsoaClient *client, QString url, QVsoaPayload payload)
{
    Q_UNUSED(client)

    QVariant param = payload.param();
    // 处理接收到的数据
    if (url == "/game") {
        // 处理游戏数据
        QString dataStr;

        // 根据实际类型转换
        if (param.type() == QVariant::String) {
            dataStr = param.toString();
        } else if (param.canConvert<QByteArray>()) {
            dataStr = QString::fromUtf8(param.toByteArray());
        } else {
            qDebug() << "不支持的参数类型:" << param.typeName();
            return;
        }

        processGameData(dataStr);
    } else if (url == "/ctrl") {
        // 处理控制数据的反馈
        qDebug() << "接收到控制数据反馈：" << param;
    }
}

void CustomWindow::processGameData(const QString &jsonData)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "无效的JSON数据";
        return;
    }

    QJsonObject gameObj = doc.object();

    // 调试：输出接收到的数据结构
    static int debugCount = 0;
    if (debugCount++ < 5) { // 只输出前5次，避免日志过多
        qDebug() << "游戏数据键:" << gameObj.keys();
    }

    // 更新游戏状态
    if (gameObj.contains("left_time")) {
        m_gameLeftTime = gameObj["left_time"].toInt();
    }

    if (gameObj.contains("stage")) {
        QString newStage = gameObj["stage"].toString();

        // 调试输出游戏状态变化
        static int stageDebugCount = 0;
        if (stageDebugCount++ < 10) {
            qDebug() << "CUSTOM WINDOW STAGE:" << m_gameStage << "=>" << newStage;
        }

        // 如果游戏状态发生变化，输出日志
        if (m_gameStage != newStage) {
            qDebug() << "CUSTOM WINDOW STATE CHANGE:" << m_gameStage << "->" << newStage;
        }

        // 检查游戏状态是否变为finish，如果是则重置所有数据
        if (newStage == "finish" && m_gameStage != "finish") {
            qDebug() << "CUSTOM WINDOW GAME FINISHED - RESETTING DATA";
            resetAllData();
        }

        m_gameStage = newStage;
    }

    // 更新无人机信息
    if (gameObj.contains("drones") && gameObj["drones"].isArray()) {
        QJsonArray drones = gameObj["drones"].toArray();

        // 清空之前的数据
        m_dronesInfo.clear();

        for (const QJsonValue &value : drones) {
            QJsonObject droneObj = value.toObject();

            QString uid = droneObj["uid"].toString();
            CustomDroneInfo info;
            
            info.uid = uid;
            info.hp = droneObj["hp"].toInt();
            info.status = droneObj["status"].toString();
            info.team = droneObj["team"].toString();
            info.x = droneObj["x"].toDouble();
            info.y = droneObj["y"].toDouble();
            info.vx = droneObj["vx"].toDouble();
            info.vy = droneObj["vy"].toDouble();
            info.isOnline = (info.status == "alive");
            
            // 计算速度和方向
            info.speed = qSqrt(info.vx * info.vx + info.vy * info.vy);

            // 计算飞行方向（地理角度：北为0度，顺时针）
            if (info.speed > 0.1) { // 只有在移动时才计算方向
                // 注意：屏幕坐标系Y轴向下，需要取反来匹配地理坐标系
                info.heading = qRadiansToDegrees(qAtan2(static_cast<double>(info.vx), -static_cast<double>(info.vy)));
                if (info.heading < 0) info.heading += 360;
            } else {
                // 静止时保持上次的方向
                if (m_dronesInfo.contains(uid)) {
                    info.heading = m_dronesInfo[uid].heading;
                } else {
                    info.heading = 0; // 默认朝北
                }
            }
            
            // 计算到最近敌机的距离（如果有敌机的话）
            info.altitude = calculateDistanceToNearestTarget(info.x, info.y, uid);
            
            m_dronesInfo[uid] = info;
        }
    }

    // 更新障碍物信息（参考vsoamanager的实现）
    if (gameObj.contains("obstacles") && gameObj["obstacles"].isArray()) {
        QJsonArray obstacles = gameObj["obstacles"].toArray();

        // 清空雷达障碍物
        m_dashboard->clearObstacles();

        // 更新障碍物信息
        for (const QJsonValue &obstacleValue : obstacles) {
            if (obstacleValue.isObject()) {
                QJsonObject obstacle = obstacleValue.toObject();

                QString id = obstacle["id"].toString();
                double r = obstacle["r"].toDouble();
                QString type = obstacle["type"].toString();
                double x = obstacle["x"].toDouble();
                double y = obstacle["y"].toDouble();

                // 转换障碍物类型
                ObstacleType obsType;
                if (type == "mountain") {
                    obsType = MOUNTAIN_OBS;
                } else if (type == "radar") {
                    obsType = RADAR_OBS;
                } else if (type == "cloud") {
                    obsType = CLOUD_OBS;
                } else {
                    continue; // 未知类型，跳过
                }

                // 添加到雷达显示
                m_dashboard->addObstacle(id, QPoint(x, y), r, obsType);
            }
        }
    }
}

void CustomWindow::updateUIDisplay()
{
    updateDroneData();
    updateRadarData();
}

void CustomWindow::updateDroneData()
{
    // 获取当前选择的蓝方无人机位置（用于探测范围计算）
    QPoint currentDronePos;
    bool hasCurrentDrone = false;
    if (m_dronesInfo.contains(m_selectedDroneId)) {
        const CustomDroneInfo &currentInfo = m_dronesInfo[m_selectedDroneId];
        currentDronePos = QPoint(currentInfo.x, currentInfo.y);
        hasCurrentDrone = true;
    }

    // 更新所有无人机状态
    for (auto it = m_dronesInfo.begin(); it != m_dronesInfo.end(); ++it) {
        const CustomDroneInfo &info = it.value();

        // 对于红方无人机，需要检查是否在探测范围内
        bool shouldUpdateStatus = true;
        if (info.uid.startsWith("R") && hasCurrentDrone) {
            // 计算与当前选择无人机的距离
            QPoint enemyPos(info.x, info.y);
            QPoint relativePos = enemyPos - currentDronePos;
            double distance = qSqrt(relativePos.x() * relativePos.x() + relativePos.y() * relativePos.y());

            // 只有在探测范围内（300像素）才更新状态栏
            shouldUpdateStatus = (distance <= 300);
        }

        // 更新无人机状态信息（红方只有在探测范围内才更新）
        if (shouldUpdateStatus) {
            m_dashboard->updateDroneInfo(info.uid, info.hp, QPoint(info.x, info.y), info.isOnline);
        } else {
            // 红方无人机不在探测范围内，设置为离线状态
            m_dashboard->updateDroneInfo(info.uid, 0, QPoint(info.x, info.y), false);
        }

        // 蓝方飞行信息总是更新，红方只有在探测范围内才更新
        if (info.uid.startsWith("B") || shouldUpdateStatus) {
            m_dashboard->updateFlightInfo(info.uid, QPoint(info.x, info.y),
                                         info.speed, info.heading, info.altitude, info.hp);
        }

        // 雷达信息总是更新（雷达组件内部会处理探测范围）
        m_dashboard->updateRadarInfo(info.uid, QPoint(info.x, info.y), info.team, info.hp);

        // 调试输出（每10次输出一次，避免日志过多）
        static int updateCount = 0;
//        if (updateCount++ % 10 == 0) {
//            qDebug() << QString("更新无人机 %1: HP=%2, 位置=(%3,%4), 在线=%5, 状态栏更新=%6")
//                        .arg(info.uid).arg(info.hp).arg(info.x).arg(info.y).arg(info.isOnline).arg(shouldUpdateStatus);
//        }
    }

    // 更新指南针（使用当前选择的无人机方向）
    if (m_dronesInfo.contains(m_selectedDroneId)) {
        double heading = m_dronesInfo[m_selectedDroneId].heading;
        m_dashboard->setCompassHeading(heading);

        // 更新雷达中心位置
        QPoint pos(m_dronesInfo[m_selectedDroneId].x, m_dronesInfo[m_selectedDroneId].y);
        m_dashboard->setCurrentDronePosition(pos);
    }
}

void CustomWindow::updateRadarData()
{
    // 这里可以添加雷达特定的更新逻辑
    // 比如障碍物检测、敌方无人机探测等
}

void CustomWindow::sendControlCommand()
{
    // 如果游戏已结束或未开始，不发送控制命令
    if (m_gameStage == "finish" || m_gameStage == "init") {
        static bool hasLogged = false;
        if (!hasLogged) {
            qDebug() << "CUSTOM WINDOW STOP SENDING COMMANDS - GAME STATE:" << m_gameStage;
            hasLogged = true;
        }
        return;
    }

    // 完全沿用传统模式的摇杆控制逻辑

    // 获取当前角度和距离
    int angle = m_dashboard->getJoystickWidget()->getAngle();
    double distance = m_dashboard->getJoystickWidget()->getDistance();

    // 如果摇杆在中心位置，发送速度为0的命令
    double vx = 0.0;
    double vy = 0.0;

    if (angle != -1) {
        // 将角度转换为弧度
        double radians = angle * M_PI / 180.0;

        // 计算初始vx和vy
        double rawVx = qSin(radians);  // 注意：sin对应x方向
        double rawVy = -qCos(radians); // 注意：-cos对应y方向（屏幕坐标系）

        // 计算缩放因子，确保vx和vy的最大值为50
        double maxComponent = qMax(qAbs(rawVx), qAbs(rawVy));
        double scaleFactor = 50.0 / maxComponent;

        // 应用缩放因子和距离比例
        vx = rawVx * scaleFactor * distance;
        vy = rawVy * scaleFactor * distance;
    }

    // 获取当前选择的无人机ID
    QString droneId = getCurrentDroneUid();

    // 创建JSON对象
    QJsonObject controlObj;
    controlObj["uid"] = droneId;
    controlObj["vx"] = vx;
    controlObj["vy"] = vy;

    // 转换为JSON文档
    QJsonDocument doc(controlObj);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    // 创建payload并发送
    QVsoaPayload payload;
    payload.setParam(jsonString);

    // 发送到/ctrl通道
    m_client.sendDatagram("/ctrl", payload);

    // 调试输出
    if (angle != -1) {
        static int debugCount = 0;
        if (debugCount++ % 20 == 0) { // 每秒输出一次（20Hz * 1s）
            qDebug() << "发送控制命令: uid=" << droneId
                     << ", vx=" << vx << ", vy=" << vy
                     << ", 角度=" << angle << ", 距离=" << distance;
        }
    }
}

// sendMovementCommand 方法已删除，改用 sendControlCommand 统一处理

QString CustomWindow::getCurrentDroneUid() const
{
    // 优先使用选中的无人机ID，否则使用默认的
    if (!m_selectedDroneId.isEmpty()) {
        return m_selectedDroneId;
    }
    return QString("B%1").arg(m_currentDroneId);
}

void CustomWindow::keyPressEvent(QKeyEvent *event)
{
    handleKeyboardControl(event);
    QMainWindow::keyPressEvent(event);
}

void CustomWindow::handleKeyboardControl(QKeyEvent *event)
{
    // 键盘控制主要用于无人机选择，移动控制由摇杆负责
    switch (event->key()) {
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
            m_currentDroneId = event->key() - Qt::Key_0;
            m_selectedDroneId = QString("B%1").arg(m_currentDroneId);
            m_dashboard->setSelectedDrone(m_selectedDroneId);
            qDebug() << "键盘选择无人机:" << m_selectedDroneId;
            break;
        default:
            // 其他按键不处理
            break;
    }
}

// 删除事件驱动的摇杆控制方法，改用定时器驱动（与传统模式一致）

void CustomWindow::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // 可以添加鼠标控制逻辑
    QMainWindow::mousePressEvent(event);
}

void CustomWindow::closeEvent(QCloseEvent *event)
{
    m_client.disconnectServer();
    event->accept();
}

// 工具函数实现
double CustomWindow::calculateDroneDirection(const QString &uid) const
{
    if (m_dronesInfo.contains(uid)) {
        return m_dronesInfo[uid].heading;
    }
    return 0.0;
}

double CustomWindow::calculateDroneSpeed(const QString &uid) const
{
    if (m_dronesInfo.contains(uid)) {
        return m_dronesInfo[uid].speed;
    }
    return 0.0;
}

double CustomWindow::calculateDroneDistance(const QString &uid) const
{
    if (m_dronesInfo.contains(uid)) {
        const CustomDroneInfo &info = m_dronesInfo[uid];
        return qSqrt(info.x * info.x + info.y * info.y);
    }
    return 0.0;
}

double CustomWindow::calculateDistanceToNearestTarget(double x, double y, const QString &uid) const
{
    double minDistance = 1000.0; // 默认距离

    // 如果是蓝方无人机，计算到最近红方无人机的距离
    if (uid.startsWith("B")) {
        for (auto it = m_dronesInfo.begin(); it != m_dronesInfo.end(); ++it) {
            const CustomDroneInfo &info = it.value();
            if (info.uid.startsWith("R") && info.hp > 0) {
                double distance = qSqrt(qPow(x - info.x, 2) + qPow(y - info.y, 2));
                minDistance = qMin(minDistance, distance);
            }
        }
    }
    // 如果是红方无人机，计算到最近蓝方无人机的距离
    else if (uid.startsWith("R")) {
        for (auto it = m_dronesInfo.begin(); it != m_dronesInfo.end(); ++it) {
            const CustomDroneInfo &info = it.value();
            if (info.uid.startsWith("B") && info.hp > 0) {
                double distance = qSqrt(qPow(x - info.x, 2) + qPow(y - info.y, 2));
                minDistance = qMin(minDistance, distance);
            }
        }
    }

    return minDistance;
}

void CustomWindow::resetAllData()
{
    qDebug() << "CUSTOM WINDOW RESET ALL DATA - GAME FINISHED";

    // 清空无人机信息
    m_dronesInfo.clear();

    // 重置游戏状态
    m_gameLeftTime = 0;

    // 重置自定义仪表盘的所有数据
    if (m_dashboard) {
        m_dashboard->resetAllData();
    }

    // 重置当前选择的无人机ID
    m_currentDroneId = 1;
    m_selectedDroneId = "B1";

    qDebug() << "CUSTOM WINDOW RESET COMPLETE - GAME STATE:" << m_gameStage;
}

#include "customdashboard.h"
#include <QPaintEvent>
#include <QResizeEvent>
#include <QtMath>

// CompassWidget 实现已移至 compasswidget.cpp

// FlightControlWidget 实现
FlightControlWidget::FlightControlWidget(QWidget *parent)
    : QWidget(parent)
    , m_selectedDrone("B1")
    , m_position(0, 0)
    , m_speed(0.0)
    , m_heading(0.0)
    , m_altitude(0.0)
    , m_hp(100)
    , m_droneComboBox(nullptr)
{
    setMinimumSize(160, 100);  // 进一步减小尺寸，让面板更紧凑

    m_backgroundColor = QColor(40, 44, 52);
    m_borderColor = QColor(241, 196, 15);
    m_textColor = QColor(236, 240, 241);
    m_accentColor = QColor(241, 196, 15);

    m_titleFont = QFont("Arial", 14, QFont::Bold);  // 减小字体
    m_dataFont = QFont("Arial", 12);               // 减小字体

    setupDroneComboBox();
}

void FlightControlWidget::setSelectedDrone(const QString &droneId)
{
    m_selectedDrone = droneId;

    // 更新下拉列表选择
    if (m_droneComboBox) {
        int index = m_droneComboBox->findText(droneId);
        if (index >= 0) {
            m_droneComboBox->setCurrentIndex(index);
        }
    }

    update();
}

void FlightControlWidget::updateDroneStatus(const QString &droneId, const QPoint &pos,
                                           double speed, double heading, double altitude, int hp)
{
    if (droneId == m_selectedDrone) {
        m_position = pos;
        m_speed = speed;
        m_heading = heading;
        m_altitude = altitude;
        m_hp = hp;



        update();
    }
}

void FlightControlWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // 调整下拉列表位置和大小 - 更紧凑
    if (m_droneComboBox) {
        m_droneComboBox->setGeometry(width() - 70, 8, 55, 16);
    }
}

void FlightControlWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter);
    drawDroneSelector(painter);
    drawFlightInfo(painter);
    drawHealthIndicator(painter);
}

void FlightControlWidget::drawBackground(QPainter &painter)
{
    painter.fillRect(rect(), m_backgroundColor);
    painter.setPen(QPen(m_borderColor, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
}

void FlightControlWidget::drawDroneSelector(QPainter &painter)
{
    painter.setFont(m_titleFont);
    painter.setPen(m_accentColor);

    QRect titleRect(15, 8, width() - 80, 20);  // 调整x坐标
    painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, "当前飞机:");
}

void FlightControlWidget::setupDroneComboBox()
{
    m_droneComboBox = new QComboBox(this);

    // 添加无人机选项
    QStringList drones = {"B1", "B2", "B3", "R1", "R2", "R3"};
    m_droneComboBox->addItems(drones);
    m_droneComboBox->setCurrentText("B1");

    // 设置样式 - 更紧凑
    m_droneComboBox->setStyleSheet(
        "QComboBox {"
        "    background-color: #34495e;"
        "    color: #ecf0f1;"
        "    border: 1px solid #f1c40f;"
        "    border-radius: 2px;"
        "    padding: 1px 3px;"
        "    font-size: 8px;"
        "    font-weight: bold;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 12px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 3px solid transparent;"
        "    border-right: 3px solid transparent;"
        "    border-top: 3px solid #f1c40f;"
        "    margin-right: 2px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: #2c3e50;"
        "    color: #ecf0f1;"
        "    selection-background-color: #f1c40f;"
        "    selection-color: #2c3e50;"
        "    border: 1px solid #f1c40f;"
        "}"
    );

    // 连接信号
    connect(m_droneComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &FlightControlWidget::onDroneSelectionChanged);

    // 设置初始位置 - 调整x坐标与血条对齐
    m_droneComboBox->setGeometry(width() - 70, 15, 55, 16);
}

void FlightControlWidget::onDroneSelectionChanged(const QString &droneId)
{
    if (droneId != m_selectedDrone) {
        m_selectedDrone = droneId;
        emit droneChanged(droneId);
        update();
    }
}

void FlightControlWidget::drawFlightInfo(QPainter &painter)
{
    // 使用更紧凑的字体
    QFont compactFont = m_dataFont;
    compactFont.setPointSize(12);  // 使用更大的字体
    painter.setFont(compactFont);
    painter.setPen(m_textColor);

    int y = 55;        // 调整起始位置，将信息栏再往下移动
    int lineHeight = 16; // 增加行高，让垂直间距更大

    // 绘制飞行数据
    QStringList infoLines = {
        QString("坐标: (%1, %2)").arg(m_position.x()).arg(m_position.y()),
        QString("速度: %1 m/s").arg(m_speed, 0, 'f', 1),  // 简化标签
        QString("方向: %1°").arg(m_heading, 0, 'f', 0)
    };

    for (const QString &line : infoLines) {
        painter.drawText(15, y, line);  // 调整x坐标与标题对齐
        y += lineHeight;
    }

    // 添加方向描述
    QString directionText = getDirectionText(m_heading);
    painter.drawText(15, y, QString("朝向: %1").arg(directionText));
}

// 添加方向文字转换函数
QString FlightControlWidget::getDirectionText(double angle) const
{
    if (angle >= 337.5 || angle < 22.5) return "北";
    else if (angle >= 22.5 && angle < 67.5) return "东北";
    else if (angle >= 67.5 && angle < 112.5) return "东";
    else if (angle >= 112.5 && angle < 157.5) return "东南";
    else if (angle >= 157.5 && angle < 202.5) return "南";
    else if (angle >= 202.5 && angle < 247.5) return "西南";
    else if (angle >= 247.5 && angle < 292.5) return "西";
    else return "西北";
}

void FlightControlWidget::drawHealthIndicator(QPainter &painter)
{
    // 将血条放在下拉条下方，与下拉框x坐标对齐，增加间距
    QRect healthRect(width() - 70, 35, 55, 15);  // 调整宽度与下拉框一致(55像素)

    // 绘制血量条外框
    painter.setPen(QPen(m_textColor, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(healthRect);

    // 绘制血量
    QColor healthColor;
    if (m_hp > 60) {
        healthColor = QColor(39, 174, 96);  // 绿色
    } else if (m_hp > 30) {
        healthColor = QColor(241, 196, 15); // 黄色
    } else {
        healthColor = QColor(231, 76, 60);  // 红色
    }

    int fillWidth = (healthRect.width() - 2) * m_hp / 100;
    QRect fillRect(healthRect.left() + 1, healthRect.top() + 1, fillWidth, healthRect.height() - 2);
    painter.fillRect(fillRect, healthColor);

    // 绘制血量文字
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 8, QFont::Bold));  // 减小字体
    painter.drawText(healthRect, Qt::AlignCenter, QString("%1").arg(m_hp));  // 简化显示
}

// CustomDashboard 实现
CustomDashboard::CustomDashboard(QWidget *parent)
    : QWidget(parent)
{
    // 适配固定尺寸 786x490
    setMinimumSize(786, 490);
    setMaximumSize(786, 490);

    // 设置颜色主题
    m_backgroundColor = QColor(30, 34, 42);
    m_sectionBorderColor = QColor(52, 152, 219);
    m_titleColor = QColor(236, 240, 241);

    m_titleFont = QFont("Arial", 12, QFont::Bold);

    // 设置区域标题（6个区域）
    m_sectionTitles << "Blue Team Status" << "Tactical Radar" << "Red Team Status"
                    << "Flight Control" << "Joystick Control" << "Navigation Compass";

    // 创建子控件
    m_blueTeamStatus = new DroneStatusWidget(this);
    m_blueTeamStatus->setTeam("B");

    m_redTeamStatus = new DroneStatusWidget(this);
    m_redTeamStatus->setTeam("R");

    m_radarWidget = new RadarWidget(this);
    m_compassWidget = new CompassWidget(this);
    m_flightControlWidget = new FlightControlWidget(this);
    m_joystickWidget = new JoystickWidget(this);

    // 连接无人机选择信号（只保留飞行控制面板的下拉选择）
    connect(m_flightControlWidget, &FlightControlWidget::droneChanged,
            this, &CustomDashboard::setSelectedDrone);

    setupLayout();
}

void CustomDashboard::updateDroneInfo(const QString &droneId, int hp, const QPoint &pos, bool online)
{
    if (droneId.startsWith("B")) {
        m_blueTeamStatus->updateDroneInfo(droneId, hp, pos, online);
    } else if (droneId.startsWith("R")) {
        m_redTeamStatus->updateDroneInfo(droneId, hp, pos, online);
    }
}

void CustomDashboard::updateRadarInfo(const QString &uid, const QPoint &pos, const QString &team, int hp)
{
    m_radarWidget->updateDroneInfo(uid, pos, team, hp);
}

void CustomDashboard::updateFlightInfo(const QString &droneId, const QPoint &pos, double speed,
                                      double heading, double altitude, int hp)
{
    m_flightControlWidget->updateDroneStatus(droneId, pos, speed, heading, altitude, hp);
}

void CustomDashboard::setSelectedDrone(const QString &droneId)
{
    m_flightControlWidget->setSelectedDrone(droneId);

    // 通知外部选中状态改变
    emit droneSelectionChanged(droneId);
}

void CustomDashboard::setCompassHeading(double heading)
{
    m_compassWidget->setDirection(heading);
}



void CustomDashboard::addObstacle(const QString &id, const QPoint &pos, int radius,
                                 ObstacleType type)
{
    m_radarWidget->addObstacle(id, pos, radius, type);
}

void CustomDashboard::clearObstacles()
{
    m_radarWidget->clearObstacles();
}

void CustomDashboard::setCurrentDronePosition(const QPoint &pos)
{
    m_radarWidget->setCurrentDronePosition(pos);
}

void CustomDashboard::drawHandleShape(QPainter &painter)
{
    int handleWidth = width() * 0.85;
    int handleHeight = height() * 0.8;
    int handleX = (width() - handleWidth) / 2;
    int handleY = height() * 0.08;
    m_handleBodyRect = QRect(handleX, handleY, handleWidth, handleHeight);

    painter.setPen(QPen(QColor(70, 130, 180), 3));
    painter.setBrush(QBrush(QColor(45, 49, 57)));
    painter.drawRoundedRect(m_handleBodyRect, 25, 25);

    // 顶部装饰线
    painter.setPen(QPen(QColor(70, 130, 180), 1));
    painter.drawLine(handleX + 30, handleY + 20, handleX + handleWidth - 30, handleY + 20);
    // 底部装饰线
    painter.drawLine(handleX + 30, handleY + handleHeight - 20, handleX + handleWidth - 30, handleY + handleHeight - 20);
}

void CustomDashboard::drawHandleGrip(QPainter &painter)
{
    int gripWidth = width() * 0.5;
    int gripHeight = height() * 0.12;
    int gripX = (width() - gripWidth) / 2;
    int gripY = height() * 0.82;
    m_handleGripRect = QRect(gripX, gripY, gripWidth, gripHeight);

    painter.setPen(QPen(QColor(70, 130, 180), 2));
    painter.setBrush(QBrush(QColor(45, 49, 57)));
    painter.drawRoundedRect(m_handleGripRect, 20, 20);

    // 握把装饰线
    painter.setPen(QPen(QColor(70, 130, 180), 1));
    for (int i = 0; i < 3; ++i) {
        int y = gripY + gripHeight * (i + 1) / 4;
        painter.drawLine(gripX + 15, y, gripX + gripWidth - 15, y);
    }
}

void CustomDashboard::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawBackground(painter);
    drawHandleShape(painter);   // 先画手柄主体
    drawHandleGrip(painter);    // 再画握把
    drawSectionTitles(painter); // 最后画内容
}

void CustomDashboard::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    setupLayout();
}

void CustomDashboard::setupLayout()
{
    // 计算手柄内部可用区域
    int handleWidth = width() * 0.85;
    int handleHeight = height() * 0.8;
    int handleX = (width() - handleWidth) / 2;
    int handleY = height() * 0.08;
    QRect handleBodyRect(handleX, handleY, handleWidth, handleHeight);

    int margin = 12; // 增大边距，防止内容贴边
    int titleHeight = 0; // 去掉标题高度，因为不再显示标题
    int spacing = 8;

    // 计算区域大小 - 6部分布局（2行3列），全部在手柄内部
    int sectionWidth = (handleBodyRect.width() - 4 * margin - 2 * spacing) / 3;
    int sectionHeight = (handleBodyRect.height() - 3 * margin - spacing) / 2; // 去掉titleHeight

    // 第一行：蓝队状态、雷达、红队状态
    int compactHeight = sectionHeight * 0.85;
    m_blueTeamRect = QRect(handleBodyRect.left() + margin+10, handleBodyRect.top() + margin + 18, // 增加12像素，将蓝队状态面板往下移动
                          sectionWidth * 0.6, compactHeight);
    // 缩小雷达区域宽度和高度
    m_radarRect = QRect(handleBodyRect.left() + margin * 2 + sectionWidth * 0.6 + 30,
                       handleBodyRect.top() + margin + 12, // 从8增加到12，将雷达往下移动
                       sectionWidth * 0.95, sectionHeight * 0.85);
    m_redTeamRect = QRect(handleBodyRect.left() + margin * 3 + sectionWidth * 2.4, handleBodyRect.top() + margin + 18, // 增加12像素，将红队状态面板往下移动
                         sectionWidth * 0.6, compactHeight);

    // 第二行：摇杆、飞行控制、指南针 - 左移指南针和飞行控制面板
    int joystickOffsetX = -8; // 向左偏移8像素  
    int joystickOffsetY = -8; // 向上偏移8像素
    int bottomY = handleBodyRect.top() + margin * 2 + sectionHeight + spacing;
    m_joystickRect = QRect(handleBodyRect.left() + margin + joystickOffsetX, bottomY + joystickOffsetY, sectionWidth, sectionHeight);
    
    // 飞行控制面板左移
    int compactFlightWidth = sectionWidth * 0.7;    
    int compactFlightHeight = sectionHeight * 0.7;
    m_flightControlRect = QRect(handleBodyRect.left() + margin * 2 + sectionWidth + (sectionWidth - compactFlightWidth) / 2 - 60, // 左移60像素
                               bottomY + (sectionHeight - compactFlightHeight) / 2,
                               compactFlightWidth, compactFlightHeight);
    
    // 指南针左移
    int compassSize = qMin(sectionWidth, sectionHeight) * 0.8;
    int compassX = handleBodyRect.left() + margin * 3 + sectionWidth * 2 + (sectionWidth - compassSize) / 2 - 100; // 左移80像素
    int compassY = bottomY + (sectionHeight - compassSize) / 2;
    m_compassRect = QRect(compassX, compassY, compassSize, compassSize);

    // 调整子控件位置和大小
    m_blueTeamStatus->setGeometry(m_blueTeamRect);
    m_radarWidget->setGeometry(m_radarRect);
    m_redTeamStatus->setGeometry(m_redTeamRect);
    m_flightControlWidget->setGeometry(m_flightControlRect);
    m_joystickWidget->setGeometry(m_joystickRect);
    m_compassWidget->setGeometry(m_compassRect);
}

void CustomDashboard::drawBackground(QPainter &painter)
{
    painter.fillRect(rect(), m_backgroundColor);
}

void CustomDashboard::drawSectionTitles(QPainter &painter)
{
    // 注释掉标题绘制，让界面更简洁
    /*
    painter.setFont(m_titleFont);
    painter.setPen(m_titleColor);

    // 绘制各区域标题（6个区域）
    QList<QRect> titleRects = {
        QRect(m_blueTeamRect.left(), m_blueTeamRect.top() - 25, m_blueTeamRect.width(), 20),
        QRect(m_radarRect.left(), m_radarRect.top() - 25, m_radarRect.width(), 20),
        QRect(m_redTeamRect.left(), m_redTeamRect.top() - 25, m_redTeamRect.width(), 20),
        QRect(m_flightControlRect.left(), m_flightControlRect.top() - 25, m_flightControlRect.width(), 20),
        QRect(m_joystickRect.left(), m_joystickRect.top() - 25, m_joystickRect.width(), 20),
        QRect(m_compassRect.left(), m_compassRect.top() - 25, m_compassRect.width(), 20)
    };

    for (int i = 0; i < m_sectionTitles.size() && i < titleRects.size(); ++i) {
        painter.drawText(titleRects[i], Qt::AlignCenter, m_sectionTitles[i]);
    }
    */
}

void CustomDashboard::resetAllData()
{
    // 重置CustomDashboard所有数据

    // 重置蓝队状态
    if (m_blueTeamStatus) {
        QStringList blueDrones = {"B1", "B2", "B3"};
        for (const QString &droneId : blueDrones) {
            m_blueTeamStatus->updateDroneInfo(droneId, 0, QPoint(0, 0), false);
        }
    }

    // 重置红队状态
    if (m_redTeamStatus) {
        QStringList redDrones = {"R1", "R2", "R3"};
        for (const QString &droneId : redDrones) {
            m_redTeamStatus->updateDroneInfo(droneId, 0, QPoint(0, 0), false);
        }
    }

    // 重置雷达显示
    if (m_radarWidget) {
        m_radarWidget->clearAll();
    }

    // 重置指南针
    if (m_compassWidget) {
        m_compassWidget->setDirection(0);
    }

    // 重置飞行控制面板
    if (m_flightControlWidget) {
        m_flightControlWidget->setSelectedDrone("B1");
        m_flightControlWidget->updateDroneStatus("B1", QPoint(0, 0), 0.0, 0.0, 0.0, 100);
    }

    // 重置摇杆
    if (m_joystickWidget) {
        m_joystickWidget->resetJoystick();
    }
}

#include "dronestatuswidget.h"
#include <QPaintEvent>
#include <QResizeEvent>

/*
无人机状态显示组件
成员变量
m_team:队伍
m_itemHeight:项目高度
m_margin:边距
m_spacing:间距
*/
DroneStatusWidget::DroneStatusWidget(QWidget *parent)
    : QWidget(parent)
    , m_team("B")
    , m_itemHeight(30)  // 项目高度
    , m_margin(8)       // 增加边距，让内容离边框更远
    , m_spacing(2)      // 间距
    , m_blinkState(false)
{
    // 设置默认颜色（蓝队）
    setTeamColors(QColor(123, 179, 211), QColor(100, 150, 200));
    
    // 设置字体 
    m_idFont = QFont("Arial", 14, QFont::Bold);  // ID字体
    m_posFont = QFont("Arial", 13);              // 位置字体
    
    // 设置默认无人机列表
    if (m_team == "B") {
        setDroneList({"B1", "B2", "B3"});
    } else {
        setDroneList({"R1", "R2", "R3"});
    }
    
    // 初始化闪烁定时器
    m_blinkTimer = new QTimer(this);
    connect(m_blinkTimer, &QTimer::timeout, [this]() {
        m_blinkState = !m_blinkState;
        update();
    });
    m_blinkTimer->start(500); // 500ms闪烁一次
    
    // 设置最小尺寸 - 减小宽度，适配紧凑布局
    setMinimumSize(100, 120);
    calculateLayout();
}

//更新无人机信息，hp:血量，pos:位置，online:是否在线
void DroneStatusWidget::updateDroneInfo(const QString &droneId, int hp, const QPoint &pos, bool online)
{
    if (m_drones.contains(droneId)) {
        m_drones[droneId].hp = hp;
        m_drones[droneId].position = pos;
        m_drones[droneId].isOnline = online;
        update();
    }
}

//设置队伍，team:队伍
void DroneStatusWidget::setTeam(const QString &team)
{
    m_team = team;
    
    if (team == "B") {
        setTeamColors(QColor(123, 179, 211), QColor(100, 150, 200));
        setDroneList({"B1", "B2", "B3"});
    } else {
        setTeamColors(QColor(231, 76, 60), QColor(200, 100, 100));
        setDroneList({"R1", "R2", "R3"});
    }
}

//设置无人机列表，droneIds:无人机ID列表
void DroneStatusWidget::setDroneList(const QStringList &droneIds)
{
    m_droneOrder = droneIds;
    m_drones.clear();
    
    for (const QString &id : droneIds) {
        DroneStatusInfo info;
        info.id = id;
        info.team = m_team;
        m_drones[id] = info;
    }
    
    calculateLayout();//计算布局
    update();//更新
}

//设置队伍颜色，primaryColor:主颜色，secondaryColor:次颜色
void DroneStatusWidget::setTeamColors(const QColor &primaryColor, const QColor &secondaryColor)
{
    m_primaryColor = primaryColor;
    m_secondaryColor = secondaryColor;

    // 设置相关颜色
    m_backgroundColor = QColor(40, 44, 52);
    m_textColor = primaryColor;
    m_onlineColor = QColor(39, 174, 96);  // 绿色
    m_offlineColor = QColor(149, 165, 166); // 灰色

    update();
}

//绘制事件
void DroneStatusWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    drawBackground(painter);
    
    // 绘制每个无人机的状态
    for (int i = 0; i < m_droneOrder.size(); ++i) {
        const QString &droneId = m_droneOrder[i];
        if (m_drones.contains(droneId)) {
            QRect droneRect = getDroneRect(i);
            drawDroneStatus(painter, m_drones[droneId], droneRect);
        }
    }
}

//重置事件
void DroneStatusWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    calculateLayout();
}

//绘制背景
void DroneStatusWidget::drawBackground(QPainter &painter)
{
    // 绘制背景
    painter.fillRect(rect(), m_backgroundColor);
    
    // 绘制边框
    painter.setPen(QPen(m_primaryColor, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
}

//绘制无人机状态
void DroneStatusWidget::drawDroneStatus(QPainter &painter, const DroneStatusInfo &drone, const QRect &rect)
{
    // 绘制无人机ID
    drawDroneId(painter, drone.id, rect);

    // 绘制血条
    drawHealthBar(painter, drone.hp, rect);

    // 绘制状态指示灯
    drawStatusLed(painter, drone.isOnline, rect);

    // 绘制坐标
    drawPosition(painter, drone, rect);
}

//绘制无人机ID
void DroneStatusWidget::drawDroneId(QPainter &painter, const QString &id, const QRect &rect)
{
    painter.setFont(m_idFont);
    painter.setPen(m_textColor);

    // 无人机编号：2x2表格左上角位置
    QRect idRect(rect.left() + m_margin, rect.top() + 6, 60, 15);
    painter.drawText(idRect, Qt::AlignLeft | Qt::AlignVCenter, id + ":");
}

//绘制血条
void DroneStatusWidget::drawHealthBar(QPainter &painter, int hp, const QRect &rect)
{
    // 血条位置：2x2表格右上角位置，增加血条长度
    QRect barRect(rect.left() + m_margin + 25, rect.top() + 8, 80, 12); // 从60增加到80

    // 绘制背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(60, 60, 60));
    painter.drawRoundedRect(barRect, 3, 3);

    // 绘制血量
    if (hp > 0) {
        QColor hpColor;
        if (hp > 60) {
            hpColor = QColor(39, 174, 96);  // 绿色
        } else if (hp > 30) {
            hpColor = QColor(241, 196, 15); // 黄色
        } else {
            hpColor = QColor(231, 76, 60);  // 红色
        }

        painter.setBrush(hpColor);
        int fillWidth = (barRect.width() - 2) * hp / 100;
        QRect fillRect(barRect.left() + 1, barRect.top() + 1, fillWidth, barRect.height() - 2);
        painter.drawRoundedRect(fillRect, 2, 2);
    }

    // 绘制血量文字
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));

    // 检查是否为红方且离线状态（表示不在探测范围内）
    // 通过检查当前绘制的rect来确定是哪个无人机
    QString droneId = "";
    for (int i = 0; i < m_droneOrder.size(); ++i) {
        QRect droneRect = getDroneRect(i);
        if (droneRect == rect) {
            droneId = m_droneOrder[i];
            break;
        }
    }

    //如果无人机是红方且血量为0且离线，则显示未知
    if (droneId.startsWith("R") && hp == 0 && m_drones.contains(droneId) && !m_drones[droneId].isOnline) {
        painter.drawText(barRect, Qt::AlignCenter, "未知");
    } else {
        painter.drawText(barRect, Qt::AlignCenter, QString("%1%").arg(hp));
    }
}

//绘制状态指示灯
void DroneStatusWidget::drawStatusLed(QPainter &painter, bool isOnline, const QRect &rect)
{
    // 状态指示灯位置：2x2表格左下角位置，与ID垂直对齐，减小指示灯大小
    QPoint ledPos(rect.left() + m_margin + 8, rect.top() + 36);

    // 选择颜色
    QColor ledColor = isOnline ? m_onlineColor : m_offlineColor;

    // 如果离线，添加闪烁效果
    if (!isOnline && m_blinkState) {
        ledColor = ledColor.lighter(150);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(ledColor);
    painter.drawEllipse(ledPos, 5, 5);  //绘制指示灯

    // 添加高光效果
    if (isOnline) {
        painter.setBrush(QColor(255, 255, 255, 100));
        painter.drawEllipse(ledPos.x() + 1, ledPos.y() + 1, 2, 2); // 高光
    }
}

//绘制坐标
void DroneStatusWidget::drawPosition(QPainter &painter, const DroneStatusInfo &drone, const QRect &rect)
{
    painter.setFont(m_posFont);
    painter.setPen(m_textColor);

    // 坐标位置：2x2表格右下角位置，减少列间距
    QRect posRect(rect.left() + m_margin + 25, rect.top() + 28, 120, 15);

    QString posText;

    // 检查是否为红方且处于未知状态（HP为0且离线）
    if (drone.id.startsWith("R") && drone.hp == 0 && !drone.isOnline) {
        posText = "Pos: Unknown";
    } else if (drone.position == QPoint(0, 0)) {
        posText = "Pos: Unknown";
    } else {
        posText = QString("Pos: (%1, %2)").arg(drone.position.x()).arg(drone.position.y());
    }

    painter.drawText(posRect, Qt::AlignLeft | Qt::AlignVCenter, posText);
}

//获取无人机矩形区域
QRect DroneStatusWidget::getDroneRect(int index) const
{
    int y = m_margin + index * (m_itemHeight + m_spacing);
    return QRect(0, y, width(), m_itemHeight);
}

//计算布局
void DroneStatusWidget::calculateLayout()
{
    // 根据控件大小调整布局参数
    int totalHeight = height() - 2 * m_margin;
    int availableHeight = totalHeight - (m_droneOrder.size() - 1) * m_spacing;

    if (m_droneOrder.size() > 0) {
        m_itemHeight = qMax(30, availableHeight / m_droneOrder.size());
    }

    // 设置最小高度
    int minHeight = m_droneOrder.size() * 35 + (m_droneOrder.size() - 1) * m_spacing + 2 * m_margin;
    setMinimumHeight(minHeight);
}

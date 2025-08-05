#ifndef DRONESTATUSWIDGET_H
#define DRONESTATUSWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QFont>
#include <QFontMetrics>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QMouseEvent>

struct DroneStatusInfo {
    QString id;           // 无人机ID (B1, B2, B3, R1, R2, R3)
    int hp;              // 血量 (0-100)
    QPoint position;     // 坐标位置
    bool isOnline;       // 是否在线
    QString team;        // 队伍 ("B" 或 "R")

    DroneStatusInfo() : hp(100), position(0, 0), isOnline(false) {}
};

class DroneStatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DroneStatusWidget(QWidget *parent = nullptr);
    
    // 设置无人机信息
    void updateDroneInfo(const QString &droneId, int hp, const QPoint &pos, bool online);
    void setTeam(const QString &team); // "B" 或 "R"
    
    // 设置显示的无人机列表
    void setDroneList(const QStringList &droneIds);
    
    // 设置颜色主题
    void setTeamColors(const QColor &primaryColor, const QColor &secondaryColor);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // 绘制函数
    void drawBackground(QPainter &painter);
    void drawDroneStatus(QPainter &painter, const DroneStatusInfo &drone, const QRect &rect);
    void drawHealthBar(QPainter &painter, int hp, const QRect &rect);
    void drawStatusLed(QPainter &painter, bool isOnline, const QRect &rect);
    void drawDroneId(QPainter &painter, const QString &id, const QRect &rect);
    void drawPosition(QPainter &painter, const DroneStatusInfo &drone, const QRect &rect);
    
    // 计算布局
    QRect getDroneRect(int index) const;
    void calculateLayout();
    
    // 数据成员
    QMap<QString, DroneStatusInfo> m_drones;
    QStringList m_droneOrder;
    QString m_team;
    
    // 颜色配置
    QColor m_primaryColor;      // 主色调
    QColor m_secondaryColor;    // 次色调
    QColor m_backgroundColor;   // 背景色
    QColor m_textColor;         // 文字颜色
    QColor m_onlineColor;       // 在线指示灯颜色
    QColor m_offlineColor;      // 离线指示灯颜色
    
    // 布局参数
    int m_itemHeight;           // 每个无人机项的高度
    int m_margin;               // 边距
    int m_spacing;              // 间距
    
    // 字体
    QFont m_idFont;             // ID字体
    QFont m_posFont;            // 坐标字体
    
    // 动画
    QTimer *m_blinkTimer;       // 闪烁定时器
    bool m_blinkState;          // 闪烁状态
};

#endif // DRONESTATUSWIDGET_H

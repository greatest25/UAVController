#include "droneselectdialog.h"

DroneSelectDialog::DroneSelectDialog(QWidget *parent)
    : QDialog(parent)
    , m_selectedDrone("B1")
    , m_buttonGroup(nullptr)
{
    setWindowTitle("选择操作无人机");
    setFixedSize(300, 200);
    setModal(true);
    
    setupUI();
    setupConnections();
}

void DroneSelectDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 标题
    QLabel *titleLabel = new QLabel("请选择要操作的无人机:", this);
    titleLabel->setFont(QFont("Arial", 12, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #2c3e50; margin: 10px;");
    
    // 无人机按钮区域
    QHBoxLayout *droneLayout = new QHBoxLayout();
    
    // 蓝队区域
    QVBoxLayout *blueLayout = new QVBoxLayout();
    QLabel *blueLabel = new QLabel("蓝队", this);
    blueLabel->setAlignment(Qt::AlignCenter);
    blueLabel->setStyleSheet("color: #3498db; font-weight: bold;");
    
    m_b1Button = createDroneButton("B1", "blue");
    m_b2Button = createDroneButton("B2", "blue");
    m_b3Button = createDroneButton("B3", "blue");
    
    blueLayout->addWidget(blueLabel);
    blueLayout->addWidget(m_b1Button);
    blueLayout->addWidget(m_b2Button);
    blueLayout->addWidget(m_b3Button);
    
    // 红队区域
    QVBoxLayout *redLayout = new QVBoxLayout();
    QLabel *redLabel = new QLabel("红队", this);
    redLabel->setAlignment(Qt::AlignCenter);
    redLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
    
    m_r1Button = createDroneButton("R1", "red");
    m_r2Button = createDroneButton("R2", "red");
    m_r3Button = createDroneButton("R3", "red");
    
    redLayout->addWidget(redLabel);
    redLayout->addWidget(m_r1Button);
    redLayout->addWidget(m_r2Button);
    redLayout->addWidget(m_r3Button);
    
    droneLayout->addLayout(blueLayout);
    droneLayout->addLayout(redLayout);
    
    // 按钮组
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(m_b1Button);
    m_buttonGroup->addButton(m_b2Button);
    m_buttonGroup->addButton(m_b3Button);
    m_buttonGroup->addButton(m_r1Button);
    m_buttonGroup->addButton(m_r2Button);
    m_buttonGroup->addButton(m_r3Button);
    
    // 确认取消按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("确认", this);
    m_cancelButton = new QPushButton("取消", this);
    
    m_okButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #27ae60;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px 16px;"
        "    border-radius: 4px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2ecc71;"
        "}"
    );
    
    m_cancelButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #95a5a6;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px 16px;"
        "    border-radius: 4px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #bdc3c7;"
        "}"
    );
    
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // 组装布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(droneLayout);
    mainLayout->addLayout(buttonLayout);
    
    // 设置默认选中B1
    m_b1Button->setChecked(true);
}

void DroneSelectDialog::setupConnections()
{
    connect(m_buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this, &DroneSelectDialog::onDroneButtonClicked);
    
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QPushButton* DroneSelectDialog::createDroneButton(const QString &droneId, const QString &team)
{
    QPushButton *button = new QPushButton(droneId, this);
    button->setCheckable(true);
    button->setFixedSize(60, 30);
    
    QString color = (team == "blue") ? "#3498db" : "#e74c3c";
    QString hoverColor = (team == "blue") ? "#5dade2" : "#ec7063";
    
    button->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: 2px solid %1;"
        "    border-radius: 4px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "    border-color: %2;"
        "}"
        "QPushButton:checked {"
        "    background-color: white;"
        "    color: %1;"
        "    border: 2px solid %1;"
        "}"
    ).arg(color, hoverColor));
    
    return button;
}

void DroneSelectDialog::onDroneButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(m_buttonGroup->checkedButton());
    if (button) {
        m_selectedDrone = button->text();
    }
}

void DroneSelectDialog::setCurrentDrone(const QString &droneId)
{
    m_selectedDrone = droneId;
    
    // 设置对应按钮为选中状态
    QList<QAbstractButton*> buttons = m_buttonGroup->buttons();
    for (QAbstractButton *button : buttons) {
        if (button->text() == droneId) {
            button->setChecked(true);
            break;
        }
    }
}

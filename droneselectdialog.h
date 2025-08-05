#ifndef DRONESELECTDIALOG_H
#define DRONESELECTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QFont>
#include <QColor>

class DroneSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DroneSelectDialog(QWidget *parent = nullptr);
    
    QString getSelectedDrone() const { return m_selectedDrone; }
    void setCurrentDrone(const QString &droneId);

private slots:
    void onDroneButtonClicked();

private:
    void setupUI();
    void setupConnections();
    QPushButton* createDroneButton(const QString &droneId, const QString &team);
    
    QString m_selectedDrone;
    QButtonGroup *m_buttonGroup;
    
    // 按钮
    QPushButton *m_b1Button;
    QPushButton *m_b2Button;
    QPushButton *m_b3Button;
    QPushButton *m_r1Button;
    QPushButton *m_r2Button;
    QPushButton *m_r3Button;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // DRONESELECTDIALOG_H

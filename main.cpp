#include "customwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序信息
    a.setApplicationName("UAV Controller");
    a.setApplicationVersion("3.0");
    a.setApplicationDisplayName("无人机控制台");

    qDebug() << "启动无人机控制台 - 自定义UI模式";

    // 创建并显示自定义UI窗口
    CustomWindow w;
    w.show();

    return a.exec();
}

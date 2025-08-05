QT       += core gui vsoa

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    radarwidget.cpp \
    compasswidget.cpp \
    joystickwidget.cpp \
    dronestatuswidget.cpp \
    customdashboard.cpp \
    customwindow.cpp

HEADERS += \
    radarwidget.h \
    compasswidget.h \
    joystickwidget.h \
    dronestatuswidget.h \
    customdashboard.h \
    customwindow.h

# FORMS += \
#     mainwindow.ui
# 注释掉UI文件，现在使用完全自定义绘制的UI

# To use Qwt in your project, enable the USE_QWT=1.
#USE_QWT = 1
!isEmpty(USE_QWT) {
    LIBS += -L$$[QT_INSTALL_PREFIX]\lib -lqwt
    INCLUDEPATH += $$[QT_INSTALL_PREFIX]\include\Qwt
}

# Default rules for deployment.
sylixos: target.path = /apps/$${TARGET}
else: qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

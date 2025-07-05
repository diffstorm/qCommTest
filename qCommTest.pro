# qCommTest - Serial Communication Test Tool
QT += core gui network serialport widgets

# Add support for Qt6
QT_MAJOR_VERSION = $str_member($split(QT_VERSION, .), 0)

if(equals(QT_MAJOR_VERSION, 6)) {
    QT += serialport
}


TARGET = qCommTest
TEMPLATE = app

# Enable warnings for deprecated Qt features
DEFINES += QT_DEPRECATED_WARNINGS

# Uncomment to disable deprecated APIs up to a specific Qt version
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    main.cpp \
    tcp_server.cpp \
    mainwindow.cpp \
    serial_port.cpp

HEADERS += \
    tcp_server.h \
    mainwindow.h \
    serial_port.h

FORMS += \
    mainwindow.ui

RESOURCES += qdarkstyle/style.qrc

win32:RC_ICONS += qdarkstyle/rc/icon.ico


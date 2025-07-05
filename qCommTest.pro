# qCommTest - Serial Communication Test Tool
QT += core gui network serialport widgets

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

SOURCES +=     src/main.cpp     src/tcp_server.cpp     src/mainwindow.cpp     src/serial_port.cpp

HEADERS +=     src/tcp_server.h     src/mainwindow.h     src/serial_port.h

FORMS +=     src/mainwindow.ui

RESOURCES += src/qdarkstyle/style.qrc

win32:RC_ICONS += qdarkstyle/rc/icon.ico


/*
 * qCommTest - Serial Communication Test Tool
 * Version 	: 1.0
 * Date 	: 20.11.2017
 * Author	: Eray Ozturk  | github.com/diffstorm
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tcp_server.h"
#include "serial_port.h"

namespace Ui
{
class MainWindow;
}

enum class Channel_t
{
    TCP,
    Serial
};
enum class Test_Step_t
{
    step_Idle, // Wait for first packet from device
    step_Test, // Test
};
enum class Icon_t
{
    Disconnected,
    Connecting,
    Testing,
    TestSuccess,
    TestFailed
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Test_Step_t     m_testStep;
    qint32          m_testIndex;
    QPoint          m_dragPosition;
    bool            m_testStarted;
    qint64          m_data_size = 0;
    qint64          m_testStartAt;
    qint64          m_testFinishAt;
    qint64          m_testElapsedTime;

    Ui::MainWindow  *ui;
    TcpServer       *m_tcpServer;
    serial_port     *m_serialPort;
    QAction         *usageAction;
    QAction         *aboutAction;
    QAction         *quitAction;
    QMovie          *movie = nullptr;
    QTimer          m_timer_test;

    void Form_Init();
    void Log(const QString &);

    void TcpServer_Init();
    bool TcpServer_Start(int);
    void TcpServer_Stop();

    void SerialPort_Init();
    void SerialPort_Refresh();
    bool SerialPort_Start();
    void SerialPort_Stop();
    void SerialPort_SetEnabled(bool);

    void Clean_Counters();
    void Inc_RX();
    void Inc_TX();
    void Inc_Error();
    void SetTestStarted(bool);
    bool Send(Channel_t, QByteArray);
    qint64 PacketTimeout(Channel_t);
    qint64 ElapsedTime(Channel_t);
    QByteArray Protocol_Wrap(QByteArray);
    QByteArray Protocol_Unwrap(QByteArray);
    void Test(Channel_t, QByteArray);
    quint32 crc32(const char *, quint16);

    void SetMoodIcon(Icon_t);
    void PrintResults();

private slots:
    void About();
    void Usage();

    void onLogMessage(const QString &log);

    void onTcpClientDisconnected();
    void onTcpClientConnected();
    void onTcpDataReceivedFromClient();

    void onSerialDataReceived();

    void onTimeoutTest();

    void on_tabWidget_currentChanged(int);
    void on_serial_open_clicked();
    void on_tcp_listen_clicked();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

};

#endif // MAINWINDOW_H

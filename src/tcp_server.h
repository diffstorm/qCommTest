/*
 * qCommTest - Serial Communication Test Tool
 * Version 	: 1.0
 * Date 	: 20.11.2017
 * Author	: Eray Ozturk  | github.com/diffstorm
*/
#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <QObject>
#include <QDebug>
#include <QtNetwork>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtCore>

class TcpServer: public QObject
{
    Q_OBJECT

public:
    TcpServer();
    ~TcpServer();

    QString getClientIp() const;
    void setClientIp(const QString &value);
    int getServerPort() const;
    bool startServer(const int &value);
    bool isListenning();
    bool isClientConnected() const;
    bool Write(const QByteArray &writeData);
    QByteArray Read();
    void stopServer();
    qint64 getTimeout(qint64 data_size);
    qint64 getReceivedTime();
    qint64 getSentTime();

signals:
    void dataReceived();
    void clientConnected();
    void clientDisconnected();
    void logMessage(const QString &);

public slots:
    void dataReceivedSlot();
    void onNewClientConnection();
    void onClientConnectedSlot();
    void onClientDisconnectedSlot();
    void onSocketErrorOccured(QAbstractSocket::SocketError err);
    void onBytesWritten(qint64 bytes);
    void onTimeoutTX();
    void onTimeoutRX();

private:
    void Log(const QString &log);
    void waitForBytesWritten();

    QTcpServer     *m_tcpServer = nullptr;
    QTcpSocket     *m_socket = nullptr;
    QString         m_clientIp;
    int             m_serverPort;
    qint64          m_dataReceivedAt;
    qint64          m_dataSentAt;
    bool            m_clientConnected;
    QByteArray      m_readData;
    QByteArray      m_writeData;
    qint64          m_bytesWritten = 0;
    QTimer          m_timer_tx;
    QTimer          m_timer_rx;
};

#endif // TCP_SERVER_H

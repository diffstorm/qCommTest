/*
    qCommTest - Serial Communication Test Tool
    Version  : 1.0
    Date     : 20.11.2017
    Author   : Eray Ozturk  | github.com/diffstorm
*/
#include "tcp_server.h"

// TODO : Adaptive timeout determined by network latency measurement
#define TCP_TIMEOUT(x) (5 + x/500) // [ms]

TcpServer::TcpServer()
{
    m_socket = nullptr;
    m_tcpServer = new QTcpServer(this);
    m_tcpServer->setMaxPendingConnections(1);
    m_clientIp = "";
    m_clientConnected = false;
    connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::onNewClientConnection);
    connect(&m_timer_rx, &QTimer::timeout, this, &TcpServer::onTimeoutRX);
    connect(&m_timer_tx, &QTimer::timeout, this, &TcpServer::onTimeoutTX);
    m_timer_tx.setSingleShot(true);
    m_timer_tx.stop();
    m_timer_rx.setSingleShot(true);
    m_timer_rx.stop();
    m_dataReceivedAt = 0;
    m_dataSentAt = 0;
}

TcpServer::~TcpServer()
{
    stopServer();
    delete m_socket;
    delete m_tcpServer;
}

void TcpServer::Log(const QString &log)
{
    emit logMessage("TCP Server : " + log);
}

qint64 TcpServer::getReceivedTime()
{
    return m_dataReceivedAt;
}

qint64 TcpServer::getSentTime()
{
    return m_dataSentAt;
}

qint64 TcpServer::getTimeout(qint64 data_size)
{
    return TCP_TIMEOUT(data_size);
}

QString TcpServer::getClientIp() const
{
    return m_clientIp;
}

void TcpServer::setClientIp(const QString &value)
{
    m_clientIp = value;
}

int TcpServer::getServerPort() const
{
    return m_serverPort;
}

bool TcpServer::startServer(const int &value)
{
    bool ret;
    m_serverPort = value;

    if(m_tcpServer->isListening())
    {
        stopServer();
    }

    ret = m_tcpServer->listen(QHostAddress::Any, m_serverPort);

    if(false != ret)
    {
        Log(QString("Started listening on port %1").arg(m_serverPort));
    }
    else
    {
        Log("Start failed, error:" + m_tcpServer->errorString());
    }

    return ret;
}

void TcpServer::stopServer()
{
    if(m_tcpServer->isListening() && nullptr != m_socket)
    {
        waitForBytesWritten();
    }

    m_tcpServer->close();

    if(m_socket != nullptr)
    {
        m_socket->close();
    }

    m_timer_rx.stop();
    m_timer_tx.stop();
    Log("Stopped");
}

void TcpServer::waitForBytesWritten()
{
    qint64 bytesToWrite = m_socket->bytesToWrite();

    if(bytesToWrite)
    {
        qDebug() << "Busy! Waiting" << TCP_TIMEOUT(bytesToWrite) << "ms to write" << bytesToWrite << "bytes";
        m_socket->waitForBytesWritten(TCP_TIMEOUT(bytesToWrite));
    }
}

bool TcpServer::isListenning()
{
    return m_tcpServer->isListening();
}

bool TcpServer::isClientConnected() const
{
    return m_clientConnected;
}

void TcpServer::onNewClientConnection()
{
    if(nullptr != m_socket)
    {
        m_socket->close();
        delete m_socket;
    }

    m_socket = m_tcpServer->nextPendingConnection();
    //m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    m_socket->flush();
    m_socket->setReadBufferSize(4096);

    if(m_socket->isValid())
    {
        onClientConnectedSlot();
    }

    connect(m_socket, &QTcpSocket::connected, this, &TcpServer::onClientConnectedSlot);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnectedSlot);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpServer::onSocketErrorOccured);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpServer::dataReceivedSlot);
    connect(m_socket, &QTcpSocket::bytesWritten, this, &TcpServer::onBytesWritten);
    setClientIp(m_socket->localAddress().toString());
}

void TcpServer::onClientConnectedSlot()
{
    Log("Client connected");
    m_clientConnected = true;
    emit clientConnected();
}

void TcpServer::onClientDisconnectedSlot()
{
    Log("Client disconnected");
    m_clientConnected = false;
    emit clientDisconnected();
}

bool TcpServer::Write(const QByteArray &writeData)
{
    bool ret = false;

    if(!m_socket->isValid())
    {
        qDebug() << "Invalid socket to send";
    }
    else
    {
        waitForBytesWritten();

        if(m_socket->state() == QTcpSocket::ConnectedState &&
                m_socket->isWritable())
        {
            m_writeData.clear();
            m_writeData = writeData;
            m_dataSentAt = QDateTime::currentMSecsSinceEpoch();
            qDebug() << "Bytes to write :" << writeData.size();
            qint64 bytesWritten = m_socket->write(writeData);

            if(bytesWritten == -1)
            {
                qDebug() << "Failed to write the data -" << "error:" << m_socket->errorString();
            }
            else if(bytesWritten != m_writeData.size())
            {
                qDebug() << "Failed to write all the data -" << "error:" << m_socket->errorString();
            }
            else if(bytesWritten == m_writeData.size())
            {
                ret = true;
                qDebug() << "Buffer write successful";
            }

            qDebug() << "Remaining bytes to write :" << m_socket->bytesToWrite();
            qDebug() << "Write timeout is set to" << TCP_TIMEOUT(m_writeData.size()) << "ms to write" << m_writeData.size() << "bytes";
            m_timer_tx.start(TCP_TIMEOUT(m_writeData.size()));
        }
        else
        {
            qDebug() << "Tcp connection is not active";
        }
    }

    return ret;
}

QByteArray TcpServer::Read()
{
    QByteArray readData;

    if(m_socket->isReadable())
    {
        qint64 bytesAvailable = m_socket->bytesAvailable();

        if(bytesAvailable > 0)
        {
            qDebug() << "Busy! Waiting" << TCP_TIMEOUT(bytesAvailable) << "ms to read" << bytesAvailable << "bytes";
            m_socket->waitForReadyRead(TCP_TIMEOUT(bytesAvailable));
        }
    }
    else
    {
        qDebug() << "Socket is not readable";
    }

    readData = m_readData;
    m_readData.clear();
    Log(QString("Read %1 bytes").arg(readData.size()));
    return readData;
}

void TcpServer::dataReceivedSlot()
{
    qDebug() << "Data received.";

    if(!m_socket->isValid())
    {
        qDebug() << "Invalid socket to read";
        return;
    }

    if(m_socket->bytesAvailable() > 0 &&
            m_socket->isReadable())
    {
        qint64 bytesAvailable = m_socket->bytesAvailable();
        qDebug() << bytesAvailable << "bytes are available to read";

        if(bytesAvailable > 0)
        {
            m_readData.append(m_socket->readAll());
            m_dataReceivedAt = QDateTime::currentMSecsSinceEpoch();
        }

        qDebug() << "Read timeout is set to" << TCP_TIMEOUT(bytesAvailable) << "ms to read" << bytesAvailable << "bytes";
        m_timer_rx.start(TCP_TIMEOUT(bytesAvailable));
    }
    else
    {
        qDebug() << "Tcp connection is not active";
    }
}

void TcpServer::onBytesWritten(qint64 bytes)
{
    m_bytesWritten += bytes;

    if(m_bytesWritten == m_writeData.size())
    {
        m_timer_tx.stop();
        m_bytesWritten = 0;
        qDebug() << "Data successfully sent";
        Log(QString("Written %1 bytes").arg(m_writeData.size()));
        //qDebug() << QString("TX:" + m_writeData.toHex());
    }
    else
    {
        qint64 bytesToWrite = m_writeData.size() - m_bytesWritten;
        qDebug() << "Written" << m_bytesWritten << "/" << m_writeData.size();
        qDebug() << "Write timeout is set to" << TCP_TIMEOUT(bytesToWrite) << "ms to write" << bytesToWrite << "bytes";
        m_timer_tx.start(TCP_TIMEOUT(bytesToWrite));
    }
}

void TcpServer::onTimeoutTX()
{
    if(m_bytesWritten != m_writeData.size())
    {
        Log("Write operation timed out");
        qDebug() << "Write operation timed out";
        qDebug() << "error:" << m_socket->errorString();
    }
    else
    {
        qDebug() << "Data sent succeeded but timeout occurred";
    }
}

void TcpServer::onTimeoutRX()
{
    if(m_readData.isEmpty())
    {
        qDebug() << "No data was currently available for reading";
    }
    else
    {
        qDebug() << "Data successfully received";
        qDebug() << m_readData.length() << "bytes received.";
        //qDebug() << QString("RX:" + m_readData.toHex());
        emit dataReceived();
    }
}

void TcpServer::onSocketErrorOccured(QAbstractSocket::SocketError socketError)
{
    switch(socketError)
    {
        case QAbstractSocket::RemoteHostClosedError:
        {
            qint64 diff = QDateTime::currentMSecsSinceEpoch() - m_dataSentAt;

            if(200 < diff) // 100 ms is signal time
            {
                qDebug() << "The remote host closed the connection";
            }
        }
        break;

        case QAbstractSocket::ConnectionRefusedError:
            qDebug() << "TCP Connection refused";
            break;

        default:
            qCritical() << "TCP socket error:" << socketError << "-" << m_socket->errorString();
            break;
    }
}

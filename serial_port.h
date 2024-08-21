/*
 * qCommTest - Serial Communication Test Tool
 * Version 	: 1.0
 * Date 	: 20.11.2017
 * Author	: Eray Ozturk  | github.com/diffstorm
*/
#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <QDebug>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QByteArray>
#include <QTimer>
#include <QtCore>

class serial_port : public QObject
{
    Q_OBJECT
public:
    explicit serial_port(QObject *parent = nullptr);
    ~serial_port();
    QList<QString> Scan();
    void Configure(qint32 rate, QSerialPort::DataBits bits, QSerialPort::FlowControl flow, QSerialPort::Parity parity, QSerialPort::StopBits stopBits);
    bool Open(QString);
    bool isOpen();
    bool Write(const QByteArray &);
    QByteArray Read();
    void Close();
    qint64 getBaudTimeout(qint64);
    qint64 getReceivedTime();
    qint64 getSentTime();

signals:
    void dataReceived();
    void logMessage(const QString &);

public slots:

private slots:
    void onBytesWritten(qint64);
    void onReadyRead();
    void onTimeoutTX();
    void onTimeoutRX();
    void onError(QSerialPort::SerialPortError);

private:
    void Log(const QString &);
    void setBaudTimeout(qint64, qint8);
    void waitForBytesWritten();

    QSerialPort     *m_serialPort = nullptr;
    QByteArray      m_readData;
    QByteArray      m_writeData;
    qint64          m_bytesWritten = 0;
    QTimer          m_timer_tx;
    QTimer          m_timer_rx;
    qint64          m_dataReceivedAt;
    qint64          m_dataSentAt;
    qint64          m_baudtimeout;
};

#endif // SERIAL_PORT_H

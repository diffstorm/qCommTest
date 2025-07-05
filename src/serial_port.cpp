/*
    qCommTest - Serial Communication Test Tool
    Version  : 1.0
    Date     : 20.11.2017
    Author   : Eray Ozturk  | github.com/diffstorm
*/
#include "serial_port.h"

serial_port::serial_port(QObject *parent) : QObject(parent)
{
    m_serialPort = new QSerialPort(this);
    connect(m_serialPort, &QSerialPort::bytesWritten, this, &serial_port::onBytesWritten);
    connect(m_serialPort, &QSerialPort::readyRead, this, &serial_port::onReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &serial_port::onError);
    connect(&m_timer_rx, &QTimer::timeout, this, &serial_port::onTimeoutRX);
    connect(&m_timer_tx, &QTimer::timeout, this, &serial_port::onTimeoutTX);
    m_timer_tx.setSingleShot(true);
    m_timer_tx.stop();
    m_timer_rx.setSingleShot(true);
    m_timer_rx.stop();
    m_dataReceivedAt = 0;
    m_dataSentAt = 0;
}

serial_port::~serial_port()
{
    Close();
    delete m_serialPort;
}

void serial_port::Log(const QString &log)
{
    emit logMessage("COM Port : " + log);
}

qint64 serial_port::getReceivedTime()
{
    return m_dataReceivedAt;
}

qint64 serial_port::getSentTime()
{
    return m_dataSentAt;
}

QList<QString> serial_port::Scan()
{
    QList<QString> ports;

    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ports.append(info.portName());
        /*
            QString s = QObject::tr("Port:") + info.portName() + "\n"
                    + QObject::tr("Location:") + info.systemLocation() + "\n"
                    + QObject::tr("Description:") + info.description() + "\n"
                    + QObject::tr("Manufacturer:") + info.manufacturer() + "\n"
                    //+ QObject::tr("Serial number:") + info.serialNumber() + "\n"
                    + QObject::tr("Vendor Identifier:") + (info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Product Identifier:") + (info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString()) + "\n"
                    //+ QObject::tr("Busy:") + (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) + "\n"
                    ;
            qDebug() << s;
        */
    }

    return ports;
}

bool serial_port::Open(QString name)
{
    bool ret = false;

    if(m_serialPort->isOpen())
    {
        Close();
    }

    Log(QString("%1").arg(name));
    m_serialPort->setPortName(name);
    m_serialPort->setReadBufferSize(4096);

    if(m_serialPort->open(QIODevice::ReadWrite))
    {
        Log("Open");
        ret = true;
    }
    else
    {
        Log("Open error");
    }

    return ret;
}

bool serial_port::isOpen()
{
    return m_serialPort->isOpen();
}

void serial_port::Configure(qint32 rate, QSerialPort::DataBits bits, QSerialPort::FlowControl flow, QSerialPort::Parity parity, QSerialPort::StopBits stopBits)
{
    if(!m_serialPort->setBaudRate(rate))
    {
        Log("Baud rate error :" + m_serialPort->errorString());
    }

    if(!m_serialPort->setDataBits(bits))
    {
        Log("Data bits error :" + m_serialPort->errorString());
    }

    if(!m_serialPort->setFlowControl(flow))
    {
        Log("Flow control error :" + m_serialPort->errorString());
    }

    if(!m_serialPort->setParity(parity))
    {
        Log("Parity error :" + m_serialPort->errorString());
    }

    if(!m_serialPort->setStopBits(stopBits))
    {
        Log("Stop bits error :" + m_serialPort->errorString());
    }

    if(!m_serialPort->setDataTerminalReady(false))
    {
        Log("DTR error :" + m_serialPort->errorString());
    }

    // TODO : calculate the actual frame size
    setBaudTimeout(rate, 12); // Assume that a frame is 12 bits long as worst case
}

void serial_port::setBaudTimeout(qint64 baud_rate, qint8 frame_size)
{
    m_baudtimeout = (4 * 1000) / (baud_rate / frame_size);
    qDebug() << "Baud timeout is calculated as" << (1 + m_baudtimeout) << "ms";
}

qint64 serial_port::getBaudTimeout(qint64 data_size)
{
    return (1 + (m_baudtimeout * data_size));
}

void serial_port::waitForBytesWritten()
{
    qint64 bytesToWrite = m_serialPort->bytesToWrite();

    if(bytesToWrite)
    {
        qDebug() << "Busy! Waiting" << getBaudTimeout(bytesToWrite) << "ms to write" << bytesToWrite << "bytes";
        m_serialPort->waitForBytesWritten(getBaudTimeout(bytesToWrite));
    }
}

void serial_port::Close()
{
    if(m_serialPort->isOpen())
    {
        waitForBytesWritten();
        m_serialPort->close();
        Log("Closed");
    }

    m_timer_rx.stop();
    m_timer_tx.stop();
}

bool serial_port::Write(const QByteArray &writeData)
{
    bool ret = true;
    waitForBytesWritten();

    if(m_serialPort->isWritable())
    {
        m_writeData.clear();
        m_writeData = writeData;
        m_dataSentAt = QDateTime::currentMSecsSinceEpoch();
        qDebug() << "Bytes to write :" << writeData.size();
        qint64 bytesWritten = m_serialPort->write(writeData);

        if(bytesWritten == -1)
        {
            ret = false;
            qDebug() << "Failed to write the data to port" << m_serialPort->portName() << "error:" << m_serialPort->errorString();
        }
        else if(bytesWritten != m_writeData.size())
        {
            ret = false;
            qDebug() << "Failed to write all the data to port" << m_serialPort->portName() << "error:" << m_serialPort->errorString();
        }
        else if(bytesWritten == m_writeData.size())
        {
            qDebug() << "Buffer write successful to port" << m_serialPort->portName();
        }

        qDebug() << "Remaining bytes to write :" << m_serialPort->bytesToWrite();
        qDebug() << "Write timeout is set to" << getBaudTimeout(m_writeData.size()) << "ms to write" << m_writeData.size() << "bytes";
        m_timer_tx.start(getBaudTimeout(m_writeData.size()));
    }
    else
    {
        ret = false;
        qDebug() << "Serial port is not writable";
    }

    return ret;
}

QByteArray serial_port::Read()
{
    QByteArray readData;

    if(m_serialPort->isReadable())
    {
        qint64 bytesAvailable = m_serialPort->bytesAvailable();

        if(bytesAvailable > 0)
        {
            qDebug() << "Busy! Waiting" << getBaudTimeout(bytesAvailable) << "ms to read" << bytesAvailable << "bytes";
            m_serialPort->waitForReadyRead(getBaudTimeout(bytesAvailable));
        }
    }
    else
    {
        qDebug() << "Serial port is not readable";
    }

    readData = m_readData;
    m_readData.clear();
    Log(QString("Read %1 bytes").arg(readData.size()));
    return readData;
}

void serial_port::onReadyRead()
{
    if(m_serialPort->isReadable())
    {
        qint64 bytesAvailable = m_serialPort->bytesAvailable();
        qDebug() << bytesAvailable << "bytes are available to read";

        if(bytesAvailable > 0)
        {
            m_readData.append(m_serialPort->readAll());
            m_dataReceivedAt = QDateTime::currentMSecsSinceEpoch();
        }

        qDebug() << "Read timeout is set to" << getBaudTimeout(bytesAvailable) << "ms to read" << bytesAvailable << "bytes";
        m_timer_rx.start(getBaudTimeout(bytesAvailable));
    }
    else
    {
        qDebug() << "Serial port is not readable";
    }
}

void serial_port::onBytesWritten(qint64 bytes)
{
    m_bytesWritten += bytes;

    if(m_bytesWritten == m_writeData.size())
    {
        m_timer_tx.stop();
        m_bytesWritten = 0;
        qDebug() << "Data successfully sent to port" << m_serialPort->portName();
        Log(QString("Written %1 bytes").arg(m_writeData.size()));
        //qDebug() << QString("TX:" + m_writeData.toHex());
    }
    else
    {
        qint64 bytesToWrite = m_writeData.size() - m_bytesWritten;
        qDebug() << "Written" << m_bytesWritten << "/" << m_writeData.size();
        qDebug() << "Write timeout is set to" << getBaudTimeout(bytesToWrite) << "ms to write" << bytesToWrite << "bytes";
        m_timer_tx.start(getBaudTimeout(bytesToWrite));
    }
}

void serial_port::onTimeoutTX()
{
    if(m_bytesWritten != m_writeData.size())
    {
        Log("Write operation timed out");
        qDebug() << "Write operation timed out for port" << m_serialPort->portName();
    }
    else
    {
        qDebug() << "Data sent succeeded but timeout occurred";
    }

    if(m_serialPort->error() != QSerialPort::NoError)
    {
        qDebug() << "error:" << m_serialPort->errorString();
    }
}

void serial_port::onTimeoutRX()
{
    if(m_readData.isEmpty())
    {
        qDebug() << "No data was currently available for reading from port" << m_serialPort->portName();
    }
    else
    {
        qDebug() << "Data successfully received from port" << m_serialPort->portName();
        qDebug() << m_readData.length() << "bytes received.";
        //qDebug() << QString("RX:" + m_readData.toHex());
        emit dataReceived();
    }
}

void serial_port::onError(QSerialPort::SerialPortError serialPortError)
{
    switch(serialPortError)
    {
        case QSerialPort::NoError:
            // ignore
            break;

        case QSerialPort::WriteError:
            qDebug() << "An I/O error occurred while writing data to port" << m_serialPort->portName() << "error:" << m_serialPort->errorString();
            break;

        case QSerialPort::ReadError:
            qDebug() << "An I/O error occurred while reading data from port" << m_serialPort->portName() << "error:" << m_serialPort->errorString();
            break;

        default:
            qCritical() << "Serial Port error:" << m_serialPort->errorString();
            break;
    }
}

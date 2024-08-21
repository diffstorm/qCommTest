/*
    qCommTest - Serial Communication Test Tool
    Version  : 1.0
    Date     : 20.11.2017
    Author   : Eray Ozturk  | github.com/diffstorm
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tcp_server.h"
#include "serial_port.h"
#include <QtWidgets>

const char *LOGO                = ":/qss_icons/rc/logo.png";

const char *MOOD_DISCONNECTED   = ":/qss_icons/rc/mood/mood_disconnected.png";
const char *MOOD_CONNECTING     = ":/qss_icons/rc/mood/mood_connecting.gif";
const char *MOOD_TESTING        = ":/qss_icons/rc/mood/mood_testing.gif";
const char *MOOD_RESULT_SUCCESS = ":/qss_icons/rc/mood/mood_result_success.gif";
const char *MOOD_RESULT_FAIL    = ":/qss_icons/rc/mood/mood_result_error.gif";

const int TEST_INDEX_MAX        = 400;  // This must be changed in test code too
const int TEST_FRAME_TIMEOUT    = 500;  // [ms]

const int PROTOCOL_OVERHEAD     = 7;    // [bytes]

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    Form_Init();
    TcpServer_Init();
    SerialPort_Init();
    connect(&m_timer_test, &QTimer::timeout, this, &MainWindow::onTimeoutTest);
    m_timer_test.setSingleShot(true);
    m_timer_test.stop();
    ui->test_data_size->clear();
    m_testStartAt = 0;
    m_testFinishAt = 0;
    SetTestStarted(false);
    SetMoodIcon(Icon_t::Disconnected);
}

void MainWindow::Form_Init()
{
    // Fixed size, Frameless & borderless window
    setWindowFlags(Qt::Widget | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
    // Right click menu
    usageAction = new QAction(tr("U&sage"), this);
    connect(usageAction, &QAction::triggered, this, &MainWindow::Usage, Qt::UniqueConnection);
    addAction(usageAction);
    aboutAction = new QAction(tr("A&bout"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::About, Qt::UniqueConnection);
    addAction(aboutAction);
    quitAction = new QAction(tr("E&xit"), this);
    quitAction->setShortcut(tr("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    addAction(quitAction);
    setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->setupUi(this);
    ui->rx_progress->setMaximum(TEST_INDEX_MAX);
    ui->tx_progress->setMaximum(TEST_INDEX_MAX);
    ui->error_progress->setMaximum(TEST_INDEX_MAX);
    connect(ui->closebutton, &QPushButton::clicked, this, &QWidget::close);
}

void MainWindow::Usage()
{
    QMessageBox msgBox;
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText("<p><font size=18>Usage</font></p>"
                   "<p><ol>"
                   "<li>Configure and start the communication port</li>"
                   "<li>Run the test code on device</li>"
                   "<li>Observe Log section in case of error</li>"
                   "</ol>"
                   "</p>");
    msgBox.setIconPixmap(QPixmap(LOGO));
    msgBox.exec();
}

void MainWindow::About()
{
    QMessageBox msgBox;
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText("<p><font size=18>qCommTest</font></p>"
                   "<p>Serial Communication Test Tool on Qt</p>"
                   "<p>By Eray Öztürk | erayozturk1@gmail.com</p>"
                   "<p><a href=\"http://github.com/diffstorm\"><font color=white>github.com/diffstorm</font></a></p>");
    msgBox.setIconPixmap(QPixmap(LOGO));
    msgBox.exec();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

MainWindow::~MainWindow()
{
    delete usageAction;
    delete aboutAction;
    delete quitAction;
    delete movie;
    delete m_tcpServer;
    delete m_serialPort;
    delete ui;
}

void MainWindow::onLogMessage(const QString &log)
{
    qDebug() << log;
    ui->log_textedit->append(log);
    ui->log_textedit->verticalScrollBar()->setValue(ui->log_textedit->verticalScrollBar()->maximum());
}

void MainWindow::Log(const QString &log)
{
    onLogMessage("Test : " + log);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch(index)
    {
        case 0: // Serial Port
            SerialPort_Refresh();
            break;

        case 1: // TCP Server
            break;

        default:
            break;
    }
}

//---------------------------------------------------------------
void MainWindow::SerialPort_Init()
{
    m_serialPort = new serial_port(this);
    connect(m_serialPort, &serial_port::dataReceived, this, &MainWindow::onSerialDataReceived);
    connect(m_serialPort, &serial_port::logMessage, this, &MainWindow::onLogMessage);
    SerialPort_Refresh();
    ui->baud_rate->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baud_rate->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baud_rate->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baud_rate->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    ui->baud_rate->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baud_rate->setCurrentIndex(4);
    ui->data_bits->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->data_bits->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->data_bits->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->data_bits->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->data_bits->setCurrentIndex(3);
    ui->parity->addItem(tr("None"), QSerialPort::NoParity);
    ui->parity->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parity->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parity->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parity->addItem(tr("Space"), QSerialPort::SpaceParity);
    ui->stop_bits->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stop_bits->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stop_bits->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    ui->flow_control->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->flow_control->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flow_control->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
    SerialPort_SetEnabled(true);
}

void MainWindow::SerialPort_Refresh()
{
    auto list = m_serialPort->Scan();
    ui->serial_port_name->clear();

    if(list.size())
    {
        ui->serial_port_name->addItems(list);
    }
    else
    {
        ui->serial_status->setText("No ports found!");
    }
}

bool MainWindow::SerialPort_Start()
{
    bool ret = false;
    QString selected_port = ui->serial_port_name->currentText();

    if(m_serialPort->Open(selected_port))
    {
        m_serialPort->Configure(static_cast<QSerialPort::BaudRate>(ui->baud_rate->itemData(ui->baud_rate->currentIndex()).toInt()),
                                static_cast<QSerialPort::DataBits>(ui->data_bits->itemData(ui->data_bits->currentIndex()).toInt()),
                                static_cast<QSerialPort::FlowControl>(ui->flow_control->itemData(ui->flow_control->currentIndex()).toInt()),
                                static_cast<QSerialPort::Parity>(ui->parity->itemData(ui->parity->currentIndex()).toInt()),
                                static_cast<QSerialPort::StopBits>(ui->stop_bits->itemData(ui->stop_bits->currentIndex()).toInt()));
        ret = true;
    }
    else
    {
        ui->serial_status->setText("Failed to open " + selected_port);
    }

    return ret;
}

void MainWindow::SerialPort_Stop()
{
    m_serialPort->Close();
}

void MainWindow::onSerialDataReceived()
{
    QByteArray dataBuffer;
    dataBuffer = m_serialPort->Read();
    Test(Channel_t::Serial, dataBuffer);
}

void MainWindow::SerialPort_SetEnabled(bool enabled)
{
    ui->serial_port_name->setEnabled(enabled);
    ui->baud_rate->setEnabled(enabled);
    ui->data_bits->setEnabled(enabled);
    ui->parity->setEnabled(enabled);
    ui->stop_bits->setEnabled(enabled);
    ui->flow_control->setEnabled(enabled);
}

void MainWindow::on_serial_open_clicked()
{
    if(m_serialPort->isOpen())
    {
        SerialPort_Stop();
        ui->serial_status->setText("Serial port is closed");
    }
    else
    {
        if(false == SerialPort_Start())
        {
            ui->serial_status->setText("Serial port open failed");
        }
        else
        {
            ui->serial_status->setText("Serial port is open");
        }
    }

    if(m_serialPort->isOpen())
    {
        SerialPort_SetEnabled(false);
        ui->serial_open->setText("Close");
        SetMoodIcon(Icon_t::Connecting);
    }
    else
    {
        SerialPort_SetEnabled(true);
        ui->serial_open->setText("Open");
        SetMoodIcon(Icon_t::Disconnected);
    }

    SetTestStarted(false);

    if(m_tcpServer->isListenning() && m_serialPort->isOpen()) // Stop TCP Server
    {
        on_tcp_listen_clicked();
        SetMoodIcon(Icon_t::Connecting);
    }
}
//---------------------------------------------------------------


//---------------------------------------------------------------

void MainWindow::TcpServer_Init()
{
    m_tcpServer = new TcpServer();
    connect(m_tcpServer, &TcpServer::dataReceived, this, &MainWindow::onTcpDataReceivedFromClient);
    connect(m_tcpServer, &TcpServer::clientConnected, this, &MainWindow::onTcpClientConnected);
    connect(m_tcpServer, &TcpServer::clientDisconnected, this, &MainWindow::onTcpClientDisconnected);
    connect(m_tcpServer, &TcpServer::logMessage, this, &MainWindow::onLogMessage);
}

bool MainWindow::TcpServer_Start(int serverport)
{
    return m_tcpServer->startServer(serverport);
}

void MainWindow::TcpServer_Stop()
{
    m_tcpServer->stopServer();
}

void MainWindow::onTcpDataReceivedFromClient()
{
    QByteArray dataBuffer;

    if(m_tcpServer->isClientConnected())
    {
        dataBuffer = m_tcpServer->Read();
        Test(Channel_t::TCP, dataBuffer);
    }
}

void MainWindow::onTcpClientConnected()
{
    ui->tcp_connection_info->setText("Client is connected");
}

void MainWindow::onTcpClientDisconnected()
{
    ui->tcp_connection_info->setText("Client is disconnected");
}

void MainWindow::on_tcp_listen_clicked()
{
    if(m_tcpServer->isListenning())
    {
        TcpServer_Stop();
        ui->tcp_port->setEnabled(true);
        ui->tcp_listen->setText("Listen");
        ui->tcp_status->setText("Server is not running");
        SetMoodIcon(Icon_t::Disconnected);
    }
    else
    {
        int serverport = ui->tcp_port->value();

        if(false != TcpServer_Start(serverport))
        {
            ui->tcp_port->setEnabled(false);
            ui->tcp_listen->setText("Stop");
            ui->tcp_status->setText("Server is listening");
            SetMoodIcon(Icon_t::Connecting);
        }
        else
        {
            ui->tcp_status->setText("Server start failed");
            SetMoodIcon(Icon_t::Disconnected);
        }
    }

    SetTestStarted(false);

    if(m_tcpServer->isListenning() && m_serialPort->isOpen()) // Stop serial port
    {
        on_serial_open_clicked();
        SetMoodIcon(Icon_t::Connecting);
    }
}

//---------------------------------------------------------------



//---------------------------------------------------------------

void MainWindow::PrintResults()
{
    Log(QString("Duration : %1 ms").arg(m_testFinishAt - m_testStartAt));
    Log(QString("Communication time : %1 ms").arg(m_testElapsedTime));
    Log(QString("Transferred data size %1 bytes").arg(m_data_size));

    if(m_testElapsedTime)
    {
        Log(QString("Data rate %1 KBps").arg(m_data_size / m_testElapsedTime));
    }
}

void MainWindow::onTimeoutTest()
{
    m_timer_test.stop();
    SetTestStarted(false);
    m_testFinishAt = QDateTime::currentMSecsSinceEpoch();
    ui->test_status->setText("Test timed out");
    Log("Timeout");
    PrintResults();
    SetMoodIcon(Icon_t::TestFailed);
}

bool MainWindow::Send(Channel_t channel, QByteArray dataBuffer)
{
    bool ret = false;

    switch(channel)
    {
        case Channel_t::Serial:
            ret = m_serialPort->Write(Protocol_Wrap(dataBuffer));
            break;

        case Channel_t::TCP:
            ret = m_tcpServer->Write(Protocol_Wrap(dataBuffer));
            break;

        default:
            break;
    }

    return ret;
}

qint64 MainWindow::PacketTimeout(Channel_t channel)
{
    qint64 timeout = PROTOCOL_OVERHEAD + m_testIndex;

    switch(channel)
    {
        case Channel_t::Serial:
            timeout = m_serialPort->getBaudTimeout(timeout);
            break;

        case Channel_t::TCP:
            timeout = m_tcpServer->getTimeout(timeout);
            break;

        default:
            break;
    }

    timeout += TEST_FRAME_TIMEOUT;
    return timeout;
}

qint64 MainWindow::ElapsedTime(Channel_t channel)
{
    qint64 time = 0;

    switch(channel)
    {
        case Channel_t::Serial:
            time = m_serialPort->getReceivedTime() - m_serialPort->getSentTime();
            break;

        case Channel_t::TCP:
            time = m_tcpServer->getReceivedTime() - m_tcpServer->getSentTime();
            break;

        default:
            break;
    }

    return time;
}

void MainWindow::SetTestStarted(bool value)
{
    m_testStarted = value;
    m_testIndex = 0;
    m_testStep = Test_Step_t::step_Idle;
}

void MainWindow::SetMoodIcon(Icon_t icon)
{
    if(ui->moodicon->movie() != nullptr)
    {
        if(ui->moodicon->movie()->isValid())
        {
            ui->moodicon->movie()->stop();
        }
    }

    if(movie)
    {
        delete movie;
        movie = nullptr;
    }

    switch(icon)
    {
        case Icon_t::Disconnected:
            ui->moodicon->setPixmap(QPixmap(MOOD_DISCONNECTED));
            break;

        case Icon_t::Connecting:
            movie = new QMovie(MOOD_CONNECTING);
            ui->moodicon->setMovie(movie);
            movie->start();
            ui->test_status->setText("Waiting for a connection");
            break;

        case Icon_t::Testing:
            movie = new QMovie(MOOD_TESTING);
            ui->moodicon->setMovie(movie);
            movie->start();
            break;

        case Icon_t::TestSuccess:
            movie = new QMovie(MOOD_RESULT_SUCCESS);
            ui->moodicon->setMovie(movie);
            movie->start();
            break;

        case Icon_t::TestFailed:
            movie = new QMovie(MOOD_RESULT_FAIL);
            ui->moodicon->setMovie(movie);
            movie->start();
            break;
    }
}

void MainWindow::Clean_Counters()
{
    ui->rx_count->display(0);
    ui->rx_progress->setValue(0);
    ui->tx_count->display(0);
    ui->tx_progress->setValue(0);
    ui->error_count->display(0);
    ui->error_progress->setValue(0);
    m_data_size = 0;
    ui->test_data_size->clear();
    m_testElapsedTime = 0;
}

void MainWindow::Inc_Error()
{
    ui->error_count->display(ui->error_count->intValue() + 1);
    ui->error_progress->setValue(ui->error_progress->value() + 1);
}

void MainWindow::Inc_RX()
{
    ui->rx_count->display(ui->rx_count->intValue() + 1);
    ui->rx_progress->setValue(ui->rx_progress->value() + 1);
}

void MainWindow::Inc_TX()
{
    ui->tx_count->display(ui->tx_count->intValue() + 1);
    ui->tx_progress->setValue(ui->tx_progress->value() + 1);
}

void MainWindow::Test(Channel_t channel, QByteArray dataBuffer)
{
    bool ret = false;
    QByteArray data;
    int data_size;
    data = Protocol_Unwrap(dataBuffer);
    data_size = data.size();

    if(data_size)
    {
        switch(m_testStep)
        {
            case Test_Step_t::step_Idle:
                if(1 == data_size && 0x00 == data.at(0))
                {
                    Clean_Counters();
                    SetTestStarted(true);
                    m_testStartAt = QDateTime::currentMSecsSinceEpoch();
                    Log("Started");
                    ui->test_status->setText("Testing...");
                    m_testStep = Test_Step_t::step_Test;
                    m_testIndex = 1;
                    SetMoodIcon(Icon_t::Testing);
                }
                else
                {
                    Log("Wrong start request received");
                    Inc_Error();
                    break;
                }

                [[fallthrough]];

            case Test_Step_t::step_Test:
                if(m_testStarted && m_testIndex)
                {
                    qint32 i;

                    // Measurement
                    //------------------------------------------------------------
                    if(1 < m_testIndex)
                    {
                        m_testElapsedTime += ElapsedTime(channel);
                    }

                    //------------------------------------------------------------
                    // RX
                    //------------------------------------------------------------
                    Inc_RX();
                    Log("RX");

                    //------------------------------------------------------------

                    // Check Index
                    //------------------------------------------------------------
                    if(1 < m_testIndex)
                    {
                        data_size++;
                    }

                    if(m_testIndex != data_size)
                    {
                        Log("Wrong Index");
                        Inc_Error();
                    }

                    Log(QString("Index %1 / %2").arg(m_testIndex).arg(data_size));
                    //------------------------------------------------------------
                    // TX
                    //------------------------------------------------------------
                    dataBuffer.resize(m_testIndex);
                    dataBuffer.clear();
                    i = m_testIndex;

                    while(i)
                    {
                        dataBuffer.append((char)i);
                        i--;
                    }

                    ret = Send(channel, dataBuffer);

                    if(ret)
                    {
                        m_timer_test.start(PacketTimeout(channel));
                        Inc_TX();
                        Log("TX");
                    }
                    else
                    {
                        Log("Send failed");
                        Inc_Error();
                    }

                    //------------------------------------------------------------
                    // Next index
                    //------------------------------------------------------------
                    m_testIndex++;

                    //------------------------------------------------------------

                    // Check for finish
                    //------------------------------------------------------------
                    if(TEST_INDEX_MAX < m_testIndex)
                    {
                        m_timer_test.stop();
                        SetTestStarted(false);
                        m_testFinishAt = QDateTime::currentMSecsSinceEpoch();

                        if(0 == ui->error_count->intValue())
                        {
                            Log("Finished successfully");
                            ui->test_status->setText("Test finished successfully");
                            SetMoodIcon(Icon_t::TestSuccess);
                        }
                        else
                        {
                            Log("Finished with errors");
                            ui->test_status->setText("Test finished with errors");
                            SetMoodIcon(Icon_t::TestFailed);
                        }

                        PrintResults();
                        m_testStep = Test_Step_t::step_Idle;
                    }

                    //------------------------------------------------------------
                    ui->test_data_size->setText(QString("%1 KB").arg(m_data_size / 1024));
                }
                else
                {
                    Log("Wrong state");
                    Inc_Error();
                }

                break;

            default:
                Log("Wrong case");
                m_testStep = Test_Step_t::step_Idle;
                m_testIndex = 0;
                break;
        }
    }
    else
    {
        Log("Not a valid packet");
        Inc_Error();
    }
}

QByteArray MainWindow::Protocol_Wrap(QByteArray dataBuffer)
{
    QByteArray data;
    quint32 crc;
    crc = crc32(dataBuffer.data(), dataBuffer.size());
    data.resize(dataBuffer.size() + PROTOCOL_OVERHEAD);
    data.clear();
    data.append((char)0x00);
    data.append((dataBuffer.size() >> 8) & 0xFF);
    data.append(dataBuffer.size() & 0xFF);
    data.append(dataBuffer);
    data.append((crc >> 24) & 0xFF);
    data.append((crc >> 16) & 0xFF);
    data.append((crc >> 8) & 0xFF);
    data.append((crc) & 0xFF);
    //qDebug() << "Protocol : Wrap -" << QString(data.toHex());
    m_data_size += data.size();
    return data;
}

QByteArray MainWindow::Protocol_Unwrap(QByteArray dataBuffer)
{
    QByteArray data;
    quint16 length;
    quint32 crc, crcp;
    char *b = dataBuffer.data();
    //qDebug() << "Protocol : Unwrap -" << QString(dataBuffer.toHex());
    m_data_size += dataBuffer.size();
    data.resize(0);
    data.clear();

    if(PROTOCOL_OVERHEAD < dataBuffer.size())
    {
        if(0x00 == b[0])
        {
            length = qFromBigEndian<quint16>((uchar *)&b[1]);

            if(length)
            {
                if(PROTOCOL_OVERHEAD + length == dataBuffer.size())
                {
                    crc = crc32(&b[3], length);
                    crcp = qFromBigEndian<quint32>((uchar *)&b[3 + length]);

                    if(crc == crcp)
                    {
                        data.resize(length);
                        data.clear();
                        data.append(&b[3], length);
                    }
                    else
                    {
                        Log("Protocol : CRC mismatch");
                    }
                }
                else
                {
                    Log("Protocol : Length mismatch");
                }
            }
            else
            {
                Log("Protocol : Zero data length");
            }
        }
        else
        {
            Log("Protocol : Wrong header byte");
        }
    }
    else
    {
        Log("Protocol : Wrong length");
    }

    return data;
}

quint32 MainWindow::crc32(const char *data, quint16 length)
{
    qint32 i, j;
    quint32 byte, crc, mask;
    crc = 0xFFFFFFFF;

    for(i = 0; i < length; i++)
    {
        byte = data[i];
        crc = crc ^ byte;

        for(j = 7; j >= 0; j--)
        {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }

        i = i + 1;
    }

    return ~crc;
}

//---------------------------------------------------------------

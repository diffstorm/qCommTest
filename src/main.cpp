/*
    qCommTest - Serial Communication Test Tool
    Version  : 1.0
    Date     : 20.11.2017
    Author   : Eray Ozturk  | github.com/diffstorm
*/
#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QCommandLineParser>
#include <QCommandLineOption>

void setStylesheet()
{
    QFile f(":qdarkstyle/style.qss");

    if(!f.exists())
    {
        printf("Unable to set stylesheet, file not found\n");
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("qCommTest");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Communication Test Tool");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption tcpPortOption(QStringList() << "p" << "tcp-port",
                                     QCoreApplication::translate("main", "Start TCP server on <port>."),
                                     QCoreApplication::translate("main", "port"));
    parser.addOption(tcpPortOption);

    parser.process(a);

    MainWindow m;
    setStylesheet();

    if (parser.isSet(tcpPortOption)) {
        int port = parser.value(tcpPortOption).toInt();
        m.startTcpServer(port);
    }

    m.show();
    return a.exec();
}

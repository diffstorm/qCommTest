/*
    qCommTest - Serial Communication Test Tool
    Version  : 1.0
    Date     : 20.11.2017
    Author   : Eray Ozturk  | github.com/diffstorm
*/
#include "mainwindow.h"
#include <QApplication>
#include <QFile>

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
    MainWindow m;
    setStylesheet();
    m.show();
    return a.exec();
}

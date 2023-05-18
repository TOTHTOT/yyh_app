#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile style_file(":/style.qss");

    if(style_file.exists())
    {
        style_file.open(QFile::ReadOnly);
        QString style_sheet = QLatin1String(style_file.readAll());
        qApp->setStyleSheet(style_sheet);
        style_file.close();
    }
    else
    {
        qDebug()<<"style.qss not find"<<endl;
    }
    MainWindow w;
    w.show();

    return a.exec();
}

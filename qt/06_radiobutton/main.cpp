#include "mainwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 指定文件
    QFile style_file(":/style.qss");

    // 判断文件是否存在
    if(style_file.exists())
    {
        // 以只读方式打开
        style_file.open(QFile::ReadOnly);
        QString style_sheet = QLatin1String(style_file.readAll());  // 读取文件
        qApp->setStyleSheet(style_sheet);   // 设置全局样式
        style_file.close();
    }
    else
    {
        printf("Error style file not exist!\n");
    }

    MainWindow w;
    w.show();

    return a.exec();
}

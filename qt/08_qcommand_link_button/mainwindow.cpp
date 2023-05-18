#include "mainwindow.h"
#include <QUrl>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setGeometry(0, 0, 800, 400);

    command_link_button = new QCommandLinkButton("打开 /home 目录", "点击将调用系统的窗口打开 /home 目录", this);

    command_link_button->setGeometry(300, 200, 250, 60);

    connect(command_link_button, SIGNAL(clicked(bool)), this, SLOT(command_link_button_clicked(bool)));
}


MainWindow::~MainWindow()
{

}

void MainWindow::command_link_button_clicked(bool state)
{
    // 调用系统服务打开/home 目录
    QDesktopServices::openUrl(QUrl("file:///home/"));
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(0, 0, 800, 480);

    // 实例化定时器， 每100ms触发一次槽函数
    timer = new QTimer(this);
    timer->start(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_timeout()));

    progess_bar = new QProgressBar(this);
    progess_bar->setGeometry(300, 200, 200, 60);
    progess_bar->setRange(0, 100);
    progess_bar->setFormat("充电中%p%");

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::timer_timeout()
{
    progess_bar_value++;
    progess_bar->setValue(progess_bar_value);
    if(progess_bar_value > 100)
    {
        progess_bar_value = 0;
    }
}

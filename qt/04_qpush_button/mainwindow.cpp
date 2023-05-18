/*
 * @Description:
 * @Author: TOTHTOT
 * @Date:
 * @LastEditTime:
 * @LastEditors: TOTHTOT
 * @FilePath:
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(0, 0, 800, 400);

//    实例化对象，并设置显示文本
    qpush_button_1 = new QPushButton("窗口1皮肤", this);
    qpush_button_2 = new QPushButton("窗口皮肤2", this);

//    设置两个按钮对象的位置
    qpush_button_1->setGeometry(300, 200, 80, 40);
    qpush_button_2->setGeometry(400, 200, 80, 40);

//    设置信号槽
    connect(qpush_button_1, SIGNAL(clicked()), this, SLOT(qpush_button_1_clicked()));
    connect(qpush_button_2, SIGNAL(clicked()), this, SLOT(qpush_button_2_clicked()));

}

MainWindow::~MainWindow()
{
    delete ui;
}


/**
 * @name: qpush_button_1_clicked
 * @msg: 按钮信号槽函数
 * @return {*}
 * @author: TOTHTOT
 * @date:
 */
void MainWindow::qpush_button_1_clicked()
{
//    设置皮肤颜色
    this->setStyleSheet("QMainWindow {background-color:rgba(255, 245, 238, 100%);}");
}

/**
 * @name: qpush_button_2_clicked
 * @msg: 按钮信号槽函数
 * @return {*}
 * @author: TOTHTOT
 * @date:
 */
void MainWindow::qpush_button_2_clicked()
{
    this->setStyleSheet("QMainWindow {background-color:rgba(238, 122, 233, 100%);}");
}

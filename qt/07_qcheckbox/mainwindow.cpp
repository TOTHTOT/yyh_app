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
    this->setStyleSheet("QMainWindow {background-color: rgba(100, 100, 100, 100%);}");

    // 实例化对象
    check_box = new QCheckBox(this);
    check_box->setGeometry(350, 200, 250, 50);
    check_box->setCheckState(Qt::Checked);  // 设置三种选中状态中的checked
    check_box->setText("初始化为checked状态");
    check_box->setTristate();   // 开启三态模式，必须开启，否则只有两种状态，即Checked和Unchecked
    // 设置信号槽函数
    connect(check_box, SIGNAL(stateChanged(int)), this, SLOT(check_box_state_change(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @name:check_box_state_change
 * @msg:槽函数
 * @return {*}
 * @author: TOTHTOT
 * @date:
 */
void MainWindow::check_box_state_change(int state)
{
    switch (state) {
    case Qt::Checked:
        check_box->setText("Check 状态");
        break;
    case Qt::Unchecked:
        check_box->setText("Unchecked 状态");
        break;
    case Qt::PartiallyChecked:
        check_box->setText("PartiallyChecked 状态");
        break;
    default:
        break;
    }
}

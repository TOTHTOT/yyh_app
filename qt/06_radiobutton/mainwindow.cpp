#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setGeometry(0, 0, 800, 400);

    // 实例化 QRadioButton 对象
    radio_button_1 = new QRadioButton("开关1", this);
    radio_button_2 = new QRadioButton("开关2", this);
    radio_button_3 = new QRadioButton("开关3", this);

    // 设置大小
    radio_button_1->setGeometry(200, 200, 100, 50);
    radio_button_2->setGeometry(300, 200, 100, 50);
    radio_button_3->setGeometry(400, 200, 100, 50);

    // 设置初始状态
    radio_button_1->setChecked(false);
    radio_button_2->setChecked(true);
    radio_button_3->setChecked(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

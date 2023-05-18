#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(0, 0, 800, 480);

    spinbox = new QSpinBox(this);
    spinbox->setGeometry(350, 200, 150, 30);
    spinbox->setRange(0, 100);   // 设置范围 0～100
    spinbox->setSingleStep(10);  // 设置步长为10
    spinbox->setValue(100);      // 设置初始值
    spinbox->setSuffix("%不透明度"); // 设置后缀

    connect(spinbox, SIGNAL(valueChanged(int)), this, SLOT(spinbox_value_changed(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::spinbox_value_changed(int value)
{
    double opacity = (double)value/100;
    this->setWindowOpacity(opacity);
    qDebug()<<"opacity = "<< opacity;
}

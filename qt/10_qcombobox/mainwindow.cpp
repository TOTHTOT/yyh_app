#include "mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setGeometry(0, 0, 800, 480);

    // 实例化
    combobox = new QComboBox(this);
    combobox->setGeometry(300, 200, 150, 30);
    combobox->addItem("广东");
    combobox->addItem("深圳");
    combobox->addItem("福建");

    connect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(combobox_clicked(int)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::combobox_clicked(int index)
{
    qDebug()<<"选择的是"<<combobox->itemText(index)<<endl;
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(0, 0, 800, 480);

    // 实例化 QPushButton
    push_button = new QPushButton("弹出Dialog", this);
    push_button->setGeometry(300, 200,100, 50);
    connect(push_button, SIGNAL(clicked(bool)), this, SLOT(push_button_clicked(bool)));

    // 实例化 pushbutton
    dialogbox_push_button = new QPushButton(tr("自定义"));

    // 实例化 QDialogButtonBox
    dialog_button_box = new QDialogButtonBox(this);
    dialog_button_box->setGeometry(300, 200, 200, 30);
    // 使用 Qt 内部的 Cancel 按钮和 Yes 按钮
    dialog_button_box->addButton(QDialogButtonBox::Cancel);
    dialog_button_box->addButton(QDialogButtonBox::Yes);
    // 设置按钮的文本
    dialog_button_box->button(QDialogButtonBox::Cancel)->setText("取消");
    dialog_button_box->button(QDialogButtonBox::Yes)->setText("确认");
    dialog_button_box->addButton(dialogbox_push_button, QDialogButtonBox::ActionRole);

    // 信号槽链接
    connect(dialog_button_box, SIGNAL(clicked(QAbstractButton*)), this, SLOT(dialogbox_button_clicked(QAbstractButton*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @name: dialogbox_button_clicked
 * @msg: 对话框按钮槽函数
 * @return {*}
 * @author: TOTHTOT
 * @date:
 */
void MainWindow::dialogbox_button_clicked(QAbstractButton * button)
{
    if(button == dialog_button_box->button(QDialogButtonBox::Cancel))
    {
        qDebug()<<"Cancel pressed";
    }
    else if(button == dialog_button_box->button(QDialogButtonBox::Yes))
    {
        qDebug()<<"Yes pressed";
    }
    else if(button == dialogbox_push_button)
    {
        qDebug()<<"dialogbox_push_button pressed";
    }
}

void MainWindow::push_button_clicked(bool state)
{
    qDebug()<<"push button clicked";

}

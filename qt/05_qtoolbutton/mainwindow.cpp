#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QStyle"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(0, 0, 800, 400);

//    实例化 QtoolBar 对象
    tool_bar = new QToolBar(this);
    tool_bar->setGeometry(0, 0, 800, 100);

//    实例化 QStyle 对象，设置风格
    QStyle *style_p = QApplication::style();
    QIcon icon = style_p->standardIcon(QStyle::SP_TitleBarContextHelpButton);

//    实例化 QtoolButton 对象
    tool_button_1 = new QToolButton();
    tool_button_1->setIcon(icon);
    tool_button_1->setText("帮助");

//    设置 tool_button 的样式， 设置文本置于图标下方
    tool_button_1->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

//    将 tool_button 添加到 tool_bar
    tool_bar->addWidget(tool_button_1);



}

MainWindow::~MainWindow()
{
    delete ui;
}

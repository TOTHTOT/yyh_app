#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setGeometry(0, 0, 800, 480);
    // 实例化对象
    font_combobox = new QFontComboBox(this);
    label = new QLabel(this);

    // 设置大小与位置
    font_combobox->setGeometry(280, 200, 200, 30);
    label->setGeometry(280, 205, 300, 50);

    connect(font_combobox, SIGNAL(currentFontChanged(QFont)), this, SLOT(font_combobox_changed(QFont)));
}


MainWindow::~MainWindow()
{

}

void MainWindow::font_combobox_changed(QFont font)
{
    label->setFont(font);
    QString str = "此标签用于显示字体效果\n字体为：" + font_combobox->itemText(font_combobox->currentIndex());
    label->setText(str);
}

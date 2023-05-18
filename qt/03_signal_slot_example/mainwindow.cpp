#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(800, 400);

    push_button = new QPushButton(this);
    push_button->setText("This is a button");

    connect(push_button, SIGNAL(clicked()), this, SLOT(push_button_clicked()));
    connect(this, SIGNAL(push_button_text_changed()), this, SLOT(change_button_text()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::push_button_clicked()
{
    emit push_button_text_changed();
}

void MainWindow::change_button_text()
{
    push_button->setText("on clicked");
}

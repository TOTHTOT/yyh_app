#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(0, 0, 800, 480);

    key_sequence_edit = new QKeySequenceEdit(this);
    key_sequence_edit->setGeometry(350, 200, 150, 30);

    connect(key_sequence_edit, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(key_sequence_edit_changed(QKeySequence)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::key_sequence_edit_changed(QKeySequence key_sequence)
{
    if(key_sequence == QKeySequence(tr("Ctrl+Q")))
    {
        qDebug()<<"exit"<<endl;
        this->close();
    }
    else
    {
        qDebug()<<key_sequence.toString()<<endl;
    }
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QPushButton *qpush_button_1;
    QPushButton *qpush_button_2;
private slots:
    // 信号槽函数
    void qpush_button_1_clicked();
    void qpush_button_2_clicked();
};

#endif // MAINWINDOW_H

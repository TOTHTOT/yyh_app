#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRadioButton>

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
//    声明 QRadioButton 对象
    QRadioButton *radio_button_1;
    QRadioButton *radio_button_2;
    QRadioButton *radio_button_3;
};

#endif // MAINWINDOW_H

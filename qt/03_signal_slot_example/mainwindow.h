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
signals:
    //  my signal
    void push_button_text_changed();

public slots:
    void change_button_text();
    void push_button_clicked();
private:
    Ui::MainWindow *ui;
    QPushButton *push_button;
};

#endif // MAINWINDOW_H

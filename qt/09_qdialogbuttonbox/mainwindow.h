#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialogButtonBox>
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
    QDialogButtonBox *dialog_button_box;
    QPushButton *push_button;
    QPushButton *dialogbox_push_button;
private slots:
    // QDialogButtonBox的按钮信号槽， QAbstractButton 用于判断哪个按钮被按下。
    void dialogbox_button_clicked(QAbstractButton *);
    void push_button_clicked(bool state);
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCommandLinkButton>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    QCommandLinkButton *command_link_button;
private slots:
    void command_link_button_clicked(bool);
};

#endif // MAINWINDOW_H

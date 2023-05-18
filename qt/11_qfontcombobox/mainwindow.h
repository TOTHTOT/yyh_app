#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFontComboBox>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    QLabel *label;
    QFontComboBox *font_combobox;
private slots:
    void font_combobox_changed(QFont font);

};

#endif // MAINWINDOW_H

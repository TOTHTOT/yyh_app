#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeySequenceEdit>

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
    QKeySequenceEdit *key_sequence_edit;
private slots:
    void key_sequence_edit_changed(QKeySequence );
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QLineEdit;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  void browse(QLineEdit *edit);

private slots:
  void on_firstBrowse_clicked();
  void on_secondBrowse_clicked();

private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

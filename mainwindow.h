#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

namespace Ui {
class MainWindow;
}

class QLineEdit;
class Netlist;
class NetCmpModel;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  void browse(QLineEdit *edit);

private slots:
  void on_leftBrowse_clicked();
  void on_rightBrowse_clicked();
  void netlistReturnPressed();
  void on_showDifferenceOnly_toggled(bool checked);

private:
  Ui::MainWindow *ui;
  Netlist *m_leftNetlist;
  Netlist *m_rightNetlist;
  QMap<QLineEdit *, Netlist *> m_netlists;
  NetCmpModel *m_netCmpModel;
};

#endif // MAINWINDOW_H

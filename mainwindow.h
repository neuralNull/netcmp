#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

// Объявления используемых классов
namespace Ui {
class MainWindow;
}

class QSettings;
class QLineEdit;
class Netlist;
class NetCmpModel;
class QPrinter;



// Класс главного окна программы
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  // Конструктор
  explicit MainWindow(QWidget *parent = 0);
  // Деструктор
  ~MainWindow();

  // Метод открытия обозревателя файлов для заданного поля ввода
  void browse(QLineEdit *edit);

private slots:
  // Слоты элементов управления
  void on_leftBrowse_clicked();
  void on_rightBrowse_clicked();
  void on_font_clicked();
  void on_print_clicked();
  void netlistReturnPressed();
  void on_showDifferenceOnly_toggled(bool checked);
  void paintRequested(QPrinter *printer);

private:
  // Интерфейс главного окна
  Ui::MainWindow *ui;
  // Параметры работы
  QSettings *m_settings;
  // Последняя открытая директория
  QString m_lastDir;

  QFont m_cmpFont;

  // Левый нетлист
  Netlist *m_leftNetlist;
  // Правый нетлист
  Netlist *m_rightNetlist;
  // Ассоциативный массив нетлистов
  QMap<QLineEdit *, Netlist *> m_netlists;
  // Модель сравнения нетлистов
  NetCmpModel *m_netCmpModel;
};

#endif // MAINWINDOW_H

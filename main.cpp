#include "mainwindow.h"
#include <QApplication>

// Главная функция программы
int main(int argc, char *argv[])
{
  // Qt-приложение
  QApplication a(argc, argv);
  // Главное окно
  MainWindow w;
  // Открытие главного окна
  w.show();

  // Запуск обработчика событий
  return a.exec();
}

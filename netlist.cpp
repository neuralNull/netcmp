#include "netlist.h"

#include <QFile>
#include <QDebug>

// Конструктор класса нетлиста
Netlist::Netlist(QObject *parent) :
  QObject(parent)
{
}

// Метод чтения нетлиста из файла
void Netlist::readFromFile(const QString &filename)
{
  // Открытие файла с нетлистом по заданному пути
  QFile f(filename);
  if (!f.open(QFile::ReadOnly)) return;

  // Удаление загруженных ранее цепей и узлов
  foreach (auto net, m_netMap) net->deleteLater();
  foreach (auto node, m_nodeMap) node->deleteLater();
  m_netMap.clear();
  m_nodeMap.clear();

  // Чтение содержимого файла
  auto data = QString::fromUtf8(f.readAll());

  // Определение формата нетлиста
  if (data.contains("(netClass"))
  {
    // Извлечение данных по структуре, заданной регулярным выражением
    QRegExp ri("\\(net[^\"]\"[^\"]+\"[^\\(]*(\\(node[^\\)]+\\)[^\\(]*)+\\)");
    // Получение индекса данных в цикле
    for (int i = 0;
         (i = data.indexOf(ri, i)) != -1;
         i += ri.matchedLength())
    {
      // Регулярное выражение для извлечения информации о цепи
      QRegExp rn("\"[^\"]+\"");
      rn.indexIn(ri.cap());
      // Создание новой цепи
      auto net = new Net(rn.cap().remove('"'), this);
      // Добавление цепи в нетлист
      addNet(net);
      // Извлечение информации об узлах цепи
      QRegExp rj("node[^\\)]+");
      for (int j = 0;
           (j = ri.cap().indexOf(rj, j)) != -1;
           j += rj.matchedLength())
      {
        // Создание нового узла
        auto node = new Node(rj.cap().remove(QRegExp("\"|node ")), this);
        // Добавление узла в цепь
        addNode(net, node);
      }
    }
  }
  // Проверка другого формата нетлиста
  else if (data.contains("NETS"))
  {
    // Извлечение информации о цепях
    QRegExp ri("[^=\\n\\r]+=[^;]+;");
    for (int i = data.indexOf("NETS");
         (i = data.indexOf(ri, i)) != -1;
         i += ri.matchedLength())
    {
      // Регулярное выражение для извлечения информации о цепи
      QRegExp rn("[^ =]+");
      rn.indexIn(ri.cap());
      // Создание новой цепи
      auto net = new Net(rn.cap(), this);
      // Добавление цепи в нетлист
      addNet(net);
      // Извлечение информации об узлах цепи
      QRegExp rj("[^ \\n\\r/]+/[^ \\n\\r/]+");
      for (int j = 0;
           (j = ri.cap().indexOf(rj, j)) != -1;
           j += rj.matchedLength())
      {
        // Создание нового узла
        auto node = new Node(rj.cap(), this);
        // Добавление узла в цепь
        addNode(net, node);
      }
    }
  }
  // Использование оставшегося формата нетлиста
  else
  {
    // Извлечение информации о цепях
    QRegExp ri("\\([\n\r][^\\)]+[\n\r]\\)");
    for (int i = 0;
         (i = data.indexOf(ri, i)) != -1;
         i += ri.matchedLength())
    {
      Net *net = 0;
      // Извлечение информации об имени цепи и её узлах
      QRegExp rj("[^\\(\\)\\n\\r]+");
      for (int j = 0;
           (j = ri.cap().indexOf(rj, j)) != -1;
           j += rj.matchedLength())
      {
        // Первый найденный элемент в цикле является именем цепи
        if (!net)
        {
          // Создание новой цепи
          net = new Net(rj.cap(), this);
          // Добавление цепи в нетлист
          addNet(net);
        }
        else
        {
          // Создание нового узла
          auto node = new Node(rj.cap(), this);
          // Добавление узла в созданную на первой итерации цикла цепь
          addNode(net, node);
        }
      }
    }
  }
}

// Конструктор узла цепи
Node::Node(const QString &displayName, QObject *parent) :
  QObject(parent),
  // Унифицированное имя (по умолчанию соответствует отображаемому)
  m_name(displayName),
  // Отображаемое имя
  m_displayName(displayName)
{
  // Разделение имени на название элемента и номер пина
  QStringList nameAndPin = displayName.split('/');
  // Попытка использования различных разделителей
  if (nameAndPin.count() != 2) nameAndPin = displayName.split(',');
  if (nameAndPin.count() != 2) nameAndPin = displayName.split('-');
  if (nameAndPin.count() != 2) nameAndPin = displayName.split(' ');
  // Если невозможно разделить имя, то в качестве унифицированного
  // имени будет использоваться отображаемое
  if (nameAndPin.count() != 2) return;

  // Переменные, хранящие название элемента и номер пина
  QString name = nameAndPin.first();
  QString pin = nameAndPin.last();

  // Обработка случая, когда реальный номер пина находится в названии
  // элемента (встречается в узлах, описывающих разъёмы)
  nameAndPin = name.split(':');
  if (nameAndPin.count() == 2)
  {
    // Обновление имени и номера пина
    name = nameAndPin.first();
    pin = nameAndPin.last();
  }

  // Игнорирование виртуальных частей элемента (встречается в узлах,
  // описывающих микросхемы)
  if (name[0] != 'X') name = name.split('.').first();

  // Создание унифицированного имени
  m_name = name + ":" + pin;
}

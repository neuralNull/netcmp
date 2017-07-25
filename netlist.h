#ifndef NETLIST_H
#define NETLIST_H

#include <QObject>
#include <QMap>

// Объявления используемых классов
class Net;
class Node;



// Класс узла цепи нетлиста
class Node : public QObject
{
  Q_OBJECT

public:
  // Конструктор
  Node(const QString &displayName, QObject *parent = 0);

  // Get-метод цепи, к которой относится узел
  Net *net() const { return m_net; }
  // Get-метод унифицированного имени узла
  QString name() const { return m_name; }
  // Get-метод отображаемого имени узла
  QString displayName() const { return m_displayName; }

private:
  // Унифицированное имя узла
  QString m_name;
  // Отображаемое имя узла
  QString m_displayName;
  // Цепь, к которой относится узел
  Net *m_net;

public slots:
  // Слот задания цепи
  void setNet(Net *net) { m_net = net; }
};



// Класс цепи нетлиста
class Net : public QObject
{
  Q_OBJECT

public:
  // Конструктор
  Net(const QString &name, QObject *parent = 0) :
    QObject(parent), m_name(name) {}

  // Get-метод имени цепи
  QString name() const { return m_name; }
  // Get-метод ассоциативного массива узлов
  const QMap <QString, Node *> &nodeMap() const { return m_nodeMap; }

private:
  // Имя цепи
  QString m_name;
  // Ассоциативный массив узлов
  QMap <QString, Node *> m_nodeMap;

public slots:
  // Слот добавления узла
  void addNode(Node *node)
  { m_nodeMap[node->name()] = node; node->setNet(this); }
};



// Класс нетлиста
class Netlist : public QObject
{
  Q_OBJECT

public:
  // Конструктор
  Netlist(QObject *parent = 0);

  // Get-метод ассоциативного массива цепей
  const QMap <QString, Net *> &netMap() const { return m_netMap; }
  // Get-метод ассоциативного массива узлов
  const QMap <QString, Node *> &nodeMap() const { return m_nodeMap; }
  // Метод, возвращающий информацию о том, является ли нетлист пустым
  bool isEmpty() const { return m_netMap.isEmpty(); }

private:
  // Ассоциативный массив цепей
  QMap <QString, Net *> m_netMap;
  // Ассоциативный массив узлов
  QMap <QString, Node *> m_nodeMap;

public slots:
  // Метод загрузки нетлиста из файла
  void readFromFile(const QString &filename);
  // Метод добавления цепи в нетлист
  void addNet(Net *net)
  { if (m_netMap.contains(net->name()))
      m_netMap[net->name()]->deleteLater();
    m_netMap[net->name()] = net; }
  // Метод добавления узла в цепь
  void addNode(Net *net, Node *node)
  { if (!m_nodeMap.contains(node->name())) {
      m_nodeMap[node->name()] = node; net->addNode(node);
    } else node->deleteLater(); }
};

#endif // NETLIST_H

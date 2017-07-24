#ifndef NETLIST_H
#define NETLIST_H

#include <QObject>
#include <QMap>

class Net;
class Node;

class Node : public QObject
{
  Q_OBJECT

public:
  Node(const QString &displayName, QObject *parent = 0);

  Net *net() const { return m_net; }
  QString name() const { return m_name; }
  QString displayName() const { return m_displayName; }

private:
  QString m_name;
  QString m_displayName;
  Net *m_net;

public slots:
  void setNet(Net *net) { m_net = net; }
};

class Net : public QObject
{
  Q_OBJECT

public:
  Net(const QString &name, QObject *parent = 0) :
    QObject(parent), m_name(name) {}
  QString name() const { return m_name; }
  const QMap <QString, Node *> &nodeMap() const { return m_nodeMap; }

private:
  QString m_name;
  QMap <QString, Node *> m_nodeMap;

public slots:
  void addNode(Node *node)
  { m_nodeMap[node->name()] = node; node->setNet(this); }
};

class Netlist : public QObject
{
  Q_OBJECT

public:
  Netlist(QObject *parent = 0);
  const QMap <QString, Net *> &netMap() const { return m_netMap; }
  const QMap <QString, Node *> &nodeMap() const { return m_nodeMap; }
  bool isEmpty() const { return m_netMap.isEmpty(); }

private:
  QMap <QString, Net *> m_netMap;
  QMap <QString, Node *> m_nodeMap;

public slots:
  void readFromFile(const QString &filename);
  void addNet(Net *net)
  { if (m_netMap.contains(net->name()))
      m_netMap[net->name()]->deleteLater();
    m_netMap[net->name()] = net; }
  void addNode(Net *net, Node *node)
  { if (m_nodeMap.contains(node->name()))
      m_nodeMap[node->name()]->deleteLater();
    m_nodeMap[node->name()] = node; net->addNode(node); }
};

#endif // NETLIST_H

#include "netlist.h"

#include <QFile>
#include <QDebug>

Netlist::Netlist(QObject *parent) :
  QObject(parent)
{
}

void Netlist::readFromFile(const QString &filename)
{
  QFile f(filename);
  if (!f.open(QFile::ReadOnly)) return;

  foreach (auto net, m_netMap) net->deleteLater();
  foreach (auto node, m_nodeMap) node->deleteLater();
  m_netMap.clear();
  m_nodeMap.clear();

  auto data = QString::fromUtf8(f.readAll());

  if (data.contains("(netClass"))
  {
    QRegExp ri("\\(net[^\"]\"[^\"]+\"[^\\(]*(\\(node[^\\)]+\\)[^\\(]*)+\\)");
    for (int i = 0;
         (i = data.indexOf(ri, i)) != -1;
         i += ri.matchedLength())
    {
      QRegExp rn("\"[^\"]+\"");
      rn.indexIn(ri.cap());
      auto net = new Net(rn.cap().remove('"'), this);
      addNet(net);
      QRegExp rj("node[^\\)]+");
      for (int j = 0;
           (j = ri.cap().indexOf(rj, j)) != -1;
           j += rj.matchedLength())
      {
        auto node = new Node(rj.cap().remove(QRegExp("\"|node ")), this);
        addNode(net, node);
      }
    }
  }
  else if (data.contains("NETS"))
  {
    QRegExp ri("[^=\\n\\r]+=[^;]+;");
    for (int i = data.indexOf("NETS");
         (i = data.indexOf(ri, i)) != -1;
         i += ri.matchedLength())
    {
      QRegExp rn("[^ =]+");
      rn.indexIn(ri.cap());
      auto net = new Net(rn.cap(), this);
      addNet(net);
      QRegExp rj("[^ \\n\\r/]+/[^ \\n\\r/]+");
      for (int j = 0;
           (j = ri.cap().indexOf(rj, j)) != -1;
           j += rj.matchedLength())
      {
        auto node = new Node(rj.cap(), this);
        addNode(net, node);
      }
    }
  }
  else
  {
    QRegExp ri("\\([\n\r][^\\)]+[\n\r]\\)");
    for (int i = 0;
         (i = data.indexOf(ri, i)) != -1;
         i += ri.matchedLength())
    {
      Net *net = 0;
      QRegExp rj("[^\\(\\)\\n\\r]+");
      for (int j = 0;
           (j = ri.cap().indexOf(rj, j)) != -1;
           j += rj.matchedLength())
      {
        if (!net)
        {
          net = new Net(rj.cap(), this);
          addNet(net);
        }
        else
        {
          auto node = new Node(rj.cap(), this);
          addNode(net, node);
        }
      }
    }
  }
}

Node::Node(const QString &displayName, QObject *parent) :
  QObject(parent), m_name(displayName), m_displayName(displayName)
{
  QStringList nameAndPin = displayName.split('/');
  if (nameAndPin.count() != 2) nameAndPin = displayName.split(',');
  if (nameAndPin.count() != 2) nameAndPin = displayName.split('-');
  if (nameAndPin.count() != 2) nameAndPin = displayName.split(' ');
  if (nameAndPin.count() != 2) return;

  QString name = nameAndPin.first();
  QString pin = nameAndPin.last();

  nameAndPin = name.split(':');
  if (nameAndPin.count() == 2)
  {
    name = nameAndPin.first();
    pin = nameAndPin.last();
  }

  name = name.split('.').first();

  m_name = name + ":" + pin;
}

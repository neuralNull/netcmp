#ifndef NETCMPMODEL_H
#define NETCMPMODEL_H

#include <QAbstractItemModel>

class NetEntry;
class Netlist;
class Net;

class CompareEntry
{
public:
  virtual ~CompareEntry() {}

  QString leftDisplayName() const
  { return m_leftDisplayName; }
  QString rightDisplayName() const
  { return m_rightDisplayName; }

  void setLeftDisplayName(const QString &leftDisplayName)
  { m_leftDisplayName = leftDisplayName; }
  void setRightDisplayName(const QString &rightDisplayName)
  { m_rightDisplayName = rightDisplayName; }

  virtual bool isEqual() const = 0;

private:
  QString m_leftDisplayName;
  QString m_rightDisplayName;
};

class NodeEntry : public CompareEntry
{
public:
  NodeEntry(NetEntry *netEntry) : m_netEntry(netEntry) {}
  void setLeftName(const QString &leftName)
  { m_leftName = leftName; }
  void setRightName(const QString &rightName)
  { m_rightName = rightName; }

  bool isEqual() const override { return m_leftName == m_rightName; }
  NetEntry *netEntry() const { return m_netEntry; }

private:
  QString m_leftName;
  QString m_rightName;
  NetEntry *m_netEntry;
};

class NetEntry : public CompareEntry
{
public:
  ~NetEntry()
  { while (m_nodeEntries.count()) delete m_nodeEntries.takeLast(); }

  bool isEqual() const override
  { foreach (auto nodeEntry, m_nodeEntries)
      if (!nodeEntry->isEqual()) return false;
    return true; }
  int nodeEntryCount() const { return m_nodeEntries.count(); }
  NodeEntry *nodeEntry(int index) const
  { return m_nodeEntries[index]; }

  NodeEntry *createNodeEntry()
  { m_nodeEntries.append(new NodeEntry(this));
    return m_nodeEntries.last(); }

private:
  QVector<NodeEntry *> m_nodeEntries;
};

class NetCmpModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit NetCmpModel(QObject *parent = nullptr);
  void compareNetlists(Netlist *leftNetlist, Netlist *rightNetlist);
  void clear();

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row,
                    int column,
                    const QModelIndex &parent
                    = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &parent
               = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent
                  = QModelIndex()) const override;

  NetEntry *netEntry(int index) const { return m_netEntries[index]; }
  int netEntryCount() const { return m_netEntries.count(); }

private:
  QVector<NetEntry *> m_netEntries;

  NetEntry *createNetEntry()
  { m_netEntries.append(new NetEntry); return m_netEntries.last(); }
  void addComparison(Net *leftNet, Net *rightNet);
};

#endif // NETCMPMODEL_H

#include "netcmpmodel.h"
#include "netlist.h"

#include <QColor>
#include <QDebug>

NetCmpModel::NetCmpModel(QObject *parent) :
  QAbstractItemModel(parent)
{
}

void NetCmpModel::compareNetlists(Netlist *leftNetlist,
                                  Netlist *rightNetlist)
{
  clear();

  beginResetModel();

  QList<Net *> leftNets = leftNetlist->netMap().values();
  QList<Net *> rightNets = rightNetlist->netMap().values();

  while (true)
  {
    Net *associationLeft;
    Net *associationRight;
    int associationScore = 0;

    foreach (auto net, leftNets)
    {
      QMap<Net *, int> currentScores;

      foreach (auto node, net->nodeMap().values())
      {
        if (!rightNetlist->nodeMap().contains(node->name())) continue;
        Net *supposedNet =
            rightNetlist->nodeMap()[node->name()]->net();

        if (rightNets.contains(supposedNet) &&
            ++currentScores[supposedNet] > associationScore)
        {
          associationLeft = net;
          associationRight = supposedNet;
          associationScore = currentScores[supposedNet];
        }
      }
    }

    if (associationScore)
    {
      leftNets.removeAll(associationLeft);
      rightNets.removeAll(associationRight);

      addComparison(associationLeft, associationRight);
    }
    else break;
  }

  foreach (auto net, leftNets) addComparison(net, nullptr);
  foreach (auto net, rightNets) addComparison(nullptr, net);

  endResetModel();
}

void NetCmpModel::addComparison(Net *leftNet, Net *rightNet)
{
  NetEntry *netEntry = createNetEntry();
  QList<Node *> leftNodes;
  QList<Node *> rightNodes;
  QList<QString> leftNodeNames;
  QList<QString> rightNodeNames;

  if (leftNet)
  {
    netEntry->setLeftDisplayName(leftNet->name());
    leftNodes = leftNet->nodeMap().values();
    leftNodeNames = leftNet->nodeMap().keys();
  }
  if (rightNet)
  {
    netEntry->setRightDisplayName(rightNet->name());
    rightNodes = rightNet->nodeMap().values();
    rightNodeNames = rightNet->nodeMap().keys();
  }

  for (auto leftIt = leftNodes.begin(), rightIt = rightNodes.begin();
       leftIt != leftNodes.end() || rightIt != rightNodes.end();)
  {
    NodeEntry *nodeEntry = netEntry->createNodeEntry();

    if (leftIt != leftNodes.end())
    {
      if (rightIt != rightNodes.end())
      {
        if (rightNodeNames.contains((*leftIt)->name()))
        {
          if ((*leftIt)->name() == (*rightIt)->name())
          {
            nodeEntry->setLeftName((*leftIt)->name());
            nodeEntry->setLeftDisplayName((*leftIt++)->displayName());
          }
          nodeEntry->setRightName((*rightIt)->name());
          nodeEntry->setRightDisplayName((*rightIt++)->displayName());
        }
        else
        {
          nodeEntry->setLeftName((*leftIt)->name());
          nodeEntry->setLeftDisplayName((*leftIt++)->displayName());
          if (!leftNodeNames.contains((*rightIt)->name()))
          {
            nodeEntry->setRightName((*rightIt)->name());
            nodeEntry->setRightDisplayName((*rightIt++)->displayName());
          }
        }
      }
      else
      {
        nodeEntry->setLeftName((*leftIt)->name());
        nodeEntry->setLeftDisplayName((*leftIt++)->displayName());
      }
    }
    else
    {
      nodeEntry->setRightName((*rightIt)->name());
      nodeEntry->setRightDisplayName((*rightIt++)->displayName());
    }
  }
}

void NetCmpModel::clear()
{
  beginResetModel();
  while (m_netEntries.count())
    delete m_netEntries.takeLast();
  endResetModel();
}

QVariant NetCmpModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) return QVariant();

  CompareEntry *compareEntry =
      static_cast<CompareEntry *>(index.internalPointer());

  switch (role)
  {
    case Qt::DisplayRole:
      if (index.column() == 0)
        return compareEntry->leftDisplayName();
      if (index.column() == 1)
        return compareEntry->rightDisplayName();
      break;
    case Qt::ForegroundRole:
      if (!compareEntry->isEqual())
        return QColor(255, 20, 20);
      break;
    case Qt::DecorationRole:
      if (index.parent().isValid())
        return QColor(0, 0, 0, 0);
      break;
    default:
      break;
  }

  return QVariant();
}

Qt::ItemFlags NetCmpModel::flags(const QModelIndex &index) const
{
  if (!index.isValid()) return 0;
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex NetCmpModel::index(int row,
                               int column,
                               const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent)) return QModelIndex();

  if (!parent.isValid())
  {
    if (row < netEntryCount())
      return createIndex(row, column, netEntry(row));
  }
  else
  {
    CompareEntry *compareEntry =
        static_cast<CompareEntry *>(parent.internalPointer());
    NetEntry *netEntry = dynamic_cast<NetEntry *>(compareEntry);
    if (netEntry && row < netEntry->nodeEntryCount())
      return createIndex(row, column, netEntry->nodeEntry(row));
  }

  return QModelIndex();
}

QModelIndex NetCmpModel::parent(const QModelIndex &child) const
{
  if (!child.isValid()) return QModelIndex();

  CompareEntry *compareEntry =
      static_cast<CompareEntry *>(child.internalPointer());
  NodeEntry *nodeEntry = dynamic_cast<NodeEntry *>(compareEntry);
  if (nodeEntry) {
    for (int row = 0; row < netEntryCount(); row++)
    {
      if (netEntry(row) == nodeEntry->netEntry())
        return createIndex(row, 0, nodeEntry->netEntry());
    }
  }

  return QModelIndex();
}

QVariant NetCmpModel::headerData(int, Qt::Orientation, int) const
{
  return QVariant();
}

int NetCmpModel::rowCount(const QModelIndex &parent) const
{
  if (parent.column() > 0) return 0;
  if (!parent.isValid()) return netEntryCount();

  CompareEntry *compareEntry =
      static_cast<CompareEntry *>(parent.internalPointer());
  NetEntry *netEntry = dynamic_cast<NetEntry *>(compareEntry);
  if (netEntry) return netEntry->nodeEntryCount();

  return 0;
}

int NetCmpModel::columnCount(const QModelIndex &) const
{
  return 2;
}

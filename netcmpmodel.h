#ifndef NETCMPMODEL_H
#define NETCMPMODEL_H

#include <QAbstractItemModel>

// Объявления используемых классов
class NetEntry;
class Netlist;
class Net;



// Абстрактный класс элемента сравнения
class CompareEntry
{
public:
  // Виртуальный деструктор
  virtual ~CompareEntry() {}

  // Get-метод левого отображаемого имени
  QString leftDisplayName() const
  { return m_leftDisplayName; }
  // Get-метод правого отображаемого имени
  QString rightDisplayName() const
  { return m_rightDisplayName; }

  // Set-метод левого отображаемого имени
  void setLeftDisplayName(const QString &leftDisplayName)
  { m_leftDisplayName = leftDisplayName; }
  // Set-метод правого отображаемого имени
  void setRightDisplayName(const QString &rightDisplayName)
  { m_rightDisplayName = rightDisplayName; }

  // Виртуальный метод сравнения
  virtual bool isEqual() const = 0;

private:
  // Левое отображаемое имя
  QString m_leftDisplayName;
  // Правое отображаемое имя
  QString m_rightDisplayName;
};



// Класс элемента сравнения узла цепи
class NodeEntry : public CompareEntry
{
public:
  // Конструктор
  NodeEntry(NetEntry *netEntry) : m_netEntry(netEntry) {}
  // Set-метод левого сравниваемого имени
  void setLeftName(const QString &leftName)
  { m_leftName = leftName; }
  // Set-метод правого сравниваемого имени
  void setRightName(const QString &rightName)
  { m_rightName = rightName; }

  // Перегрузка метода сравнения
  bool isEqual() const override { return m_leftName == m_rightName; }
  // Get-метод элемента сравнения цепи, к которой относится узел
  NetEntry *netEntry() const { return m_netEntry; }

private:
  // Левое сравниваемое имя
  QString m_leftName;
  // Правое сравниваемое имя
  QString m_rightName;
  // Элемент сравнения цепи, к которой относится узел
  NetEntry *m_netEntry;
};



// Класс элемента сравнения цепи
class NetEntry : public CompareEntry
{
public:
  // Деструктор
  ~NetEntry()
  // Удаление всех элементов сравнения узлов цепи
  { while (m_nodeEntries.count()) delete m_nodeEntries.takeLast(); }

  // Перегрузка метода сравнения
  bool isEqual() const override
  // Успешное сравнение только в случае совпадения сравниваемых имён
  // всех узлов цепи
  { foreach (auto nodeEntry, m_nodeEntries)
      if (!nodeEntry->isEqual()) return false;
    return true; }
  // Метод получения количества элементов сравнения узлов цепи
  int nodeEntryCount() const { return m_nodeEntries.count(); }
  // Метод получения элемента сравнения узла цепи
  NodeEntry *nodeEntry(int index) const
  { return m_nodeEntries[index]; }

  // Метод создания элемента сравнения узла цепи
  NodeEntry *createNodeEntry()
  { m_nodeEntries.append(new NodeEntry(this));
    return m_nodeEntries.last(); }

private:
  // Вектор элементов сравнения узлов цепи
  QVector<NodeEntry *> m_nodeEntries;
};



// Модель сравнения нетлистов
class NetCmpModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  // Конструктор
  explicit NetCmpModel(QObject *parent = nullptr);

  // Метод сравнения нетлистов
  void compareNetlists(Netlist *leftNetlist, Netlist *rightNetlist);
  // Метод очистки результатов сравнения
  void clear();

  // Перегрузка базовых методов работы с моделью
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

  // Метод получения количества элементов сравнения цепей
  int netEntryCount() const { return m_netEntries.count(); }
  // Метод получения элемента сравнения цепи
  NetEntry *netEntry(int index) const { return m_netEntries[index]; }

private:
  // Вектор элементов сравнения цепей
  QVector<NetEntry *> m_netEntries;

  // Метод создания элемента сравнения цепи
  NetEntry *createNetEntry()
  { m_netEntries.append(new NetEntry); return m_netEntries.last(); }
  // Метод добавления элементов сравнения для двух заданных цепей
  void addComparison(Net *leftNet, Net *rightNet);
};

#endif // NETCMPMODEL_H

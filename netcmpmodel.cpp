#include "netcmpmodel.h"
#include "netlist.h"

#include <QColor>
#include <QDebug>

// Конструктор модели сравнения нетлистов
NetCmpModel::NetCmpModel(QObject *parent) :
  QAbstractItemModel(parent)
{
}

// Метод сравнения нетлистов
void NetCmpModel::compareNetlists(Netlist *leftNetlist,
                                  Netlist *rightNetlist)
{
  // Очистка предыдущих результатов сравнения
  clear();

  // Инициализация сброса модели (уведомляет связанные представления)
  beginResetModel();

  // Список цепей левого нетлиста
  QList<Net *> leftNets = leftNetlist->netMap().values();
  // Список цепей правого нетлиста
  QList<Net *> rightNets = rightNetlist->netMap().values();

  // Цикл определения максимально соответствующих цепей в нетлистах
  while (true)
  {
    // Указатели на соответствующие друг другу цепи
    Net *associationLeft;
    Net *associationRight;
    // Количество совпадающих узлов в цепях
    int associationScore = 0;

    // Цикл по цепям левого нетлиста
    foreach (auto net, leftNets)
    {
      // Количество совпадающих узлов для каждой цепи из правого
      // нетлиста
      QMap<Net *, int> currentScores;

      // Цикл по узлам текущей цепи левого нетлиста
      foreach (auto node, net->nodeMap().values())
      {
        // Проверка наличия узла в правом нетлисте
        if (!rightNetlist->nodeMap().contains(node->name())) continue;
        // Получение цепи правого нетлиста, в которой находится узел
        Net *supposedNet =
            rightNetlist->nodeMap()[node->name()]->net();

        // Проверка цепи правого нетлиста на наличие в списке
        // (отсутствие свидетельствует о том, что цепь уже была
        // использована как наиболее подходящая)
        if (rightNets.contains(supposedNet) &&
            // Увеличение количества совпадающих узлов для цепи
            // и сравнение с количеством совпадающих узлов
            // наиболее подходящей пары
            ++currentScores[supposedNet] > associationScore)
        {
          // В случае, если количество совпадающих узлов превышает
          // текущий максимум, текущая пара сохраняется как наиболее
          // подходящая, а максимум обновляется
          associationLeft = net;
          associationRight = supposedNet;
          associationScore = currentScores[supposedNet];
        }
      }
    }

    // Если соответствующие друг другу цепи были найдены
    if (associationScore)
    {
      // Удаление цепей из списков нетлистов
      leftNets.removeAll(associationLeft);
      rightNets.removeAll(associationRight);

      // Сравнение цепей и добавление результатов сравнения в модель
      addComparison(associationLeft, associationRight);
    }
    // Иначе цикл завершается
    else break;
  }

  // Обработка цепей, для которых не было найдено соответствий
  foreach (auto net, leftNets) addComparison(net, nullptr);
  foreach (auto net, rightNets) addComparison(nullptr, net);

  // Окончание сброса модели
  endResetModel();
}

// Метод добавления результатов сравнения двух цепей в модель
void NetCmpModel::addComparison(Net *leftNet, Net *rightNet)
{
  // Создание элемента сравнения цепи
  NetEntry *netEntry = createNetEntry();
  // Список узлов левой цепи
  QList<Node *> leftNodes;
  // Список узлов правой цепи
  QList<Node *> rightNodes;
  // Список имён узлов левой цепи
  QList<QString> leftNodeNames;
  // Список имён узлов правой цепи
  QList<QString> rightNodeNames;

  // Если левая цепь была задан
  if (leftNet)
  {
    // Копирование отображаемого имени в элемент сравнения цепи
    netEntry->setLeftDisplayName(leftNet->name());
    // Заполнение соответствующих списков
    leftNodes = leftNet->nodeMap().values();
    leftNodeNames = leftNet->nodeMap().keys();
  }
  // Аналогично с правой цепью
  if (rightNet)
  {
    netEntry->setRightDisplayName(rightNet->name());
    rightNodes = rightNet->nodeMap().values();
    rightNodeNames = rightNet->nodeMap().keys();
  }

  // Цикл по узлам обеих цепей
  for (auto leftIt = leftNodes.begin(), rightIt = rightNodes.begin();
       leftIt != leftNodes.end() || rightIt != rightNodes.end();)
  {
    // Создание элемента сравнения узла
    NodeEntry *nodeEntry = netEntry->createNodeEntry();

    // Проверка левого итератора (сравнение с концом списка)
    if (leftIt != leftNodes.end())
    {
      // Проверка правого итератора
      if (rightIt != rightNodes.end())
      {
        // Если в списке имён узлов правого итератора есть имя
        // левого узла
        if (rightNodeNames.contains((*leftIt)->name()))
        {
          // Если совпадают текущие имена
          if ((*leftIt)->name() == (*rightIt)->name())
          {
            // Заполнение информации о левом узле
            // Копирование унифицированного имени левого узла
            // в элемент сравнения узла
            nodeEntry->setLeftName((*leftIt)->name());
            // Копирование отображаемого имени левого узла
            // в элемент сравнения узла с последующим сдвигом
            // левого итератора
            nodeEntry->setLeftDisplayName((*leftIt++)->displayName());
          }
          // Заполнение информации о правом узле,
          // сдвиг правого итератора
          nodeEntry->setRightName((*rightIt)->name());
          nodeEntry->setRightDisplayName((*rightIt++)->displayName());
        }
        // Если в списке имён узлов правого итератора нет имени
        // левого узла
        else
        {
          // Заполнение информации о левом узле,
          // сдвиг левого итератора
          nodeEntry->setLeftName((*leftIt)->name());
          nodeEntry->setLeftDisplayName((*leftIt++)->displayName());
          // Если в списке имён узлов левого итератора нет имени
          // правого узла
          if (!leftNodeNames.contains((*rightIt)->name()))
          {
            // Заполнение информации о правом узле,
            // сдвиг правого итератора
            nodeEntry->setRightName((*rightIt)->name());
            nodeEntry->setRightDisplayName((*rightIt++)->displayName());
          }
        }
      }
      // Если правый итератор указывает на конец списка
      else
      {
        // Заполнение информации о левом узле,
        // сдвиг левого итератора
        nodeEntry->setLeftName((*leftIt)->name());
        nodeEntry->setLeftDisplayName((*leftIt++)->displayName());
      }
    }
    // Если левый итератор указывает на конец списка
    else
    {
      // Заполнение информации о правом узле,
      // сдвиг правого итератора
      nodeEntry->setRightName((*rightIt)->name());
      nodeEntry->setRightDisplayName((*rightIt++)->displayName());
    }
  }
}

// Метод очистки результатов сравнения
void NetCmpModel::clear()
{
  // Инициализация сброса модели
  beginResetModel();
  // Удаление всех элементов сравнения
  while (m_netEntries.count())
    delete m_netEntries.takeLast();
  // Окончание сброса модели
  endResetModel();
}

// Метод получения данных из модели
QVariant NetCmpModel::data(const QModelIndex &index, int role) const
{
  // Проверка индекса
  if (!index.isValid()) return QVariant();

  // Получение элемента сравнения из индекса
  CompareEntry *compareEntry =
      static_cast<CompareEntry *>(index.internalPointer());

  switch (role)
  {
    // Если запрашиваются данные для отображения
    case Qt::DisplayRole:
      // Возвращение отображаемого имени для запрашиваемой колонки
      if (index.column() == 0)
        return compareEntry->leftDisplayName();
      if (index.column() == 1)
        return compareEntry->rightDisplayName();
      break;
    // Если запрашиваются данные о цвете
    case Qt::ForegroundRole:
      // Возвращение красного цвета для несовпадающих элементов
      if (!compareEntry->isEqual())
        return QColor(255, 20, 20);
      break;
    // Если запрашиваются данные о декорации
    case Qt::DecorationRole:
      // Возвращение прозрачной декорации для элемента сравнения узлов
      // (визуально отображается как отступ)
      if (index.parent().isValid())
        return QColor(0, 0, 0, 0);
      break;
    // В остальных случаях возвращаются пустые данные
    default:
      break;
  }

  return QVariant();
}

// Метод получения флагов взаимодействия с элементами модели
Qt::ItemFlags NetCmpModel::flags(const QModelIndex &index) const
{
  // Проверка индекса
  if (!index.isValid()) return 0;
  // Возвращение флагов "включенный" и "выбираемый"
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

// Метод получения индекса для заданной строки, столбца и
// родительского индекса
QModelIndex NetCmpModel::index(int row,
                               int column,
                               const QModelIndex &parent) const
{
  // Проверка наличия запрашиваемого индекса в модели
  if (!hasIndex(row, column, parent)) return QModelIndex();

  // Если родительский индекс соответствует корневому индексу
  if (!parent.isValid())
  {
    // Если запрашиваемая строка не превышает количество элементов
    // сравнения цепи
    if (row < netEntryCount())
      // Возвращение индекса, созданного для соответствующего элемента
      // сравнения цепи
      return createIndex(row, column, netEntry(row));
  }
  // Если родительский не является корневым
  else
  {
    // Получение элемента сравнения из родительского индекса
    CompareEntry *compareEntry =
        static_cast<CompareEntry *>(parent.internalPointer());
    // Получение элемента сравнения цепи
    NetEntry *netEntry = dynamic_cast<NetEntry *>(compareEntry);
    // Если удалось получить элемент сравнения цепи, и запрашиваемая
    // строка не превышает количество узлов в полученном элементе, то
    // создаётся и возвращается индекс для соответствующего элемента
    // сравнения узла
    if (netEntry && row < netEntry->nodeEntryCount())
      return createIndex(row, column, netEntry->nodeEntry(row));
  }

  // Во всех остальных случаях возвращается корневой индекс
  return QModelIndex();
}

// Метод получения родительского индекса для заданного индекса
QModelIndex NetCmpModel::parent(const QModelIndex &child) const
{
  // Проверка заданного индекса
  if (!child.isValid()) return QModelIndex();

  // Получение элемента сравнения из заданного индекса
  CompareEntry *compareEntry =
      static_cast<CompareEntry *>(child.internalPointer());
  // Получение элемента сравнения узла
  NodeEntry *nodeEntry = dynamic_cast<NodeEntry *>(compareEntry);
  // Если удалось получить элемент сравнения узла (другими словами,
  // если заданный индекс соответствует элементу сравнения узла)
  if (nodeEntry) {
    // Поиск номера родительского элемента сравнения в цикле
    for (int row = 0; row < netEntryCount(); row++)
    {
      // Если элемент сравнения цепи с текущим номером равен
      // родительскому элементу сравнения узла, то создаётся и
      // возвращается индекс, соответствующий этому элементу
      if (netEntry(row) == nodeEntry->netEntry())
        return createIndex(row, 0, nodeEntry->netEntry());
    }
  }

  // Во всех остальных случаях возвращается корневой индекс
  return QModelIndex();
}

// Метод получения данных о заголовках
QVariant NetCmpModel::headerData(int, Qt::Orientation, int) const
{
  // В связи с тем, что заголовки не используются, возвращаются
  // пустые данные
  return QVariant();
}

// Метод получения количества строк модели
int NetCmpModel::rowCount(const QModelIndex &parent) const
{
  // Проверка индекса
  if (parent.column() > 0) return 0;
  // Количество строк коренного индекса соответствует количеству
  // элементов сравнения цепи
  if (!parent.isValid()) return netEntryCount();

  // Получение элемента сравнения из заданного индекса
  CompareEntry *compareEntry =
      static_cast<CompareEntry *>(parent.internalPointer());
  // Получение элемента сравнения цепи
  NetEntry *netEntry = dynamic_cast<NetEntry *>(compareEntry);
  // Если удалось получить элемент сравнения цепи, то возвращается
  // количество элементов сравнения узлов этой цепи
  if (netEntry) return netEntry->nodeEntryCount();

  // В остальных случаях возвращается нулевое количество строк
  return 0;
}

// Метод получения количества столбцов модели
int NetCmpModel::columnCount(const QModelIndex &) const
{
  // В модели всегда два столбца (для левого и правого нетлистов)
  return 2;
}

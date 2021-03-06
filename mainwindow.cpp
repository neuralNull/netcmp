#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "netlist.h"
#include "netcmpmodel.h"

#include <QDebug>
#include <QLineEdit>
#include <QFileDialog>
#include <QSettings>
#include <QDir>
#include <QPainter>
#include <QPrinter>
#include <QFontDialog>
#include <QPrintPreviewDialog>

// Конструктор главного окна программы
MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  // Интерфейс окна
  ui(new Ui::MainWindow),
  // Параметры работы
  m_settings(new QSettings("NPCAP", "netcmp", this))
{
  // Установка интерфейса
  ui->setupUi(this);

  // Создание нетлистов и привязка их к полям ввода
  m_netlists[ui->leftNetlist] = m_leftNetlist = new Netlist(this);
  m_netlists[ui->rightNetlist] = m_rightNetlist = new Netlist(this);

  // Создание модели сравнения нетлистов
  m_netCmpModel = new NetCmpModel(this);
  // Привязка модели к представлению сравнения
  ui->netCmpView->setModel(m_netCmpModel);
  // Задание режима изменения ширины колонок
  ui->netCmpView->header()->setSectionResizeMode(QHeaderView::Stretch);

  // Подключение сигнала поля ввода файла к слоту обработки нетлиста
  connect(ui->leftNetlist, &QLineEdit::returnPressed,
          this, &MainWindow::netlistReturnPressed);
  connect(ui->rightNetlist, &QLineEdit::returnPressed,
          this, &MainWindow::netlistReturnPressed);

  // Восстановление размера и положения окна через интерфейс настроек
  restoreGeometry(m_settings->value("geometry",
                                    saveGeometry()).toByteArray());
  // Восстановление последней открытой директории
  m_lastDir = m_settings->value("lastDir", QDir::homePath()).toString();
  // Восстановление флага отображения различий
  ui->showDifferenceOnly->setChecked(m_settings->value("showDifferenceOnly",
                                                       false).toBool());
  m_cmpFont.fromString(m_settings->value("cmpFont").toString());

  ui->netCmpView->setFont(m_cmpFont);
}

// Деструктор главного окна
MainWindow::~MainWindow()
{
  // Сохранение параметров работы
  m_settings->setValue("geometry", saveGeometry());
  m_settings->setValue("lastDir", m_lastDir);
  m_settings->setValue("showDifferenceOnly",
                       ui->showDifferenceOnly->isChecked());
  m_settings->setValue("cmpFont", m_cmpFont.toString());
  // Удаление элементов интерфейса окна
  delete ui;
}

// Слот нажатия на кнопку "Обзор" левого нетлиста
void MainWindow::on_leftBrowse_clicked()
{
  // Открытие обозревателя для соответствующего поля ввода
  browse(ui->leftNetlist);
}

// Слот нажатия на кнопку "Обзор" правого нетлиста
void MainWindow::on_rightBrowse_clicked()
{
  // Открытие обозревателя для соответствующего поля ввода
  browse(ui->rightNetlist);
}

void MainWindow::paintRequested(QPrinter *printer)
{
  QPainter painter;
  QFont font = m_cmpFont;
  int offset = 20;
  int lineHeight = QFontMetrics(font).height();
  int textPos = offset;

  painter.begin(printer);
  painter.setFont(font);

  int lineHalfWidth = painter.window().width()/2;

  for (int i = 0; i < m_netCmpModel->netEntryCount(); i++)
  {
    NetEntry *netEntry = m_netCmpModel->netEntry(i);
    QModelIndex parentIndex = m_netCmpModel->index(i, 0);

    if (ui->netCmpView->isRowHidden(i, QModelIndex())) continue;

    font.setUnderline(!netEntry->isEqual());
    painter.setFont(font);
    painter.setPen(netEntry->isEqual() ? QColor(0, 0, 0) : QColor(255, 20, 20));

    painter.drawText(QRect(0, textPos, lineHalfWidth/2, lineHeight*2), Qt::AlignRight, netEntry->leftDisplayName());
    painter.drawText(QRect(lineHalfWidth, textPos, lineHalfWidth/2, lineHeight*2), Qt::AlignRight, netEntry->rightDisplayName());

    textPos += lineHeight;
    if (textPos + lineHeight > painter.window().height() - offset)
    {
      textPos = offset;
      printer->newPage();
    }

    if (ui->netCmpView->isExpanded(parentIndex))
    {
      for (int j = 0; j < netEntry->nodeEntryCount(); j++)
      {
        NodeEntry *nodeEntry = netEntry->nodeEntry(j);

        if (ui->netCmpView->isRowHidden(j, parentIndex)) continue;

        font.setUnderline(!nodeEntry->isEqual());
        painter.setFont(font);
        painter.setPen(nodeEntry->isEqual() ? QColor(0, 0, 0) : QColor(255, 20, 20));

        painter.drawText(QRect(lineHalfWidth/2, textPos, lineHalfWidth/2, lineHeight*2), Qt::AlignLeft, nodeEntry->leftDisplayName());
        painter.drawText(QRect(3*lineHalfWidth/2, textPos, lineHalfWidth/2, lineHeight*2), Qt::AlignLeft, nodeEntry->rightDisplayName());

        textPos += lineHeight;
        if (textPos + lineHeight > painter.window().height() - offset)
        {
          textPos = offset;
          printer->newPage();
        }
      }
    }
  }
}

void MainWindow::on_font_clicked()
{
  QFontDialog fontDialog(m_cmpFont, this);

  if (fontDialog.exec() == QDialog::Accepted)
  {
    m_cmpFont = fontDialog.currentFont();
    ui->netCmpView->setFont(m_cmpFont);
  }
}

void MainWindow::on_print_clicked()
{
  QPrintPreviewDialog printPreviewDialog(this);

  connect(&printPreviewDialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(paintRequested(QPrinter*)));
  printPreviewDialog.exec();
}

// Открытие обозревателя для поля ввода
void MainWindow::browse(QLineEdit *edit)
{
  // Открытие диалога выбора файла
  QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Открыть нетлист"),
                                                  m_lastDir,
                                                  tr("Нетлист") +
                                                  " (*.NET)");
  // Проверка пути файла
  if (filename.isNull()) return;
  // Сохранение директории файла
  m_lastDir = QFileInfo(filename).absoluteDir().path();
  // Задание имени файла в поле ввода
  edit->setText(filename);
  // Отправка сигнала нажатия клавиши "Enter" от лица поля ввода
  emit edit->returnPressed();
}

// Слот обработки нажатия клавиши "Enter" полей ввода
void MainWindow::netlistReturnPressed()
{
  // Получение отправителя сигнала
  QLineEdit *edit = dynamic_cast<QLineEdit *>(sender());
  // Проверка отправителя
  if (!edit) return;
  // Получение нетлиста, соответствующего полю ввода
  Netlist *netlist = m_netlists[edit];

  // Загрузка нетлиста из заданного файла
  netlist->readFromFile(edit->text());

  // В случае, если содержимое обоих нетлистов не является пустым,
  // происходит их обработка
  if (!m_leftNetlist->isEmpty() && !m_rightNetlist->isEmpty())
  {
    // Сравнение нетлистов
    m_netCmpModel->compareNetlists(m_leftNetlist, m_rightNetlist);
    // Раскрытие элементов сравнения цепей, в которых имеются
    // несоответствия
    for (int i = 0; i < m_netCmpModel->netEntryCount(); i++)
    {
      NetEntry *netEntry = m_netCmpModel->netEntry(i);
      QModelIndex index = m_netCmpModel->index(i, 0);
      if (!netEntry->isEqual()) ui->netCmpView->expand(index);
    }
    // Применение фильтра совпадающих элементов
    on_showDifferenceOnly_toggled(ui->showDifferenceOnly->isChecked());
  }
}

// Слот переключения флага, отвечающего за отображение
// совпадающих элементов
void MainWindow::on_showDifferenceOnly_toggled(bool checked)
{
  // Цикл по элементам сравнения цепей
  for (int i = 0; i < m_netCmpModel->netEntryCount(); i++)
  {
    NetEntry *netEntry = m_netCmpModel->netEntry(i);
    // Проверка наличия различающихся узлов
    if (!netEntry->isEqual())
    {
      // Цикл по элементам сравнения узлов цепи
      for (int j = 0; j < netEntry->nodeEntryCount(); j++)
      {
        NodeEntry *nodeEntry = netEntry->nodeEntry(j);
        // Проверка узлов на совпадение
        if (nodeEntry->isEqual())
        {
          QModelIndex parentIndex = m_netCmpModel->index(i, 0);
          // Применение флага к элементу сравнения узла
          ui->netCmpView->setRowHidden(j, parentIndex, checked);
        }
      }
    }
    // Применение флага к элементу сравнения цепи
    else ui->netCmpView->setRowHidden(i, QModelIndex(), checked);
  }
}

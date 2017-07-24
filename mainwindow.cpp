#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "netlist.h"
#include "netcmpmodel.h"

#include <QDebug>
#include <QLineEdit>
#include <QFileDialog>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_netlists[ui->leftNetlist] = m_leftNetlist = new Netlist(this);
  m_netlists[ui->rightNetlist] = m_rightNetlist = new Netlist(this);

  m_netCmpModel = new NetCmpModel(this);
  ui->netCmpView->setModel(m_netCmpModel);
  ui->netCmpView->header()->setSectionResizeMode(QHeaderView::Stretch);

  connect(ui->leftNetlist, &QLineEdit::returnPressed,
          this, &MainWindow::netlistReturnPressed);
  connect(ui->rightNetlist, &QLineEdit::returnPressed,
          this, &MainWindow::netlistReturnPressed);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_leftBrowse_clicked()
{
  browse(ui->leftNetlist);
}

void MainWindow::on_rightBrowse_clicked()
{
  browse(ui->rightNetlist);
}

void MainWindow::browse(QLineEdit *edit)
{
  QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Открыть нетлист"),
                                                  edit->text(),
                                                  tr("Нетлист") +
                                                  " (*.net *.NET *.Net)");
  if (filename.isNull()) return;
  edit->setText(filename);
  emit edit->returnPressed();
}

void MainWindow::netlistReturnPressed()
{
  QLineEdit *edit = dynamic_cast<QLineEdit *>(sender());
  if (!edit) return;
  Netlist *netlist = m_netlists[edit];

  netlist->readFromFile(edit->text());

  if (!m_leftNetlist->isEmpty() && !m_rightNetlist->isEmpty())
  {
    m_netCmpModel->compareNetlists(m_leftNetlist, m_rightNetlist);
    for (int i = 0; i < m_netCmpModel->netEntryCount(); i++)
    {
      NetEntry *netEntry = m_netCmpModel->netEntry(i);
      QModelIndex index = m_netCmpModel->index(i, 0);
      if (!netEntry->isEqual()) ui->netCmpView->expand(index);
    }
    on_showDifferenceOnly_toggled(ui->showDifferenceOnly->isChecked());
  }
}

void MainWindow::on_showDifferenceOnly_toggled(bool checked)
{
  for (int i = 0; i < m_netCmpModel->netEntryCount(); i++)
  {
    NetEntry *netEntry = m_netCmpModel->netEntry(i);
    if (!netEntry->isEqual())
    {
      for (int j = 0; j < netEntry->nodeEntryCount(); j++)
      {
        NodeEntry *nodeEntry = netEntry->nodeEntry(j);
        if (nodeEntry->isEqual())
        {
          QModelIndex parentIndex = m_netCmpModel->index(i, 0);
          ui->netCmpView->setRowHidden(j, parentIndex, checked);
        }
      }
    }
    else ui->netCmpView->setRowHidden(i, QModelIndex(), checked);
  }
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QLineEdit>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_firstBrowse_clicked()
{
  browse(ui->firstNetlist);
}

void MainWindow::on_secondBrowse_clicked()
{
  browse(ui->secondNetlist);
}

void MainWindow::browse(QLineEdit *edit)
{
  QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Open netlist"),
                                                  edit->text(),
                                                  tr("Netlist") +
                                                  "(*.net)");
  if (filename.isEmpty()) return;
  edit->setText(filename);
}

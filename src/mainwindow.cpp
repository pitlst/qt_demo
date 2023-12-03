#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QVariant"
#include "QVariantList"
#include "QDir"
#include "QFileDialog"
#include "QMessageBox"
#include "QVector"
#include "QTime"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    main_graph = new DataGroup::operation_graph();
    connect(main_graph, &DataGroup::operation_graph::send_str, this, &MainWindow::show_ui);
    connect(this, &MainWindow::send_str, main_graph, &DataGroup::operation_graph::get_str);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_toolButton_clicked()
{
    ui->textBrowser->append(QString("注意，检查前需要关闭excel，否则会导致写入失败"));
    QString fileName = QFileDialog::getOpenFileName(this,tr("open a file."),"./",tr("excel files(*.xls *.xlsx);;All files(*.*)"));
    if (fileName.isEmpty())
    {
        ui->textBrowser->append(QString("打开文件失败，没有找到文件"));
        QApplication::processEvents();
    }
    else
    {
        // 保存选择的路径
        this->file_path = fileName;
        QString append_text = "已选择文件: " + this->file_path;
        ui->textBrowser->append(append_text);
        ui->textEdit->setText(this->file_path);
        QApplication::processEvents();
    }
    ui->textBrowser->append(this->file_path);
}

void MainWindow::show_ui(const std::string & input_str)
{
    ui->textBrowser->append(QString::fromStdString(input_str));
    QApplication::processEvents();
}

void MainWindow::on_pushButton_clicked()
{
    this->file_path = ui->textEdit->toPlainText();
    emit this->send_str(this->file_path.toStdString());
    main_graph->start();
}


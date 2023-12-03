#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "QMainWindow"
#include "QFile"
#include "QApplication"
#include "QThread"

#include "line_process.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

// 主窗口类
class MainWindow : public QMainWindow
{
Q_OBJECT
public:
    DataGroup::operation_graph * main_graph;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void send_str(const std::string & input_str);

public slots:
    void show_ui(const std::string & input_str);

private slots:
    void on_toolButton_clicked();
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QString file_path;   // 读取文件的路径
};

// qss加载
class CommonHelper
{
public:
    static void setStyle(const QString &style)
    {
        QFile qss(style);
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }
};

#endif // MAINWINDOW_H

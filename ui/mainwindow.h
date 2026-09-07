#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

#include "src/hospital.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 主窗：使用 Windows 原生窗口装饰（系统标题栏/最小化最大化关闭）
class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    Hospital& hospital();

private slots:
    void onNavChanged(int index);

private:
    void buildPages();

private:
    Ui::MainWindow* ui;
    Hospital        hospital_;
};

#endif // MAINWINDOW_H

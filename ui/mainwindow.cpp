#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "theme.h"

#include <QFile>
#include <QLabel>
#include <QListWidgetItem>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent), ui(new Ui::MainWindow), hospital_()
{
    ui->setupUi(this);

    setWindowTitle(Theme::AppTitle);
    resize(1080, 680);

    // 侧栏导航
    const QStringList nav = { QStringLiteral("首页概览"), QStringLiteral("医生管理"),
                              QStringLiteral("预约挂号"), QStringLiteral("预约查询") };
    for (const QString& text : nav) {
        auto* item = new QListWidgetItem(text);
        item->setSizeHint(QSize(0, 44));
        ui->navList->addItem(item);
    }
    ui->navList->setCurrentRow(0);

    buildPages();

    connect(ui->navList, &QListWidget::currentRowChanged, this, &MainWindow::onNavChanged);

    QFile qss(":/style.qss");
    if (qss.open(QIODevice::ReadOnly)) {
        setStyleSheet(QString::fromUtf8(qss.readAll()));
    }

    // 启动即自动加载磁盘数据（保存由各页面每次增删改后调用）
    hospital_.load();
}

MainWindow::~MainWindow()
{
    delete ui;
}

Hospital& MainWindow::hospital()
{
    return hospital_;
}

void MainWindow::buildPages()
{
    // S2 暂用占位页，后续环节替换为真实页面（保持与侧栏顺序一致）
    ui->contentStack->addWidget(makePlaceholderPage(QStringLiteral("首页概览")));
    ui->contentStack->addWidget(makePlaceholderPage(QStringLiteral("医生管理")));
    ui->contentStack->addWidget(makePlaceholderPage(QStringLiteral("预约挂号")));
    ui->contentStack->addWidget(makePlaceholderPage(QStringLiteral("预约查询")));
    ui->contentStack->setCurrentIndex(0);
}

QWidget* MainWindow::makePlaceholderPage(const QString& text)
{
    auto* page = new QWidget;
    auto* lay  = new QVBoxLayout(page);
    lay->setContentsMargins(24, 20, 24, 20);
    auto* label = new QLabel(text, page);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QStringLiteral("font-size:22px; font-weight:600; color:#9AA0A6;"));
    lay->addWidget(label);
    return page;
}

void MainWindow::onNavChanged(int index)
{
    if (index >= 0 && index < ui->contentStack->count())
        ui->contentStack->setCurrentIndex(index);
}

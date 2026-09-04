#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "theme.h"

#include <QFile>
#include <QLabel>
#include <QListWidgetItem>
#include <QMetaObject>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include "pages/doctorspage.h"
#include "pages/registerpage.h"

namespace {

// 唯一例外：Qt 不允许 QSS 对 ::item 设置字重（font-weight 被静默忽略），
// 选中加粗只能在此改 option.font。其余视觉一律在 resources/style.qss。
class NavItemDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        if (option->state.testFlag(QStyle::State_Selected))
            option->font.setBold(true);
    }
};

} // namespace

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
        // 行高由 style.qss 的 #navList::item 决定，勿在此 setSizeHint 覆盖
        ui->navList->addItem(item);
    }
    ui->navList->setCurrentRow(0);
    ui->navList->setItemDelegate(new NavItemDelegate(ui->navList));

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
    // 顺序与侧栏一致：首页概览 / 医生管理 / 预约挂号 / 预约查询
    ui->contentStack->addWidget(makePlaceholderPage(QStringLiteral("首页概览")));

    auto* doctorsPage = new DoctorsPage(hospital_, this);
    ui->contentStack->addWidget(doctorsPage); // index 1

    auto* registerPage = new RegisterPage(hospital_, this);
    ui->contentStack->addWidget(registerPage); // index 2
    ui->contentStack->addWidget(makePlaceholderPage(QStringLiteral("预约查询")));
    connect(doctorsPage, &DoctorsPage::requestRegisterAppointment, this,
            [this, registerPage](const QString& doctorId) {
                ui->navList->setCurrentRow(2);
                registerPage->selectDoctor(doctorId);
            });

    ui->contentStack->setCurrentIndex(0);
}

QWidget* MainWindow::makePlaceholderPage(const QString& text)
{
    auto* page = new QWidget;
    auto* lay  = new QVBoxLayout(page);
    lay->setContentsMargins(24, 20, 24, 20);
    auto* label = new QLabel(text, page);
    label->setAlignment(Qt::AlignCenter);
    label->setProperty("placeholder", true); // 样式见 style.qss 的 QLabel[placeholder="true"]
    lay->addWidget(label);
    return page;
}

void MainWindow::onNavChanged(int index)
{
    if (index >= 0 && index < ui->contentStack->count()) {
        ui->contentStack->setCurrentIndex(index);

        // 切页后调用页面自身的 refresh()（占位页没有该方法则忽略）
        if (QWidget* page = ui->contentStack->currentWidget()) {
            QMetaObject::invokeMethod(page, "refresh", Qt::DirectConnection);
        }
    }
}

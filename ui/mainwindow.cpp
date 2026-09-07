#include "mainwindow.h"
#include "theme.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QListWidgetItem>
#include <QMetaObject>
#include <QStyle>
#include <QStyledItemDelegate>

#include "pages/doctorspage.h"
#include "pages/dashboardpage.h"
#include "pages/registerpage.h"

#include "pages/appointmentspage.h"

namespace {

// 唯一例外：Qt 不允许 QSS 对 ::item 设置字重（font-weight 被静默忽略），
// 选中加粗只能在此改 option.font。其余视觉一律在 resources/style.qss。
class NavItemDelegate : public QStyledItemDelegate {
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
    resize(1200, 680);

    // 侧栏导航
    const QStringList nav = {QStringLiteral("首页概览"), QStringLiteral("预约挂号"),
                             QStringLiteral("预约查询"), QStringLiteral("医生管理")};
    for (const QString& text : nav) {
        auto* item = new QListWidgetItem(text);
        // 行高由 style.qss 的 #navList::item 决定，勿在此 setSizeHint 覆盖
        ui->navList->addItem(item);
    }
    ui->navList->setCurrentRow(0);
    ui->navList->setItemDelegate(new NavItemDelegate(ui->navList));

    // 页面首次构造和刷新前先加载数据，避免页面初始状态为空。
    hospital_.load();
    buildPages();

    connect(ui->navList, &QListWidget::currentRowChanged, this, &MainWindow::onNavChanged);

    QFile qss(":/style.qss");
    if (qss.open(QIODevice::ReadOnly)) {
        setStyleSheet(QString::fromUtf8(qss.readAll()));
    }
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
    // 顺序与侧栏一致：首页概览 / 预约挂号 / 预约查询 / 医生管理
    auto* dashboardPage = new DashboardPage(hospital_, this);
    ui->contentStack->addWidget(dashboardPage); // index 0

    auto* registerPage = new RegisterPage(hospital_, this);
    ui->contentStack->addWidget(registerPage); // index 1

    auto* appointmentsPage = new AppointmentsPage(hospital_, this);
    ui->contentStack->addWidget(appointmentsPage); // index 2

    auto* doctorsPage = new DoctorsPage(hospital_, this);
    ui->contentStack->addWidget(doctorsPage); // index 3
    connect(dashboardPage, &DashboardPage::navigateRequested, this,
            [this](int pageIndex) { ui->navList->setCurrentRow(pageIndex); });
    connect(doctorsPage, &DoctorsPage::requestRegisterAppointment, this,
            [this, registerPage](const QString& doctorId) {
                ui->navList->setCurrentRow(1);
                registerPage->selectDoctor(doctorId);
            });

    ui->contentStack->setCurrentIndex(0);
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

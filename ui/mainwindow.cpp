#include "mainwindow.h"
#include "theme.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
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
    connect(ui->btnImportData, &QPushButton::clicked, this, &MainWindow::importData);
    connect(ui->btnExportData, &QPushButton::clicked, this, &MainWindow::exportData);

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

void MainWindow::refreshPages()
{
    for (int index = 0; index < ui->contentStack->count(); ++index) {
        if (QWidget* page = ui->contentStack->widget(index))
            QMetaObject::invokeMethod(page, "refresh", Qt::DirectConnection);
    }
}

void MainWindow::importData()
{
    const QString doctorsPath = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择医生数据文件"), {},
        QStringLiteral("医生数据 (doctors.dat *.dat);;所有文件 (*)"));
    if (doctorsPath.isEmpty())
        return;

    const QString appointmentsPath = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择预约信息文件"), QFileInfo(doctorsPath).absolutePath(),
        QStringLiteral("预约数据 (appointments.dat *.dat);;所有文件 (*)"));
    if (appointmentsPath.isEmpty())
        return;

    if (QMessageBox::question(
            this, QStringLiteral("确认导入"),
            QStringLiteral("导入后将替换当前医生和预约数据，是否继续？"))
        != QMessageBox::Yes) {
        return;
    }

    if (!hospital_.importData(doctorsPath, appointmentsPath)) {
        QMessageBox::warning(this, QStringLiteral("导入失败"),
                             QStringLiteral("数据文件无法读取或保存，请确认选择了正确的医生和预约文件。"));
        return;
    }

    refreshPages();
    QMessageBox::information(
        this, QStringLiteral("导入成功"),
        QStringLiteral("已导入 %1 位医生和 %2 条预约。")
            .arg(hospital_.getDoctors().size())
            .arg(hospital_.getAppointments().size()));
}

void MainWindow::exportData()
{
    const QString outputDirectory = QFileDialog::getExistingDirectory(
        this, QStringLiteral("选择数据导出文件夹"));
    if (outputDirectory.isEmpty())
        return;

    const QDir outputDir(outputDirectory);
    const bool doctorsExist = QFile::exists(outputDir.filePath(QStringLiteral("doctors.dat")));
    const bool appointmentsExist =
        QFile::exists(outputDir.filePath(QStringLiteral("appointments.dat")));
    if (doctorsExist || appointmentsExist) {
        QStringList existingFiles;
        if (doctorsExist)
            existingFiles.append(QStringLiteral("doctors.dat"));
        if (appointmentsExist)
            existingFiles.append(QStringLiteral("appointments.dat"));
        if (QMessageBox::question(
                this, QStringLiteral("确认覆盖"),
                QStringLiteral("目标文件夹已存在以下文件：\n%1\n\n选择“是”将覆盖原文件，是否继续？")
                    .arg(existingFiles.join(QLatin1Char('\n'))),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes) {
            return;
        }
    }

    if (!hospital_.exportData(outputDirectory)) {
        QMessageBox::warning(this, QStringLiteral("导出失败"),
                             QStringLiteral("无法写入所选文件夹。"));
        return;
    }

    QMessageBox::information(
        this, QStringLiteral("导出成功"),
        QStringLiteral("医生和预约数据已导出到：\n%1").arg(QDir::toNativeSeparators(outputDirectory)));
}

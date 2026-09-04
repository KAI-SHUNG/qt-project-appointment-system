#include "doctorspage.h"
#include "ui_doctorspage.h"
#include "../datadefs.h"
#include "../theme.h"
#include "../dialogs/doctordialog.h"
#include "../dialogs/timeslotdialog.h"
#include "../persistence.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidgetItem>
#include <stdexcept>

DoctorsPage::DoctorsPage(Hospital& hospital, QWidget* parent)
    : QWidget(parent), ui(new Ui::DoctorsPage), hospital_(hospital)
{
    ui->setupUi(this);

    ui->btnAdd->setProperty("primary", true);
    ui->tblDoctors->verticalHeader()->setVisible(false);
    ui->tblDoctors->setRowHeight(0, Theme::TableRowHeight);
    ui->tblDoctors->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblDoctors->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
    ui->tblDoctors->horizontalHeader()->resizeSection(7, Theme::ActionsColumnWidth);
    ui->tblDoctors->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->btnAdd, &QPushButton::clicked, this, &DoctorsPage::onAdd);
    connect(ui->btnSearch, &QPushButton::clicked, this, &DoctorsPage::onSearch);
    connect(ui->btnReset, &QPushButton::clicked, this, &DoctorsPage::onReset);
    connect(ui->edtSearch, &QLineEdit::returnPressed, this, &DoctorsPage::onSearch);
    connect(ui->cmbDept, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DoctorsPage::refresh);
    connect(ui->tblDoctors, &QTableWidget::cellDoubleClicked, this,
            [this](int row, int) {
                if (row >= 0) {
                    ui->tblDoctors->selectRow(row);
                    editDoctor(currentDoctorId());
                }
            });

    fillDeptFilter();
    refresh();
}

DoctorsPage::~DoctorsPage()
{
    delete ui;
}

void DoctorsPage::refresh()
{
    fillDeptFilter();
    populateTable();
}

QString DoctorsPage::currentDoctorId() const
{
    const int row = ui->tblDoctors->currentRow();
    if (row < 0)
        return {};
    QTableWidgetItem* item = ui->tblDoctors->item(row, 0);
    return item ? item->data(Qt::UserRole).toString() : QString();
}

QString DoctorsPage::nextFreeDoctorId() const
{
    for (int i = 1; i <= 9999; ++i) {
        const QString id = QStringLiteral("D%1").arg(i, 3, 10, QLatin1Char('0'));
        if (hospital_.findDoctor(id) == nullptr)
            return id;
    }
    return QStringLiteral("D9999");
}

void DoctorsPage::fillDeptFilter()
{
    const QString cur = ui->cmbDept->currentText();

    ui->cmbDept->blockSignals(true);
    ui->cmbDept->clear();
    ui->cmbDept->addItem(QStringLiteral("全部科室"));

    QStringList depts = AppData::departments();
    for (const Doctor& d : hospital_.getDoctors()) {
        if (!depts.contains(d.getDepartment()) && !d.getDepartment().isEmpty())
            depts << d.getDepartment();
    }
    ui->cmbDept->addItems(depts);

    const int idx = ui->cmbDept->findText(cur);
    ui->cmbDept->setCurrentIndex(idx >= 0 ? idx : 0);
    ui->cmbDept->blockSignals(false);
}

void DoctorsPage::populateTable()
{
    const QString keyword = ui->edtSearch->text().trimmed();
    const QString dept    = ui->cmbDept->currentText();

    QList<const Doctor*> matched;
    for (const Doctor& d : hospital_.getDoctors()) {
        if (dept != QStringLiteral("全部科室") && d.getDepartment() != dept)
            continue;
        if (!keyword.isEmpty() && !d.getName().contains(keyword, Qt::CaseInsensitive)
            && !d.getDoctorId().contains(keyword, Qt::CaseInsensitive)
            && !d.getDepartment().contains(keyword, Qt::CaseInsensitive)) {
            continue;
        }
        matched.append(&d);
    }

    ui->tblDoctors->clearContents();
    ui->tblDoctors->setRowCount(matched.size());

    for (int r = 0; r < matched.size(); ++r) {
        const Doctor& d = *matched.at(r);

        QStringList texts = { d.getDoctorId(), d.getName(),
                              d.getGender() == Human::Gender::Male ? QStringLiteral("男")
                                                                   : QStringLiteral("女"),
                              QString::number(d.getAge()),
                              d.getTitle(),
                              d.getDepartment(),
                              QStringLiteral("%1 个时段").arg(d.getSchedule().size()) };

        for (int c = 0; c < 7; ++c) {
            auto* item = new QTableWidgetItem(texts.at(c));
            item->setTextAlignment(Qt::AlignCenter);
            if (c == 0)
                item->setData(Qt::UserRole, d.getDoctorId());
            if (c == 6 && !d.getSchedule().isEmpty())
                item->setToolTip(scheduleText(d));
            ui->tblDoctors->setItem(r, c, item);
        }
        addRowActions(r, d.getDoctorId());
    }

    ui->lblCount->setText(QStringLiteral("共 %1 位专家").arg(matched.size()));
}

QString DoctorsPage::scheduleText(const Doctor& doctor) const
{
    QStringList lines;
    for (const Timeslot& slot : doctor.getSchedule()) {
        const int dayIndex = static_cast<int>(slot.getDayOfWeek()) - 1;
        lines << QStringLiteral("%1 %2-%3（容量 %4）")
                     .arg(AppData::weekdays().value(dayIndex),
                          slot.getStartTime().toString(QStringLiteral("HH:mm")),
                          slot.getEndTime().toString(QStringLiteral("HH:mm")))
                     .arg(slot.getCapability());
    }
    return lines.join(QStringLiteral("\n"));
}

void DoctorsPage::addRowActions(int row, const QString& doctorId)
{
    auto* container = new QWidget(ui->tblDoctors);
    auto* lay       = new QHBoxLayout(container);
    lay->setContentsMargins(4, 0, 4, 0);
    lay->setSpacing(2);

    for (const QString& label :
         { QStringLiteral("排班"), QStringLiteral("编辑"), QStringLiteral("删除"),
           QStringLiteral("预约") }) {
        auto* btn = new QPushButton(label, container);
        btn->setProperty("link", true);
        btn->setCursor(Qt::PointingHandCursor);
        lay->addWidget(btn);

        if (label == QStringLiteral("排班"))
            connect(btn, &QPushButton::clicked, this,
                    [this, doctorId] { scheduleDoctor(doctorId); });
        else if (label == QStringLiteral("编辑"))
            connect(btn, &QPushButton::clicked, this,
                    [this, doctorId] { editDoctor(doctorId); });
        else if (label == QStringLiteral("删除"))
            connect(btn, &QPushButton::clicked, this,
                    [this, doctorId] { deleteDoctor(doctorId); });
        else
            connect(btn, &QPushButton::clicked, this,
                    [this, doctorId] { viewDoctorAppointments(doctorId); });
    }
    ui->tblDoctors->setCellWidget(row, 7, container);
}

void DoctorsPage::onAdd()
{
    DoctorDialog dlg(this);
    dlg.setSuggestedId(nextFreeDoctorId());

    if (dlg.exec() != QDialog::Accepted)
        return;

    const Doctor doctor = dlg.doctor();
    try {
        hospital_.addDoctor(doctor);
    }
    catch (const std::exception& e) {
        QMessageBox::warning(this, QStringLiteral("新增专家"),
                             QString::fromUtf8(e.what()));
        return;
    }

    if (persistHospital(hospital_, this))
        refresh();
}

void DoctorsPage::onSearch()
{
    populateTable();
}

void DoctorsPage::onReset()
{
    ui->edtSearch->clear();
    ui->cmbDept->setCurrentIndex(0);
    populateTable();
}

void DoctorsPage::editDoctor(const QString& doctorId)
{
    Doctor* doctor = hospital_.findDoctor(doctorId);
    if (doctor == nullptr)
        return;

    DoctorDialog dlg(this);
    dlg.setDoctor(*doctor);
    if (dlg.exec() != QDialog::Accepted)
        return;

    const Doctor fresh = dlg.doctor();
    doctor->setName(fresh.getName());
    doctor->setAge(fresh.getAge());
    doctor->setGender(fresh.getGender());
    doctor->setTitle(fresh.getTitle());
    doctor->setDepartment(fresh.getDepartment());

    if (persistHospital(hospital_, this))
        refresh();
}

void DoctorsPage::deleteDoctor(const QString& doctorId)
{
    Doctor* doctor = hospital_.findDoctor(doctorId);
    if (doctor == nullptr)
        return;

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, QStringLiteral("删除专家"),
        QStringLiteral("确定删除专家 %1（%2）吗？\n该医生的全部预约也会一并删除。")
            .arg(doctor->getName(), doctor->getDoctorId()));
    if (answer != QMessageBox::Yes)
        return;

    hospital_.removeDoctor(doctorId);
    if (persistHospital(hospital_, this))
        refresh();
}

void DoctorsPage::scheduleDoctor(const QString& doctorId)
{
    Doctor* doctor = hospital_.findDoctor(doctorId);
    if (doctor == nullptr)
        return;

    TimeslotDialog dlg(*doctor, this);
    if (dlg.exec() == QDialog::Accepted && persistHospital(hospital_, this))
        refresh();
}

void DoctorsPage::viewDoctorAppointments(const QString& doctorId)
{
    if (doctorId.isEmpty())
        return;
    emit requestViewAppointments(doctorId);
}
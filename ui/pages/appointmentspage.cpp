#include "appointmentspage.h"
#include "ui_appointmentspage.h"

#include "../dialogs/appointmentdialog.h"
#include "../persistence.h"
#include "../theme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

AppointmentsPage::AppointmentsPage(Hospital& hospital, QWidget* parent)
    : QWidget(parent), ui(new Ui::AppointmentsPage), hospital_(hospital)
{
    ui->setupUi(this);
    ui->appointmentsTable->verticalHeader()->setVisible(false);
    ui->appointmentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->appointmentsTable->setColumnHidden(6, true);
    ui->appointmentsTable->setColumnHidden(9, true);

    connect(ui->btnSearch, &QPushButton::clicked, this, &AppointmentsPage::populateTable);
    connect(ui->btnReset, &QPushButton::clicked, this, &AppointmentsPage::resetFilters);
    connect(ui->edtKeyword, &QLineEdit::returnPressed, this, &AppointmentsPage::populateTable);
    connect(ui->cmbDoctor, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AppointmentsPage::populateTable);
    connect(ui->chkCompleted, &QCheckBox::toggled, this, &AppointmentsPage::populateTable);
    connect(ui->appointmentsTable, &QTableWidget::cellDoubleClicked, this,
            [this](int row, int) {
                if (row >= 0 && ui->appointmentsTable->item(row, 0))
                    showDetails(ui->appointmentsTable->item(row, 0)->data(Qt::UserRole).toString());
            });
    refresh();
}

AppointmentsPage::~AppointmentsPage()
{
    delete ui;
}

void AppointmentsPage::refresh()
{
    fillDoctors();
    populateTable();
}

void AppointmentsPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    QTimer::singleShot(0, this, &AppointmentsPage::adjustColumnWidths);
}

void AppointmentsPage::selectDoctor(const QString& doctorId)
{
    fillDoctors();
    const int index = ui->cmbDoctor->findData(doctorId);
    ui->cmbDoctor->setCurrentIndex(index >= 0 ? index : 0);
    populateTable();
}

void AppointmentsPage::fillDoctors()
{
    const QString selectedId = ui->cmbDoctor->currentData().toString();
    ui->cmbDoctor->blockSignals(true);
    ui->cmbDoctor->clear();
    ui->cmbDoctor->addItem(QStringLiteral("全部医生"), QString());
    for (const Doctor& doctor : hospital_.getDoctors()) {
        ui->cmbDoctor->addItem(
            QStringLiteral("%1 · %2 · %3")
                .arg(doctor.getDoctorId(), doctor.getName(), doctor.getDepartment()),
            doctor.getDoctorId());
    }
    const int index = ui->cmbDoctor->findData(selectedId);
    ui->cmbDoctor->setCurrentIndex(index >= 0 ? index : 0);
    ui->cmbDoctor->blockSignals(false);
}

void AppointmentsPage::populateTable()
{
    const QString doctorId = ui->cmbDoctor->currentData().toString();
    const QString keyword = ui->edtKeyword->text().trimmed();
    const QDateTime now = QDateTime::currentDateTime();

    QList<const Appointment*> matched;
    for (const Appointment& appointment : hospital_.getAppointments()) {
        if (!doctorId.isEmpty() && appointment.getDoctorId() != doctorId)
            continue;
        if (!ui->chkCompleted->isChecked() && appointment.hasEnded(now))
            continue;

        const Patient patient = appointment.getPatient();
        if (!keyword.isEmpty()
            && !appointment.getAppointId().contains(keyword, Qt::CaseInsensitive)
            && !patient.getName().contains(keyword, Qt::CaseInsensitive)
            && !patient.getPatientId().contains(keyword, Qt::CaseInsensitive)) {
            continue;
        }
        matched.append(&appointment);
    }

    std::sort(matched.begin(), matched.end(), [this](const Appointment* left,
                                                      const Appointment* right) {
        if (left->getDate() != right->getDate())
            return left->getDate() < right->getDate();
        if (left->getTimeslot().getStartTime() != right->getTimeslot().getStartTime())
            return left->getTimeslot().getStartTime() < right->getTimeslot().getStartTime();
        const Doctor* leftDoctor = hospital_.findDoctor(left->getDoctorId());
        const Doctor* rightDoctor = hospital_.findDoctor(right->getDoctorId());
        const QString leftName = leftDoctor ? leftDoctor->getName() : left->getDoctorId();
        const QString rightName = rightDoctor ? rightDoctor->getName() : right->getDoctorId();
        const int doctorOrder = QString::localeAwareCompare(leftName, rightName);
        if (doctorOrder != 0)
            return doctorOrder < 0;
        return left->getAppointId() < right->getAppointId();
    });

    ui->appointmentsTable->clearContents();
    ui->appointmentsTable->setRowCount(matched.size());
    for (int row = 0; row < matched.size(); ++row) {
        const Appointment& appointment = *matched.at(row);
        const Patient patient = appointment.getPatient();
        const Doctor* doctor = hospital_.findDoctor(appointment.getDoctorId());
        const Timeslot slot = appointment.getTimeslot();

        const QStringList values = {
            appointment.getAppointId(),
            doctor ? doctor->getName() : appointment.getDoctorId(),
            doctor ? doctor->getDepartment() : QStringLiteral("未知"),
            patient.getName(),
            patient.getGender() == Human::Gender::Male ? QStringLiteral("男") : QStringLiteral("女"),
            QString::number(patient.getAge()),
            patient.getPatientId(),
            appointment.getDate().toString(QStringLiteral("yyyy-MM-dd")),
            QStringLiteral("%1-%2").arg(slot.getStartTime().toString(QStringLiteral("HH:mm")),
                                        slot.getEndTime().toString(QStringLiteral("HH:mm"))),
            patient.getPhoneNumber()
        };

        for (int column = 0; column < values.size(); ++column) {
            auto* item = new QTableWidgetItem(values.at(column));
            item->setTextAlignment(Qt::AlignCenter);
            if (column == 0) {
                item->setData(Qt::UserRole, appointment.getAppointId());
                item->setToolTip(appointment.getAppointId());
            }
            ui->appointmentsTable->setItem(row, column, item);
        }
        ui->appointmentsTable->setRowHeight(row, Theme::TableRowHeight);
        addRowActions(row, appointment.getAppointId(), appointment.hasEnded(now));
    }
    ui->lblCount->setText(QStringLiteral("共 %1 条预约").arg(matched.size()));
    QTimer::singleShot(0, this, &AppointmentsPage::adjustColumnWidths);
}

void AppointmentsPage::adjustColumnWidths()
{
    QTableWidget* table = ui->appointmentsTable;
    table->resizeColumnsToContents();

    QList<int> visibleColumns;
    int contentWidth = 0;
    for (int column = 0; column < table->columnCount(); ++column) {
        if (table->isColumnHidden(column))
            continue;
        visibleColumns.append(column);
        contentWidth += table->columnWidth(column);
    }

    const int availableWidth = table->viewport()->width();
    int remaining = availableWidth - contentWidth;
    if (remaining <= 0)
        return;

    // 操作列保持按钮所需宽度，其余数据列按当前内容宽度分享空余区域。
    const int actionColumn = table->columnCount() - 1;
    int expandableWidth = contentWidth - table->columnWidth(actionColumn);
    for (const int column : visibleColumns) {
        if (column == actionColumn)
            continue;
        const int addition = expandableWidth > 0
                                 ? remaining * table->columnWidth(column) / expandableWidth
                                 : 0;
        table->setColumnWidth(column, table->columnWidth(column) + addition);
        remaining -= addition;
        expandableWidth -= table->columnWidth(column) - addition;
    }

    // 整数除法产生的少量余数交给最后一个数据列，确保正好铺满表格。
    if (remaining > 0) {
        for (auto it = visibleColumns.crbegin(); it != visibleColumns.crend(); ++it) {
            if (*it != actionColumn) {
                table->setColumnWidth(*it, table->columnWidth(*it) + remaining);
                break;
            }
        }
    }
}

void AppointmentsPage::addRowActions(int row, const QString& appointmentId, bool completed)
{
    auto* container = new QWidget(ui->appointmentsTable);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(2, 0, 2, 0);
    layout->setSpacing(0);

    QStringList actions{QStringLiteral("详情")};
    if (!completed)
        actions.append(QStringLiteral("退号"));
    for (const QString& text : actions) {
        auto* button = new QPushButton(text, container);
        button->setProperty("link", true);
        if (text == QStringLiteral("退号"))
            button->setProperty("danger", true);
        layout->addWidget(button);
        if (text == QStringLiteral("详情"))
            connect(button, &QPushButton::clicked, this,
                    [this, appointmentId] { showDetails(appointmentId); });
        else
            connect(button, &QPushButton::clicked, this,
                    [this, appointmentId] { cancelAppointment(appointmentId); });
    }
    ui->appointmentsTable->setCellWidget(row, 10, container);
}

void AppointmentsPage::showDetails(const QString& appointmentId)
{
    Appointment* appointment = hospital_.findAppointment(appointmentId);
    if (!appointment)
        return;

    const QDate oldDate = appointment->getDate();
    const Timeslot oldSlot = appointment->getTimeslot();
    const QString oldSymptom = appointment->getSymptom();

    AppointmentDialog dialog(hospital_, *appointment, !appointment->hasEnded(), this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QDate newDate = dialog.selectedDate();
    const QString newAppointmentId =
        newDate == oldDate ? appointmentId : hospital_.nextAppointmentId(newDate);
    if (newAppointmentId.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("修改预约"),
                             QStringLiteral("新日期的预约号已达到上限，无法修改。"));
        return;
    }

    appointment->setDate(newDate);
    appointment->setTimeslot(dialog.selectedTimeslot());
    appointment->setSymptom(dialog.symptom());
    if (!hospital_.changeAppointmentId(appointmentId, newAppointmentId)) {
        appointment->setDate(oldDate);
        appointment->setTimeslot(oldSlot);
        appointment->setSymptom(oldSymptom);
        QMessageBox::warning(this, QStringLiteral("修改预约"),
                             QStringLiteral("生成的新预约号不可用，请重试。"));
        return;
    }
    if (!persistHospital(hospital_, this)) {
        hospital_.changeAppointmentId(newAppointmentId, appointmentId);
        appointment->setDate(oldDate);
        appointment->setTimeslot(oldSlot);
        appointment->setSymptom(oldSymptom);
        return;
    }
    QMessageBox::information(
        this, QStringLiteral("修改成功"),
        newAppointmentId == appointmentId
            ? QStringLiteral("预约已修改，预约号保持为 %1。").arg(newAppointmentId)
            : QStringLiteral("预约已修改，新预约号为 %1。").arg(newAppointmentId));
    populateTable();
}

void AppointmentsPage::cancelAppointment(const QString& appointmentId)
{
    Appointment* appointment = hospital_.findAppointment(appointmentId);
    if (!appointment)
        return;
    if (appointment->hasEnded()) {
        QMessageBox::information(this, QStringLiteral("无法退号"),
                                 QStringLiteral("该预约时段已经结束，不能退号。"));
        populateTable();
        return;
    }
    const Patient patient = appointment->getPatient();
    if (QMessageBox::question(
            this, QStringLiteral("确认退号"),
            QStringLiteral("确定退掉预约 %1（患者：%2）吗？")
                .arg(appointmentId, patient.getName()))
        != QMessageBox::Yes) {
        return;
    }

    hospital_.removeAppointment(appointmentId);
    if (!persistHospital(hospital_, this)) {
        hospital_.load();
        return;
    }
    populateTable();
}

void AppointmentsPage::resetFilters()
{
    ui->cmbDoctor->setCurrentIndex(0);
    ui->chkCompleted->setChecked(false);
    ui->edtKeyword->clear();
    populateTable();
}

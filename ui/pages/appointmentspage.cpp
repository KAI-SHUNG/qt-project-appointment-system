#include "appointmentspage.h"
#include "ui_appointmentspage.h"

#include "../dialogs/appointmentdialog.h"
#include "../persistence.h"
#include "../theme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

AppointmentsPage::AppointmentsPage(Hospital& hospital, QWidget* parent)
    : QWidget(parent), ui(new Ui::AppointmentsPage), hospital_(hospital)
{
    ui->setupUi(this);
    ui->dateStart->setDate(QDate::currentDate());
    ui->dateEnd->setDate(QDate::currentDate().addDays(28));
    ui->appointmentsTable->verticalHeader()->setVisible(false);
    ui->appointmentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->appointmentsTable->horizontalHeader()->setSectionResizeMode(11, QHeaderView::Fixed);
    ui->appointmentsTable->setColumnWidth(11, Theme::ActionsColumnWidth);
    ui->appointmentsTable->setColumnHidden(6, true);
    ui->appointmentsTable->setColumnHidden(9, true);

    connect(ui->btnSearch, &QPushButton::clicked, this, &AppointmentsPage::populateTable);
    connect(ui->btnReset, &QPushButton::clicked, this, &AppointmentsPage::resetFilters);
    connect(ui->edtKeyword, &QLineEdit::returnPressed, this, &AppointmentsPage::populateTable);
    connect(ui->cmbDoctor, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AppointmentsPage::populateTable);
    connect(ui->chkStartDate, &QCheckBox::toggled, ui->dateStart, &QDateEdit::setEnabled);
    connect(ui->chkEndDate, &QCheckBox::toggled, ui->dateEnd, &QDateEdit::setEnabled);
    connect(ui->appointmentsTable, &QTableWidget::cellDoubleClicked, this,
            [this](int row, int) {
                if (row >= 0 && ui->appointmentsTable->item(row, 0))
                    editAppointment(ui->appointmentsTable->item(row, 0)->data(Qt::UserRole).toString());
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
    if (ui->chkStartDate->isChecked() && ui->chkEndDate->isChecked()
        && ui->dateStart->date() > ui->dateEnd->date()) {
        QMessageBox::warning(this, QStringLiteral("预约查询"),
                             QStringLiteral("起始日期不能晚于截止日期。"));
        return;
    }

    const QString doctorId = ui->cmbDoctor->currentData().toString();
    const QString keyword = ui->edtKeyword->text().trimmed();

    QList<const Appointment*> matched;
    for (const Appointment& appointment : hospital_.getAppointments()) {
        if (!doctorId.isEmpty() && appointment.getDoctorId() != doctorId)
            continue;
        if (ui->chkStartDate->isChecked() && appointment.getDate() < ui->dateStart->date())
            continue;
        if (ui->chkEndDate->isChecked() && appointment.getDate() > ui->dateEnd->date())
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
            patient.getPhoneNumber(),
            appointment.getSymptom()
        };

        for (int column = 0; column < values.size(); ++column) {
            auto* item = new QTableWidgetItem(values.at(column));
            item->setTextAlignment(column == 10 ? Qt::AlignLeft | Qt::AlignVCenter
                                                : Qt::AlignCenter);
            if (column == 0)
                item->setData(Qt::UserRole, appointment.getAppointId());
            if (column == 10)
                item->setToolTip(appointment.getSymptom());
            ui->appointmentsTable->setItem(row, column, item);
        }
        ui->appointmentsTable->setRowHeight(row, Theme::TableRowHeight);
        addRowActions(row, appointment.getAppointId());
    }
    ui->lblCount->setText(QStringLiteral("共 %1 条预约").arg(matched.size()));
}

void AppointmentsPage::addRowActions(int row, const QString& appointmentId)
{
    auto* container = new QWidget(ui->appointmentsTable);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(2, 0, 2, 0);
    layout->setSpacing(0);

    for (const QString& text :
         {QStringLiteral("详情"), QStringLiteral("改约"), QStringLiteral("退号")}) {
        auto* button = new QPushButton(text, container);
        button->setProperty("link", true);
        if (text == QStringLiteral("退号"))
            button->setProperty("danger", true);
        layout->addWidget(button);
        if (text == QStringLiteral("详情"))
            connect(button, &QPushButton::clicked, this,
                    [this, appointmentId] { showDetails(appointmentId); });
        else if (text == QStringLiteral("改约"))
            connect(button, &QPushButton::clicked, this,
                    [this, appointmentId] { editAppointment(appointmentId); });
        else
            connect(button, &QPushButton::clicked, this,
                    [this, appointmentId] { cancelAppointment(appointmentId); });
    }
    ui->appointmentsTable->setCellWidget(row, 11, container);
}

void AppointmentsPage::showDetails(const QString& appointmentId)
{
    Appointment* appointment = hospital_.findAppointment(appointmentId);
    if (!appointment)
        return;
    AppointmentDialog dialog(hospital_, *appointment, false, this);
    dialog.exec();
}

void AppointmentsPage::editAppointment(const QString& appointmentId)
{
    Appointment* appointment = hospital_.findAppointment(appointmentId);
    if (!appointment)
        return;

    const QDate oldDate = appointment->getDate();
    const Timeslot oldSlot = appointment->getTimeslot();
    const QString oldSymptom = appointment->getSymptom();

    AppointmentDialog dialog(hospital_, *appointment, true, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    appointment->setDate(dialog.selectedDate());
    appointment->setTimeslot(dialog.selectedTimeslot());
    appointment->setSymptom(dialog.symptom());
    if (!persistHospital(hospital_, this)) {
        appointment->setDate(oldDate);
        appointment->setTimeslot(oldSlot);
        appointment->setSymptom(oldSymptom);
        return;
    }
    populateTable();
}

void AppointmentsPage::cancelAppointment(const QString& appointmentId)
{
    Appointment* appointment = hospital_.findAppointment(appointmentId);
    if (!appointment)
        return;
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
    ui->chkStartDate->setChecked(false);
    ui->chkEndDate->setChecked(false);
    ui->dateStart->setDate(QDate::currentDate());
    ui->dateEnd->setDate(QDate::currentDate().addDays(28));
    ui->edtKeyword->clear();
    populateTable();
}

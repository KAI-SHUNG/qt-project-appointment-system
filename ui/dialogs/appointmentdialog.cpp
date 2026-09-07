#include "appointmentdialog.h"
#include "ui_appointmentdialog.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

AppointmentDialog::AppointmentDialog(Hospital& hospital, const Appointment& appointment,
                                     bool editable, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::AppointmentDialog),
      hospital_(hospital),
      appointment_(appointment),
      doctor_(hospital.findDoctor(appointment.getDoctorId())),
      editable_(editable)
{
    ui->setupUi(this);
    setWindowTitle(editable ? QStringLiteral("改约") : QStringLiteral("预约详情"));

    const Patient patient = appointment.getPatient();
    ui->lblAppointmentId->setText(appointment.getAppointId());
    ui->lblDoctor->setText(
        doctor_ ? QStringLiteral("%1 · %2 · %3")
                      .arg(doctor_->getDoctorId(), doctor_->getName(), doctor_->getDepartment())
                : appointment.getDoctorId());
    ui->lblPatient->setText(
        QStringLiteral("%1 · %2 · %3岁")
            .arg(patient.getName(),
                 patient.getGender() == Human::Gender::Male ? QStringLiteral("男")
                                                           : QStringLiteral("女"))
            .arg(patient.getAge()));
    ui->lblPatientId->setText(patient.getPatientId());
    ui->lblPhone->setText(patient.getPhoneNumber());
    ui->dateAppointment->setDate(appointment.getDate());
    ui->dateAppointment->setMinimumDate(QDate::currentDate());
    ui->dateAppointment->setMaximumDate(QDate::currentDate().addDays(28));
    ui->edtSymptom->setPlainText(appointment.getSymptom());

    ui->buttonBox->setStandardButtons(
        editable ? QDialogButtonBox::Save | QDialogButtonBox::Cancel
                 : QDialogButtonBox::Close);
    confirmButton_ = ui->buttonBox->button(
        editable ? QDialogButtonBox::Save : QDialogButtonBox::Close);
    confirmButton_->setText(editable ? QStringLiteral("保存改约") : QStringLiteral("关闭"));
    confirmButton_->setProperty("primary", editable);
    ui->dateAppointment->setEnabled(editable);
    ui->cmbSlot->setEnabled(editable);
    ui->edtSymptom->setReadOnly(!editable);

    connect(ui->dateAppointment, &QDateEdit::dateChanged,
            this, &AppointmentDialog::refreshSlots);
    if (editable)
        connect(confirmButton_, &QPushButton::clicked, this, &AppointmentDialog::acceptChanges);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    refreshSlots();
}

AppointmentDialog::~AppointmentDialog()
{
    delete ui;
}

void AppointmentDialog::refreshSlots()
{
    ui->cmbSlot->clear();
    if (!doctor_) {
        confirmButton_->setEnabled(false);
        return;
    }

    const QDate date = ui->dateAppointment->date();
    const auto& schedule = doctor_->getSchedule();
    for (int index = 0; index < schedule.size(); ++index) {
        const Timeslot& slot = schedule.at(index);
        if (static_cast<int>(slot.getDayOfWeek()) != date.dayOfWeek())
            continue;

        int occupied = hospital_.countAppointments(doctor_->getDoctorId(), date, slot);
        if (date == appointment_.getDate() && slot == appointment_.getTimeslot())
            --occupied;
        const int remaining = slot.getCapability() - occupied;
        if (remaining <= 0)
            continue;

        ui->cmbSlot->addItem(
            QStringLiteral("%1-%2（剩余 %3）")
                .arg(slot.getStartTime().toString(QStringLiteral("HH:mm")),
                     slot.getEndTime().toString(QStringLiteral("HH:mm")))
                .arg(remaining),
            index);
        if (date == appointment_.getDate() && slot == appointment_.getTimeslot())
            ui->cmbSlot->setCurrentIndex(ui->cmbSlot->count() - 1);
    }
    confirmButton_->setEnabled(!editable_ || ui->cmbSlot->count() > 0);
}

void AppointmentDialog::acceptChanges()
{
    if (!doctor_ || ui->cmbSlot->currentIndex() < 0) {
        QMessageBox::warning(this, QStringLiteral("改约"),
                             QStringLiteral("所选日期没有可预约时段。"));
        return;
    }
    if (ui->edtSymptom->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("改约"),
                             QStringLiteral("症状描述不能为空。"));
        return;
    }
    accept();
}

QDate AppointmentDialog::selectedDate() const
{
    return ui->dateAppointment->date();
}

Timeslot AppointmentDialog::selectedTimeslot() const
{
    return doctor_->getSchedule().at(ui->cmbSlot->currentData().toInt());
}

QString AppointmentDialog::symptom() const
{
    return ui->edtSymptom->toPlainText().trimmed();
}

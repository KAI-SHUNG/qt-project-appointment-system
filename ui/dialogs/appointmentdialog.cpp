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
                                     bool canModify, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::AppointmentDialog),
      hospital_(hospital),
      appointment_(appointment),
      doctor_(hospital.findDoctor(appointment.getDoctorId())),
      canModify_(canModify)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("预约详情"));

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
    // 只读详情必须先扩展日期范围再赋值，否则 QDateEdit 会把历史日期夹到今天。
    ui->dateAppointment->setMinimumDate(qMin(QDate::currentDate(), appointment.getDate()));
    ui->dateAppointment->setMaximumDate(qMax(QDate::currentDate().addDays(28),
                                              appointment.getDate()));
    ui->dateAppointment->setDate(appointment.getDate());
    ui->edtSymptom->setPlainText(appointment.getSymptom());

    ui->buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
    modifyButton_ = ui->buttonBox->addButton(QStringLiteral("修改"),
                                              QDialogButtonBox::ActionRole);
    modifyButton_->setEnabled(canModify_);
    if (!canModify_)
        modifyButton_->setToolTip(QStringLiteral("已完成的预约不能修改"));
    confirmButton_ = modifyButton_;
    ui->dateAppointment->setEnabled(false);
    ui->cmbSlot->setEnabled(false);
    ui->edtSymptom->setReadOnly(true);

    connect(ui->dateAppointment, &QDateEdit::dateChanged,
            this, &AppointmentDialog::refreshSlots);
    connect(modifyButton_, &QPushButton::clicked, this, &AppointmentDialog::enterEditMode);
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

    if (!editable_) {
        const Timeslot slot = appointment_.getTimeslot();
        ui->cmbSlot->addItem(
            QStringLiteral("%1-%2")
                .arg(slot.getStartTime().toString(QStringLiteral("HH:mm")),
                     slot.getEndTime().toString(QStringLiteral("HH:mm"))));
        return;
    }

    const QDate date = ui->dateAppointment->date();
    const QDateTime now = QDateTime::currentDateTime();
    const auto& schedule = doctor_->getSchedule();
    for (int index = 0; index < schedule.size(); ++index) {
        const Timeslot& slot = schedule.at(index);
        if (static_cast<int>(slot.getDayOfWeek()) != date.dayOfWeek())
            continue;
        if (QDateTime(date, slot.getStartTime()) < now)
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
    confirmButton_->setEnabled(ui->cmbSlot->count() > 0);
}

void AppointmentDialog::enterEditMode()
{
    if (!canModify_)
        return;

    editable_ = true;
    ui->buttonBox->removeButton(modifyButton_);
    modifyButton_->deleteLater();
    modifyButton_ = nullptr;
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    confirmButton_ = ui->buttonBox->button(QDialogButtonBox::Save);
    confirmButton_->setText(QStringLiteral("保存修改"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    confirmButton_->setProperty("primary", true);

    ui->dateAppointment->setMinimumDate(QDate::currentDate());
    ui->dateAppointment->setMaximumDate(QDate::currentDate().addDays(28));
    ui->dateAppointment->setEnabled(true);
    ui->cmbSlot->setEnabled(true);
    ui->edtSymptom->setReadOnly(false);
    connect(confirmButton_, &QPushButton::clicked, this, &AppointmentDialog::acceptChanges);
    refreshSlots();
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

    const Timeslot slot = selectedTimeslot();
    if (QMessageBox::question(
            this, QStringLiteral("确认修改"),
            QStringLiteral("确定将预约 %1 修改为 %2 %3-%4 吗？")
                .arg(appointment_.getAppointId(),
                     selectedDate().toString(QStringLiteral("yyyy-MM-dd")),
                     slot.getStartTime().toString(QStringLiteral("HH:mm")),
                     slot.getEndTime().toString(QStringLiteral("HH:mm"))))
        != QMessageBox::Yes) {
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

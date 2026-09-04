#include "appointmentdialog.h"

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
      hospital_(hospital),
      appointment_(appointment),
      doctor_(hospital.findDoctor(appointment.getDoctorId())),
      editable_(editable)
{
    setWindowTitle(editable ? QStringLiteral("改约") : QStringLiteral("预约详情"));
    setMinimumWidth(500);

    auto* root = new QVBoxLayout(this);
    auto* form = new QFormLayout;
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(12);

    const Patient patient = appointment.getPatient();
    auto addValue = [this, form](const QString& label, const QString& value) {
        auto* valueLabel = new QLabel(value, this);
        valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        valueLabel->setWordWrap(true);
        form->addRow(label, valueLabel);
    };

    addValue(QStringLiteral("预约号"), appointment.getAppointId());
    addValue(QStringLiteral("医生"),
             doctor_ ? QStringLiteral("%1 · %2 · %3")
                           .arg(doctor_->getDoctorId(), doctor_->getName(), doctor_->getDepartment())
                     : appointment.getDoctorId());
    addValue(QStringLiteral("患者"),
             QStringLiteral("%1 · %2 · %3岁")
                 .arg(patient.getName(),
                      patient.getGender() == Human::Gender::Male ? QStringLiteral("男")
                                                                : QStringLiteral("女"))
                 .arg(patient.getAge()));
    addValue(QStringLiteral("身份证"), patient.getPatientId());
    addValue(QStringLiteral("手机号"), patient.getPhoneNumber());

    dateEdit_ = new QDateEdit(appointment.getDate(), this);
    dateEdit_->setCalendarPopup(true);
    dateEdit_->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    dateEdit_->setMinimumDate(QDate::currentDate());
    dateEdit_->setMaximumDate(QDate::currentDate().addDays(28));
    slotCombo_ = new QComboBox(this);
    symptomEdit_ = new QTextEdit(appointment.getSymptom(), this);
    symptomEdit_->setMinimumHeight(100);

    form->addRow(QStringLiteral("日期"), dateEdit_);
    form->addRow(QStringLiteral("时段"), slotCombo_);
    form->addRow(QStringLiteral("症状"), symptomEdit_);
    root->addLayout(form);

    auto* buttons = new QDialogButtonBox(this);
    confirmButton_ = buttons->addButton(editable ? QStringLiteral("保存改约")
                                                  : QStringLiteral("关闭"),
                                         editable ? QDialogButtonBox::AcceptRole
                                                  : QDialogButtonBox::RejectRole);
    if (editable)
        buttons->addButton(QStringLiteral("取消"), QDialogButtonBox::RejectRole);
    confirmButton_->setProperty("primary", editable);
    root->addWidget(buttons);

    dateEdit_->setEnabled(editable);
    slotCombo_->setEnabled(editable);
    symptomEdit_->setReadOnly(!editable);

    connect(dateEdit_, &QDateEdit::dateChanged, this, &AppointmentDialog::refreshSlots);
    if (editable)
        connect(confirmButton_, &QPushButton::clicked, this, &AppointmentDialog::acceptChanges);
    else
        connect(confirmButton_, &QPushButton::clicked, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshSlots();
}

void AppointmentDialog::refreshSlots()
{
    slotCombo_->clear();
    if (!doctor_) {
        confirmButton_->setEnabled(false);
        return;
    }

    const QDate date = dateEdit_->date();
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

        slotCombo_->addItem(
            QStringLiteral("%1-%2（剩余 %3）")
                .arg(slot.getStartTime().toString(QStringLiteral("HH:mm")),
                     slot.getEndTime().toString(QStringLiteral("HH:mm")))
                .arg(remaining),
            index);
        if (date == appointment_.getDate() && slot == appointment_.getTimeslot())
            slotCombo_->setCurrentIndex(slotCombo_->count() - 1);
    }
    confirmButton_->setEnabled(!editable_ || slotCombo_->count() > 0);
}

void AppointmentDialog::acceptChanges()
{
    if (!doctor_ || slotCombo_->currentIndex() < 0) {
        QMessageBox::warning(this, QStringLiteral("改约"),
                             QStringLiteral("所选日期没有可预约时段。"));
        return;
    }
    if (symptomEdit_->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("改约"),
                             QStringLiteral("症状描述不能为空。"));
        return;
    }
    accept();
}

QDate AppointmentDialog::selectedDate() const
{
    return dateEdit_->date();
}

Timeslot AppointmentDialog::selectedTimeslot() const
{
    return doctor_->getSchedule().at(slotCombo_->currentData().toInt());
}

QString AppointmentDialog::symptom() const
{
    return symptomEdit_->toPlainText().trimmed();
}

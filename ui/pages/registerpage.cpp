#include "registerpage.h"
#include "../persistence.h"
#include "ui_registerpage.h"

#include <QBrush>
#include <QCompleter>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSignalBlocker>

RegisterPage::RegisterPage(Hospital& hospital, QWidget* parent)
    : QWidget(parent), ui(new Ui::RegisterPage), hospital_(hospital)
{
    ui->setupUi(this);
    ui->btnSubmit->setProperty("primary", true);
    ui->cmbDoctor->setEditable(true);
    ui->cmbDoctor->setInsertPolicy(QComboBox::NoInsert);
    ui->cmbDoctor->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    ui->cmbDoctor->completer()->setFilterMode(Qt::MatchContains);
    ui->cmbDoctor->completer()->setCompletionMode(QCompleter::PopupCompletion);
    ui->calendar->setMinimumDate(QDate::currentDate());
    ui->calendar->setMaximumDate(QDate::currentDate().addDays(28));
    ui->calendar->setSelectedDate(QDate::currentDate());
    ui->calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    connect(ui->cmbDoctor->lineEdit(), &QLineEdit::textEdited,
            this, [this] { onDoctorChanged(); });
    connect(ui->cmbDoctor, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RegisterPage::onDoctorChanged);
    connect(ui->calendar, &QCalendarWidget::selectionChanged,
            this, &RegisterPage::refreshSlots);
    connect(ui->lstSlots, &QListWidget::itemSelectionChanged,
            this, &RegisterPage::updateSubmitState);
    connect(ui->btnSubmit, &QPushButton::clicked,
            this, &RegisterPage::submitAppointment);
    refresh();
}

RegisterPage::~RegisterPage()
{
    delete ui;
}

void RegisterPage::refresh()
{
    fillDoctors(currentDoctorId());
}

void RegisterPage::selectDoctor(const QString& doctorId)
{
    fillDoctors(doctorId);
}

QString RegisterPage::currentDoctorId() const
{
    const int index = ui->cmbDoctor->currentIndex();
    if (index < 0 || ui->cmbDoctor->currentText() != ui->cmbDoctor->itemText(index))
        return {};
    return ui->cmbDoctor->itemData(index).toString();
}

void RegisterPage::fillDoctors(const QString& preferredDoctorId)
{
    QSignalBlocker blocker(ui->cmbDoctor);
    ui->cmbDoctor->clear();
    for (const Doctor& doctor : hospital_.getDoctors()) {
        ui->cmbDoctor->addItem(
            QStringLiteral("%1 %2 %3")
                .arg(doctor.getDoctorId(), doctor.getName(), doctor.getDepartment()),
            doctor.getDoctorId());
    }

    int index = ui->cmbDoctor->findData(preferredDoctorId);
    if (index < 0 && ui->cmbDoctor->count() > 0)
        index = 0;
    ui->cmbDoctor->setCurrentIndex(index);
    updateDoctorCard();
    refreshSlots();
}

void RegisterPage::onDoctorChanged()
{
    updateDoctorCard();
    refreshSlots();
}

void RegisterPage::updateDoctorCard()
{
    Doctor* doctor = hospital_.findDoctor(currentDoctorId());
    if (doctor == nullptr) {
        ui->lblDoctorInfo->setText(QStringLiteral("暂无可预约医生，请先在医生管理中添加医生和排班。"));
        return;
    }

    ui->lblDoctorInfo->setText(
        QStringLiteral("%1　%2\n%3 · %4岁 · 编号 %5")
            .arg(doctor->getTitle(), doctor->getDepartment(),
                 doctor->getGender() == Human::Gender::Male ? QStringLiteral("男") : QStringLiteral("女"))
            .arg(doctor->getAge())
            .arg(doctor->getDoctorId()));
}

void RegisterPage::refreshSlots()
{
    ui->lstSlots->clear();
    Doctor* doctor = hospital_.findDoctor(currentDoctorId());
    if (doctor == nullptr) {
        updateSubmitState();
        return;
    }

    const QDate date     = ui->calendar->selectedDate();
    const auto& schedule = doctor->getSchedule();
    for (int i = 0; i < schedule.size(); ++i) {
        const Timeslot& slot = schedule.at(i);
        if (static_cast<int>(slot.getDayOfWeek()) != date.dayOfWeek())
            continue;

        const int remaining = slot.getCapability()
                              - hospital_.countAppointments(doctor->getDoctorId(), date, slot);
        auto*     item      = new QListWidgetItem(
            QStringLiteral("%1–%2    剩余 %3 个号")
                .arg(slot.getStartTime().toString(QStringLiteral("HH:mm")),
                     slot.getEndTime().toString(QStringLiteral("HH:mm")))
                .arg(qMax(remaining, 0)),
            ui->lstSlots);
        item->setData(Qt::UserRole, i);
        item->setTextAlignment(Qt::AlignVCenter);
        if (remaining <= 0) {
            item->setForeground(QBrush(QColor(QStringLiteral("#9AA1AA"))));
            item->setBackground(QBrush(QColor(QStringLiteral("#F5F6F8"))));
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
        }
        else if (remaining <= 2) {
            item->setForeground(QBrush(QColor(QStringLiteral("#D97706"))));
            item->setBackground(QBrush(QColor(QStringLiteral("#FFF7E6"))));
        }
        else {
            item->setForeground(QBrush(QColor(QStringLiteral("#1976D2"))));
        }
    }

    if (ui->lstSlots->count() == 0) {
        auto* empty = new QListWidgetItem(QStringLiteral("该日期无出诊排班"), ui->lstSlots);
        empty->setFlags(Qt::NoItemFlags);
        updateSubmitState();
        return;
    }
    for (int i = 0; i < ui->lstSlots->count(); ++i) {
        QListWidgetItem* item = ui->lstSlots->item(i);
        if (item->flags().testFlag(Qt::ItemIsEnabled)) {
            ui->lstSlots->setCurrentItem(item);
            break;
        }
    }
    updateSubmitState();
}

void RegisterPage::updateSubmitState()
{
    const QListWidgetItem* item = ui->lstSlots->currentItem();
    const bool hasDoctor = hospital_.findDoctor(currentDoctorId()) != nullptr;
    const bool hasSlot = item != nullptr && item->flags().testFlag(Qt::ItemIsEnabled)
                         && item->data(Qt::UserRole).isValid();
    ui->btnSubmit->setEnabled(hasDoctor && hasSlot);
}

void RegisterPage::submitAppointment()
{
    Doctor* doctor = hospital_.findDoctor(currentDoctorId());
    if (doctor == nullptr) {
        QMessageBox::warning(this, QStringLiteral("预约挂号"), QStringLiteral("请选择医生。"));
        return;
    }

    QListWidgetItem* selected  = ui->lstSlots->currentItem();
    const int        slotIndex = selected ? selected->data(Qt::UserRole).toInt() : -1;
    if (slotIndex < 0 || slotIndex >= doctor->getSchedule().size()
        || !selected->flags().testFlag(Qt::ItemIsEnabled)) {
        QMessageBox::warning(this, QStringLiteral("预约挂号"),
                             QStringLiteral("请选择可预约的出诊时段。"));
        return;
    }

    const QString name      = ui->edtName->text().trimmed();
    const QString patientId = ui->edtPatientId->text().trimmed().toUpper();
    const QString phone     = ui->edtPhone->text().trimmed();
    const QString symptom   = ui->edtSymptom->toPlainText().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("预约挂号"), QStringLiteral("请输入患者姓名。"));
        return;
    }
    static const QRegularExpression idPattern(QStringLiteral("^[0-9]{17}[0-9X]$"));
    if (!idPattern.match(patientId).hasMatch()) {
        QMessageBox::warning(this, QStringLiteral("预约挂号"),
                             QStringLiteral("请输入正确的18位身份证号。"));
        return;
    }
    ui->edtPatientId->setText(patientId);
    static const QRegularExpression phonePattern(QStringLiteral("^1\\d{10}$"));
    if (!phonePattern.match(phone).hasMatch()) {
        QMessageBox::warning(this, QStringLiteral("预约挂号"),
                             QStringLiteral("请输入正确的11位手机号码。"));
        return;
    }
    if (symptom.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("预约挂号"), QStringLiteral("请填写症状描述。"));
        return;
    }

    const QDate   date          = ui->calendar->selectedDate();
    const QString appointmentId = hospital_.nextAppointmentId(date);
    if (appointmentId.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("预约挂号"),
                             QStringLiteral("该日期预约号已达到上限。"));
        return;
    }

    const Timeslot                    slot = doctor->getSchedule().at(slotIndex);
    const Patient                     patient(name, ui->spnAge->value(),
                                              ui->cmbGender->currentIndex() == 0 ? Human::Gender::Male : Human::Gender::Female,
                                              patientId, phone);
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, QStringLiteral("确认预约"),
        QStringLiteral("确认预约：%1（%2），患者 %3，%4 %5-%6？")
            .arg(doctor->getName(), doctor->getDepartment(), name,
                 date.toString(QStringLiteral("yyyy-MM-dd")),
                 slot.getStartTime().toString(QStringLiteral("HH:mm")),
                 slot.getEndTime().toString(QStringLiteral("HH:mm"))));
    if (answer != QMessageBox::Yes)
        return;
    try {
        hospital_.addAppointment(
            Appointment(appointmentId, *doctor, patient, symptom, date, slot));
    }
    catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("预约挂号"), QString::fromUtf8(error.what()));
        refreshSlots();
        return;
    }

    if (!persistHospital(hospital_, this)) {
        hospital_.removeAppointment(appointmentId);
        return;
    }

    QMessageBox::information(
        this, QStringLiteral("预约成功"),
        QStringLiteral("预约号：%1\n医生：%2（%3）\n日期：%4\n时段：%5–%6")
            .arg(appointmentId, doctor->getName(), doctor->getDepartment(),
                 date.toString(QStringLiteral("yyyy-MM-dd")),
                 slot.getStartTime().toString(QStringLiteral("HH:mm")),
                 slot.getEndTime().toString(QStringLiteral("HH:mm"))));
    clearPatientForm();
    refreshSlots();
}

void RegisterPage::clearPatientForm()
{
    ui->edtName->clear();
    ui->cmbGender->setCurrentIndex(0);
    ui->spnAge->setValue(30);
    ui->edtPatientId->clear();
    ui->edtPhone->clear();
    ui->edtSymptom->clear();
}

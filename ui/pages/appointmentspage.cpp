#include "appointmentspage.h"

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
    : QWidget(parent), hospital_(hospital)
{
    buildUi();
    refresh();
}

void AppointmentsPage::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("预约查询"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    root->addWidget(title);

    auto* filterCard = new QFrame(this);
    filterCard->setProperty("card", true);
    auto* filters = new QHBoxLayout(filterCard);
    filters->setContentsMargins(16, 14, 16, 14);
    filters->setSpacing(10);

    doctorFilter_ = new QComboBox(filterCard);
    doctorFilter_->setMinimumWidth(190);
    useStartDate_ = new QCheckBox(QStringLiteral("起始"), filterCard);
    startDate_ = new QDateEdit(QDate::currentDate(), filterCard);
    startDate_->setCalendarPopup(true);
    startDate_->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    useEndDate_ = new QCheckBox(QStringLiteral("截止"), filterCard);
    endDate_ = new QDateEdit(QDate::currentDate().addDays(28), filterCard);
    endDate_->setCalendarPopup(true);
    endDate_->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    keywordEdit_ = new QLineEdit(filterCard);
    keywordEdit_->setPlaceholderText(QStringLiteral("姓名 / 身份证 / 预约号"));
    keywordEdit_->setClearButtonEnabled(true);

    auto* searchButton = new QPushButton(QStringLiteral("查询"), filterCard);
    searchButton->setProperty("primary", true);
    auto* resetButton = new QPushButton(QStringLiteral("重置"), filterCard);

    filters->addWidget(new QLabel(QStringLiteral("医生"), filterCard));
    filters->addWidget(doctorFilter_);
    filters->addWidget(useStartDate_);
    filters->addWidget(startDate_);
    filters->addWidget(useEndDate_);
    filters->addWidget(endDate_);
    filters->addWidget(keywordEdit_, 1);
    filters->addWidget(searchButton);
    filters->addWidget(resetButton);
    root->addWidget(filterCard);

    table_ = new QTableWidget(this);
    table_->setObjectName(QStringLiteral("appointmentsTable"));
    table_->setColumnCount(12);
    table_->setHorizontalHeaderLabels({
        QStringLiteral("预约号"), QStringLiteral("医生"), QStringLiteral("科室"),
        QStringLiteral("患者"), QStringLiteral("性别"), QStringLiteral("年龄"),
        QStringLiteral("身份证"), QStringLiteral("日期"), QStringLiteral("时段"),
        QStringLiteral("电话"), QStringLiteral("症状"), QStringLiteral("操作")
    });
    table_->verticalHeader()->setVisible(false);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setAlternatingRowColors(true);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(11, QHeaderView::Fixed);
    table_->setColumnWidth(11, Theme::ActionsColumnWidth);
    table_->setColumnHidden(6, true); // 身份证仅用于搜索和详情
    table_->setColumnHidden(9, true); // 电话仅用于详情
    root->addWidget(table_, 1);

    countLabel_ = new QLabel(this);
    countLabel_->setProperty("weak", true);
    root->addWidget(countLabel_);

    connect(searchButton, &QPushButton::clicked, this, &AppointmentsPage::populateTable);
    connect(resetButton, &QPushButton::clicked, this, &AppointmentsPage::resetFilters);
    connect(keywordEdit_, &QLineEdit::returnPressed, this, &AppointmentsPage::populateTable);
    connect(doctorFilter_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AppointmentsPage::populateTable);
    connect(useStartDate_, &QCheckBox::toggled, startDate_, &QDateEdit::setEnabled);
    connect(useEndDate_, &QCheckBox::toggled, endDate_, &QDateEdit::setEnabled);
    connect(table_, &QTableWidget::cellDoubleClicked, this,
            [this](int row, int) {
                if (row >= 0 && table_->item(row, 0))
                    editAppointment(table_->item(row, 0)->data(Qt::UserRole).toString());
            });
    startDate_->setEnabled(false);
    endDate_->setEnabled(false);
}

void AppointmentsPage::refresh()
{
    fillDoctors();
    populateTable();
}

void AppointmentsPage::selectDoctor(const QString& doctorId)
{
    fillDoctors();
    const int index = doctorFilter_->findData(doctorId);
    doctorFilter_->setCurrentIndex(index >= 0 ? index : 0);
    populateTable();
}

void AppointmentsPage::fillDoctors()
{
    const QString selectedId = doctorFilter_->currentData().toString();
    doctorFilter_->blockSignals(true);
    doctorFilter_->clear();
    doctorFilter_->addItem(QStringLiteral("全部医生"), QString());
    for (const Doctor& doctor : hospital_.getDoctors()) {
        doctorFilter_->addItem(
            QStringLiteral("%1 · %2 · %3")
                .arg(doctor.getDoctorId(), doctor.getName(), doctor.getDepartment()),
            doctor.getDoctorId());
    }
    const int index = doctorFilter_->findData(selectedId);
    doctorFilter_->setCurrentIndex(index >= 0 ? index : 0);
    doctorFilter_->blockSignals(false);
}

void AppointmentsPage::populateTable()
{
    if (useStartDate_->isChecked() && useEndDate_->isChecked()
        && startDate_->date() > endDate_->date()) {
        QMessageBox::warning(this, QStringLiteral("预约查询"),
                             QStringLiteral("起始日期不能晚于截止日期。"));
        return;
    }

    const QString doctorId = doctorFilter_->currentData().toString();
    const QString keyword = keywordEdit_->text().trimmed();

    QList<const Appointment*> matched;
    for (const Appointment& appointment : hospital_.getAppointments()) {
        if (!doctorId.isEmpty() && appointment.getDoctorId() != doctorId)
            continue;
        if (useStartDate_->isChecked() && appointment.getDate() < startDate_->date())
            continue;
        if (useEndDate_->isChecked() && appointment.getDate() > endDate_->date())
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

    table_->clearContents();
    table_->setRowCount(matched.size());
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
            table_->setItem(row, column, item);
        }
        table_->setRowHeight(row, Theme::TableRowHeight);
        addRowActions(row, appointment.getAppointId());
    }
    countLabel_->setText(QStringLiteral("共 %1 条预约").arg(matched.size()));
}

void AppointmentsPage::addRowActions(int row, const QString& appointmentId)
{
    auto* container = new QWidget(table_);
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
    table_->setCellWidget(row, 11, container);
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
    doctorFilter_->setCurrentIndex(0);
    useStartDate_->setChecked(false);
    useEndDate_->setChecked(false);
    startDate_->setDate(QDate::currentDate());
    endDate_->setDate(QDate::currentDate().addDays(28));
    keywordEdit_->clear();
    populateTable();
}

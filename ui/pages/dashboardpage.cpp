#include "dashboardpage.h"
#include "ui_dashboardpage.h"

#include "../theme.h"

#include <QDate>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <algorithm>

DashboardPage::DashboardPage(Hospital& hospital, QWidget* parent)
    : QWidget(parent), ui(new Ui::DashboardPage), hospital_(hospital)
{
    ui->setupUi(this);
    ui->tblUpcoming->verticalHeader()->setVisible(false);
    ui->tblUpcoming->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblDepartments->verticalHeader()->setVisible(false);
    ui->tblDepartments->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tblDepartments->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    connect(ui->btnRegister, &QPushButton::clicked, this,
            [this] { emit navigateRequested(1); });
    connect(ui->btnAppointments, &QPushButton::clicked, this,
            [this] { emit navigateRequested(2); });
    connect(ui->btnDoctors, &QPushButton::clicked, this,
            [this] { emit navigateRequested(3); });
    connect(ui->tblUpcoming, &QTableWidget::cellDoubleClicked, this,
            [this](int, int) { emit navigateRequested(2); });
    refresh();
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::refresh()
{
    refreshStatistics();
    refreshUpcomingAppointments();
    refreshDepartments();
}

void DashboardPage::refreshStatistics()
{
    const QDate today = QDate::currentDate();
    int todayAppointments = 0;
    for (const Appointment& appointment : hospital_.getAppointments()) {
        if (appointment.getDate() == today)
            ++todayAppointments;
    }

    int remaining = 0;
    for (const Doctor& doctor : hospital_.getDoctors()) {
        for (const Timeslot& slot : doctor.getSchedule()) {
            if (static_cast<int>(slot.getDayOfWeek()) != today.dayOfWeek())
                continue;
            remaining += qMax(
                0, slot.getCapability()
                       - hospital_.countAppointments(doctor.getDoctorId(), today, slot));
        }
    }

    ui->lblDoctorCount->setText(QString::number(hospital_.getDoctors().size()));
    ui->lblAppointmentCount->setText(QString::number(hospital_.getAppointments().size()));
    ui->lblTodayCount->setText(QString::number(todayAppointments));
    ui->lblRemainingCount->setText(QString::number(remaining));
}

void DashboardPage::refreshUpcomingAppointments()
{
    QList<const Appointment*> upcoming;
    const QDate today = QDate::currentDate();
    for (const Appointment& appointment : hospital_.getAppointments()) {
        if (appointment.getDate() >= today)
            upcoming.append(&appointment);
    }
    std::sort(upcoming.begin(), upcoming.end(),
              [](const Appointment* left, const Appointment* right) {
                  if (left->getDate() != right->getDate())
                      return left->getDate() < right->getDate();
                  return left->getTimeslot().getStartTime()
                         < right->getTimeslot().getStartTime();
              });

    const int rowCount = qMin(6, upcoming.size());
    ui->tblUpcoming->clearContents();
    ui->tblUpcoming->setRowCount(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        const Appointment& appointment = *upcoming.at(row);
        const Doctor* doctor = hospital_.findDoctor(appointment.getDoctorId());
        const Timeslot slot = appointment.getTimeslot();
        const QStringList values = {
            appointment.getDate().toString(QStringLiteral("MM-dd")),
            QStringLiteral("%1-%2").arg(
                slot.getStartTime().toString(QStringLiteral("HH:mm")),
                slot.getEndTime().toString(QStringLiteral("HH:mm"))),
            appointment.getPatient().getName(),
            doctor ? doctor->getName() : appointment.getDoctorId()
        };
        for (int column = 0; column < values.size(); ++column) {
            auto* item = new QTableWidgetItem(values.at(column));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tblUpcoming->setItem(row, column, item);
        }
        ui->tblUpcoming->setRowHeight(row, Theme::TableRowHeight);
    }
}

void DashboardPage::refreshDepartments()
{
    QMap<QString, int> counts;
    for (const Doctor& doctor : hospital_.getDoctors())
        ++counts[doctor.getDepartment().isEmpty() ? QStringLiteral("未分类")
                                                  : doctor.getDepartment()];

    QList<QPair<QString, int>> departments;
    for (auto it = counts.cbegin(); it != counts.cend(); ++it)
        departments.append(qMakePair(it.key(), it.value()));
    std::sort(departments.begin(), departments.end(),
              [](const auto& left, const auto& right) {
                  if (left.second != right.second)
                      return left.second > right.second;
                  return left.first < right.first;
              });

    ui->tblDepartments->clearContents();
    ui->tblDepartments->setRowCount(departments.size());
    for (int row = 0; row < departments.size(); ++row) {
        auto* name = new QTableWidgetItem(departments.at(row).first);
        auto* count = new QTableWidgetItem(
            QStringLiteral("%1 位").arg(departments.at(row).second));
        name->setTextAlignment(Qt::AlignCenter);
        count->setTextAlignment(Qt::AlignCenter);
        ui->tblDepartments->setItem(row, 0, name);
        ui->tblDepartments->setItem(row, 1, count);
        ui->tblDepartments->setRowHeight(row, Theme::TableRowHeight);
    }
}

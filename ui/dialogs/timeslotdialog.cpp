#include "timeslotdialog.h"
#include "ui_timeslotdialog.h"
#include "../datadefs.h"

#include <QHeaderView>
#include <QMessageBox>
#include <algorithm>

TimeslotDialog::TimeslotDialog(Doctor& doctor, QWidget* parent)
    : QDialog(parent), ui(new Ui::TimeslotDialog), doctor_(doctor), slots_(doctor.getSchedule())
{
    ui->setupUi(this);

    ui->cmbWeek->addItems(AppData::weekdays());
    ui->btnSave->setProperty("primary", true);

    connect(ui->btnAdd, &QPushButton::clicked, this, &TimeslotDialog::addSlot);
    connect(ui->btnRemove, &QPushButton::clicked, this, &TimeslotDialog::removeSelected);
    connect(ui->btnSave, &QPushButton::clicked, this, &TimeslotDialog::saveSchedule);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    ui->tblSlots->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblSlots->verticalHeader()->setVisible(false);
    ui->tblSlots->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblSlots->setSelectionMode(QAbstractItemView::SingleSelection);

    reloadTable();
}

TimeslotDialog::~TimeslotDialog()
{
    delete ui;
}

void TimeslotDialog::reloadTable()
{
    ui->tblSlots->setRowCount(slots_.size());
    for (int i = 0; i < slots_.size(); ++i) {
        const Timeslot& slot = slots_.at(i);
        const int dayIndex   = static_cast<int>(slot.getDayOfWeek()) - 1; // Qt::Monday == 1

        auto* w = new QTableWidgetItem(AppData::weekdays().value(dayIndex));
        w->setTextAlignment(Qt::AlignCenter);
        ui->tblSlots->setItem(i, 0, w);

        auto* s = new QTableWidgetItem(slot.getStartTime().toString(QStringLiteral("HH:mm")));
        s->setTextAlignment(Qt::AlignCenter);
        ui->tblSlots->setItem(i, 1, s);

        auto* e = new QTableWidgetItem(slot.getEndTime().toString(QStringLiteral("HH:mm")));
        e->setTextAlignment(Qt::AlignCenter);
        ui->tblSlots->setItem(i, 2, e);

        auto* c = new QTableWidgetItem(QString::number(slot.getCapability()));
        c->setTextAlignment(Qt::AlignCenter);
        ui->tblSlots->setItem(i, 3, c);
    }
}

void TimeslotDialog::addSlot()
{
    const QTime start = ui->timeStart->time();
    const QTime end   = ui->timeEnd->time();
    if (start >= end) {
        QMessageBox::warning(this, QStringLiteral("提示"),
                             QStringLiteral("结束时间必须晚于开始时间。"));
        return;
    }

    const Timeslot candidate(
        static_cast<Qt::DayOfWeek>(ui->cmbWeek->currentIndex() + 1),
        start,
        end,
        ui->spnCap->value());

    for (const Timeslot& slot : slots_) {
        if (slot.overlaps(candidate)) {
            QMessageBox::warning(this, QStringLiteral("提示"),
                                 QStringLiteral("与已有出诊时段冲突，请勿重复设置。"));
            return;
        }
    }

    slots_.append(candidate);
    std::sort(slots_.begin(), slots_.end());
    reloadTable();
}

void TimeslotDialog::removeSelected()
{
    const int row = ui->tblSlots->currentRow();
    if (row < 0 || row >= slots_.size()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选中要删除的时段。"));
        return;
    }
    slots_.removeAt(row);
    reloadTable();
}

void TimeslotDialog::saveSchedule()
{
    doctor_.setSchedule(slots_);
    accept();
}
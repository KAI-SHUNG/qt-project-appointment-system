#ifndef TIMESLOTDIALOG_H
#define TIMESLOTDIALOG_H

#include <QDialog>

#include "src/doctor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class TimeslotDialog;
}
QT_END_NAMESPACE

// 编辑某专家每周出诊时段；保存时整体替换该医生排班
class TimeslotDialog : public QDialog {
    Q_OBJECT

public:
    explicit TimeslotDialog(Doctor& doctor, QWidget* parent = nullptr);
    ~TimeslotDialog() override;

private slots:
    void addSlot();
    void removeSelected();
    void saveSchedule();

private:
    void reloadTable();

    Ui::TimeslotDialog* ui;
    Doctor&             doctor_;
    QList<Timeslot>     slots_;
};

#endif // TIMESLOTDIALOG_H
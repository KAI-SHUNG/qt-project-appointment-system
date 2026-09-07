#ifndef APPOINTMENTDIALOG_H
#define APPOINTMENTDIALOG_H

#include "src/hospital.h"
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class AppointmentDialog;
}
QT_END_NAMESPACE

class QPushButton;

class AppointmentDialog : public QDialog {
    Q_OBJECT
public:
    AppointmentDialog(Hospital& hospital, const Appointment& appointment,
                      bool editable, QWidget* parent = nullptr);
    ~AppointmentDialog() override;

    QDate selectedDate() const;
    Timeslot selectedTimeslot() const;
    QString symptom() const;

private slots:
    void refreshSlots();
    void acceptChanges();

private:
    Ui::AppointmentDialog* ui;
    Hospital& hospital_;
    const Appointment& appointment_;
    const Doctor* doctor_;
    bool editable_;
    QPushButton* confirmButton_;
};

#endif // APPOINTMENTDIALOG_H

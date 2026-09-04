#ifndef APPOINTMENTDIALOG_H
#define APPOINTMENTDIALOG_H

#include "src/hospital.h"
#include <QDialog>

class QComboBox;
class QDateEdit;
class QLabel;
class QPushButton;
class QTextEdit;

class AppointmentDialog : public QDialog {
    Q_OBJECT
public:
    AppointmentDialog(Hospital& hospital, const Appointment& appointment,
                      bool editable, QWidget* parent = nullptr);

    QDate selectedDate() const;
    Timeslot selectedTimeslot() const;
    QString symptom() const;

private slots:
    void refreshSlots();
    void acceptChanges();

private:
    Hospital& hospital_;
    const Appointment& appointment_;
    const Doctor* doctor_;
    bool editable_;
    QDateEdit* dateEdit_;
    QComboBox* slotCombo_;
    QTextEdit* symptomEdit_;
    QPushButton* confirmButton_;
};

#endif // APPOINTMENTDIALOG_H

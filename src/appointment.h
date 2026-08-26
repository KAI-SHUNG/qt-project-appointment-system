#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <QObject>
#include <QDateTime>
#include "patient.h"
#include "src/doctor.h"
#include "timeslot.h"

class Appointment
{
public:
    Appointment(const QString& appointId,
                const Doctor& doctor,
                const Patient& patient,
                const QDate& date,
                const Timeslot& timeslot,
                const QString& symptom);

    QString getAppointId() const;
    QString getDoctorId() const;
    QString getPatientId() const;
    QDate getDate() const;
    void setDate(const QDate &newDate);
    Timeslot getTimeslot() const;
    void setTimeslot(const Timeslot &newTimeslot);
    QString getSymptom() const;
    void setSymptom(const QString &newSymptom);

private:
    QString appointId;
    QString doctorId;
    QString patientId;

    QDate date;
    Timeslot timeslot;

    QString symptom;
};

#endif // APPOINTMENT_H

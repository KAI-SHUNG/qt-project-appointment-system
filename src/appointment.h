#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include "patient.h"
#include "src/doctor.h"
#include "timeslot.h"
#include <QDateTime>
#include <QObject>

class Appointment {
public:
    Appointment(const QString& appointId, const Doctor& doctor, const Patient& patient,
                const QDate& date, const Timeslot& timeslot, const QString& symptom);

    QString  getAppointId() const;
    QString  getDoctorId() const;
    QString  getPatientId() const;
    QDate    getDate() const;
    Timeslot getTimeslot() const;
    QString  getSymptom() const;

    void setDate(const QDate& newDate);
    void setTimeslot(const Timeslot& newTimeslot);
    void setSymptom(const QString& newSymptom);

private:
    QString appointId;
    QString doctorId;
    QString patientId;

    QDate    date;
    Timeslot timeslot;

    QString symptom;
};

#endif // APPOINTMENT_H

#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include "src/patient.h"
#include "src/doctor.h"
#include "src/timeslot.h"
#include <QDateTime>
#include <QObject>

class Appointment {
public:
    Appointment(const QString& appointId, const Doctor& doctor, const Patient& patient,
                const QString& symptom, const QDate& date, const Timeslot& timeslot);

    QString  getAppointId() const;
    QString  getDoctorId() const;
    Patient  getPatient() const;
    Timeslot getTimeslot() const;
    QDate    getDate() const;
    QString  getSymptom() const;

    void setDate(const QDate& newDate);
    void setTimeslot(const Timeslot& newTimeslot);
    void setSymptom(const QString& newSymptom);

private:
    QString appointId;
    QString doctorId;

    Patient patient;
    QString symptom;

    QDate    date;
    Timeslot timeslot;
};

#endif // APPOINTMENT_H

#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include "src/patient.h"
#include "src/doctor.h"
#include "src/timeslot.h"
#include <QDateTime>
#include <QObject>

class Appointment {
    friend class Hospital;

public:
    Appointment(const QString& appointId, const Doctor& doctor, const Patient& patient,
                const QString& symptom, const QDate& date, const Timeslot& timeslot);

    QString  getAppointId() const;
    QString  getDoctorId() const;
    Patient  getPatient() const;
    Timeslot getTimeslot() const;
    QDate    getDate() const;
    QString  getSymptom() const;

    bool hasEnded(const QDateTime& reference = QDateTime::currentDateTime()) const;
    bool hasStarted(const QDateTime& reference = QDateTime::currentDateTime()) const;

    void setDate(const QDate& newDate);
    void setTimeslot(const Timeslot& newTimeslot);
    void setSymptom(const QString& newSymptom);

private:
    void setAppointId(const QString& newAppointId);

    QString appointId;
    QString doctorId;

    Patient patient;
    QString symptom;

    QDate    date;
    Timeslot timeslot;
};

#endif // APPOINTMENT_H

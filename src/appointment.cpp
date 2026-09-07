#include "appointment.h"

Appointment::Appointment(const QString& appointId, const Doctor& doctor, const Patient& patient,
                         const QString& symptom, const QDate& date, const Timeslot& timeslot)
    : appointId(appointId), doctorId(doctor.getDoctorId()), patient(patient), symptom(symptom),
      date(date), timeslot(timeslot)
{
    if (date.dayOfWeek() != timeslot.getDayOfWeek()) {
        throw std::runtime_error("预约日期不一致！");
    }
}

QString Appointment::getAppointId() const
{
    return appointId;
}

QString Appointment::getDoctorId() const
{
    return doctorId;
}

Patient Appointment::getPatient() const
{
    return patient;
}

QDate Appointment::getDate() const
{
    return date;
}

Timeslot Appointment::getTimeslot() const
{
    return timeslot;
}

QString Appointment::getSymptom() const
{
    return symptom;
}

bool Appointment::hasEnded(const QDateTime& reference) const
{
    return QDateTime(date, timeslot.getEndTime()) <= reference;
}

void Appointment::setAppointId(const QString& newAppointId)
{
    appointId = newAppointId;
}

void Appointment::setDate(const QDate& newDate)
{
    date = newDate;
}

void Appointment::setTimeslot(const Timeslot& newTimeslot)
{
    timeslot = newTimeslot;
}

void Appointment::setSymptom(const QString& newSymptom)
{
    symptom = newSymptom;
}

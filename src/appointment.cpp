#include "appointment.h"

Appointment::Appointment(const QString& appointId, const Doctor& doctor, const Patient& patient,
                         const QDate& date, const Timeslot& timeslot, const QString& symptom)
    : appointId(appointId), doctorId(doctor.getDoctorId()), patientId(patient.getPatientId()),
      date(date), timeslot(timeslot), symptom(symptom)
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

QString Appointment::getPatientId() const
{
    return patientId;
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

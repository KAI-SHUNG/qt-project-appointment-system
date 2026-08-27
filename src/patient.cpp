#include "patient.h"

Patient::Patient(const QString& name, int age, const Gender& gender, const QString& patientId,
                 const QString& phoneNumber)
    : Human(name, age, gender), patientId(patientId), phoneNumber(phoneNumber)
{
}

QString Patient::getPatientId() const
{
    return patientId;
}

QString Patient::getPhoneNumber() const
{
    return phoneNumber;
}

void Patient::setPatientId(const QString& newPatientId)
{
    patientId = newPatientId;
}

void Patient::setPhoneNumber(const QString& newPhoneNumber)
{
    phoneNumber = newPhoneNumber;
}

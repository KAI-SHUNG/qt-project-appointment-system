#include "patient.h"

Patient::Patient(const QString& name,
                 int age,
                 const Gender& gender,
                 const QString& patientId,
                 const QString& phoneNumber,
                 const QString& symptom)
    :human(name, age, gender),
    patientId(patientId),phoneNumber(phoneNumber),symptom(symptom)
{}

QString Patient::getPatientId() const
{
    return patientId;
}

QString Patient::getPhoneNumber() const
{
    return phoneNumber;
}

QString Patient::getSymptom() const
{
    return symptom;
}

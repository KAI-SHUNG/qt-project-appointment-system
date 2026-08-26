#ifndef PATIENT_H
#define PATIENT_H

#include <QObject>
#include "human.h"

class Patient : public human
{
public:
    Patient(){}
    Patient(const QString& name,
            int age,
            const Gender& gender,
            const QString& patientId,
            const QString& phoneNumber,
            const QString& symptom);

    QString getPatientId() const;
    QString getPhoneNumber() const;
    QString getSymptom() const;

private:
    QString patientId;
    QString phoneNumber;
    QString symptom;
};

#endif // PATIENT_H

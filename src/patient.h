#ifndef PATIENT_H
#define PATIENT_H

#include <QObject>
#include "human.h"

class Patient : public Human
{
public:
    Patient(const QString& name,
            int age,
            const Gender& gender,
            const QString& patientId,
            const QString& phoneNumber);

    QString getPatientId() const;
    QString getPhoneNumber() const;
    QString getSymptom() const;

private:
    QString patientId;
    QString phoneNumber;
};

#endif // PATIENT_H

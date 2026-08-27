#ifndef PATIENT_H
#define PATIENT_H

#include "human.h"
#include <QObject>

class Patient : public Human {
public:
    Patient(const QString& name, int age, const Gender& gender, const QString& patientId,
            const QString& phoneNumber);

    QString getPatientId() const;
    QString getPhoneNumber() const;

private:
    QString patientId;
    QString phoneNumber;
};

#endif // PATIENT_H

#ifndef DOCTOR_H
#define DOCTOR_H

#include <QObject>
#include <QDateTime>
#include "human.h"
#include "timeslot.h"

class Doctor: public Human
{
public:
    Doctor(){}
    Doctor(
        const QString& name,
        int age,
        const Gender& gender,
        const QString& doctorId,
        const QString& title,
        const QString& department
        );

    void addTimeslot(const Timeslot& timeslot);
    void removeTimeslot(const Timeslot& timeslot);

    QString getDoctorId() const;
    QString getTitle() const;
    QString getDepartment() const;

    const QList<Timeslot>& getSchedule() const;

private:
    bool isConflict(const Timeslot& timeslot) const;

    QString doctorId;
    QString title;
    QString department;

    QList<Timeslot> schedule;
};

#endif // DOCTOR_H

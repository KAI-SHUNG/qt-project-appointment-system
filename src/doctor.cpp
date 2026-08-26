#include "doctor.h"

Doctor::Doctor(
    const QString& name,
    int age,
    const Gender& gender,
    const QString& doctorId,
    const QString& title,
    const QString& department
    ):Human(name, age,gender),
    doctorId(doctorId),title(title),department(department)
{}

void Doctor::addTimeslot(const Timeslot& timeslot)
{
    if(this->isConflict(timeslot)){
        throw std::invalid_argument("日程冲突！");
    }

    schedule.append(timeslot);
    std::sort(schedule.begin(), schedule.end());
}

bool Doctor::isConflict(const Timeslot &timeslot) const
{
    for(auto& t: schedule){
        if(t.overlaps(timeslot)){
            return true;
        }
    }

    return false;
}

QString Doctor::getDoctorId() const
{
    return doctorId;
}

QString Doctor::getTitle() const
{
    return title;
}

QString Doctor::getDepartment() const
{
    return department;
}

const QList<Timeslot>& Doctor::getSchedule() const
{
    return schedule;
}

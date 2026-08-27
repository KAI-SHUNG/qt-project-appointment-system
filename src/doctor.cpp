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

void Doctor::removeTimeslot(const Timeslot &timeslot)
{
    bool flag = false;
    for(auto it = schedule.begin(); it != schedule.end(); ++it){
        if (timeslot == *it){
            flag = true;
            schedule.erase(it);
            break;
        }
    }
    if (!flag){
        qWarning() << "找不到时间段："
                   << "dayOfWeek: " << timeslot.getDayOfWeek()
                   << "startTime: " << timeslot.getStartTime()
                   << "endTime: " << timeslot.getEndTime()
                   << "capability: " << timeslot.getCapability();
    }
}

bool Doctor::isConflict(const Timeslot &timeslot) const
{
    for(const auto& t: schedule){
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

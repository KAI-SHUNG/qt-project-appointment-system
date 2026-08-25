#include "timeslot.h"

int Timeslot::default_capability = 5;

Timeslot::Timeslot(Qt::DayOfWeek dayOfWeek,
                   QTime startTime,
                   QTime endTime,
                   int   capability)
    :dayOfWeek(dayOfWeek), startTime(startTime), endTime(endTime), capability(capability)
{
    if (startTime >= endTime) {
        throw std::invalid_argument("endTime must be later than startTime");
    }
    if (capability <= 0) {
        throw std::invalid_argument("capability must be greater than zero");
    }
}

bool Timeslot::operator<(const Timeslot& other) const
{
    return dayOfWeek < other.dayOfWeek or
           (dayOfWeek == other.dayOfWeek and startTime < other.startTime);
}

bool Timeslot::operator==(const Timeslot& other) const
{
    return dayOfWeek == other.dayOfWeek
           && startTime == other.startTime
           && endTime == other.endTime;
}

bool Timeslot::contains(QDateTime datetime) const
{
    if (datetime.date().dayOfWeek() != dayOfWeek){
        return false;
    }
    QTime time = datetime.time();
    return (time >= startTime and time < endTime);
}

int Timeslot::getCapability() const
{
    return capability;
}

QTime Timeslot::getStartTime() const
{
    return startTime;
}

QTime Timeslot::getEndTime() const
{
    return endTime;
}

Qt::DayOfWeek Timeslot::getDayOfWeek() const
{
    return dayOfWeek;
}
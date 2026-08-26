#include "timeslot.h"

int Timeslot::default_capability = 5;

Timeslot::Timeslot(Qt::DayOfWeek dayOfWeek,
                   QTime startTime,
                   QTime endTime,
                   int   capability)
    :dayOfWeek(dayOfWeek), startTime(startTime), endTime(endTime), capability(capability)
{
    if (startTime >= endTime) {
        throw std::invalid_argument("结束时间必须晚于开始时间！");
    }
    if (capability <= 0) {
        throw std::invalid_argument("容量必须大于零！");
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

bool Timeslot::contains(const QDateTime& datetime) const
{
    if (datetime.date().dayOfWeek() != dayOfWeek){
        return false;
    }
    QTime time = datetime.time();
    return (time >= startTime and time < endTime);
}

bool Timeslot::overlaps(const Timeslot& other) const
{
    if (dayOfWeek != other.dayOfWeek)
        return false;

    return startTime < other.endTime &&
           other.startTime < endTime;
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
#include "timeslot.h"

int Timeslot::default_capability = 5;

QMap<QString, Qt::DayOfWeek> Timeslot::mapDayOfWeek = {
    {"Mon", Qt::Monday},
    {"Tue", Qt::Tuesday},
    {"Wed", Qt::Wednesday},
    {"Thu", Qt::Thursday},
    {"Fri", Qt::Friday},
    {"Sat", Qt::Saturday},
    {"Sun", Qt::Sunday},
    };

Qt::DayOfWeek Timeslot::toDayOfWeek(QDateTime datetime)
{
    return toDayOfWeek(datetime.date());
}
Qt::DayOfWeek Timeslot::toDayOfWeek(QDate date)
{
    return mapDayOfWeek[date.toString("ddd")];
}

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

bool Timeslot::operator<(const Timeslot& other)
{
    return dayOfWeek < other.dayOfWeek or
        (dayOfWeek == other.dayOfWeek and startTime < other.startTime);
}

bool Timeslot::contains(QDateTime datetime) const
{
    if (toDayOfWeek(datetime) != dayOfWeek){
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
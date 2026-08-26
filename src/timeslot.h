#ifndef TIMESLOT_H
#define TIMESLOT_H

#include <QObject>
#include <QDateTime>

class Timeslot
{
public:
    Timeslot(Qt::DayOfWeek, QTime, QTime, int = default_capability);
    // to sequence timeslots from mon to sun
    bool operator<(const Timeslot&) const;
    bool operator==(const Timeslot&) const;

    // judge whether a time is in timeslot
    bool contains(const QDateTime&) const;
    // judge whether two timeslot overlap
    bool overlaps(const Timeslot&) const;

    // getter
    Qt::DayOfWeek getDayOfWeek() const;
    QTime getStartTime() const;
    QTime getEndTime() const;
    int getCapability() const;

private:
    Qt::DayOfWeek dayOfWeek;
    QTime startTime;
    QTime endTime;
    int capability;
    static int default_capability;
};

#endif // TIMESLOT_H

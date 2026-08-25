#ifndef TIMESLOT_H
#define TIMESLOT_H

#include <QObject>
#include <QDateTime>

class Timeslot
{
public:
    Timeslot(Qt::DayOfWeek, QTime, QTime, int = default_capability);
    // to sequence timeslots from mon to sun
    bool operator<(const Timeslot&);

    // judge whether a time is in timeslot
    bool contains(QDateTime) const;

    // transform a datetime or date type to dayofweek
    static Qt::DayOfWeek toDayOfWeek(QDateTime);
    static Qt::DayOfWeek toDayOfWeek(QDate);

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
    static QMap<QString, Qt::DayOfWeek> mapDayOfWeek;
};

#endif // TIMESLOT_H

#ifndef APPOINTMENTCALENDAR_H
#define APPOINTMENTCALENDAR_H

#include <QCalendarWidget>

class AppointmentCalendar : public QCalendarWidget {
public:
    explicit AppointmentCalendar(QWidget* parent = nullptr);

protected:
    void paintCell(QPainter* painter, const QRect& rect, QDate date) const override;
};

#endif // APPOINTMENTCALENDAR_H

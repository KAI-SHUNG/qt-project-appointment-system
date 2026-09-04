#include "appointmentcalendar.h"

#include <QPainter>

AppointmentCalendar::AppointmentCalendar(QWidget* parent)
    : QCalendarWidget(parent)
{
}

void AppointmentCalendar::paintCell(QPainter* painter, const QRect& rect, QDate date) const
{
    if (date.year() != yearShown() || date.month() != monthShown()) {
        painter->fillRect(rect, palette().base());
        return;
    }

    if (date < minimumDate() || date > maximumDate()) {
        painter->save();
        painter->fillRect(rect, QColor(QStringLiteral("#F5F6F8")));
        painter->setPen(QColor(QStringLiteral("#B9C0C9")));
        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
        return;
    }

    QCalendarWidget::paintCell(painter, rect, date);
}

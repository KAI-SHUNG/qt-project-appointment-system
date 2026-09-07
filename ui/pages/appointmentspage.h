#ifndef APPOINTMENTSPAGE_H
#define APPOINTMENTSPAGE_H

#include "src/hospital.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class AppointmentsPage;
}
QT_END_NAMESPACE

class AppointmentsPage : public QWidget {
    Q_OBJECT
public:
    explicit AppointmentsPage(Hospital& hospital, QWidget* parent = nullptr);
    ~AppointmentsPage() override;

public slots:
    void refresh();
    void selectDoctor(const QString& doctorId);

private slots:
    void resetFilters();

private:
    void fillDoctors();
    void populateTable();
    void addRowActions(int row, const QString& appointmentId);
    void showDetails(const QString& appointmentId);
    void editAppointment(const QString& appointmentId);
    void cancelAppointment(const QString& appointmentId);

    Ui::AppointmentsPage* ui;
    Hospital& hospital_;
};

#endif // APPOINTMENTSPAGE_H

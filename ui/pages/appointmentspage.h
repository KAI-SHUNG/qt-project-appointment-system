#ifndef APPOINTMENTSPAGE_H
#define APPOINTMENTSPAGE_H

#include "src/hospital.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QDateEdit;
class QLabel;
class QLineEdit;
class QTableWidget;

class AppointmentsPage : public QWidget {
    Q_OBJECT
public:
    explicit AppointmentsPage(Hospital& hospital, QWidget* parent = nullptr);

public slots:
    void refresh();
    void selectDoctor(const QString& doctorId);

private slots:
    void resetFilters();

private:
    void buildUi();
    void fillDoctors();
    void populateTable();
    void addRowActions(int row, const QString& appointmentId);
    void showDetails(const QString& appointmentId);
    void editAppointment(const QString& appointmentId);
    void cancelAppointment(const QString& appointmentId);

    Hospital& hospital_;
    QComboBox* doctorFilter_;
    QCheckBox* useStartDate_;
    QDateEdit* startDate_;
    QCheckBox* useEndDate_;
    QDateEdit* endDate_;
    QLineEdit* keywordEdit_;
    QTableWidget* table_;
    QLabel* countLabel_;
};

#endif // APPOINTMENTSPAGE_H

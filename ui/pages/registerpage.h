#ifndef REGISTERPAGE_H
#define REGISTERPAGE_H

#include <QWidget>
#include "src/hospital.h"

QT_BEGIN_NAMESPACE
namespace Ui { class RegisterPage; }
QT_END_NAMESPACE

class RegisterPage : public QWidget {
    Q_OBJECT
public:
    explicit RegisterPage(Hospital& hospital, QWidget* parent = nullptr);
    ~RegisterPage() override;

public slots:
    void refresh();
    void selectDoctor(const QString& doctorId);

private slots:
    void onDoctorChanged();
    void refreshSlots();
    void submitAppointment();

private:
    QString currentDoctorId() const;
    QString nextAppointmentId(const QDate& date) const;
    void fillDoctors(const QString& preferredDoctorId = {});
    void updateDoctorCard();
    void clearPatientForm();

    Ui::RegisterPage* ui;
    Hospital& hospital_;
};

#endif // REGISTERPAGE_H

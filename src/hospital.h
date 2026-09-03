#ifndef HOSPITAL_H
#define HOSPITAL_H

#include "src/appointment.h"
#include "src/doctor.h"

#include <QDate>

class Hospital {
public:
    Hospital(const QString& doctorsPath      = {},
             const QString& appointmentsPath = {});

    // Doctor (duplicate id -> throws std::invalid_argument)
    void    addDoctor(const Doctor& doctor);
    bool    removeDoctor(const QString& doctorId); // cascades its appointments
    Doctor* findDoctor(const QString& doctorId);
    // Appointment (rule violations throw std::invalid_argument)
    void         addAppointment(const Appointment& appointment);
    bool         removeAppointment(const QString& appointId);
    Appointment* findAppointment(const QString& appointId);

    // Query helpers
    int  countAppointments(const QString& doctorId,
                           const QDate&    date,
                           const Timeslot& timeslot) const;
    bool hasPatientAppointment(const QString& patientId) const;

    bool load();
    bool save() const;

    // Getter
    const QList<Doctor>&      getDoctors() const;
    const QList<Appointment>& getAppointments() const;

private:
    bool loadDoctors();
    bool loadAppointments();

    bool saveDoctors() const;
    bool saveAppointments() const;

private:
    QString doctorsFilePath;
    QString appointmentsFilePath;

    QList<Doctor>      doctors;
    QList<Appointment> appointments;
};

#endif // HOSPITAL_H

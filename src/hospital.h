#ifndef HOSPITAL_H
#define HOSPITAL_H

#include "src/appointment.h"
#include "src/doctor.h"

class Hospital {
public:
    Hospital(const QString& doctorsPath      = {},
             const QString& appointmentsPath = {});

    // Doctor
    bool    addDoctor(const Doctor& doctor);
    bool    removeDoctor(const QString& doctorId);
    Doctor* findDoctor(const QString& doctorId);
    // Appointment
    bool         addAppointment(const Appointment& appointment);
    bool         removeAppointment(const QString& appointId);
    Appointment* findAppointment(const QString& appointId);

    bool load();
    bool save() const;

    // Getter
    const QList<Doctor>&      getDoctors() const;
    const QList<Appointment>& getAppointments() const;

private:
    bool loadDoctors();
    bool loadAppointments();

    bool saveDoctors();
    bool saveAppointments();

private:
    QString doctorsFilePath;
    QString appointmentsFilePath;

    QList<Doctor>      doctors;
    QList<Appointment> appointments;
};

#endif // HOSPITAL_H

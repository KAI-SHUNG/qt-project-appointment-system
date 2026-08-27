#include "hospital.h"
#include <QCoreApplication>
#include <QDir>

Hospital::Hospital(const QString& doctorsPath,
                   const QString& appointmentsPath)
{
    QString dataDir = QCoreApplication::applicationDirPath() + "/data";

    QDir().mkpath(dataDir);

    doctorsFilePath = doctorsPath.isEmpty() ? QDir(dataDir).filePath("doctors.dat") : doctorsPath;

    appointmentsFilePath = appointmentsPath.isEmpty() ? QDir(dataDir).filePath("appointments.dat") : appointmentsPath;
}

bool Hospital::addDoctor(const Doctor& doctor)
{
}

bool Hospital::removeDoctor(const QString& doctorId)
{
}

Doctor* Hospital::findDoctor(const QString& doctorId)
{
}

bool Hospital::addAppointment(const Appointment& appointment)
{
}

bool Hospital::removeAppointment(const QString& appointId)
{
}

Appointment* Hospital::findAppointment(const QString& appointId)
{
}

bool Hospital::load()
{
}

bool Hospital::save() const
{
}

const QList<Doctor>& Hospital::getDoctors() const
{
}

const QList<Appointment>& Hospital::getAppointments() const
{
}

bool Hospital::loadDoctors()
{
    QFile file(doctorsFilePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    QDataStream in(&file);

    quint32 count;
    in >> count;

    if (in.status() != QDataStream::Ok)
        return false;
    QList<Doctor> loadedDoctors;

    for (quint32 i = 0; i < count; ++i) {
        QString       name;
        int           age;
        Human::Gender gender;
        QString       doctorId;
        QString       title;
        QString       department;

        in >> name;
        in >> age;
        in >> gender;
        in >> doctorId;
        in >> title;
        in >> department;

        Doctor doctor(
            name,
            age,
            gender,
            doctorId,
            title,
            department);

        quint32 scheduleSize;
        in >> scheduleSize;

        for (quint32 j = 0; j < scheduleSize; ++j) {
            int   day;
            QTime startTime;
            QTime endTime;
            int   capability;

            in >> day;
            in >> startTime;
            in >> endTime;
            in >> capability;

            Timeslot timeslot(
                static_cast<Qt::DayOfWeek>(day),
                startTime,
                endTime,
                capability);
            doctor.addTimeslot(timeslot);
        }

        loadedDoctors.append(doctor);
    }
    doctors = loadedDoctors;

    return in.status() == QDataStream::Ok;
}

bool Hospital::loadAppointments()
{
    QFile file(appointmentsFilePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    QDataStream in(&file);

    quint32 count;
    in >> count;

    if (in.status() != QDataStream::Ok)
        return false;
    QList<Appointment> loadedAppointments;

    for (quint32 i = 0; i < count; ++i) {
        // Appointment
        QString appointId;
        QString doctorId;
        // Patient
        QString name;
        int     age;
        qint32  gender;
        QString patientId;
        QString phoneNumber;
        // Appointment
        QString symptom;
        QDate   date;
        // Timeslot
        qint32 dayOfWeek;
        QTime  startTime;
        QTime  endTime;
        int    capability;

        // Appointment 基本信息
        in >> appointId;
        in >> doctorId;
        // Patient
        in >> name;
        in >> age;
        in >> gender;
        in >> patientId;
        in >> phoneNumber;
        // Appointment
        in >> symptom;
        in >> date;
        // Timeslot
        in >> dayOfWeek;
        in >> startTime;
        in >> endTime;
        in >> capability;
        if (in.status() != QDataStream::Ok)
            return false;

        Patient patient(
            name,
            age,
            static_cast<Human::Gender>(gender),
            patientId,
            phoneNumber);
        Timeslot timeslot(
            static_cast<Qt::DayOfWeek>(dayOfWeek),
            startTime,
            endTime,
            capability);
        Appointment appointment(
            appointId,
            *findDoctor(doctorId),
            patient,
            symptom,
            date,
            timeslot);

        loadedAppointments.append(appointment);
    }
    appointments = loadedAppointments;

    return true;
}

bool Hospital::saveDoctors()
{
    QFile file(doctorsFilePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);
    // save number of doctors
    out << quint32(doctors.size());

    for (const Doctor& doctor : std::as_const(doctors)) {
        out << doctor.getName();
        out << doctor.getAge();
        out << doctor.getGender();
        out << doctor.getDoctorId();
        out << doctor.getTitle();
        out << doctor.getDepartment();

        const auto& schedule = doctor.getSchedule();
        // save number of each doctor's schedule
        out << quint32(schedule.size());

        for (const Timeslot& timeslot : schedule) {
            out << timeslot.getDayOfWeek();
            out << timeslot.getStartTime();
            out << timeslot.getEndTime();
            out << timeslot.getCapability();
        }
    }

    return out.status() == QDataStream::Ok;
}

bool Hospital::saveAppointments()
{
    QFile file(appointmentsFilePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);

    out << quint32(appointments.size());

    for (const auto& appointment : std::as_const(appointments)) {
        out << appointment.getAppointId();
        out << appointment.getDoctorId();

        Patient patient = appointment.getPatient();
        out << patient.getName();
        out << patient.getAge();
        out << static_cast<qint32>(patient.getGender());
        out << patient.getPatientId();
        out << patient.getPhoneNumber();

        out << appointment.getSymptom();
        out << appointment.getDate();

        Timeslot timeslot = appointment.getTimeslot();

        out << static_cast<qint32>(timeslot.getDayOfWeek());
        out << timeslot.getStartTime();
        out << timeslot.getEndTime();
        out << timeslot.getCapability();
    }

    return out.status() == QDataStream::Ok;
}

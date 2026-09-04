#include "hospital.h"
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <stdexcept>

Hospital::Hospital(const QString& doctorsPath,
                   const QString& appointmentsPath)
{
    QString dataDir;
    if (doctorsPath.isEmpty() || appointmentsPath.isEmpty()) {
        dataDir = QCoreApplication::applicationDirPath() + "/data";
        QDir().mkpath(dataDir);
    }

    doctorsFilePath = doctorsPath.isEmpty() ? QDir(dataDir).filePath("doctors.dat") : doctorsPath;

    appointmentsFilePath = appointmentsPath.isEmpty() ? QDir(dataDir).filePath("appointments.dat") : appointmentsPath;
}

void Hospital::addDoctor(const Doctor& doctor)
{
    if (findDoctor(doctor.getDoctorId()) != nullptr) {
        throw std::invalid_argument("医生编号已存在：" + doctor.getDoctorId().toStdString());
    }

    doctors.append(doctor);
}

bool Hospital::removeDoctor(const QString& doctorId)
{
    int index = -1;
    for (int i = 0; i < doctors.size(); ++i) {
        if (doctors.at(i).getDoctorId() == doctorId) {
            index = i;
            break;
        }
    }
    if (index < 0)
        return false;

    doctors.removeAt(index);

    // cascade: remove every appointment of this doctor
    for (int i = appointments.size() - 1; i >= 0; --i) {
        if (appointments.at(i).getDoctorId() == doctorId)
            appointments.removeAt(i);
    }

    return true;
}

Doctor* Hospital::findDoctor(const QString& doctorId)
{
    for (auto it = doctors.begin(); it != doctors.end(); ++it) {
        if (it->getDoctorId() == doctorId)
            return &(*it);
    }
    return nullptr;
}

void Hospital::addAppointment(const Appointment& appointment)
{
    if (findDoctor(appointment.getDoctorId()) == nullptr) {
        throw std::invalid_argument("医生不存在，无法添加预约");
    }
    if (findAppointment(appointment.getAppointId()) != nullptr) {
        throw std::invalid_argument("预约号已存在：" + appointment.getAppointId().toStdString());
    }
    if (hasPatientAppointment(appointment.getPatient().getPatientId())) {
        throw std::invalid_argument("该身份证号已存在预约，请勿重复预约");
    }

    Doctor* doctor = findDoctor(appointment.getDoctorId());
    bool    inSchedule = false;
    for (const Timeslot& slot : doctor->getSchedule()) {
        if (slot == appointment.getTimeslot()) {
            inSchedule = true;
            break;
        }
    }
    if (!inSchedule) {
        throw std::invalid_argument("所选时段不在该医生的出诊排班中");
    }

    const Timeslot& timeslot = appointment.getTimeslot();
    if (countAppointments(appointment.getDoctorId(), appointment.getDate(), timeslot)
        >= timeslot.getCapability()) {
        throw std::invalid_argument("该时段预约已满");
    }

    appointments.append(appointment);
}

bool Hospital::removeAppointment(const QString& appointId)
{
    int index = -1;
    for (int i = 0; i < appointments.size(); ++i) {
        if (appointments.at(i).getAppointId() == appointId) {
            index = i;
            break;
        }
    }
    if (index < 0)
        return false;

    appointments.removeAt(index);
    return true;
}

Appointment* Hospital::findAppointment(const QString& appointId)
{
    for (auto it = appointments.begin(); it != appointments.end(); ++it) {
        if (it->getAppointId() == appointId)
            return &(*it);
    }
    return nullptr;
}

int Hospital::countAppointments(const QString& doctorId, const QDate& date,
                                const Timeslot& timeslot) const
{
    int count = 0;
    for (const Appointment& appointment : appointments) {
        if (appointment.getDoctorId() == doctorId && appointment.getDate() == date
            && appointment.getTimeslot() == timeslot) {
            ++count;
        }
    }
    return count;
}

bool Hospital::hasPatientAppointment(const QString& patientId) const
{
    for (const Appointment& appointment : appointments) {
        if (appointment.getPatient().getPatientId() == patientId)
            return true;
    }
    return false;
}

QString Hospital::nextAppointmentId(const QDate& date) const
{
    const QString prefix = QStringLiteral("A") + date.toString(QStringLiteral("yyyyMMdd"));
    for (int i = 1; i <= 999; ++i) {
        const QString id = prefix + QStringLiteral("%1").arg(i, 3, 10, QChar(0x30));
        bool exists = false;
        for (const Appointment& appointment : appointments) {
            if (appointment.getAppointId() == id) {
                exists = true;
                break;
            }
        }
        if (!exists)
            return id;
    }
    return {};
}

bool Hospital::load()
{
    bool ok = true;

    if (QFile::exists(doctorsFilePath)) {
        ok = loadDoctors() && ok;
    } else {
        doctors.clear();
    }

    if (QFile::exists(appointmentsFilePath)) {
        ok = loadAppointments() && ok;
    } else {
        appointments.clear();
    }

    return ok;
}

bool Hospital::save() const
{
    bool ok = saveDoctors();
    ok      = saveAppointments() && ok;
    return ok;
}

const QList<Doctor>& Hospital::getDoctors() const
{
    return doctors;
}

const QList<Appointment>& Hospital::getAppointments() const
{
    return appointments;
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
        QString name;
        int     age;
        qint32  gender;
        QString doctorId;
        QString title;
        QString department;

        in >> name;
        in >> age;
        in >> gender;
        in >> doctorId;
        in >> title;
        in >> department;
        if (in.status() != QDataStream::Ok)
            return false;

        Doctor doctor(
            name,
            age,
            static_cast<Human::Gender>(gender),
            doctorId,
            title,
            department);

        quint32 scheduleSize;
        in >> scheduleSize;

        for (quint32 j = 0; j < scheduleSize; ++j) {
            qint32 day;
            QTime  startTime;
            QTime  endTime;
            int    capability;

            in >> day;
            in >> startTime;
            in >> endTime;
            in >> capability;
            if (in.status() != QDataStream::Ok)
                return false;

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

    return true;
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

        // Skip corrupt records that reference a doctor that does not exist.
        Doctor* doctor = findDoctor(doctorId);
        if (doctor == nullptr)
            continue;

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
            *doctor,
            patient,
            symptom,
            date,
            timeslot);

        loadedAppointments.append(appointment);
    }
    appointments = loadedAppointments;

    return true;
}

bool Hospital::saveDoctors() const
{
    QFile file(doctorsFilePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);
    // save number of doctors
    out << quint32(doctors.size());

    for (const Doctor& doctor : doctors) {
        out << doctor.getName();
        out << doctor.getAge();
        out << static_cast<qint32>(doctor.getGender());
        out << doctor.getDoctorId();
        out << doctor.getTitle();
        out << doctor.getDepartment();

        const auto& schedule = doctor.getSchedule();
        // save number of each doctor's schedule
        out << quint32(schedule.size());

        for (const Timeslot& timeslot : schedule) {
            out << static_cast<qint32>(timeslot.getDayOfWeek());
            out << timeslot.getStartTime();
            out << timeslot.getEndTime();
            out << timeslot.getCapability();
        }
    }

    return out.status() == QDataStream::Ok;
}

bool Hospital::saveAppointments() const
{
    QFile file(appointmentsFilePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);

    out << quint32(appointments.size());

    for (const auto& appointment : appointments) {
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

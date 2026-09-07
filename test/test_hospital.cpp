#include "src/hospital.h"
#include <QDir>
#include <QTemporaryDir>
#include <cassert>
#include <stdexcept>

static QDate futureDate()
{
    return QDate::currentDate().addDays(3);
}

static Timeslot slotOf(const QDate& date, int capability)
{
    return Timeslot(
        static_cast<Qt::DayOfWeek>(date.dayOfWeek()),
        QTime(9, 0),
        QTime(10, 0),
        capability);
}

static Doctor makeDoctor(const QString& id)
{
    return Doctor(
        "张医生",
        40,
        Human::Gender::Male,
        id,
        "主任医师",
        "心内科");
}

static Doctor makeDoctorWithSlot(const QString& id, const Timeslot& slot)
{
    Doctor doctor = makeDoctor(id);
    doctor.addTimeslot(slot);
    return doctor;
}

static Patient makePatient(const QString& patientId)
{
    return Patient(
        "王小明",
        30,
        Human::Gender::Female,
        patientId,
        "13800000000");
}

static bool throwsOnAdd(Hospital& hospital, const Appointment& appointment)
{
    try {
        hospital.addAppointment(appointment);
    }
    catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

void testHospitalAddDoctor()
{
    QTemporaryDir dir;
    Hospital h(dir.filePath("doctors.dat"), dir.filePath("appointments.dat"));

    h.addDoctor(makeDoctor("D001"));

    bool duplicateThrown = false;
    try {
        h.addDoctor(makeDoctor("D001"));
    }
    catch (const std::invalid_argument&) {
        duplicateThrown = true;
    }
    assert(duplicateThrown);

    h.addDoctor(makeDoctor("D002"));
    assert(h.getDoctors().size() == 2);
    assert(h.findDoctor("D001") != nullptr);
    assert(h.findDoctor("D002") != nullptr);
    assert(h.findDoctor("D999") == nullptr);

    qDebug("TestHospitalAddDoctor Passed.");
}

void testHospitalRemoveDoctorCascade()
{
    QTemporaryDir dir;
    Hospital h(dir.filePath("doctors.dat"), dir.filePath("appointments.dat"));

    QDate     date = futureDate();
    Timeslot  slot = slotOf(date, 5);
    h.addDoctor(makeDoctorWithSlot("D100", slot));

    Appointment appointment(
        "A001",
        *h.findDoctor("D100"),
        makePatient("110101199001011234"),
        "头痛",
        date,
        slot);
    h.addAppointment(appointment);
    assert(h.getAppointments().size() == 1);

    assert(h.removeDoctor("D100"));
    assert(h.findDoctor("D100") == nullptr);
    assert(h.getAppointments().empty());

    assert(!h.removeDoctor("D100"));

    qDebug("TestHospitalRemoveDoctorCascade Passed.");
}

void testHospitalAppointmentRules()
{
    QTemporaryDir dir;
    Hospital h(dir.filePath("doctors.dat"), dir.filePath("appointments.dat"));

    QDate    date = futureDate();
    Timeslot slot = slotOf(date, 5);

    // 1) doctor must exist
    Doctor ghost = makeDoctor("GHOST");
    Appointment toMissingDoctor(
        "A1", ghost, makePatient("pid-1"), "s", date, slot);
    assert(throwsOnAdd(h, toMissingDoctor));

    // 2) slot must be in the doctor's schedule
    h.addDoctor(makeDoctor("D200"));
    Appointment notInSchedule(
        "A2", *h.findDoctor("D200"), makePatient("pid-2"), "s", date, slot);
    assert(throwsOnAdd(h, notInSchedule));

    // 3) capacity limit (1 seat)
    Timeslot tight = slotOf(date, 1);
    h.addDoctor(makeDoctorWithSlot("D201", tight));
    Appointment first("A3", *h.findDoctor("D201"), makePatient("pid-3"), "s", date, tight);
    h.addAppointment(first);
    Appointment full("A4", *h.findDoctor("D201"), makePatient("pid-4"), "s", date, tight);
    assert(throwsOnAdd(h, full));
    assert(h.countAppointments("D201", date, tight) == 1);

    // 4) a patient id may not book the same date and timeslot twice
    Timeslot roomy = slotOf(date, 5);
    h.addDoctor(makeDoctorWithSlot("D202", roomy));
    Appointment one("A5", *h.findDoctor("D202"), makePatient("pid-5"), "s", date, roomy);
    h.addAppointment(one);
    Appointment again("A6", *h.findDoctor("D202"), makePatient("pid-5"), "s", date, roomy);
    assert(throwsOnAdd(h, again));
    assert(h.hasPatientAppointment("pid-5", date, roomy));

    // The same patient may book a different date or a different timeslot.
    const QDate otherDate = date.addDays(7);
    h.addAppointment(Appointment("A6", *h.findDoctor("D202"), makePatient("pid-5"),
                                 "s", otherDate, roomy));
    const Timeslot later(static_cast<Qt::DayOfWeek>(date.dayOfWeek()),
                         QTime(11, 0), QTime(12, 0), 5);
    h.findDoctor("D202")->addTimeslot(later);
    h.addAppointment(Appointment("A8", *h.findDoctor("D202"), makePatient("pid-5"),
                                 "s", date, later));

    // 5) appointment id must be unique
    Appointment dupId("A7", *h.findDoctor("D202"), makePatient("pid-7"), "s", date, roomy);
    Appointment dupIdAgain("A7", *h.findDoctor("D202"), makePatient("pid-8"), "s", date, roomy);
    h.addAppointment(dupId);
    assert(throwsOnAdd(h, dupIdAgain));
    assert(h.findAppointment("A7") != nullptr);
    assert(h.removeAppointment("A7"));
    assert(h.findAppointment("A7") == nullptr);
    assert(!h.removeAppointment("A7"));

    // 6) obstetrics appointments accept female patients only
    h.addDoctor(Doctor("赵医生", 40, Human::Gender::Female, "D203",
                       "主治医师", "妇产科"));
    h.findDoctor("D203")->addTimeslot(roomy);
    const Patient malePatient("王先生", 30, Human::Gender::Male,
                              "pid-male", "13800000000");
    assert(throwsOnAdd(h, Appointment("A9", *h.findDoctor("D203"), malePatient,
                                      "s", date, roomy)));

    // 7) a new appointment may not target an already-started timeslot
    const QDate pastDate = QDate::currentDate().addDays(-1);
    const Timeslot pastSlot = slotOf(pastDate, 5);
    h.addDoctor(makeDoctorWithSlot("D204", pastSlot));
    assert(throwsOnAdd(h, Appointment("A10", *h.findDoctor("D204"),
                                      makePatient("pid-past"), "s", pastDate, pastSlot)));

    qDebug("TestHospitalAppointmentRules Passed.");
}

void testHospitalSaveLoadRoundTrip()
{
    QTemporaryDir dir;
    QString doctorsPath = dir.filePath("doctors.dat");
    QString appointmentsPath = dir.filePath("appointments.dat");

    QDate    date = futureDate();
    Timeslot slot = slotOf(date, 5);

    {
        Hospital h(doctorsPath, appointmentsPath);
        h.addDoctor(makeDoctorWithSlot("D300", slot));

        Appointment appointment(
            "A100",
            *h.findDoctor("D300"),
            makePatient("110101198501011111"),
            "咳嗽一周",
            date,
            slot);
        h.addAppointment(appointment);

        assert(h.save());
    }

    {
        Hospital h(doctorsPath, appointmentsPath);
        assert(h.load());

        assert(h.getDoctors().size() == 1);
        Doctor* doc = h.findDoctor("D300");
        assert(doc != nullptr);
        assert(doc->getName() == "张医生");
        assert(doc->getDepartment() == "心内科");
        assert(doc->getSchedule().size() == 1);

        assert(h.getAppointments().size() == 1);
        Appointment* apt = h.findAppointment("A100");
        assert(apt != nullptr);
        assert(apt->getPatient().getPatientId() == "110101198501011111");
        assert(apt->getDate() == date);
        assert(apt->getSymptom() == "咳嗽一周");
    }

    qDebug("TestHospitalSaveLoadRoundTrip Passed.");
}

void testHospitalLoadMissingFiles()
{
    QTemporaryDir dir;
    Hospital h(dir.filePath("missing-doctors.dat"), dir.filePath("missing-appointments.dat"));

    assert(h.load());
    assert(h.getDoctors().empty());
    assert(h.getAppointments().empty());

    qDebug("TestHospitalLoadMissingFiles Passed.");
}

void testAppointmentCompletionIncludesDateAndEndTime()
{
    const QDate date(2026, 9, 7);
    const Timeslot slot(static_cast<Qt::DayOfWeek>(date.dayOfWeek()),
                        QTime(9, 0), QTime(10, 0));
    const Doctor doctor = makeDoctorWithSlot("D400", slot);
    const Appointment appointment("A400", doctor, makePatient("P400"),
                                  "复诊", date, slot);

    assert(!appointment.hasEnded(QDateTime(date.addDays(-1), QTime(23, 59))));
    assert(!appointment.hasEnded(QDateTime(date, QTime(9, 30))));
    assert(appointment.hasEnded(QDateTime(date, QTime(10, 0))));
    assert(appointment.hasEnded(QDateTime(date.addDays(1), QTime(0, 0))));
    assert(!appointment.hasStarted(QDateTime(date, QTime(9, 0))));
    assert(appointment.hasStarted(QDateTime(date, QTime(9, 1))));

    qDebug("TestAppointmentCompletionIncludesDateAndEndTime Passed.");
}

void testHospitalChangeAppointmentId()
{
    QTemporaryDir dir;
    Hospital h(dir.filePath("doctors.dat"), dir.filePath("appointments.dat"));
    const QDate date = futureDate();
    const Timeslot slot = slotOf(date, 5);
    h.addDoctor(makeDoctorWithSlot("D500", slot));
    h.addAppointment(Appointment("A-OLD", *h.findDoctor("D500"), makePatient("P500"),
                                 "复诊", date, slot));

    assert(h.changeAppointmentId("A-OLD", "A-NEW"));
    assert(h.findAppointment("A-OLD") == nullptr);
    assert(h.findAppointment("A-NEW") != nullptr);
    assert(!h.changeAppointmentId("A-MISSING", "A-OTHER"));

    qDebug("TestHospitalChangeAppointmentId Passed.");
}

void testAppointmentIdUsesTwoDigitYear()
{
    QTemporaryDir dir;
    Hospital h(dir.filePath("doctors.dat"), dir.filePath("appointments.dat"));

    assert(h.nextAppointmentId(QDate(2026, 9, 7)) == "A260907001");

    qDebug("TestAppointmentIdUsesTwoDigitYear Passed.");
}

void testHospitalImportExport()
{
    QTemporaryDir dir;
    const QString sourceDir = dir.filePath("source");
    const QString currentDir = dir.filePath("current");
    const QString exportDir = dir.filePath("exported");
    QDir().mkpath(sourceDir);
    QDir().mkpath(currentDir);

    const QDate date = futureDate();
    const Timeslot slot = slotOf(date, 5);
    Hospital source(QDir(sourceDir).filePath("doctors.dat"),
                    QDir(sourceDir).filePath("appointments.dat"));
    source.addDoctor(makeDoctorWithSlot("D600", slot));
    source.addAppointment(Appointment("A600", *source.findDoctor("D600"),
                                      makePatient("P600"), "复查", date, slot));
    assert(source.save());

    Hospital current(QDir(currentDir).filePath("doctors.dat"),
                     QDir(currentDir).filePath("appointments.dat"));
    assert(current.importData(QDir(sourceDir).filePath("doctors.dat"),
                              QDir(sourceDir).filePath("appointments.dat")));
    assert(current.getDoctors().size() == 1);
    assert(current.getAppointments().size() == 1);
    assert(current.exportData(exportDir));

    Hospital exported(QDir(exportDir).filePath("doctors.dat"),
                      QDir(exportDir).filePath("appointments.dat"));
    assert(exported.load());
    assert(exported.findDoctor("D600") != nullptr);
    assert(exported.findAppointment("A600") != nullptr);

    // Existing files must be atomically replaced on a confirmed re-export.
    current.addDoctor(makeDoctorWithSlot("D601", slot));
    assert(current.exportData(exportDir));
    Hospital overwritten(QDir(exportDir).filePath("doctors.dat"),
                         QDir(exportDir).filePath("appointments.dat"));
    assert(overwritten.load());
    assert(overwritten.getDoctors().size() == 2);
    assert(overwritten.findDoctor("D601") != nullptr);

    qDebug("TestHospitalImportExport Passed.");
}

void testHospital()
{
    qDebug("=== Testing Hospital ===");

    testHospitalAddDoctor();
    testHospitalRemoveDoctorCascade();
    testHospitalAppointmentRules();
    testHospitalSaveLoadRoundTrip();
    testHospitalLoadMissingFiles();
    testAppointmentCompletionIncludesDateAndEndTime();
    testHospitalChangeAppointmentId();
    testAppointmentIdUsesTwoDigitYear();
    testHospitalImportExport();

    qDebug("=== Hospital Test Passed ===");
}

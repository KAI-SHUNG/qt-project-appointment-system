#include "../src/doctor.h"
#include <cassert>

void testDoctorConstruct()
{
    Doctor doctor(
        "李医生",
        35,
        Human::Gender::Female,
        "D001",
        "主任医师",
        "心内科");

    assert(doctor.getName() == "李医生");
    assert(doctor.getAge() == 35);
    assert(doctor.getGender() == Human::Gender::Female);
    assert(doctor.getDoctorId() == "D001");
    assert(doctor.getTitle() == "主任医师");
    assert(doctor.getDepartment() == "心内科");

    qDebug("TestDoctorConstruct Passed.");
}

void testDoctorSchedule()
{
    Doctor doctor(
        "李医生",
        35,
        Human::Gender::Female,
        "D001",
        "主任医师",
        "心内科");

    Timeslot afternoon(Qt::Monday, QTime(14, 0), QTime(15, 0), 6);
    Timeslot morning(Qt::Monday, QTime(9, 0), QTime(10, 0), 6);

    doctor.addTimeslot(afternoon);
    doctor.addTimeslot(morning);

    const QList<Timeslot>& schedule = doctor.getSchedule();
    assert(schedule.size() == 2);
    assert(schedule.at(0) == morning);
    assert(schedule.at(1) == afternoon);

    bool conflictThrown = false;
    try {
        doctor.addTimeslot(
            Timeslot(Qt::Monday, QTime(9, 30), QTime(10, 30), 3));
    }
    catch (const std::invalid_argument&) {
        conflictThrown = true;
    }
    assert(conflictThrown);

    doctor.removeTimeslot(morning);
    assert(doctor.getSchedule().size() == 1);
    assert(doctor.getSchedule().at(0) == afternoon);

    qDebug("TestDoctorSchedule Passed.");
}

void testDoctor()
{
    qDebug("=== Testing Doctor ===");

    testDoctorConstruct();
    testDoctorSchedule();

    qDebug("=== Doctor Test Passed ===");
}

#include "../src/patient.h"
#include <cassert>

void testPatientConstruct()
{
    Patient patient(
        "王小明",
        28,
        Human::Gender::Male,
        "P001",
        "13800138000");

    assert(patient.getName() == "王小明");
    assert(patient.getAge() == 28);
    assert(patient.getGender() == Human::Gender::Male);
    assert(patient.getPatientId() == "P001");
    assert(patient.getPhoneNumber() == "13800138000");

    qDebug("TestPatientConstruct Passed.");
}

void testPatientSetters()
{
    Patient patient(
        "王小明",
        28,
        Human::Gender::Male,
        "P001",
        "13800138000");

    patient.setName("王小强");
    patient.setAge(29);
    patient.setGender(Human::Gender::Female);

    assert(patient.getName() == "王小强");
    assert(patient.getAge() == 29);
    assert(patient.getGender() == Human::Gender::Female);

    qDebug("TestPatientSetters Passed.");
}

void testPatient()
{
    qDebug("=== Testing Patient ===");

    testPatientConstruct();
    testPatientSetters();

    qDebug("=== Patient Test Passed ===");
}

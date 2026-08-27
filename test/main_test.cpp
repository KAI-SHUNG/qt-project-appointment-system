#include "test/test_doctor.cpp"
#include "test/test_human.cpp"
#include "test/test_patient.cpp"
#include "test/test_timeslot.cpp"

int main()
{
    testTimeslot();
    testHuman();
    testDoctor();
    testPatient();
}
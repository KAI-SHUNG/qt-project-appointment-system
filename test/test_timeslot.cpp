#include "../src/timeslot.h"
#include <cassert>

Timeslot timeslot1(Qt::Monday, QTime(9,0), QTime(10, 0), 6);

void TestConstruct(){
    try{
        Timeslot timeslot2(Qt::Tuesday, QTime(12,0), QTime(10, 0), 0);
    }
    catch(std::exception& e)
    {
        qDebug("%s", e.what());
    }

    try{
        Timeslot timeslot3(Qt::Monday, QTime(7,0), QTime(23, 0), 0);
    }
    catch(std::exception& e)
    {
        qDebug("%s", e.what());
    }

    try{
        Timeslot timeslot4(Qt::Monday, QTime(9,0), QTime(9, 0), 6);
    }
    catch(std::exception& e)
    {
        qDebug("%s", e.what());
    }

    qDebug("TestConstruct Passed.");
}


void TestGetter(){
    assert(timeslot1.getDayOfWeek()==Qt::Monday);
    assert(timeslot1.getStartTime()==QTime(9, 0));
    assert(timeslot1.getEndTime()==QTime(10, 0));
    assert(timeslot1.getCapability()==6);

    qDebug("TestGetter Passed.");
}


void TestContain(){
    assert(timeslot1.contains(QDateTime(QDate(2026, 8, 24),QTime(9, 30))) == true);
    assert(timeslot1.contains(QDateTime(QDate(2026, 8, 25),QTime(9, 30))) == false);
    assert(timeslot1.contains(QDateTime(QDate(2026, 8, 24),QTime(9, 0))) == true);
    assert(timeslot1.contains(QDateTime(QDate(2026, 8, 24),QTime(10, 30))) == false);
    assert(timeslot1.contains(QDateTime(QDate(2026, 8, 24),QTime(10, 0))) == false);

    qDebug("TestContain Passed.");
}


void TestCompare(){
    Timeslot t1(Qt::Monday, QTime(9,0), QTime(10, 0));
    Timeslot t2(Qt::Tuesday, QTime(9,0), QTime(10, 0));
    Timeslot t3(Qt::Monday, QTime(10,0), QTime(11, 0));
    Timeslot t4(Qt::Friday, QTime(16,0), QTime(17, 0));

    assert(t1 < t2);
    assert(t1 < t3);
    assert(t1 < t4);
    assert(t2 < t4);
    assert(t3 < t2);

    qDebug("TestCompare Passed.");
}

int main(){
    qDebug("=== Testing Timeslot ===");

    TestConstruct();
    TestGetter();
    TestContain();
    TestCompare();

    qDebug("=== All Test Passed ===");
}
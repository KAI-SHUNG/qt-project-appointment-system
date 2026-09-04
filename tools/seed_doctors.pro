QT += core
QT -= gui

CONFIG += console c++17
CONFIG -= app_bundle

TARGET = seed_doctors

INCLUDEPATH += ..

SOURCES += \
    seed_doctors.cpp \
    ../src/appointment.cpp \
    ../src/doctor.cpp \
    ../src/hospital.cpp \
    ../src/human.cpp \
    ../src/patient.cpp \
    ../src/timeslot.cpp

HEADERS += \
    ../src/appointment.h \
    ../src/doctor.h \
    ../src/hospital.h \
    ../src/human.h \
    ../src/patient.h \
    ../src/timeslot.h

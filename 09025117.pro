QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# ---- model layer, always compiled ----
SOURCES += \
    src/appointment.cpp \
    src/doctor.cpp \
    src/hospital.cpp \
    src/human.cpp \
    src/patient.cpp \
    src/timeslot.cpp

HEADERS += \
    src/appointment.h \
    src/doctor.h \
    src/hospital.h \
    src/human.h \
    src/patient.h \
    src/timeslot.h

# ---- GUI app (default) vs console test runner ----
# 运行测试: qmake 09025117.pro BUILD_TESTS=1 && mingw32-make
isEmpty(BUILD_TESTS) {
    SOURCES += \
        main.cpp \
        ui/mainwindow.cpp \
        ui/pages/dashboardpage.cpp \
        ui/pages/doctorspage.cpp \
        ui/pages/registerpage.cpp \
        ui/pages/appointmentspage.cpp \
        ui/widgets/appointmentcalendar.cpp \
        ui/dialogs/doctordialog.cpp \
        ui/dialogs/timeslotdialog.cpp \
        ui/dialogs/appointmentdialog.cpp

    HEADERS += \
        ui/mainwindow.h \
        ui/pages/dashboardpage.h \
        ui/theme.h \
        ui/datadefs.h \
        ui/persistence.h \
        ui/pages/doctorspage.h \
        ui/pages/registerpage.h \
        ui/pages/appointmentspage.h \
        ui/widgets/appointmentcalendar.h \
        ui/dialogs/doctordialog.h \
        ui/dialogs/timeslotdialog.h \
        ui/dialogs/appointmentdialog.h

    FORMS += \
        ui/mainwindow.ui \
        ui/pages/doctorspage.ui \
        ui/pages/registerpage.ui \
        ui/pages/dashboardpage.ui \
        ui/pages/appointmentspage.ui \
        ui/dialogs/appointmentdialog.ui \
        ui/dialogs/doctordialog.ui \
        ui/dialogs/timeslotdialog.ui

    RESOURCES += \
        resources/resources.qrc
} else {
    CONFIG += console
    TARGET = hospital_tests

    SOURCES += \
        test/main_test.cpp \
        test/test_timeslot.cpp \
        test/test_human.cpp \
        test/test_doctor.cpp \
        test/test_patient.cpp \
        test/test_hospital.cpp
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

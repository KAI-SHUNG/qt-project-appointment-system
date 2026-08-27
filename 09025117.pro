QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    src/appointment.cpp \
    src/doctor.cpp \
    src/human.cpp \
    src/patient.cpp \
    src/timeslot.cpp \
    test/main_test.cpp \
    test/test_human.cpp \
    test/test_timeslot.cpp

HEADERS += \
    mainwindow.h \
    src/appointment.h \
    src/doctor.h \
    src/human.h \
    src/patient.h \
    src/timeslot.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

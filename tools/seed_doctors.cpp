#include "src/hospital.h"

#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTime>

namespace {

struct SlotData {
    Qt::DayOfWeek day;
    int startHour;
    int endHour;
    int capacity;
};

Doctor makeDoctor(const QString& name, int age, Human::Gender gender,
                  const QString& id, const QString& title,
                  const QString& department,
                  std::initializer_list<SlotData> schedule)
{
    Doctor doctor(name, age, gender, id, title, department);
    for (const SlotData& slot : schedule) {
        doctor.addTimeslot(Timeslot(slot.day, QTime(slot.startHour, 0),
                                    QTime(slot.endHour, 0), slot.capacity));
    }
    return doctor;
}

QString patientName(int index)
{
    static const QStringList surnames = {
        QStringLiteral("王"), QStringLiteral("李"), QStringLiteral("张"),
        QStringLiteral("刘"), QStringLiteral("陈"), QStringLiteral("杨"),
        QStringLiteral("赵"), QStringLiteral("黄"), QStringLiteral("周"),
        QStringLiteral("吴")};
    static const QStringList givenNames = {
        QStringLiteral("明"), QStringLiteral("芳"), QStringLiteral("建华"),
        QStringLiteral("秀英"), QStringLiteral("子涵"), QStringLiteral("志强"),
        QStringLiteral("雨欣"), QStringLiteral("文博")};
    return surnames.at(index % surnames.size())
           + givenNames.at((index / surnames.size()) % givenNames.size());
}

QString symptomForDepartment(const QString& department, int index)
{
    if (department == QStringLiteral("心内科"))
        return index % 2 ? QStringLiteral("心悸伴胸闷") : QStringLiteral("血压波动复诊");
    if (department == QStringLiteral("呼吸内科"))
        return index % 2 ? QStringLiteral("咳嗽咳痰") : QStringLiteral("气促复查");
    if (department == QStringLiteral("神经内科"))
        return index % 2 ? QStringLiteral("反复头晕") : QStringLiteral("偏头痛");
    if (department == QStringLiteral("消化内科"))
        return index % 2 ? QStringLiteral("上腹不适") : QStringLiteral("胃胀反酸");
    if (department == QStringLiteral("普外科"))
        return index % 2 ? QStringLiteral("术后复查") : QStringLiteral("腹部疼痛");
    if (department == QStringLiteral("妇产科"))
        return index % 2 ? QStringLiteral("常规产检") : QStringLiteral("月经不规律");
    if (department == QStringLiteral("骨科"))
        return index % 2 ? QStringLiteral("膝关节疼痛") : QStringLiteral("腰背酸痛");
    if (department == QStringLiteral("儿科"))
        return index % 2 ? QStringLiteral("发热咳嗽") : QStringLiteral("食欲不振");
    if (department == QStringLiteral("皮肤科"))
        return index % 2 ? QStringLiteral("皮肤瘙痒") : QStringLiteral("面部皮疹");
    if (department == QStringLiteral("眼科"))
        return index % 2 ? QStringLiteral("视力下降") : QStringLiteral("眼睛干涩");
    if (department == QStringLiteral("耳鼻喉科"))
        return index % 2 ? QStringLiteral("咽痛鼻塞") : QStringLiteral("耳鸣复查");
    return index % 2 ? QStringLiteral("血糖复查") : QStringLiteral("甲状腺复诊");
}

void addAppointments(Hospital& hospital)
{
    const QDate firstDate(2026, 9, 5);
    const QDate lastDate(2026, 9, 18);
    int patientIndex = 0;

    for (QDate date = firstDate; date <= lastDate; date = date.addDays(1)) {
        const auto& doctors = hospital.getDoctors();
        for (int doctorIndex = 0; doctorIndex < doctors.size(); ++doctorIndex) {
            const Doctor& doctor = doctors.at(doctorIndex);
            for (const Timeslot& slot : doctor.getSchedule()) {
                if (static_cast<int>(slot.getDayOfWeek()) != date.dayOfWeek())
                    continue;

                const int appointmentCount = 1 + ((date.day() + doctorIndex) % 3);
                for (int number = 0; number < appointmentCount; ++number) {
                    const QString appointmentId = hospital.nextAppointmentId(date);
                    const QString patientId =
                        QStringLiteral("32010119900101%1%2")
                            .arg(patientIndex + 1, 3, 10, QChar('0'))
                            .arg(patientIndex % 10);
                    const QString phone =
                        QStringLiteral("138%1").arg(patientIndex + 1, 8, 10, QChar('0'));
                    const Human::Gender gender =
                        doctor.getDepartment() == QStringLiteral("妇产科")
                            ? Human::Gender::Female
                            : (patientIndex % 2 == 0 ? Human::Gender::Female
                                                    : Human::Gender::Male);
                    const Patient patient(
                        patientName(patientIndex), 18 + (patientIndex * 7) % 63,
                        gender, patientId, phone);
                    hospital.addAppointment(Appointment(
                        appointmentId, doctor, patient,
                        symptomForDepartment(doctor.getDepartment(), patientIndex), date, slot),
                        true); // Seed intentionally includes completed historical appointments.
                    ++patientIndex;
                }
            }
        }
    }
}

bool verifyData(const QString& outputDir, int expectedDoctors, int expectedAppointments)
{
    Hospital verification(QDir(outputDir).filePath(QStringLiteral("doctors.dat")),
                          QDir(outputDir).filePath(QStringLiteral("appointments.dat")));
    if (!verification.load()
        || verification.getDoctors().size() != expectedDoctors
        || verification.getAppointments().size() != expectedAppointments
        || verification.getAppointments().isEmpty()) {
        return false;
    }

    QDate minimumDate;
    QDate maximumDate;
    for (const Appointment& appointment : verification.getAppointments()) {
        const Doctor* doctor = verification.findDoctor(appointment.getDoctorId());
        if (doctor && doctor->getDepartment() == QStringLiteral("妇产科")
            && appointment.getPatient().getGender() != Human::Gender::Female) {
            return false;
        }
        if (!minimumDate.isValid() || appointment.getDate() < minimumDate)
            minimumDate = appointment.getDate();
        if (!maximumDate.isValid() || appointment.getDate() > maximumDate)
            maximumDate = appointment.getDate();
    }
    return minimumDate == QDate(2026, 9, 5) && maximumDate == QDate(2026, 9, 18);
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QStringList outputDirs;
    if (argc > 1) {
        for (int index = 1; index < argc; ++index)
            outputDirs.append(QDir::fromNativeSeparators(QString::fromLocal8Bit(argv[index])));
    } else {
        QString projectRoot = QDir::currentPath();
        if (!QFile::exists(QDir(projectRoot).filePath(QStringLiteral("09025117.pro")))) {
            QDir applicationDir(QCoreApplication::applicationDirPath());
            applicationDir.cdUp();
            applicationDir.cdUp();
            applicationDir.cdUp();
            projectRoot = applicationDir.absolutePath();
        }
        outputDirs.append(projectRoot);
        outputDirs.append(QDir(projectRoot).filePath(QStringLiteral("build/debug/data")));
    }

    const QString outputDir = outputDirs.first();

    if (!QDir().mkpath(outputDir)) {
        qCritical() << "Cannot create output directory:" << outputDir;
        return 1;
    }

    Hospital hospital(QDir(outputDir).filePath(QStringLiteral("doctors.dat")),
                      QDir(outputDir).filePath(QStringLiteral("appointments.dat")));

    hospital.addDoctor(makeDoctor(QStringLiteral("张伟"), 48, Human::Gender::Male,
        QStringLiteral("D001"), QStringLiteral("主任医师"), QStringLiteral("心内科"),
        {{Qt::Monday, 8, 11, 12}, {Qt::Wednesday, 14, 17, 10}, {Qt::Friday, 8, 11, 12}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("李娜"), 42, Human::Gender::Female,
        QStringLiteral("D002"), QStringLiteral("副主任医师"), QStringLiteral("呼吸内科"),
        {{Qt::Tuesday, 8, 11, 10}, {Qt::Thursday, 14, 17, 10}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("王建国"), 55, Human::Gender::Male,
        QStringLiteral("D003"), QStringLiteral("主任医师"), QStringLiteral("神经内科"),
        {{Qt::Monday, 14, 17, 8}, {Qt::Thursday, 8, 11, 8}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("陈晓梅"), 39, Human::Gender::Female,
        QStringLiteral("D004"), QStringLiteral("主治医师"), QStringLiteral("消化内科"),
        {{Qt::Tuesday, 14, 17, 12}, {Qt::Friday, 8, 11, 12}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("刘洋"), 46, Human::Gender::Male,
        QStringLiteral("D005"), QStringLiteral("副主任医师"), QStringLiteral("普外科"),
        {{Qt::Wednesday, 8, 11, 10}, {Qt::Friday, 14, 17, 8}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("赵敏"), 37, Human::Gender::Female,
        QStringLiteral("D006"), QStringLiteral("主治医师"), QStringLiteral("妇产科"),
        {{Qt::Monday, 8, 11, 10}, {Qt::Wednesday, 8, 11, 10}, {Qt::Friday, 8, 11, 10}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("周强"), 44, Human::Gender::Male,
        QStringLiteral("D007"), QStringLiteral("副主任医师"), QStringLiteral("骨科"),
        {{Qt::Tuesday, 8, 11, 8}, {Qt::Thursday, 14, 17, 8}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("孙丽"), 41, Human::Gender::Female,
        QStringLiteral("D008"), QStringLiteral("副主任医师"), QStringLiteral("儿科"),
        {{Qt::Monday, 14, 17, 12}, {Qt::Wednesday, 14, 17, 12}, {Qt::Saturday, 8, 11, 10}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("吴磊"), 35, Human::Gender::Male,
        QStringLiteral("D009"), QStringLiteral("主治医师"), QStringLiteral("皮肤科"),
        {{Qt::Tuesday, 14, 17, 15}, {Qt::Thursday, 8, 11, 15}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("郑芳"), 50, Human::Gender::Female,
        QStringLiteral("D010"), QStringLiteral("主任医师"), QStringLiteral("眼科"),
        {{Qt::Monday, 8, 11, 10}, {Qt::Thursday, 8, 11, 10}, {Qt::Saturday, 14, 17, 8}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("黄俊"), 43, Human::Gender::Male,
        QStringLiteral("D011"), QStringLiteral("副主任医师"), QStringLiteral("耳鼻喉科"),
        {{Qt::Wednesday, 8, 11, 12}, {Qt::Friday, 14, 17, 12}}));
    hospital.addDoctor(makeDoctor(QStringLiteral("林静"), 38, Human::Gender::Female,
        QStringLiteral("D012"), QStringLiteral("主治医师"), QStringLiteral("内分泌科"),
        {{Qt::Tuesday, 8, 11, 10}, {Qt::Thursday, 14, 17, 10}}));

    addAppointments(hospital);

    if (!hospital.save()) {
        qCritical() << "Failed to save sample data to:" << outputDir;
        return 2;
    }

    for (int index = 1; index < outputDirs.size(); ++index) {
        if (!hospital.exportData(outputDirs.at(index))) {
            qCritical() << "Failed to save sample data to:" << outputDirs.at(index);
            return 2;
        }
    }

    for (const QString& directory : outputDirs) {
        if (!verifyData(directory, hospital.getDoctors().size(),
                        hospital.getAppointments().size())) {
            qCritical() << "Generated data could not be verified:" << directory;
            return 3;
        }
        qInfo() << "Saved" << hospital.getDoctors().size() << "doctors and"
                << hospital.getAppointments().size() << "appointments to" << directory;
    }
    return 0;
}

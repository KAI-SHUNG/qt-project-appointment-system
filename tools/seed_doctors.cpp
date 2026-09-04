#include "src/hospital.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
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

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    const QString outputDir = argc > 1
        ? QDir::fromNativeSeparators(QString::fromLocal8Bit(argv[1]))
        : QDir::current().filePath(QStringLiteral("data"));

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

    if (!hospital.save()) {
        qCritical() << "Failed to save sample data to:" << outputDir;
        return 2;
    }

    qInfo() << "Saved" << hospital.getDoctors().size() << "doctors to" << outputDir;
    return 0;
}

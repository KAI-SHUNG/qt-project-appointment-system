#ifndef DOCTORSPAGE_H
#define DOCTORSPAGE_H

#include <QWidget>

#include "src/hospital.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class DoctorsPage;
}
QT_END_NAMESPACE

class QResizeEvent;

// 医生管理：列表 / 过滤查询 / 新增编辑删除 / 出诊排班
class DoctorsPage : public QWidget {
    Q_OBJECT

public:
    explicit DoctorsPage(Hospital& hospital, QWidget* parent = nullptr);
    ~DoctorsPage() override;

public slots:
    void refresh();

protected:
    void resizeEvent(QResizeEvent* event) override;

signals:
    // 请求跳转到“预约挂号”并预选该医生
    void requestRegisterAppointment(const QString& doctorId);

private slots:
    void onAdd();
    void onSearch();
    void onReset();

private:
    void   fillDeptFilter();
    void   populateTable();
    void   adjustColumnWidths();
    void   addRowActions(int row, const QString& doctorId);
    QString scheduleText(const Doctor& doctor) const;
    QString currentDoctorId() const;
    QString nextFreeDoctorId() const;

    void editDoctor(const QString& doctorId);
    void deleteDoctor(const QString& doctorId);
    void scheduleDoctor(const QString& doctorId);
    void registerWithDoctor(const QString& doctorId);

    Ui::DoctorsPage* ui;
    Hospital&        hospital_;
};

#endif // DOCTORSPAGE_H

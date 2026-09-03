#ifndef DOCTORDIALOG_H
#define DOCTORDIALOG_H

#include <QDialog>

#include "src/doctor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class DoctorDialog;
}
QT_END_NAMESPACE

// 新增/编辑专家。setDoctor() 预填并锁定编号 -> 编辑模式；否则为新增模式
class DoctorDialog : public QDialog {
    Q_OBJECT

public:
    explicit DoctorDialog(QWidget* parent = nullptr);
    ~DoctorDialog() override;

    void   setDoctor(const Doctor& doctor); // 编辑：预填并锁定编号
    void   setSuggestedId(const QString& id); // 新增：预填一个可用编号
    Doctor doctor() const;                  // 由当前表单构建
    bool   editing() const;

private slots:
    void onOkClicked();

private:
    Ui::DoctorDialog* ui;
    bool              editing_ = false;
};

#endif // DOCTORDIALOG_H

#include "doctordialog.h"
#include "ui_doctordialog.h"
#include "../datadefs.h"

#include <QMessageBox>

DoctorDialog::DoctorDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::DoctorDialog)
{
    ui->setupUi(this);

    ui->cmbTitle->addItems(AppData::titles());
    ui->cmbDept->addItems(AppData::departments());
    ui->cmbTitle->setCurrentIndex(2); // 主治医师
    ui->btnOk->setProperty("primary", true);

    connect(ui->btnOk, &QPushButton::clicked, this, &DoctorDialog::onOkClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

DoctorDialog::~DoctorDialog()
{
    delete ui;
}

void DoctorDialog::setSuggestedId(const QString& id)
{
    ui->edtId->setText(id);
}

void DoctorDialog::setDoctor(const Doctor& doctor)
{
    editing_ = true;
    setWindowTitle(QStringLiteral("编辑专家"));

    ui->edtName->setText(doctor.getName());
    ui->cmbGender->setCurrentIndex(doctor.getGender() == Human::Gender::Male ? 0 : 1);
    ui->spnAge->setValue(doctor.getAge());
    ui->edtId->setText(doctor.getDoctorId());
    ui->cmbTitle->setEditText(doctor.getTitle());
    ui->cmbDept->setEditText(doctor.getDepartment());

    // 编辑时不允许改编号（预约与文件按编号关联）
    ui->edtId->setEnabled(false);
    ui->edtId->setToolTip(QStringLiteral("编号建立后不可修改"));
}

Doctor DoctorDialog::doctor() const
{
    return Doctor(
        ui->edtName->text().trimmed(),
        ui->spnAge->value(),
        ui->cmbGender->currentIndex() == 0 ? Human::Gender::Male : Human::Gender::Female,
        ui->edtId->text().trimmed(),
        ui->cmbTitle->currentText().trimmed(),
        ui->cmbDept->currentText().trimmed());
}

bool DoctorDialog::editing() const
{
    return editing_;
}

void DoctorDialog::onOkClicked()
{
    const QString name = ui->edtName->text().trimmed();
    const QString id   = ui->edtId->text().trimmed();
    const QString title = ui->cmbTitle->currentText().trimmed();
    const QString dept  = ui->cmbDept->currentText().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请输入专家姓名。"));
        return;
    }
    if (!editing_ && id.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"),
                             QStringLiteral("请输入或自动生成专家编号。"));
        return;
    }
    if (title.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请选择或填写职称。"));
        return;
    }
    if (dept.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请选择或填写科室。"));
        return;
    }

    accept();
}

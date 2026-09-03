#ifndef UI_PERSISTENCE_H
#define UI_PERSISTENCE_H

#include "src/hospital.h"

#include <QMessageBox>
#include <QWidget>

// 供各页面在每次增删改成功后调用，将内存数据写回磁盘。
// 返回 true 表示保存成功；失败时弹出警告并返回 false。
inline bool persistHospital(Hospital& hospital, QWidget* parent)
{
    if (hospital.save())
        return true;

    QMessageBox::warning(parent, QStringLiteral("保存数据"),
                         QStringLiteral("数据保存失败，请检查磁盘权限或文件路径。"));
    return false;
}

#endif // UI_PERSISTENCE_H

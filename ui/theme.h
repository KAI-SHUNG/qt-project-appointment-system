#ifndef UI_THEME_H
#define UI_THEME_H

#include <QString>

// 视觉样式以 resources/style.qss 为唯一来源。
// 本头文件只放“代码必须读取”的程序级/结构常量。
namespace Theme {

// 程序常量
inline const QString AppTitle = QStringLiteral("医院专家门诊预约管理系统");

// 表格结构几何（QSS 无法设置行高/列宽，故由代码读取）
inline constexpr int TableRowHeight     = 44;  // 医生表格默认行高
inline constexpr int ActionsColumnWidth = 170; // 医生表格操作列固定宽

} // namespace Theme

#endif // UI_THEME_H

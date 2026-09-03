#ifndef UI_THEME_H
#define UI_THEME_H

#include <QColor>
#include <QString>

// 浅色 Material 风格视觉常量（与 resources/style.qss 保持一致，供代码动态取色）
namespace Theme {

inline const QColor Primary(0x21, 0x96, 0xF3);
inline const QColor PrimaryHover(0x1E, 0x88, 0xE5);
inline const QColor PrimaryActive(0x19, 0x76, 0xD2);
inline const QColor PrimaryLight(0x33, 0x96, 0xF3, 0x10);

inline const QColor Success(0x4C, 0xAF, 0x50);
inline const QColor Warning(0xFF, 0x98, 0x00);
inline const QColor Danger(0xF4, 0x43, 0x36);

inline const QColor BgPage(0xF4, 0xF6, 0xF8);
inline const QColor CardBg(0xFF, 0xFF, 0xFF);
inline const QColor Border(0xE6, 0xEA, 0xF0);

inline const QColor TextMain(0x1F, 0x23, 0x29);
inline const QColor TextSub(0x5A, 0x60, 0x68);
inline const QColor TextWeak(0x9A, 0xA0, 0xA6);

inline const QString FontFamily = "Microsoft YaHei UI";
inline const QString AppTitle = "医院专家门诊预约管理系统";

} // namespace Theme

#endif // UI_THEME_H

#ifndef UI_DATADEFS_H
#define UI_DATADEFS_H

#include <QStringList>

// 供多个页面/弹窗复用的下拉选项
namespace AppData {

inline QStringList departments()
{
    return { QStringLiteral("心内科"),   QStringLiteral("消化内科"),
             QStringLiteral("呼吸内科"), QStringLiteral("神经内科"),
             QStringLiteral("骨科"),     QStringLiteral("儿科"),
             QStringLiteral("耳鼻喉科"), QStringLiteral("眼科"),
             QStringLiteral("皮肤科"),   QStringLiteral("内分泌科") };
}

inline QStringList titles()
{
    return { QStringLiteral("主任医师"), QStringLiteral("副主任医师"),
             QStringLiteral("主治医师"), QStringLiteral("医师"),
             QStringLiteral("住院医师") };
}

inline QStringList weekdays()
{
    return { QStringLiteral("周一"), QStringLiteral("周二"), QStringLiteral("周三"),
             QStringLiteral("周四"), QStringLiteral("周五"), QStringLiteral("周六"),
             QStringLiteral("周日") };
}

} // namespace AppData

#endif // UI_DATADEFS_H

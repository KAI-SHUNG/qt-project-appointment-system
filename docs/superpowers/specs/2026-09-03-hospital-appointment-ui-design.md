# 医院专家门诊预约管理系统 —— Qt Widgets UI 设计

- 日期：2026-09-03
- 技术栈：Qt 6.11.2 (MinGW, Windows) · qmake · Qt Widgets (C++17)
- 参考视觉：mengps/HuskarUI（Qt QML 版 Ant Design）→ 在本工程以 Widgets + QSS 复刻其**视觉语言**，不引入 QML

## 1. 背景与目标

课内 Qt 作业（README「题目2：医院专家门诊预约管理系统」）。现状：

- `src/` 数据模型已基本完成：`Human` → `Doctor` / `Patient`，`Timeslot`、`Appointment`。
- `src/hospital.{h,cpp}` 中 `add/remove/find/load/save` 及 getters 均为空实现（数据层未完成）。
- UI 为空壳：`mainwindow.ui` 无内容，`main.cpp` 仅显示空窗口。
- `.pro` 同时编译 `main.cpp` 与 `test/main_test.cpp`（两个 `main`）→ 需构建开关拆分。

本次任务交付：**现代浅色 UI（四功能页）+ 补全数据层**，页面真正可增删改查并读写 `.dat` 文件。

## 2. 需求 → 页面映射

| README 要求 | 承载页面 |
|---|---|
| 1. 创建/管理专家对象；7a 显示所有专家 | 医生管理 |
| 2. 创建/管理预约对象 | 预约挂号、预约查询 |
| 3. 在专家下添加预约、身份证不重复 | 预约挂号（注册流程）|
| 4. 专家、预约信息变更管理 | 医生管理(编辑/删除)、预约查询(改约/退号/改症状) |
| 5. 基本查询功能 | 医生管理搜索、预约查询筛选 |
| 6. 数据文件读写 | 主窗数据文件按钮 + `Hospital::load/save` |
| 7b 特定专家门诊预约显示 | 医生管理选中专家 → 预约查询联动 |
| 8. 可选：预约时间防重复/防满 | 时段容量校验（低于 Timeslot.capability 才放号）|

## 3. 关键技术决策

1. **Widgets + QSS，非 QML**。HuskarUI 是 QML 组件库；嵌入需 QtQuick 与重构，得不偿失。仅沿用其 Ant Design 视觉（扁平、卡片、轻阴影、主色强调）。
2. **Designer (.ui) 为主**。四个页面与三个弹窗各建 `.ui` 表单 + 同名包装类 `ui->setupUi(this)`；Qt Designer 可打开微调。无边框、标题栏行为、动态样式、行内按钮等 Designer 难表达的部分用代码补充。
3. **无边框自绘标题栏**。`Qt::FramelessWindowHint`；标题栏拖动移动、双击最大化/还原、窗口边缘缩放（Windows 下 `WM_NCHITTEST`，`#ifdef Q_OS_WIN`）。
4. **纯代码装 UI 壳**：`mainwindow.ui` 作为主壳（侧栏 + QStackedWidget），四个页面运行时 `addWidget`。
5. **主色 `#2196F3`（Material 蓝），浅色主题**。颜色/尺寸集中在 `ui/theme.h` + `resources/style.qss`。
6. **主窗口标题栏无 logo**（仅文字标题）。
7. 无边框后不再使用 `QMenuBar`；README 的“菜单/信号与槽”交互以按钮、表格、输入框的信号槽与 `QAction`-free 组件满足。

## 4. 总体布局

```
┌────────────────────────────────────────────────────────────┐
│ 医院专家门诊预约管理系统                    —   □   ✕     │ ← TitleBar 48px(代码) 可拖动
├──────────────┬─────────────────────────────────────────────┤
│ 首页概览      │  (页面标题区 + 右上动作/搜索工具条)         │
│ 医生管理      │                                             │
│ 预约挂号      │        contentStack (QStackedWidget)        │
│ 预约查询      │        四页切换，各自可滚动                  │
│              │                                             │
│ ······        │                                             │
│ 数据文件 保存/加载 │                                         │
└──────────────┴─────────────────────────────────────────────┘
```

- 窗口默认 1280×800，最小 1000×680。
- 侧栏固定宽 228，白底，右侧 `#E6EAF0` 分隔线；选中项为「主色浅底圆角胶囊 + 主色文字」。
- 底部固定区放「数据文件：加载 / 保存」两个按钮。
- 页面切换：`sideNav`(QListWidget) `currentRowChanged` → `contentStack::setCurrentIndex`，切页时页面 `refresh()`。
- 数据改动即 `Hospital` 内存态已变；保存由用户点「保存」触发；退出主窗若有未保存改动弹确认（保存/放弃/取消）。

## 5. 视觉 Token（写入 QSS）

- 主色 `#2196F3` / hover `#1E88E5` / active `#1976D2` / 主色浅底 `rgba(33,150,243,.10)`
- 成功 `#4CAF50` · 警告 `#FF9800` · 危险 `#F44336` · 信息 `#2196F3`
- 背景 `#F4F6F8`；卡片 `#FFFFFF`；卡片边框 `#E6EAF0`；卡片阴影 `0 2px 8px rgba(31,35,41,.06)`
- 文字 主 `#1F2329` / 次 `#5A6068` / 弱 `#9AA0A6`；分隔线 `#E6EAF0`
- 字体 `Microsoft YaHei UI`；标题 20px/600 · 卡片标题 16px/600 · 正文 14px · 注释 12px
- 圆角：卡片 10px、胶囊 999px、输入框 8px
- 组件样式：Primary/Default/Text 按钮、输入框 focus 主色描边、`QTableWidget` 无网格+行 hover、细圆角滚动条、`.card`/`.stat-card`/`.tag`/`.pill` 等 objectName 样式

## 6. 页面规格

> 每页 = `.ui` + `class XxxPage : QWidget`，对外提供 `refresh(const Hospital&)` 或持有 Hospital 指针；控件带稳定 objectName 便于 QSS 与代码定位。

### 6.1 首页概览 OverviewPage
- 统计卡 ×4：门诊专家数 / 科室数（去重）/ 今日预约数 / 预约总数，数值由 Hospital 现算。
- 「最近预约」QTableWidget 预览（最近 5 条：医生/病人/日期/时段/症状）。
- 快捷入口：去预约挂号 / 去医生管理（发射信号切页）。

### 6.2 医生管理 DoctorsPage
- 工具条：搜索 QLineEdit（姓名/编号/科室关键字）、科室过滤 QComboBox、`新增专家` PrimaryButton。
- 主表 QTableWidget 列：编号|姓名|性别|年龄|职称|科室|出诊段数|操作(排班|编辑|删除)。
- 操作列按钮为行内 QToolButton/QPushButton（代码生成）。
- 选中行 → 页脚「出诊排班卡」：周一到周日的 Timeslot 芯片（`周X HH:mm-HH:mm · 容量n · 已约m`）。
- 删除专家二次确认，文案提示将级联删除其所有预约。
- 刷新策略：任何变更后整体重载（数据层返回值为值容器，不长期持有指针）。

**DoctorDialog（新增/编辑专家）** `.ui` 字段：姓名、性别(Combo)、年龄(Spin 18–100)、编号(LineEdit，新增可留空自动生成且避开既有编号、编辑锁定)、职称(Combo：主任医师/副主任医师/主治医师/医师)、科室(可编辑 Combo，预置心内科/消化内科/呼吸内科/神经内科/骨科/儿科 等)、[确认/取消]。重复编号由数据层拒绝并展示 `lastError`。

> 编号自动生成策略：`D` + 3 位序号；取 `D001…` 中首个 `findDoctor` 未命中的号，保证跨会话、删除后仍唯一。

**TimeslotDialog（管理排班）** 上表列出该医生现有 Timeslot（周X 开始 结束 容量），提供新增行（周 Combo + QTimeEdit×2 + 容量 Spin）+ 删除选中。新增复用 `Doctor::addTimeslot`（内部冲突检测抛 `std::invalid_argument`），异常转中文提示。

### 6.3 预约挂号 RegisterPage
- 左列：医生下拉（QComboBox 展示 编号·姓名·科室）→ 医生信息卡（职称/科室/性别/年龄）。
- 选日期 `QCalendarWidget`（限制最近 60 天）。
- 该医生该日可约时段列表：时段行含 剩余号 chip（余 0 置灰禁点 / 余 ≤2 橙色 / 其余主色），点击选中并高亮。
- 右列病人表单：姓名、性别、年龄、身份证(QLineEdit)、电话、症状(QTextEdit)。
- 提交校验：身份证位数/基本格式；重复预约（同身份证已存在预约）拒绝；满号拒绝。失败显示 `Hospital::lastError()` 或页面提示。
- 成功 → 生成预约号 → 弹「预约单」信息框 → 清空表单并刷新号源。
- 预约号生成：`A` + yyyyMMdd + 3 位序号；取该前缀下首个 `findAppointment` 未命中的序号（同前缀跨会话、删号后仍唯一），上限 999 时提示当日已满。

### 6.4 预约查询 AppointmentsPage
- 工具条：按医生 QComboBox（含“全部”）、起止日期（QDateEdit ×2 可空）、关键字 LineEdit（病人姓名/身份证/预约号）、`查询`+`重置`。
- 表 QTableWidget 列：预约号|医生|科室|病人|性别|年龄|身份证|日期|时段|电话|症状|操作(详情|改约|退号)。
- 详情/改约弹 **AppointmentDialog**：查看病人与医生信息；可改日期+时段（重新校验容量/日期与星期一致）、改症状；退号二次确认后 `Hospital::removeAppointment`。
- 医生管理选中专家联动本页：只显示该专家预约。

## 7. 数据层补全（`Hospital` 及辅助）

补全空实现并新增帮助接口：

- `addDoctor`：doctorId 唯一（重复 → false + lastError）。
- `removeDoctor`：按 id 删除，并**级联删除**该医生所有预约。
- `findDoctor(id)` → `Doctor*`（nullptr 表示无）。
- `addAppointment` 前置规则：医生存在；appointId 唯一；**病人身份证号在既有预约中不可重复**；同一 `(doctorId, date, timeslot)` 已约数 < `capability`（防满/防重复）；否则失败并写 `lastError`。
- `removeAppointment(appointId)` / `findAppointment(appointId)`。
- `load()` / `save()`：分别串联 doctors/appointments；load 先医生后预约（预约依赖医生存在）。
- 健壮性：`loadAppointments` 中若某条预约引用的医生不存在（脏数据），跳过该条而非崩溃。
- getters：返回常量引用。
- 新增 `QString lastError() const`：记录最近一次失败原因，供 UI 提示。

> 指针有效性：返回的 `Doctor*`/`Appointment*` 在容器被改动前有效；页面一律「操作后整体重载」，不跨操作保存指针。

测试 `test/test_hospital.cpp`（沿用 assert + qDebug 风格）：编号重复拒绝；身份证防重拒绝；满号拒绝；级联删除；`save`→`load` 往返一致性；`find` 命中/未命中；`load` 空文件行为。

## 8. 构建与测试

`.pro` 加开关，避免两个 `main` 冲突：

```
isEmpty(BUILD_TESTS) {
  SOURCES += main.cpp            # GUI 应用（默认）
} else {
  SOURCES += test/main_test.cpp  # 控制台测试集
  CONFIG += console
}
```

- 模型源 `src/*.cpp` 恒编译；GUI 仅默认分支。
- `test/main_test.cpp` 将 `#include "test/test_*.cpp"` 改为前置声明（函数在各自 `test_*.cpp` 中定义并按 BUILD_TESTS 分支加入编译），避免函数多重定义。
- VS Code `tasks.json` 增加「Qt: build tests」任务（`qmake 09025117.pro BUILD_TESTS=1` → make）。
- README 增加「构建 / 运行 GUI」「运行测试」「数据结构与文件」说明。

## 9. 文件结构

```
09025117.pro                  # 修改：移除 FORMS 引用调整、BUILD_TESTS 开关
main.cpp                      # 不变
resources/
  style.qss                   # 全量样式
  resources.qrc               # qss + icons
  icons/*.svg                 # 侧栏/按钮单色图标
ui/
  theme.h                     # 颜色/尺寸常量
  titlebar.{h,cpp}            # 无边框标题栏（代码）
  mainwindow.{ui,h,cpp}       # 主壳：侧栏 + contentStack + 底部数据文件
  pages/overviewpage.{ui,h,cpp}
  pages/doctorspage.{ui,h,cpp}
  pages/registerpage.{ui,h,cpp}
  pages/appointmentspage.{ui,h,cpp}
  dialogs/doctordialog.{ui,h,cpp}
  dialogs/timeslotdialog.{ui,h,cpp}
  dialogs/appointmentdialog.{ui,h,cpp}
src/*                         # 模型不变 + hospital.cpp 补全
test/main_test.cpp            # 改为前置声明
test/test_hospital.cpp        # 新增
docs/superpowers/plans/…      # 实现计划
```

> `ui/mainwindow.h/.cpp` 由根目录迁移而来；旧根 `mainwindow.ui` 废弃移除，不再引用。

## 10. 实施顺序（预演，供计划细化）

1. 数据层：补全 `Hospital` + `test/test_hospital.cpp` + `.pro` 开关；先跑测试绿。
2. 主题与基座：`theme.h`、`style.qss`、qrc、`TitleBar`、`mainwindow.ui` 骨架与切页。
3. 医生管理（含 DoctorDialog/TimeslotDialog）。
4. 预约挂号。
5. 预约查询与退改（含 AppointmentDialog）。
6. 首页概览、联调、README、运行验证清单。

## 11. 非目标 / 已知边界

- 不做用户登录/角色权限。
- 不做多数据文件管理（固定 `data/doctors.dat`、`data/appointments.dat`）。
- 停诊管理（README 可选提升）本期不做。
- 身份证仅做位数与基本格式校验，不做真实 18 位校验位算法（可后续加）。

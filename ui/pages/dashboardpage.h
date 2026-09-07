#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include "src/hospital.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class DashboardPage;
}
QT_END_NAMESPACE

class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(Hospital& hospital, QWidget* parent = nullptr);
    ~DashboardPage() override;

public slots:
    void refresh();

signals:
    void navigateRequested(int pageIndex);

private:
    void refreshStatistics();
    void refreshUpcomingAppointments();
    void refreshDepartments();

    Ui::DashboardPage* ui;
    Hospital& hospital_;
};

#endif // DASHBOARDPAGE_H

#include "ui/mainwindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QFont font(QStringLiteral("Microsoft YaHei UI"), 10);
    font.setStyleStrategy(QFont::PreferAntialias);
    a.setFont(font);

    MainWindow w;
    w.show();
    return QApplication::exec();
}

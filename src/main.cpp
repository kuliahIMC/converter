
#include <QApplication>
#include "ui/main_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Marhaban Studio");
    QCoreApplication::setApplicationName("Marhaban Converter");

    MainWindow window;
    window.show();

    return app.exec();
}

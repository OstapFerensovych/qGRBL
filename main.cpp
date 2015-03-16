#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Bohdan Tymkiv");
    QCoreApplication::setOrganizationDomain("void.in.ua");
    QCoreApplication::setApplicationName("grblComm");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

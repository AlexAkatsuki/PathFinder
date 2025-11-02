#include <QApplication>
#include <QTranslator>
#include <QLocale>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("PathFinder");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ProSoft");

    MainWindow window;
    window.show();

    return app.exec();
}

#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    // TODO: consider app stylesheet
    //a.setStyleSheet("QwtPlotCanvas { background: black; } ");

    QCoreApplication::setOrganizationName("MayerLab");
    QCoreApplication::setOrganizationDomain("mrn.org");
    QCoreApplication::setApplicationName("MainStreamDropDAQ");

    MainWindow w;
    //w.show();
    //w.showFullScreen();
    w.showMaximized();
    return a.exec();
}

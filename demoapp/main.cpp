#include <QApplication>
#include <unistd.h>
#include <QDebug>
#include <QProcess>

#include <libals.h>
#include "mainwindow.h"
#include "card_reader.hpp"
#include <cstdio>

int main(int argc, char *argv[])
{
    // Suspend splash screen
    als::SplashScreen::Suspend();

    als::QtApp::SetupEnvironment(als::QtApp::AppType::WIDGET);

    // CardReader reader;
    // if (!reader.initialize())
    //     return 1;

    // qDebug() << "Waiting for the coupler to be ready...";
    // if (!reader.waitForReady(90000)) {
    //     printf("Coupler not ready - timeout.\n");
    //     reader.shutdown();
    //     return 1;
    // }

    // qDebug() << "Coupler ready.";
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

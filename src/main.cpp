#include "inc/mainwindow.h"
#include "inc/udpserver.h"
#include "inc/appcontroller.h"
#include <QApplication>
#include <QAbstractSocket>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("None");
    app.setOrganizationDomain("None");
    app.setApplicationName("TraceTerminalPlus");
    app.setApplicationVersion("1.0");

    app.setAttribute(Qt::AA_DontShowIconsInMenus);
    qRegisterMetaType<QHostAddress>("QHostAddress");

    MainWindow mainWindow;
    AppController appController(new UdpServer(), mainWindow.getLiveView());

    mainWindow.show();
    return app.exec();
}

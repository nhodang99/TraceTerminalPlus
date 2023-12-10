#include "inc/mainwindow.h"
#include "inc/udpserver.h"
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

    UdpServer server;
    MainWindow mainWindow;

    QObject::connect(mainWindow.getLiveView(), &TraceView::changeHost,
                     &server, &UdpServer::onHostChangeRequested, Qt::QueuedConnection);
    QObject::connect(mainWindow.getLiveView(), &TraceView::changePort,
                     &server, &UdpServer::onPortChangeRequested, Qt::QueuedConnection);
    QObject::connect(&server, &UdpServer::bindResult,
                     mainWindow.getLiveView(), &TraceView::onSocketBindResult, Qt::QueuedConnection);
    QObject::connect(&server, &UdpServer::newDataReady,
                     mainWindow.getLiveView(), &TraceView::onNewDataReady, Qt::QueuedConnection);

    server.initSocket();
    mainWindow.show();
    return app.exec();
}

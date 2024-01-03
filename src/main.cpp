#include "inc/mainwindow.h"
#include "inc/udpserver.h"
#include "inc/tracemanager.h"
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
    TraceManager& traceMgr = TraceManager::instance();

    // Connect server to trace view
    QObject::connect(&server, &UdpServer::bindResult,
                     mainWindow.getLiveView(), &TraceView::onSocketBindResult, Qt::QueuedConnection);

    // Connect trace view to server
    QObject::connect(mainWindow.getLiveView(), &TraceView::changeHost,
                     &server, &UdpServer::onHostChangeRequested, Qt::QueuedConnection);
    QObject::connect(mainWindow.getLiveView(), &TraceView::changePort,
                     &server, &UdpServer::onPortChangeRequested, Qt::QueuedConnection);

    // Connect server to trace manager
    QObject::connect(&server, &UdpServer::newDataReady,
                     &traceMgr, &TraceManager::onNewDataReady, Qt::QueuedConnection);

    server.initSocket();
    // Set view to manager like this can be bad,
    // but we need it to perform append view from non-gui thread and keep sending data in order
    traceMgr.setView(mainWindow.getLiveView());
    mainWindow.show();
    return app.exec();
}

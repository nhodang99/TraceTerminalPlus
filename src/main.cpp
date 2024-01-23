#include "inc/mainwindow.h"
#include "inc/traceserver.h"
#include "inc/tracemanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("None");
    app.setOrganizationDomain("None");
    app.setApplicationName("TraceTerminal++");
    app.setApplicationVersion("1.0");
    app.setAttribute(Qt::AA_DontShowIconsInMenus);

    TraceServer& server = TraceServer::instance();
    TraceManager& traceMgr = TraceManager::instance();
    MainWindow mainWindow;

    // Connect server to trace view
    QObject::connect(&server, &TraceServer::bindResult,
                     mainWindow.getLiveView(), &TraceView::onSocketBindResult, Qt::QueuedConnection);
    // Connect trace view to server
    QObject::connect(mainWindow.getLiveView(), &TraceView::interfaceChangeRequested,
                     &server, &TraceServer::onInterfaceChangeRequested, Qt::QueuedConnection);
    QObject::connect(mainWindow.getLiveView(), &TraceView::portChangeRequested,
                     &server, &TraceServer::onPortChangeRequested, Qt::QueuedConnection);

    // Connect server to trace manager
    QObject::connect(&server, &TraceServer::newDataReady,
                     &traceMgr, &TraceManager::onNewDataReady);
    // Connect manager to trace view
    QObject::connect(&traceMgr, &TraceManager::newTracesReady,
                     mainWindow.getLiveView(), &TraceView::onNewTracesReady, Qt::QueuedConnection);

    server.init();
    mainWindow.show();
    return app.exec();
}

#include "inc/mainwindow.h"
#include "inc/traceserver.h"
#include "inc/tracemanager.h"
#include "inc/searchdock.h"
#include <QApplication>
#include <QSettings>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("None");
    app.setOrganizationDomain("None");
    app.setApplicationName("TraceTerminal++");
    app.setApplicationVersion("1.4");
    app.setAttribute(Qt::AA_DontShowIconsInMenus);

    // initialize objects
    TraceServer& server = TraceServer::instance();
    TraceManager& traceManager = TraceManager::instance();

    auto liveView = new LiveTraceView;
    auto searchDock = new SearchDock;
    MainWindow mainWindow(liveView, searchDock);

    // Connect server to live view
    QObject::connect(&server, &TraceServer::bindResult,
                     liveView, &LiveTraceView::onSocketBindResult);
    // Connect live view to server
    QObject::connect(liveView, &LiveTraceView::interfaceChangeRequested,
                     &server, &TraceServer::onInterfaceChangeRequested);
    QObject::connect(liveView, &LiveTraceView::portChangeRequested,
                     &server, &TraceServer::onPortChangeRequested);

    // Connect server to trace manager
    QObject::connect(&server, &TraceServer::newDataReady,
                     &traceManager, &TraceManager::onNewDataReady, Qt::QueuedConnection);
    // Connect manager to live view
    QObject::connect(&traceManager, &TraceManager::newTracesReady,
                     liveView, &LiveTraceView::onNewTracesReady, Qt::QueuedConnection);

    server.init();
    mainWindow.show();
    int ret = app.exec();
    return ret;
}

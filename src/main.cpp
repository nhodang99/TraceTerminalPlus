#include "inc/mainwindow.h"
#include "inc/constants.h"
#include "inc/traceserver.h"
#include "inc/tracemanager.h"
#include "inc/searchdock.h"
#include <QApplication>
#include <QSettings>
//#include <QThread>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("None");
    app.setOrganizationDomain("None");
    app.setApplicationName("TraceTerminal++");
    app.setApplicationVersion("1.3");
    app.setAttribute(Qt::AA_DontShowIconsInMenus);

    // Read settings
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    bool autoscroll = settings.value(Config::TRACEVIEW_AUTOSCROLL, true).toBool();
    bool caseSensitive = settings.value(Config::SEARCH_CASESENSITIVE, false).toBool();
    bool loopSearch = settings.value(Config::SEARCH_LOOPSEARCH, false).toBool();

    // initialize objects
    TraceServer& server = TraceServer::instance();
    TraceManager& traceMgr = TraceManager::instance();
    auto liveView = new TraceView(true, autoscroll);
    auto searchDock = new SearchDock(nullptr, caseSensitive, loopSearch);
    MainWindow mainWindow(liveView, searchDock);

    // Connect server to trace view
    QObject::connect(&server, &TraceServer::bindResult,
                     mainWindow.getLiveView(), &TraceView::onSocketBindResult);
    // Connect trace view to server
    QObject::connect(mainWindow.getLiveView(), &TraceView::interfaceChangeRequested,
                     &server, &TraceServer::onInterfaceChangeRequested);
    QObject::connect(mainWindow.getLiveView(), &TraceView::portChangeRequested,
                     &server, &TraceServer::onPortChangeRequested);

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

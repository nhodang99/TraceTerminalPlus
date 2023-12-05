#include "appcontroller.h"
#include "udpserver.h"
#include "traceview.h"
#include <QSettings>
#include <QtConcurrent/QtConcurrentRun>
#include <QThread>

namespace {
const QString CONFIG     = QStringLiteral("./config.ini");
const QString INTERFACE  = QStringLiteral("udpSocket/interface");
}

AppController::AppController(UdpServer* server, TraceView* view)
    : m_server(server)
    , m_liveview(view)
{
    connect(m_server, &UdpServer::bindResult,
            this, &AppController::onSocketBindResult, Qt::QueuedConnection);
    connect(m_server, &UdpServer::newDataReady,
            this, &AppController::onNewDataReady, Qt::QueuedConnection);

    connect(m_liveview, &TraceView::changeInterface,
            this, &AppController::onItfChangeRequested, Qt::QueuedConnection);

    // Sending QString to gui thread from a non main thread must use signal slot
    connect(this, &AppController::newDataReady,
            m_liveview, &TraceView::onNewDataReady, Qt::QueuedConnection);

    QSettings settings(CONFIG, QSettings::IniFormat);
    auto addr = settings.value(INTERFACE, QHostAddress(QHostAddress::Any).toString()).toString();
    m_server->initSocket(QHostAddress(addr));
}

AppController::~AppController()
{
    QSettings settings(CONFIG, QSettings::IniFormat);
    settings.setValue(INTERFACE, m_currentItf.toString());

    delete m_server;
    m_server = nullptr;
}

///
/// \brief AppController::onItfChangeRequested
/// \param itf
///
void AppController::onItfChangeRequested(QHostAddress itf)
{
    if (m_currentItf == itf && m_lastBindSuccess)
    {
        return;
    }
    m_server->setHost(itf);
}

///
/// \brief AppController::onSocketBindResult
/// \param itf
/// \param res
///
void AppController::onSocketBindResult(QHostAddress itf, bool res)
{
    if (res)
    {
        m_currentItf = itf;
    }
    m_lastBindSuccess = res;
    m_liveview->onSocketBindResult(itf, res);
}

///
/// \brief AppController::onNewDataReady
/// \param data
///
void AppController::onNewDataReady(QStringList data)
{
    // Process incoming data in another thread
    QtConcurrent::run(this, &AppController::sendDataToView, data);
}

///
/// \brief Send data to view line by line
/// \param data
///
void AppController::sendDataToView(QStringList& data)
{
    QMutexLocker locker(&m_mutex);
    for (const auto& line : data)
    {
        emit newDataReady(line);
        // On windows, QThread::usleep is rounded up to 1ms...
        QThread::usleep(500);
    }
}
#include "inc/appcontroller.h"
#include "inc/udpserver.h"
#include "inc/traceview.h"
#include "inc/utils.h"
#include <QSettings>
#include <QtConcurrent/QtConcurrentRun>
#include <QThread>

AppController::AppController(UdpServer* server, TraceView* view)
    : m_server(server)
    , m_liveview(view)
{
    connect(m_server, &UdpServer::bindResult,
            this, &AppController::onSocketBindResult, Qt::QueuedConnection);
    connect(m_liveview, &TraceView::changeInterface,
            this, &AppController::onItfChangeRequested, Qt::QueuedConnection);
    connect(m_server, &UdpServer::newDataReady,
            this, &AppController::onNewDataReady, Qt::QueuedConnection);

    // Sending QString to gui thread from a non main thread must use signal slot
    connect(this, &AppController::newDataReady,
            m_liveview, &TraceView::onNewDataReady, Qt::QueuedConnection);

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    auto addr = settings.value(Config::INTERFACE, QHostAddress(QHostAddress::Any).toString()).toString();
    // Connecting port must be modified by user before starting the application
    auto port = settings.value(Config::PORT, 911).toInt();
    m_server->initSocket(QHostAddress(addr), port);
}

AppController::~AppController()
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    settings.setValue(Config::INTERFACE, m_currentItf.toString());

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
/// \brief Send data to view block by block
/// \note The usleep function on Windows is rounded up to 1ms, so if emit line by line
/// the scroll speed is very slow. Instead send the block of 3 lines.
/// \param data
///
void AppController::sendDataToView(QStringList& data)
{
    auto length = data.length();
    for (auto i = 0; i < length; i += 3)
    {
        auto textBlock = data.at(i);
        if (i + 1 < length)
        {
            textBlock += "<br>" + data.at(i + 1);
            if (i + 2 < length)
            {
                textBlock += "<br>" + data.at(i + 2);
            }
        }
        emit newDataReady(textBlock);
        // On windows, QThread::usleep is rounded up to 1ms anyway...
        QThread::msleep(1);
    }
}

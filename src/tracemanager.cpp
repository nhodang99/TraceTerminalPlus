#include "inc/tracemanager.h"
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QThread>

namespace
{
const int LOAD_TRACE_TIME = 10;
const int TRACE_LINE_PER_TIME = 4;
}

TraceManager::TraceManager()
{
    m_timer = new QTimer;
    m_timer->moveToThread(&m_sendTraceThread);
    m_timer->setInterval(LOAD_TRACE_TIME);
    connect(&m_sendTraceThread, SIGNAL(started()), m_timer, SLOT(start()));
    connect(&m_sendTraceThread, SIGNAL(finished()), m_timer, SLOT(stop()));
    connect(m_timer, &QTimer::timeout, this, &TraceManager::processAndSendTraceToViewAsync);
    m_sendTraceThread.start();
}

TraceManager::~TraceManager()
{
    m_sendTraceThread.quit();
    m_sendTraceThread.wait();
}

///
/// \brief TraceManager::instance
/// \return manager The only instance of the class
///
TraceManager& TraceManager::instance()
{
    static TraceManager unique;
    return unique;
}

///
/// \brief TraceManager::onNewDataReady
/// \param raw
///
void TraceManager::onNewDataReady(const QByteArray raw)
{
    // qDebug() << QString(raw);
    m_rawData += QString(raw);
}

///
/// \brief TraceManager::processAndSendTraceToView
/// \param data
///
void TraceManager::processAndSendTraceToViewAsync()
{
    QMutexLocker lock(&m_mutex);

    filterIncompletedFromRawData();
    sendPendingDataToView();
}

///
/// \brief TraceManager::filterIncompletedFromData
/// Last incoming data is not always a complete sentence
///
/// \param data
///
void TraceManager::filterIncompletedFromRawData()
{
    if (m_rawData.isEmpty())
    {
        //qDebug() << "Empty incoming data";
        return;
    }

    QString processedData;

    int length = m_rawData.length();
    // Remove the last \r\n, it introduces a new blank line after apppending text
    if (m_rawData.endsWith("\r\n"))
    {
        processedData = m_rawData.left(length - 2);
        m_rawData.clear();
    }
    else
    {
        int idx = m_rawData.lastIndexOf("\r\n");
        if (idx != -1)
        {
            processedData = m_rawData.left(idx);
            m_rawData = m_rawData.right(length - (idx + 1) - 1);
        }
        else
        {
            // Keep m_rawData as it is
        }
    }

    // Now append processedData into queue
    if (!processedData.isEmpty())
    {
        QStringList pendingDataArr = processedData.split("\r\n");
        for (const auto& data : pendingDataArr)
        {
            m_pendingTraces.enqueue(data);
        }
    }
}

void TraceManager::sendPendingDataToView()
{
    QStringList tracesToSend;
    for (int i = 0; i < TRACE_LINE_PER_TIME; ++i)
    {
        if (m_pendingTraces.isEmpty())
        {
            break;
        }
        tracesToSend.append(m_pendingTraces.dequeue());
    }
    if (!tracesToSend.isEmpty())
    {
        emit newTracesReady(tracesToSend);
    }
}

///
/// \brief TraceManager::readFile
/// \param url
/// \return
///
bool TraceManager::readFile(const QString& url, QString& data)
{
    data.clear();
    QFile file(url);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        data = in.readAll();
        return true;
    }
    return false;
}

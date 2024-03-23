#include "inc/tracemanager.h"
#include <QSettings>
#include <QFile>
#include <QDebug>

TraceManager::TraceManager()
{
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
    QString data = QString(raw);
    processAndSendTraceToView(data);
}

///
/// \brief TraceManager::filterIncompletedFromData
/// \param data
///
void TraceManager::filterIncompletedFromData(QString& data)
{
    if (data.isEmpty())
    {
        qDebug() << "Empty incoming data";
    }

    // Last incoming data is not always a complete sentence
    // prepend the incompleted data to the incoming one if has
    if (!m_pendingData.isEmpty())
    {
        data = m_pendingData + data;
        m_pendingData.clear();
    }

    int length = data.length();
    // Remove the last \r\n, it introduces a new blank line after apppending text
    if (data.endsWith("\r\n"))
    {
        data = data.left(length - 2);
    }
    else
    {
        int idx = data.lastIndexOf("\r\n");
        if (idx != -1)
        {
            m_pendingData = data.right(length - (idx + 1) - 1);
            data = data.left(idx);
        }
        else
        {
            m_pendingData = data;
            data.clear();
        }
    }
    //    qDebug() << "Return   :" << data;
    //    qDebug() << "Remaining:" << m_pendingData;
    //    qDebug() << "---------------------";
}

///
/// \brief TraceManager::processAndSendTraceToView
/// \param data
///
void TraceManager::processAndSendTraceToView(QString& data)
{
    if (data.isEmpty())
    {
        return;
    }

    filterIncompletedFromData(data);
    if (data.isEmpty())
    {
        return;
    }

    auto traces = data.split("\r\n");
    emit newTracesReady(traces);
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
        while (!in.atEnd())
        {
            data += in.readLine() + "\r\n";
        }
        return true;
    }
    return false;
}

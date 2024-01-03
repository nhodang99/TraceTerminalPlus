#include "inc/tracemanager.h"
#include "inc/constants.h"
#include <QSettings>
#include <QFileInfo>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>

TraceManager::TraceManager()
{
    defaultColors = {
        { "red",        " ERROR - " },
        { "orange",     " WARNG - " },
        { "purple",     " PANIC - " },
        { "slategrey",  " PRINT - " },
        { "black",      " TRACE - " },
        };

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    auto customs = settings.value(Config::CUSTOMS, QStringList()).toStringList();
    setCustoms(customs);
}

TraceManager::~TraceManager()
{
    // Do not delete the memory, it is managed in MainWindow
    m_view = nullptr;
}

///
/// \brief TraceManager::instance
/// \return manager The only instance of the class
///
TraceManager& TraceManager::instance()
{
    static TraceManager manager;
    return manager;
}

///
/// \brief TraceManager::setCustoms
/// \param list
///
void TraceManager::setCustoms(QStringList& list)
{
    if (list.length() != Highlight::CUSTOM_COLOR_NUMBER)
    {
        qDebug() << "custom highlight strings error" << list;
        return;
    }
    for (int i = 0; i < Highlight::CUSTOM_COLOR_NUMBER; ++i)
    {
        customColors[Highlight::customs[i]] = list[i];
    }

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    settings.setValue(Config::CUSTOMS, list);
}

///
/// \brief TraceManager::onNewDataReady
/// \param raw
///
void TraceManager::onNewDataReady(QByteArray raw)
{
    auto data = QString(raw);
    QtConcurrent::run(this, &TraceManager::processAndSendTraceToView, data);
}

///
/// \brief processTraceLine
/// \param line
///
void TraceManager::processTraceLine(QString& line)
{
    QString tag;
    const QString endTag = "</span>";
    QMapIterator<QString, QString> it(defaultColors);
    while(it.hasNext())
    {
        it.next();
        if (!it.value().isEmpty() && line.contains(it.value()))
        {
            tag = QString("<span style=\"color:%1\">").arg(it.key());
            break;
        }
    }

    //  The custom highlight can override the default one
    QMapIterator<QString, QString> cIt(customColors);
    while(cIt.hasNext())
    {
        cIt.next();
        if (!cIt.value().isEmpty() && line.contains(cIt.value()))
        {
            tag = QString("<span style=\"color:%1\">").arg(cIt.key());
            break;
        }
    }
    line.replace("\n", "<br>");
    if (!tag.isEmpty())
    {
        line = tag + line + endTag;
    }
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

    auto length = data.length();
    // Remove the last \r\n, it introduces a new blank line after apppending text
    if (data.endsWith("\r\n"))
    {
        data = data.left(length - 2);
    }
    else
    {
        auto idx = data.lastIndexOf("\r\n");
        // Normally we should be able to find idx
        if (idx != -1)
        {
            m_pendingData = data.right(length - (idx + 1) - 1);
            data = data.left(idx);
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
    QMutexLocker locker(&m_mutex);
    if (data.isEmpty())
    {
        return;
    }
    filterIncompletedFromData(data);
    auto traces = data.split("\r\n");
    for (auto& trace : traces)
    {
        processTraceLine(trace);
//        qDebug() << trace;
        // Calling slot append from a non-gui thread
        // It helps the UI not blocked: when text being appended user can still interract with the UI
        // Also, append line by line help us using blockNumber() to get the number of a line
        QMetaObject::invokeMethod(m_view, "append",
                                  Qt::QueuedConnection, Q_ARG(QString, trace));
    }
    QMetaObject::invokeMethod(m_view, "onNewTraceReady", Qt::QueuedConnection);
}

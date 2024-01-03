#ifndef TRACEMANAGER_H
#define TRACEMANAGER_H

#include <QObject>
#include <QMutex>
#include "inc/traceview.h"

class TraceManager : public QObject
{
    Q_OBJECT
public:
    ~TraceManager();
    static TraceManager& instance();
    void setCustoms(QStringList&);

    void setView(TraceView* view) { m_view = view; }
    void processTraceLine(QString&);

public slots:
    void onNewDataReady(QByteArray);

private:
    TraceManager();
    void filterIncompletedFromData(QString& data);
    void processAndSendTraceToView(QString& data);

    QMutex      m_mutex;
    QString     m_pendingData;
    TraceView*  m_view;
    QMap<QString, QString> defaultColors;
    QMap<QString, QString> customColors;
};

#endif // TRACEMANAGER_H

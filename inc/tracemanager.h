#ifndef TRACEMANAGER_H
#define TRACEMANAGER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QThread>

class TraceManager : public QObject
{
    Q_OBJECT
public:
    static TraceManager& instance();
    ~TraceManager();
    bool readFile(const QString& url, QString&);

public slots:
    void onNewDataReady(const QByteArray);

signals:
    void newTracesReady(QStringList);

private:
    TraceManager();
    void filterIncompletedFromRawData();
    void sendPendingDataToView();

    // Methods for async process
    void processAndSendTraceToViewAsync();

    QMutex          m_mutex;
    QString         m_rawData;

    QTimer*         m_timer{nullptr};
    QQueue<QString> m_pendingTraces;

    QThread         m_sendTraceThread;
};

#endif // TRACEMANAGER_H

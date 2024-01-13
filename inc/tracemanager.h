#ifndef TRACEMANAGER_H
#define TRACEMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
//#include <QMutex>

class TraceManager : public QObject
{
    Q_OBJECT
public:
    static TraceManager& instance();

    void setCustoms(QStringList&);
    void processTraceLine(QString&);

public slots:
    void onNewDataReady(const QByteArray);

signals:
    void newTracesReady(QStringList);

private:
    TraceManager();
    void filterIncompletedFromData(QString& data);
    void processAndSendTraceToView(QString& data);

//    QMutex      m_mutex;
    QString                m_pendingData;
    QMap<QString, QString> defaultColors;
    QMap<QString, QString> customColors;
};

#endif // TRACEMANAGER_H

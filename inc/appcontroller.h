#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QMutex>
#include <QHostAddress>

QT_BEGIN_NAMESPACE
class UdpServer;
class TraceView;
QT_END_NAMESPACE

class AppController : public QObject
{
    Q_OBJECT
public:
    AppController(UdpServer*, TraceView*);
    ~AppController();

    void onItfChangeRequested(QHostAddress);
    void processTraceData(QString&);

private slots:
    void onNewDataReady(QStringList);
    void onSocketBindResult(QHostAddress, bool);

signals:
    void newDataReady(QString);
private:
    void sendDataToView(QStringList&);
    QHostAddress m_currentItf{ QHostAddress::Any };
    UdpServer* m_server{ nullptr };
    TraceView* m_liveview{ nullptr };
    bool m_lastBindSuccess{true};

    //QMutex m_mutex;
};

#endif // APPCONTROLLER_H

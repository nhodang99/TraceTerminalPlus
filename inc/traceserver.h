#ifndef TRACESERVER_H
#define TRACESERVER_H

#include <QUdpSocket>
#include <QSerialPort>
#include <QTimer>

class TraceServer : public QObject
{
    Q_OBJECT
public:
    ~TraceServer();
    static TraceServer& instance();

    void init();
    void reinit();
    void onInterfaceChangeRequested(QString);
    void onPortChangeRequested(quint16);

private slots:
    void onReadyRead();
    void retryRemoteConnecting();

signals:
    void bindResult(QString, quint16, bool);
    void newDataReady(const QByteArray);

private:
    TraceServer();
    void configSerialPort();
    bool establishConnection();

    QUdpSocket*  m_udpSocket{nullptr};
    QString      m_interface{"0.0.0.0"};
    quint16      m_port{911};

    QSerialPort* m_serial{nullptr};

    bool         m_lastBindSuccess{true};
    QTimer*      m_timer;
};

#endif // TRACESERVER_H

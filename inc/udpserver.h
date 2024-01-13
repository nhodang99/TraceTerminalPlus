#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QUdpSocket>
#include <QTimer>

class UdpServer : public QUdpSocket
{
    Q_OBJECT
public:
    ~UdpServer();
    static UdpServer& instance();

    void initSocket();
    void reinitSocket();
    void onHostChangeRequested(QString);
    void onPortChangeRequested(quint16);

private slots:
    void onReadyRead();
    void retryRemoteConnecting();

signals:
    void bindResult(QString, quint16, bool);
    void newDataReady(const QByteArray);

private:
    UdpServer();

    QUdpSocket* m_udpSocket{nullptr};
    QString     m_host{"0.0.0.0"};
    quint16     m_port{911};
    bool        m_lastBindSuccess{true};
    QTimer*     m_timer;
};

#endif // UDPSERVER_H

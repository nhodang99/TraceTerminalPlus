#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QUdpSocket>

class UdpServer : public QUdpSocket
{
    Q_OBJECT
public:
    ~UdpServer();
    static UdpServer& instance();

    void initSocket();
    void reinitSocket();
    void onHostChangeRequested(QHostAddress);
    void onPortChangeRequested(quint16);

private slots:
    void onReadyRead();

signals:
    void bindResult(QHostAddress, quint16, bool);
    void newDataReady(QByteArray);

private:
    UdpServer();

    QUdpSocket*  m_udpSocket{nullptr};
    QHostAddress m_host{QHostAddress::Any};
    quint16      m_port{911};
    bool         m_lastBindSuccess{true};
};

#endif // UDPSERVER_H

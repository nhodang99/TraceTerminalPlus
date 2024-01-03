#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QUdpSocket>

class UdpServer : public QUdpSocket
{
    Q_OBJECT
public:
    UdpServer();
    ~UdpServer();

    void initSocket();
    void setInterface(QHostAddress&, quint16);
    void onHostChangeRequested(QHostAddress);
    void onPortChangeRequested(quint16);

private slots:
    void onReadyRead();

signals:
    void bindResult(QHostAddress, quint16, bool);
    void newDataReady(QByteArray);

private:
    QUdpSocket* m_udpSocket{ nullptr };

    QHostAddress m_host{ QHostAddress::Any };
    quint16      m_port{ 911 };
    bool         m_lastBindSuccess{true};
};

#endif // UDPSERVER_H

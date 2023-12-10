#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QUdpSocket>
#include <QMutex>

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
    //void newDataReady(QStringList);
    void bindResult(QHostAddress, quint16, bool);
    void newDataReady(QString);

private:
    void processRawData(const QByteArray&);

    QUdpSocket* m_udpSocket{ nullptr };
    QString     m_remainingData; // Save the incompleted data that do not form a sentence
    QMutex      m_mutex;

    QHostAddress m_host{ QHostAddress::Any }; // @todo: where to save these value?
    quint16      m_port{ 911 };               //        controller or server?
    bool         m_lastBindSuccess{true};
};

#endif // UDPSERVER_H

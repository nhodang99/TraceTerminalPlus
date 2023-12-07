#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QUdpSocket>
#include <QMutex>

const quint16 PORT = 911;

class UdpServer : public QUdpSocket
{
    Q_OBJECT
public:
    UdpServer();
    ~UdpServer();

    void initSocket(QHostAddress hostAddr = QHostAddress::Any);
    void setHost(QHostAddress&);

private slots:
    void onReadyRead();

signals:
    void newDataReady(QStringList);
    void bindResult(QHostAddress, bool);

private:
    void processRawData(const QByteArray&);

    QUdpSocket *m_udpSocket{ nullptr };
    QString m_remainingData; // Save the incompleted data that do not form a sentence
    QMutex m_mutex;
};

#endif // UDPSERVER_H

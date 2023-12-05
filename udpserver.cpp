#include "udpserver.h"
#include <QNetworkDatagram>
#include <qDebug>
#include <QtConcurrent/QtConcurrentRun>
#include "utils.h"

UdpServer::UdpServer()
{
    m_remainingData.clear();
}

UdpServer::~UdpServer()
{
    m_udpSocket->close();
    delete m_udpSocket;
    m_udpSocket = nullptr;
}

///
/// \brief UdpServer::initSocket
/// \param hostAddr
/// \param port
///
void UdpServer::initSocket(QHostAddress hostAddr)
{
    m_udpSocket = new QUdpSocket(this);
    bool success = m_udpSocket->bind(hostAddr, PORT, QAbstractSocket::DontShareAddress);
    emit bindResult(hostAddr, success);

    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &UdpServer::onReadyRead);
}

///
/// \brief Set host address to listen to
/// \param host: host address (local, remote IPv4,..)
///
void UdpServer::setHost(QHostAddress& hostAddr)
{
    m_udpSocket->close();
    bool success = m_udpSocket->bind(hostAddr, PORT, QAbstractSocket::DontShareAddress);
    emit bindResult(hostAddr, success);
}

///
/// \brief slot to receive the datagram and send it to controller
///
void UdpServer::onReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        // Process incoming data in another thread
        QtConcurrent::run(this, &UdpServer::processRawData, datagram.data());
    }
}

///
/// \brief UdpServer::processRawData
/// \param data
///
void UdpServer::processRawData(const QByteArray& raw)
{
    QMutexLocker locker(&m_mutex);
    if (raw.isEmpty())
    {
        qDebug() << "Empty data";
        return;
    }
    auto data = QString(raw);
    // Last incoming data is not always a complete sentence
    // Save the incompleted data and prepend it to the new data if has
    if (!m_remainingData.isEmpty())
    {
        data = m_remainingData + data;
        m_remainingData.clear();
    }

    // Remove the last \r\n, it introduces a new blank line after inserts text
    if (data.endsWith("\r\n"))
    {
        data = data.left(data.length() - 2);
    }
    else
    {
        auto idx = data.lastIndexOf("\r\n");
        m_remainingData = data.right(data.length() - (idx + 1) - 1);
        data = data.left(idx);
    }

    // Make every single line rich text
    auto textList = data.split("\r\n");
    for (auto& text : textList)
    {
        processLine(text);
    }
    //qDebug() << textList;
    //qDebug() << "-----------------";

    emit newDataReady(textList);
}

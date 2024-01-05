#include "inc/udpserver.h"
#include "inc/constants.h"
#include <QNetworkDatagram>
#include <qDebug>
#include <QSettings>

UdpServer::UdpServer()
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    m_host = settings.value(Config::CONNECTED_HOST, QHostAddress(QHostAddress::Any).toString()).toString();
    m_port = settings.value(Config::CONNECTED_PORT, 911).toInt();
}

UdpServer::~UdpServer()
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    settings.setValue(Config::CONNECTED_HOST, m_host.toString());
    settings.setValue(Config::CONNECTED_PORT, m_port);

    m_udpSocket->close();
    delete m_udpSocket;
    m_udpSocket = nullptr;
}

///
/// \brief UdpServer::instance
/// \return
///
UdpServer& UdpServer::instance()
{
    static UdpServer unique;
    return unique;
}

///
/// \brief UdpServer::initSocket
/// \param hostAddr
/// \param port
///
void UdpServer::initSocket()
{
    m_udpSocket = new QUdpSocket(this);
    m_lastBindSuccess = m_udpSocket->bind(m_host, m_port, QAbstractSocket::DontShareAddress);
    emit bindResult(m_host, m_port, m_lastBindSuccess);

    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &UdpServer::onReadyRead);
}

///
/// \brief UdpServer::onHostChangeRequested
/// \param host
///
void UdpServer::onHostChangeRequested(QHostAddress host)
{
    if (m_host == host && m_lastBindSuccess)
    {
        return;
    }
    m_host = host;
    reinitSocket();
}

///
/// \brief UdpServer::onPortChangeRequested
/// \param port
///
void UdpServer::onPortChangeRequested(quint16 port)
{
    if (m_port == port && m_lastBindSuccess)
    {
        return;
    }
    m_port = port;
    reinitSocket();
}

///
/// \brief UdpServer::reinitSocket
///
void UdpServer::reinitSocket()
{
    m_udpSocket->close();
    auto success = m_udpSocket->bind(m_host, m_port, QAbstractSocket::DontShareAddress);
    emit bindResult(m_host, m_port, success);
    m_lastBindSuccess = success;
}

///
/// \brief slot to receive the datagram and send it to trace manager
///
void UdpServer::onReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams())
    {
        auto datagram = m_udpSocket->receiveDatagram();
        auto raw = datagram.data();
        emit newDataReady(raw);
    }
}

#include "inc/udpserver.h"
#include "inc/constants.h"
#include <QNetworkDatagram>
#include <qDebug>
#include <QSettings>

UdpServer::UdpServer()
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    m_host = settings.value(Config::HOST, QHostAddress(QHostAddress::Any).toString()).toString();
    m_port = settings.value(Config::PORT, 911).toInt();
}

UdpServer::~UdpServer()
{
    m_udpSocket->close();
    delete m_udpSocket;
    m_udpSocket = nullptr;

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    settings.setValue(Config::HOST, m_host.toString());
    settings.setValue(Config::PORT, m_port);
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
    setInterface(host, m_port);
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
    setInterface(m_host, port);
}

///
/// \brief Set host address to listen to
/// \param host: host address (local, remote IPv4,..)
///
void UdpServer::setInterface(QHostAddress& hostAddr, quint16 port)
{
    m_udpSocket->close();
    m_lastBindSuccess = m_udpSocket->bind(hostAddr, port, QAbstractSocket::DontShareAddress);
    if (m_lastBindSuccess)
    {
        m_host = hostAddr;
        m_port = port;
    }
    emit bindResult(hostAddr, port, m_lastBindSuccess);
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

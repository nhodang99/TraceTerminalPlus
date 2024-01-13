#include "inc/udpserver.h"
#include "inc/constants.h"
#include <QNetworkDatagram>
#include <qDebug>
#include <QSettings>
#include <QNetworkInterface>

namespace
{
const int BINDING_RETRY_TIME = 500;
}

QHostAddress toHostAddress(QString& addr, bool& isRemote)
{
    isRemote = false;
    QHash<QString, QHostAddress> hash = {
        { SpecialInterface::ANY_INTERFACE,       QHostAddress::Any },
        { SpecialInterface::ANY_IPV6_INTERFACE,  QHostAddress::AnyIPv6 },
        { SpecialInterface::LOCALHOST_INTERFACE, QHostAddress::LocalHost }
    };
    auto host = hash[addr];
    if (host.isNull())
    {
        isRemote = true;
        return QHostAddress(addr);
    }
    return host;
}

UdpServer::UdpServer()
{
//    foreach(const QHostAddress &laddr, QNetworkInterface::allAddresses())
//    {
//        qDebug() << "Found IP:" << laddr.toString();
//    }
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    m_host = settings.value(Config::CONNECTED_HOST, QHostAddress(QHostAddress::Any).toString()).toString();
    m_port = settings.value(Config::CONNECTED_PORT, 911).toInt();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &UdpServer::retryRemoteConnecting);
}

UdpServer::~UdpServer()
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    settings.setValue(Config::CONNECTED_HOST, m_host);
    settings.setValue(Config::CONNECTED_PORT, m_port);

    m_udpSocket->close();
    delete m_udpSocket;
    m_udpSocket = nullptr;

    if (m_timer->isActive())
    {
        m_timer->stop();
    }
    delete m_timer;
    m_timer = nullptr;
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
/// \param port
///
void UdpServer::initSocket()
{
    m_udpSocket = new QUdpSocket(this);
    bool isRemote = false;
    auto res = m_udpSocket->bind(toHostAddress(m_host, isRemote), m_port, QAbstractSocket::DontShareAddress);
    if (!res)
    {
        qDebug() << "init failed" << m_udpSocket->errorString();
        if (isRemote)
        {
            m_timer->start(BINDING_RETRY_TIME);
        }
    }
    emit bindResult(m_host, m_port, res);
    m_lastBindSuccess = res;

    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &UdpServer::onReadyRead);
}

///
/// \brief UdpServer::onHostChangeRequested
/// \param host
///
void UdpServer::onHostChangeRequested(QString host)
{
    qDebug() << m_host << host << m_lastBindSuccess;
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
    if (m_timer->isActive())
    {
        m_timer->stop();
    }
    bool isRemote = false;
    auto res = m_udpSocket->bind(toHostAddress(m_host, isRemote), m_port, QAbstractSocket::DontShareAddress);
    if (!res)
    {
        qDebug() << "Reinit failed" << m_udpSocket->errorString();
        if (isRemote)
        {
            m_timer->start(BINDING_RETRY_TIME);
        }
    }
    emit bindResult(m_host, m_port, res);
    m_lastBindSuccess = res;
}

///
/// \brief slot to receive the datagram and send it to trace manager
///
void UdpServer::onReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams())
    {
        auto datagram = m_udpSocket->receiveDatagram();
        emit newDataReady(datagram.data());
    }
}

void UdpServer::retryRemoteConnecting()
{
    qDebug() << "Retry Remote connecting";
    reinitSocket();
}

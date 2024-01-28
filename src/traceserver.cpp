#include "inc/traceserver.h"
#include "inc/constants.h"
#include <QNetworkDatagram>
#include <qDebug>
#include <QSettings>
#include <QNetworkInterface>
#include <QSerialPortInfo>

namespace
{
const int BINDING_RETRY_TIME = 500;
}

static QHostAddress toHostAddress(QString& addr, bool& retryOnFail)
{
    retryOnFail = false;
    QHash<QString, QHostAddress> hash = {
        { SpecialInterface::ANY_INTERFACE,       QHostAddress::Any },
        { SpecialInterface::ANY_IPV6_INTERFACE,  QHostAddress::AnyIPv6 },
        { SpecialInterface::LOCALHOST_INTERFACE, QHostAddress::LocalHost },
    };
    auto host = hash[addr];
    if (host.isNull())
    {
        retryOnFail = true;
        return QHostAddress(addr);
    }
    return host;
}

TraceServer::TraceServer()
    : m_udpSocket(new QUdpSocket(this))
    , m_serial(new QSerialPort(this))
{
#if 0
    foreach(const QHostAddress &laddr, QNetworkInterface::allAddresses())
    {
        qDebug() << "Found IP:" << laddr.toString();
    }
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &portInfo, serialPortInfos)
    {
        qDebug() << "\n"
                 << "Port:" << portInfo.portName() << "\n"
                 << "Location:" << portInfo.systemLocation() << "\n"
                 << "Description:" << portInfo.description() << "\n"
                 << "Manufacturer:" << portInfo.manufacturer() << "\n"
                 << "Serial number:" << portInfo.serialNumber() << "\n"
                 << "Vendor Identifier:"
                 << (portInfo.hasVendorIdentifier()
                         ? QByteArray::number(portInfo.vendorIdentifier(), 16)
                         : QByteArray()) << "\n"
                 << "Product Identifier:"
                 << (portInfo.hasProductIdentifier()
                         ? QByteArray::number(portInfo.productIdentifier(), 16)
                         : QByteArray());
    }
#endif

    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    m_interface = settings.value(Config::INTERFACE, QHostAddress(QHostAddress::Any).toString()).toString();
    m_port = settings.value(Config::PORT, 911).toInt();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TraceServer::retryRemoteConnecting);
    // Setup default value serial port
    configSerialPort();
}

TraceServer::~TraceServer()
{
    QSettings settings(Config::CONFIG_DIR, QSettings::IniFormat);
    settings.setValue(Config::INTERFACE, m_interface);
    settings.setValue(Config::PORT, m_port);

    if (m_timer->isActive())
    {
        m_timer->stop();
    }
    delete m_timer;
    m_timer = nullptr;

    // UDP socket
    if (m_udpSocket->state() != QUdpSocket::UnconnectedState)
    {
        m_udpSocket->close();
    }
    delete m_udpSocket;
    m_udpSocket = nullptr;
    // Serial port
    if (m_serial->isOpen())
    {
        m_serial->close();
    }
    delete m_serial;
    m_serial = nullptr;
}

///
/// \brief UdpServer::instance
/// \return
///
TraceServer& TraceServer::instance()
{
    static TraceServer unique;
    return unique;
}

///
/// \brief UdpServer::init
///
void TraceServer::init()
{
    bool res = establishConnection();
    emit bindResult(m_interface, m_port, res);

    connect(m_udpSocket, &QUdpSocket::readyRead, this, &TraceServer::onReadyRead);
    connect(m_serial, &QSerialPort::readyRead, this, &TraceServer::onReadyRead);
}

///
/// \brief UdpServer::reinit
///
void TraceServer::reinit()
{
    if (m_timer->isActive())
    {
        m_timer->stop();
    }
    if (m_serial->isOpen())
    {
        m_serial->close();
    }
    if (m_udpSocket->state() != QUdpSocket::UnconnectedState)
    {
        m_udpSocket->close();
    }

    bool res = establishConnection();
    emit bindResult(m_interface, m_port, res);
}

///
/// \brief TraceServer::configSerialPort
///
void TraceServer::configSerialPort()
{
    m_serial->setPortName("COM1");
    m_serial->setBaudRate(115200);              // Baud rate for the CAN converter
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setParity(QSerialPort::NoParity);
}

///
/// \brief TraceServer::establishConnection
/// \return
///
bool TraceServer::establishConnection()
{
    bool res = false;
    if (m_interface == SpecialInterface::SERIAL_INTERFACE)
    {
        res = m_serial->open(QIODevice::ReadOnly);
        if (!res)
        {
            qDebug() << "Open serial failed" << m_serial->errorString();
            return res;
        }
        m_serial->setDataTerminalReady(true); // Enables DTR line when opened, and leaves it on
        m_serial->setRequestToSend(true);     // Enables RTS line when opened, and leaves it on
    }
    else
    {
        bool retryOnFail = false;
        auto host = toHostAddress(m_interface, retryOnFail);
        res = m_udpSocket->bind(host, m_port, QAbstractSocket::DontShareAddress);
        if (!res)
        {
            qDebug() << "Bind udp failed" << m_udpSocket->errorString();
            if (retryOnFail)
            {
                m_timer->start(BINDING_RETRY_TIME);
            }
        }
    }
    return res;
}

///
/// \brief slot to receive the new data and send it to trace manager
///
void TraceServer::onReadyRead()
{
    QByteArray data;
    if (m_interface == SpecialInterface::SERIAL_INTERFACE)
    {
        data = m_serial->readAll();
        emit newDataReady(data);
    }
    else
    {
        while (m_udpSocket->hasPendingDatagrams())
        {
            auto datagram = m_udpSocket->receiveDatagram();
            data = datagram.data();
            emit newDataReady(data);
        }
    }
}

///
/// \brief UdpServer::onInterfaceChangeRequested
/// \param itf
///
void TraceServer::onInterfaceChangeRequested(QString itf)
{
    m_interface = itf;
    reinit();
}

///
/// \brief UdpServer::onPortChangeRequested
/// \param port
///
void TraceServer::onPortChangeRequested(quint16 port)
{
    m_port = port;
    reinit();
}

///
/// \brief TraceServer::retryRemoteConnecting
///
void TraceServer::retryRemoteConnecting()
{
    qDebug() << "Retry Remote connecting";
    reinit();
}

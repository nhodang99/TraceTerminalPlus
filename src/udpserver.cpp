#include "inc/udpserver.h"
#include "inc/utils.h"
#include <QNetworkDatagram>
#include <qDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QSettings>

UdpServer::UdpServer()
{
    m_remainingData.clear();
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
/// \brief Send data to view block by block
/// \note The usleep function on Windows is rounded up to 1ms, so if emit line by line
/// the scroll speed is very slow. Instead send the block of 3 lines.
/// \param raw
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

    // Remove the last \r\n, it introduces a new blank line after apppending text
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
//        qDebug() << text;
//        qDebug() << "-----------------";
        processLine(text);
    }

    // TEST
    auto length = textList.length();
    for (auto i = 0; i < length; i += 3)
    {
        auto textBlock = textList.at(i);
        if (i + 1 < length)
        {
            textBlock += "<br>" + textList.at(i + 1);
            if (i + 2 < length)
            {
                textBlock += "<br>" + textList.at(i + 2);
            }
        }
        emit newDataReady(textBlock);
        // On windows, QThread::usleep is rounded up to 1ms anyway...
        QThread::msleep(1);
    }
}

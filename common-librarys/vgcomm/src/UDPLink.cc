#include <QtGlobal>
#if QT_VERSION > 0x050401
#define UDP_BROKEN_SIGNAL 1
#else
#define UDP_BROKEN_SIGNAL 0
#endif

#define QGC_UDP_LOCAL_PORT 14550

#include <QTimer>
#include <QList>
#include <QDebug>
#include <QMutexLocker>
#include <QNetworkProxy>
#include <QNetworkInterface>
#include <iostream>
#include <QUdpSocket>

#include "UDPLink.h"
#include "base.h"
#include <QHostInfo>
#include <QSettings>
#include <QThread>
#include <QDateTime>

#define REMOVE_GONE_HOSTS 0

static const char* kZeroconfRegistration = "_qgroundcontrol._udp";

static bool is_ip(const QString& address)
{
    int a,b,c,d;
    if (sscanf(address.toStdString().c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4
            && strcmp("::1", address.toStdString().c_str())) {
        return false;
    } else {
        return true;
    }
}

static QString get_ip_address(const QString& address)
{
    if(is_ip(address))
        return address;
    // Need to look it up
    QHostInfo info = QHostInfo::fromName(address);
    if (info.error() == QHostInfo::NoError)
    {
        QList<QHostAddress> hostAddresses = info.addresses();
        QHostAddress address;
        for (int i = 0; i < hostAddresses.size(); i++)
        {
            // Exclude all IPv6 addresses
            if (!hostAddresses.at(i).toString().contains(":"))
            {
                return hostAddresses.at(i).toString();
            }
        }
    }
    return QString("");
}

UDPLink::UDPLink(UDPCommand* cmd):LinkInterface(cmd)
, m_socket(NULL), m_thread(new QThread)
#if defined(QGC_ZEROCONF_ENABLED)
, _dnssServiceRef(NULL)
#endif
{
    if (m_thread)
        moveToThread(m_thread);

    connect(this, &UDPLink::_sendUdp, this, &UDPLink::sendUdp);
    m_thread->start(QThread::NormalPriority);
}

UDPLink::~UDPLink()
{
    _disconnect();
    delete m_socket;
    delete m_thread;
}

void UDPLink::_restartConnection()
{
    if(this->isConnected())
    {
        _disconnect();
        _connect();
    }
}

void UDPLink::writeBytes(const QByteArray &array)
{
    if (!m_socket || !m_thread || !m_thread->isFinished())
        return;

     m_outQueue << array;
     emit sendUdp();
}

bool UDPLink::_dequeBytes()
{
    UDPCommand *cmd = udpCommand();
    QString host;
    int port;
    if (!cmd || !cmd->GetLastRcvHost(host, port))
        return false;

    while (m_outQueue.count() > 0 && m_socket)
    {
        QByteArray qdata = m_outQueue.takeFirst();
        QHostAddress currentHost(host);
        if (m_socket->writeDatagram(qdata.data(), qdata.size(), currentHost, (quint16)port) >= 0)
            _logOutputDataRate(qdata.size(), QDateTime::currentMSecsSinceEpoch());
    }
    return (m_outQueue.count() > 0);
}

UDPCommand *UDPLink::udpCommand() const
{
    return dynamic_cast<UDPCommand *>(getLinkCommand());
}

void UDPLink::readBytes()
{
    QByteArray databuffer;
    while (m_socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        m_socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        databuffer.append(datagram);
        //-- Wait a bit before sending it over
        if(databuffer.size() > 10 * 1024)
        {
            emit bytesReceived(this, databuffer);
            databuffer.clear();
        }
        _logInputDataRate(datagram.length(), QDateTime::currentMSecsSinceEpoch());
        if (UDPCommand *cmd = dynamic_cast<UDPCommand *>(getLinkCommand()))
            cmd->SetLastRcvHost(sender.toString(), (int)senderPort);

        if(!m_thread || m_thread->isRunning())
            break;
    }
    //-- Send whatever is left
    if(databuffer.size()>0)
        emit bytesReceived(this, databuffer);
}

void UDPLink::sendUdp()
{
    _dequeBytes();
}

void UDPLink::_disconnect(void)
{
    if (!m_thread || !m_thread->isRunning())
        return;

    m_thread->quit();
    m_thread->wait();
    if (m_socket)
    {
        m_socket->deleteLater();
        m_socket = NULL;
        emit connected(false);
    }
    m_connectState = false;
}

bool UDPLink::_connect(void)
{
    if (!m_thread)
        m_thread = new QThread();

    if(m_thread && m_thread->isRunning())
    {
        m_thread->quit();
        m_thread->wait();
    }
    _hardwareConnect();
    return true;
}

bool UDPLink::_hardwareConnect()
{
    UDPCommand *cmd = udpCommand();
    if (!cmd || !cmd->IsValid())
        return false;

    if (m_socket)
    {
        m_socket->deleteLater();
        m_socket = NULL;
    }
    m_socket = new QUdpSocket();
    m_socket->setProxy(QNetworkProxy::NoProxy);
    m_connectState = m_socket->bind(QHostAddress(cmd->GetHost()), cmd->localPort(), QAbstractSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    if (m_connectState)
    {
        //-- Make sure we have a large enough IO buffers
#ifdef __mobile
        m_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption,     64 * 1024);
        m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 128 * 1024);
#else
        m_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption,    256 * 1024);
        m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 512 * 1024);
#endif
        _registerZeroconf(cmd->localPort(), kZeroconfRegistration);
        //-- Connect signal if this version of Qt is not broken
        
        connect(m_socket, &QUdpSocket::readyRead, this, &UDPLink::readBytes);
        emit connected(true);
    }
    else
    {
        emit communicationError("UDP Link Error", "Error binding UDP port");
    }
    return m_connectState;
}

/**
 * @brief Check if connection is active.
 *
 * @return True if link is connected, false otherwise.
 **/
bool UDPLink::isConnected() const
{
    return m_connectState;
}

qint64 UDPLink::getConnectionSpeed() const
{
    return 54000000; // 54 Mbit
}

qint64 UDPLink::getCurrentInDataRate() const
{
    return 0;
}

qint64 UDPLink::getCurrentOutDataRate() const
{
    return 0;
}

void UDPLink::_registerZeroconf(uint16_t port, const std::string &regType)
{
#if defined(QGC_ZEROCONF_ENABLED)
    DNSServiceErrorType result = DNSServiceRegister(&_dnssServiceRef, 0, 0, 0,
        regType.c_str(),
        NULL,
        NULL,
        htons(port),
        0,
        NULL,
        NULL,
        NULL);
    if (result != kDNSServiceErr_NoError)
    {
        emit communicationError("UDP Link Error", "Error registering Zeroconf");
        _dnssServiceRef = NULL;
    }
#else
    Q_UNUSED(port);
    Q_UNUSED(regType);
#endif
}

void UDPLink::_deregisterZeroconf()
{
#if defined(QGC_ZEROCONF_ENABLED)
    if (_dnssServiceRef)
     {
         DNSServiceRefDeallocate(_dnssServiceRef);
         _dnssServiceRef = NULL;
     }
#endif
}
//--------------------------------------------------------------------------
//-- UDPCommand
//--------------------------------------------------------------------------
UDPCommand::UDPCommand(QObject *p) : LinkCommand(p)
, m_rcvPort(-1)
{
    loadSettings(*Application::Instance()->GetSettings(), "udpLinkConfig");
}

UDPCommand::~UDPCommand()
{
    disconnectLink();
    delete m_link;
}

LinkCommand::LinkType UDPCommand::type()const
{
    return LinkCommand::TypeUdp;
}

void UDPCommand::connectLink()
{
    if (m_link)
        m_link->deleteLater();

    UDPLink *link = new UDPLink(this);
    m_link = link;
    if (link)
        link->_connect();
}

QString UDPCommand::getName() const
{
    return QString("%1:%2").arg(m_hostName).arg(m_localPort);
}

void UDPCommand::setName(const QString &)
{
}

void UDPCommand::setHost(const QString host, uint16_t port)
{
    m_hostName = host;
    m_localPort = port;
    emit hostChanged(host);
    emit localPortChanged(port);
    emit nameChanged(getName());
}

void UDPCommand::SetLastRcvHost(const QString &host, int port)
{
    QString host_add;
    if (host.contains(":"))
        m_rcvHost = host.split(":").last();
    else
        m_rcvHost = host;

    m_rcvPort = port;
}

bool UDPCommand::GetLastRcvHost(QString& host, int& port)
{
    host = m_rcvHost;
    port = m_rcvPort;
    return !host.isEmpty() && port != 0;
}

quint16 UDPCommand::localPort() const
{
    return m_localPort;
}

void UDPCommand::setLocalPort(quint16 port)
{
    m_localPort = port;
}

QString UDPCommand::GetHost() const
{
    return m_hostName;
}

bool UDPCommand::IsValid() const
{
    return m_localPort > 0 && !m_hostName.isEmpty();
}

void UDPCommand::saveSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    settings.setValue("localPort", m_localPort);
    settings.setValue("ip", m_hostName);
    settings.endGroup();
}

void UDPCommand::loadSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    m_localPort = (quint16)settings.value("localPort", QGC_UDP_LOCAL_PORT).toUInt();
    m_hostName = settings.value("ip", "127.0.0.1").toString();
    settings.endGroup();
}

void UDPCommand::updateSettings()
{
    if (UDPLink* ulink = dynamic_cast<UDPLink*>(m_link))
        ulink->_restartConnection();
}

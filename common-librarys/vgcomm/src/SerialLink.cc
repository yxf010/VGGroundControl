/*=====================================================================
======================================================================*/
/**
 * @file
 *   @brief Cross-platform support for serial ports
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <QMutexLocker>

#ifdef __android__
#include "qserialport.h"
#else
#include <QSerialPort>
#endif

#include "SerialLink.h"
#include "base.h"
#include <QThread>
#include <QDateTime>

#include "SerialPortInfo.h"

static QStringList kSupportedBaudRates;
SerialLink::SerialLink(SerialCommand* cmd) :LinkInterface(cmd)
, _bytesRead(0), _port(NULL), _stopp(false), _reqReset(false)
{
    qDebug() << "Create SerialLink " << cmd->getName() << cmd->baud() << cmd->flowControl()
             << cmd->parity() << cmd->dataBits() << cmd->stopBits();
}

void SerialLink::requestReset()
{
    QMutexLocker locker(&this->_stoppMutex);
    _reqReset = true;
}

SerialLink::~SerialLink()
{
    _disconnect();
    if(_port)
        delete _port;
    _port = NULL;
}

bool SerialLink::_isBootloader()
{
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    if( portList.count() == 0){
        return false;
    }
 
    foreach (const QSerialPortInfo &info, portList)
    {
        if ( info.portName().trimmed() == devName() &&
             (     info.description().toLower().contains("bootloader")
                || info.description().toLower().contains("px4 bl")
                || info.description().toLower().contains("px4 fmu v1.6")) )
            return true;
    }
    // Not found
    return false;
}

void SerialLink::writeBytes(const QByteArray &array)
{
    if(_port && _port->isOpen())
    {
        _logOutputDataRate(array.size(), QDateTime::currentMSecsSinceEpoch());
        _port->write(array.data(), array.size());
    }
    else
    {
        _emitLinkError(tr("Could not send data - link %1 is disconnected!").arg(devName()));
    }
}

/**
 * @brief Disconnect the connection.
 *
 * @return True if connection has been disconnected, false if connection couldn't be disconnected.
 **/
void SerialLink::_disconnect(void)
{
    if (_port)
    {
        _port->close();
        delete _port;
        _port = NULL;
        emit connected(false);
    }
#ifdef __android__
    //qgcApp()->toolbox()->linkManager()->suspendConfigurationUpdates(false);
#endif
}

/**
 * @brief Connect the connection.
 *
 * @return True if connection has been established, false if connection couldn't be established.
 **/
bool SerialLink::_connect(void)
{
    _disconnect();
#ifdef __android__
    //qgcApp()->toolbox()->linkManager()->suspendConfigurationUpdates(true);
#endif
    QSerialPort::SerialPortError    error;
    QString                         errorString;
    // Initialize the connection
    if (!_hardwareConnect(error, errorString))
    {
        _emitLinkError(QString("Error connecting: Could not create port. %1").arg(errorString));
        return false;
    }
    return true;
}

/// Performs the actual hardware port connection.
///     @param[out] error if failed
///     @param[out] error string if failed
/// @return success/fail
bool SerialLink::_hardwareConnect(QSerialPort::SerialPortError& error, QString& errorString)
{
    if (_port)
    {
        _port->close();
        QThread::sleep(50000);
        delete _port;
        _port = NULL;
    }

    if (_isBootloader())
    {
        const unsigned retry_limit = 12;
        unsigned retries;
        for (retries = 0; retries < retry_limit; retries++)
        {
            if (!_isBootloader())
            {
                QThread::msleep(500);
                break;
            }
            QThread::msleep(500);
        }
        // Check limit
        if (retries == retry_limit)
        {
            // bail out
            qDebug() << "Timeout waiting for something other than booloader";
            return false;
        }
    }

    _port = new QSerialPort(devName());
    if (!_port)
        return false; // couldn't create serial port.

    QObject::connect(_port, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
                     this, &SerialLink::linkError);
    QObject::connect(_port, &QIODevice::readyRead, this, &SerialLink::_readBytes);

#ifdef __android__
    _port->open(QIODevice::ReadWrite);
#else
    for (int openRetries = 0; openRetries < 4; openRetries++)
    {
        if (!_port->open(QIODevice::ReadWrite))
        {
            qDebug() << "Port open failed, retrying";
            QThread::msleep(500);
        }
        else 
        {
            break;
        }
    }
#endif
    if (!_port->isOpen())
    {
        qDebug() << "open failed" << _port->errorString() << _port->error();
        error = _port->error();
        errorString = _port->errorString();
        _port->close();
        delete _port;
        _port = NULL;
        return false; // couldn't open serial port
    }

	_port->setDataTerminalReady(true);
    qDebug() << "Configuring port";
    if (SerialCommand *cmd = dynamic_cast<SerialCommand*>(getLinkCommand()))
    {
        _port->setBaudRate(cmd->baud());
        _port->setDataBits(static_cast<QSerialPort::DataBits>(cmd->dataBits()));
        _port->setFlowControl(static_cast<QSerialPort::FlowControl>(cmd->flowControl()));
        _port->setStopBits(static_cast<QSerialPort::StopBits>(cmd->stopBits()));
        _port->setParity(static_cast<QSerialPort::Parity>(cmd->parity()));
        emit communicationUpdate(devName(), "Opened port!");
        emit connected(true);
        qDebug() << "Connection SeriaLink: " << "with settings" << cmd->getName()
            << cmd->baud() << cmd->dataBits() << cmd->parity() << cmd->stopBits();

        return true; // successful connection
    }
    return false;
}

void SerialLink::_readBytes(void)
{
    qint64 byteCount = _port->bytesAvailable();
    if (byteCount > 0)
    {
        QByteArray buffer;
        buffer.resize(byteCount);
        _port->read(buffer.data(), buffer.size());
        emit bytesReceived(this, buffer);
    }
}

void SerialLink::linkError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError)
        qDebug() << "SerialLink::linkError" << error;
}

bool SerialLink::isConnected() const
{
    bool bConnected = false;
    if (_port)
        bConnected = _port->isOpen();

    return bConnected;
}

qint64 SerialLink::getConnectionSpeed() const
{
    int baudRate = 0;
    if (_port)
        baudRate = _port->baudRate();
    else if (SerialCommand *cmd = dynamic_cast<SerialCommand*>(getLinkCommand()))
        baudRate = cmd->baud();
 
    qint64 dataRate;
    switch (baudRate)
    {
        case QSerialPort::Baud1200:
            dataRate = 1200;
            break;
        case QSerialPort::Baud2400:
            dataRate = 2400;
            break;
        case QSerialPort::Baud4800:
            dataRate = 4800;
            break;
        case QSerialPort::Baud9600:
            dataRate = 9600;
            break;
        case QSerialPort::Baud19200:
            dataRate = 19200;
            break;
        case QSerialPort::Baud38400:
            dataRate = 38400;
            break;
        case QSerialPort::Baud57600:
            dataRate = 57600;
            break;
        case QSerialPort::Baud115200:
            dataRate = 115200;
            break;
        default:
            dataRate = -1;
            break;
    }
    return dataRate;
}

void SerialLink::_resetConfiguration()
{
    SerialCommand *cmd = dynamic_cast<SerialCommand*>(getLinkCommand());
    if (_port && cmd)
    {
        _port->setBaudRate      (cmd->baud());
        _port->setDataBits      (static_cast<QSerialPort::DataBits> (cmd->dataBits()));
        _port->setFlowControl   (static_cast<QSerialPort::FlowControl>(cmd->flowControl()));
        _port->setStopBits      (static_cast<QSerialPort::StopBits>(cmd->stopBits()));
        _port->setParity        (static_cast<QSerialPort::Parity>(cmd->parity()));
    }
}

void SerialLink::_emitLinkError(const QString& errorMsg)
{
    QString msg("Error on link %1. %2");
    emit communicationError(QString::fromStdWString(L"串口通信错误"), msg.arg(devName()).arg(errorMsg));
}
//--------------------------------------------------------------------------
//-- SerialCommand

SerialCommand::SerialCommand(QObject *p) : LinkCommand(p)
{
    m_baud       = 57600;
    m_flowControl= QSerialPort::NoFlowControl;
    m_parity     = QSerialPort::NoParity;
    m_dataBits   = 8;
    m_stopBits   = 1;
    m_usbDirect  = false;
}

int SerialCommand::baud() const
{
    return m_baud;
}

int SerialCommand::dataBits() const
{
    return m_dataBits;
}

int SerialCommand::flowControl() const
{
    return m_flowControl;
}

int SerialCommand::stopBits() const
{
    return m_stopBits;
}

int SerialCommand::parity() const
{
    return m_parity;
}

bool SerialCommand::usbDirect() const
{
    return m_usbDirect;
}

void SerialCommand::updateSettings()
{
    if(m_link) {
        SerialLink* serialLink = dynamic_cast<SerialLink*>(m_link);
        if(serialLink)
            serialLink->_resetConfiguration();
    }
}

QString SerialCommand::getName() const
{
    return m_portName;
}

void SerialCommand::setName(const QString& portName)
{
    if (!portName.isEmpty() && portName != m_portName)
    {
        m_portName = portName;
        m_portDisplayName = cleanPortDisplayname(portName);
        emit nameChanged(portName);
        emit portDisplayNameChanged();
    }
}

QString SerialCommand::portDisplayName() const
{
    return m_portDisplayName;
}

void SerialCommand::connectLink()
{
    if (m_link)
        m_link->deleteLater();

    SerialLink *link = new SerialLink(this);
    m_link = link;
    link->_connect();
}

void SerialCommand::setBaud(int baud)
{
    m_baud = baud;
}

void SerialCommand::setDataBits(int databits)
{
    m_dataBits = databits;
}

void SerialCommand::setFlowControl(int flowControl)
{
    m_flowControl = flowControl;
}

void SerialCommand::setStopBits(int stopBits)
{
    m_stopBits = stopBits;
}

void SerialCommand::setParity(int parity)
{
    m_parity = parity;
}

QString SerialCommand::cleanPortDisplayname(const QString name)
{
    QString pname = name.trimmed();
#ifdef Q_OS_WIN
    pname.replace("\\\\.\\", "");
#else
    pname.replace("/dev/cu.", "");
    pname.replace("/dev/", "");
#endif
    return pname;
}

LinkCommand::LinkType SerialCommand::type() const
{
    return TypeSerial;
}

void SerialCommand::saveSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    settings.setValue("baud", m_baud);
    settings.setValue("dataBits", m_dataBits);
    settings.setValue("flowControl", m_flowControl);
    settings.setValue("stopBits", m_stopBits);
    settings.setValue("parity", m_parity);
    settings.setValue("portName",       m_portName);
    settings.setValue("portDisplayName",m_portDisplayName);
    settings.endGroup();
}

void SerialCommand::loadSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    if(settings.contains("baud"))
        m_baud           = settings.value("baud").toInt();
    if(settings.contains("dataBits"))
        m_dataBits       = settings.value("dataBits").toInt();
    if(settings.contains("flowControl"))
        m_flowControl    = settings.value("flowControl").toInt();
    if(settings.contains("stopBits"))
        m_stopBits       = settings.value("stopBits").toInt();
    if(settings.contains("parity"))
        m_parity         = settings.value("parity").toInt();
    if(settings.contains("portName"))
        m_portName       = settings.value("portName").toString();
    if(settings.contains("portDisplayName"))
        m_portDisplayName= settings.value("portDisplayName").toString();
    settings.endGroup();
}

QStringList SerialCommand::supportedBaudRates()
{
    if(!kSupportedBaudRates.size())
        _initBaudRates();
    return kSupportedBaudRates;
}

void SerialCommand::_initBaudRates()
{
    kSupportedBaudRates.clear();
#if USE_ANCIENT_RATES
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
    kSupportedBaudRates << "50";
    kSupportedBaudRates << "75";
#endif
    kSupportedBaudRates << "110";
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
    kSupportedBaudRates << "134";
    kSupportedBaudRates << "150";
    kSupportedBaudRates << "200";
#endif
    kSupportedBaudRates << "300";
    kSupportedBaudRates << "600";
    kSupportedBaudRates << "1200";
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
    kSupportedBaudRates << "1800";
#endif
#endif
    kSupportedBaudRates << "2400";
    kSupportedBaudRates << "4800";
    kSupportedBaudRates << "9600";
#if defined(Q_OS_WIN)
    kSupportedBaudRates << "14400";
#endif
    kSupportedBaudRates << "19200";
    kSupportedBaudRates << "38400";
#if defined(Q_OS_WIN)
    kSupportedBaudRates << "56000";
#endif
    kSupportedBaudRates << "57600";
    kSupportedBaudRates << "115200";
#if defined(Q_OS_WIN)
    kSupportedBaudRates << "128000";
#endif
    kSupportedBaudRates << "230400";
#if defined(Q_OS_WIN)
    kSupportedBaudRates << "256000";
#endif
    kSupportedBaudRates << "460800";
#if defined(Q_OS_LINUX)
    kSupportedBaudRates << "500000";
    kSupportedBaudRates << "576000";
#endif
    kSupportedBaudRates << "921600";
}

void SerialCommand::setUsbDirect(bool usbDirect)
{
    if (m_usbDirect != usbDirect)
    {
        m_usbDirect = usbDirect;
        emit usbDirectChanged(usbDirect);
    }
}

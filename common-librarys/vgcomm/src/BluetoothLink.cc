#include <QtGlobal>
#include <QTimer>
#include <QList>
#include <QDebug>

#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QBluetoothSocket>
#include <QSettings>
#include <QDateTime>

#include "BluetoothLink.h"

#define JDK_TCUID "FFE1"   //透传特征
#define MAX_WRITTEN 20     //蓝牙一次最多写20字节

BluetoothLink::BluetoothLink(LinkCommand *cmd) : LinkInterface(cmd)
, m_connectState(false), m_leCtrl(NULL), m_leSvc(NULL), m_bJDK(false)
#ifdef __ios__
, _discoveryAgent(NULL)
#endif
{
}

BluetoothLink::~BluetoothLink()
{
    _disconnect();
#ifdef __ios__
    if(_discoveryAgent)
    {
        _discoveryAgent->stop();
        _discoveryAgent->deleteLater();
        _discoveryAgent = NULL;
    }
#endif
}

LinkCommand* BluetoothLink::getLinkCommand() const
{
    return dynamic_cast<LinkCommand*>(parent());
}

void BluetoothLink::_restartConnection()
{
    if(this->isConnected())
    {
        _disconnect();
        _connect();
    }
}

void BluetoothLink::characteristicWritten(const QLowEnergyCharacteristic &, const QByteArray &)
{
}

void BluetoothLink::characteristicChanged(const QLowEnergyCharacteristic &chr, const QByteArray &array)
{
    if (!m_bJDK && m_chrWrite != chr && (chr.properties()&QLowEnergyCharacteristic::Write))
        m_chrWrite = chr;

    emit bytesReceived(this, array);
    _logInputDataRate(array.length(), QDateTime::currentMSecsSinceEpoch());
}

void BluetoothLink::writeBytes(const QByteArray &array)
{
    m_sendBuff.append(array);
    if (array.size() > 0)
        _send();
}

void BluetoothLink::_send()
{
    if (m_leSvc && m_chrWrite.isValid())
    {
        int nSend = m_sendBuff.size() < MAX_WRITTEN ? m_sendBuff.size() : MAX_WRITTEN;
        m_leSvc->writeCharacteristic(m_chrWrite, m_sendBuff.left(nSend));
        m_sendBuff.remove(0, nSend);
    }
    if (m_sendBuff.size() > 0)
        _send();
}

BluetoothCommand * BluetoothLink::bltCommand()const
{
    return dynamic_cast<BluetoothCommand*>(getLinkCommand());
}

void BluetoothLink::_prcsServiceDiscovered(QLowEnergyService *svc)
{
    foreach(const QLowEnergyCharacteristic &var, svc->characteristics())
    {
        if (m_bJDK)
            continue;

        m_bJDK = var.uuid().toString().contains(JDK_TCUID, Qt::CaseInsensitive);//补丁，JDK_TCUID JDK-模块的透传特征
        bool bCur = false;
        QLowEnergyDescriptor dsc = var.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_bJDK || dsc.isValid())
        {
            if (m_leSvc && m_leSvc != svc)
                m_leSvc->deleteLater();
            svc->writeDescriptor(dsc, QByteArray::fromHex("0100"));  //回写，接受数据
            m_leSvc = svc;
            bCur = true;
            m_connectState = true;
            emit connected(true);
        }
        bCur = (bCur || !m_chrWrite.isValid()) && m_leSvc == svc && (var.properties() & QLowEnergyCharacteristic::Write);
        if (m_bJDK || bCur)
            m_chrWrite = var;
    }
    if (m_leSvc != svc)
        svc->deleteLater();
}

void BluetoothLink::_disconnect(void)
{
#ifdef __ios__
    if(_discoveryAgent) {
        _shutDown = true;
        _discoveryAgent->stop();
        _discoveryAgent->deleteLater();
        _discoveryAgent = NULL;
    }
#endif
    if(m_leCtrl)
    {
        m_leCtrl->disconnectFromDevice();
        m_leCtrl->deleteLater();
        m_leCtrl = NULL;
        m_leSvc->deleteLater();
        m_leSvc = NULL;
    }
    if (m_connectState)
    {
        m_connectState = false;
        emit connected(false);
    }
}

bool BluetoothLink::_connect(void)
{
    _hardwareConnect();
    return true;
}

bool BluetoothLink::_hardwareConnect()
{
    BluetoothCommand *cmd = bltCommand();
    if (!cmd || !cmd->device().isValid())
        return false;

    if (m_leCtrl)
        m_leCtrl->deleteLater();

    if (m_leSvc)
        m_leSvc->deleteLater();

    m_leSvc = NULL;
    m_chrWrite = QLowEnergyCharacteristic();
    if (m_leCtrl = QLowEnergyController::createCentral(cmd->device(), this))
    {
        //connect(m_leCtrl, &QLowEnergyController::serviceDiscovered, this, &DeviceDiscoveryDialog::addDiscovered);
        m_bJDK = false;
        QObject::connect(m_leCtrl, &QLowEnergyController::discoveryFinished, this, &BluetoothLink::finishScanService);
        QObject::connect(m_leCtrl, &QLowEnergyController::disconnected, this, &BluetoothLink::disconnect);
        QObject::connect(m_leCtrl, &QLowEnergyController::stateChanged, this, &BluetoothLink::ctrlStateChanged);
        QObject::connect(m_leCtrl, SIGNAL(error(QLowEnergyController::Error)),
            this, SLOT(deviceError(QLowEnergyController::Error)));

        m_leCtrl->connectToDevice();
    }
    return true;
}

void BluetoothLink::deviceConnected()
{
    m_leCtrl->discoverServices();
}

void BluetoothLink::deviceDisconnected()
{
    m_connectState = false;
    emit connected(false);
}

void BluetoothLink::deviceError(QLowEnergyController::Error error)
{
    m_connectState = false;
    qWarning() << "BlueTooth error" << error;
    emit communicationError("BlueTooth Link Error", m_leCtrl->errorString());
}

void BluetoothLink::finishScanService()
{
    if (m_leCtrl)
    {
        foreach (const QBluetoothUuid &uuid, m_leCtrl->services())
        {
            addDiscovered(uuid);
        }
    }
}

void BluetoothLink::disconnect()
{
    _disconnect();
}

void BluetoothLink::ctrlStateChanged(QLowEnergyController::ControllerState st)
{
    switch (st)
    {
    case QLowEnergyController::UnconnectedState:
        deviceDisconnected();
        break;
    case QLowEnergyController::ConnectedState:
        deviceConnected();
        break;
    default:
        break;
    }
}

void BluetoothLink::serviceStateChanged(QLowEnergyService::ServiceState st)
{
    QLowEnergyService *svc = qobject_cast<QLowEnergyService *>(sender());
    if (!svc)
        return;

    switch (st)
    {
    case QLowEnergyService::ServiceDiscovered:
        _prcsServiceDiscovered(svc);
        break;
    default:
        break;
    }
}

void BluetoothLink::addDiscovered(const QBluetoothUuid &service)
{
    QLowEnergyService *svc = m_leCtrl->createServiceObject(service, this);
    if (svc && svc->type() == QLowEnergyService::IncludedService)
    {
        svc->deleteLater();
    }
    else if (svc)
    {
        QObject::connect(svc, &QLowEnergyService::stateChanged, this, &BluetoothLink::serviceStateChanged);
        //QObject::connect(svc, &QLowEnergyService::characteristicWritten, this, &BluetoothGPS::characteristicWritten);
        QObject::connect(svc, &QLowEnergyService::characteristicChanged, this, &BluetoothLink::characteristicChanged);
        QObject::connect(svc, &QLowEnergyService::characteristicRead, this, &BluetoothLink::characteristicChanged);

        svc->discoverDetails();
    }
}

void BluetoothLink::requestReset()
{
    if (BluetoothCommand *cmd = bltCommand())
        cmd->deleteLater();
}

bool BluetoothLink::isConnected() const
{
    return m_connectState;
}

qint64 BluetoothLink::getConnectionSpeed() const
{
    return 100000; // 1 Mbit
}

qint64 BluetoothLink::getCurrentInDataRate() const
{
    return 0;
}

qint64 BluetoothLink::getCurrentOutDataRate() const
{
    return 0;
}

bool BluetoothCommand::IsBluetoothAvailable()
{
    QBluetoothLocalDevice localDevice;
    if (localDevice.isValid())
        return true;

#ifdef _DEBUG
    return true;
#else
    return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//-- BluetoothCommand
///////////////////////////////////////////////////////////////////////////////
QBluetoothDeviceDiscoveryAgent* BluetoothCommand::s_deviceDiscover = NULL;
QList<QBluetoothDeviceInfo> BluetoothCommand::s_deviceList;
bool BluetoothCommand::s_bScan = false;
QStringList BluetoothCommand::s_devList;

BluetoothCommand::BluetoothCommand(QObject *p): LinkCommand(p)
{
    if (!s_deviceDiscover)
    {
        s_deviceDiscover = new QBluetoothDeviceDiscoveryAgent();
        emit scanningChanged(s_bScan);
    }

    connect(s_deviceDiscover, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothCommand::deviceDiscovered);
    connect(s_deviceDiscover, &QBluetoothDeviceDiscoveryAgent::finished, this, &BluetoothCommand::doneScanning);
}

BluetoothCommand::~BluetoothCommand()
{
}

void BluetoothCommand::saveSettings(QSettings& settings, const QString& root)
{
    if (!m_device.isValid())
        return;

    settings.beginGroup(root);
    settings.setValue("deviceName", m_device.name());
#ifdef __ios__
    settings.setValue("uuid",_device.uuid());
#else
    settings.setValue("address",m_device.address().toString());
#endif
    settings.endGroup();
}

void BluetoothCommand::loadSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    QString name  = settings.value("deviceName").toString();
#ifdef __ios__
    QBluetoothUuid address(settings.value("uuid", _device.uuid.toString()).toString());
#else
    QBluetoothAddress  address(settings.value("address").toString());
#endif
    if (!name.isEmpty() && !address.isNull())
        m_device = QBluetoothDeviceInfo(address, name, 0);

    settings.endGroup();
}

void BluetoothCommand::updateSettings()
{
    if(m_link)
    {
        BluetoothLink *ulink = dynamic_cast<BluetoothLink*>(m_link);
        if(ulink)
            ulink->_restartConnection();
    }
}

void BluetoothCommand::connectLink()
{
    if (m_link)
        m_link->deleteLater();

    BluetoothLink *ulink = new BluetoothLink(this);
    m_link = ulink;
    if (ulink)
        ulink->_connect();
}

void BluetoothCommand::stopScan()
{
    if(s_deviceDiscover)
    {
        s_deviceDiscover->stop();
        s_bScan = false;
        emit scanningChanged(s_bScan);
    }
}

QString BluetoothCommand::getName() const
{
    return m_device.name();
}

void BluetoothCommand::setName(const QString &s)
{
    for (const QBluetoothDeviceInfo& data : s_deviceList)
    {
        if (data.name() == s)
        {
            m_device = data;
            emit nameChanged(m_device.name());
#ifndef __ios__
            emit addressChanged(data.address().toString());
#endif
            return;
        }
    }
}

void BluetoothCommand::startScan()
{
    if (IsBluetoothAvailable() && s_deviceDiscover)
    {
        s_deviceDiscover->stop();
        s_deviceDiscover->setInquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry);
        s_deviceDiscover->start();
        s_bScan = true;
        emit scanningChanged(s_bScan);
        s_deviceList.clear();
        s_devList.clear();
        emit devicesChanged(s_devList);
    }
}

void BluetoothCommand::deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    bool bChanged = false;
    if(!info.name().isEmpty() && info.isValid())
    {
        if(!s_deviceList.contains(info))
        {
            s_deviceList += info;
            s_devList += info.name();
            emit devicesChanged(s_devList);
            bChanged = true;
        }
    }
    if (bChanged)
        countChanged(GetDevSize());
}

void BluetoothCommand::doneScanning()
{
    s_bScan = false;
    if(s_deviceDiscover)
        emit scanningChanged(s_bScan);
}

const QBluetoothDeviceInfo &BluetoothCommand::device() const
{
    return m_device;
}

QString BluetoothCommand::address()const
{
#ifdef __ios__
    return QString("");
#else
    return m_device.address().toString();
#endif
}

QStringList BluetoothCommand::devices()const
{
    return s_devList;
}

int BluetoothCommand::GetDevSize()const
{
    return s_devList.size();
}

bool BluetoothCommand::scanning() const
{
    return s_bScan;
}

LinkCommand::LinkType BluetoothCommand::type() const
{
    return LinkCommand::TypeBluetooth;
}

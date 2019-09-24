#include "LinkManager.h"
#include <QList>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <MAVLinkProtocol.h>

#ifndef __ios__
#include "SerialPortInfo.h"
#endif

#include "UDPLink.h"
#include "LinkInterface.h"

#ifdef QGC_ENABLE_BLUETOOTH
#include "BluetoothLink.h"
#endif

#ifndef __ios__
#include "SerialLink.h"
#endif

Q_DECLARE_LOGGING_CATEGORY(LinkManagerLog)
Q_DECLARE_LOGGING_CATEGORY(LinkManagerVerboseLog)

#define ActiveLinkCheckTimeoutMSecs 15000 //< Amount of time to wait for a heatbeat. Keep in mind ArduPilot stack heartbeat is slow to come.
#define AutoconnectUpdateTimerMSecs 1000
#ifdef Q_OS_WIN
#define AutoconnectConnectDelayMSecs 6000
#else
#define AutoconnectConnectDelayMSecs 1000
#endif

const char* LinkManager::_settingsGroup =            "LinkManager";
const char* LinkManager::_autoconnectPixhawkKey =    "AutoconnectPixhawk";
const char* LinkManager::_autoconnect3DRRadioKey =   "Autoconnect3DRRadio";
const char* LinkManager::_autoconnectPX4FlowKey =    "AutoconnectPX4Flow";
const char* LinkManager::_autoconnectRTKGPSKey =     "AutoconnectRTKGPS";
const char* LinkManager::_autoconnectLibrePilotKey = "AutoconnectLibrePilot";

LinkManager::LinkManager(QObject *parent)
    : QObject(parent)
    , _configurationsLoaded(false)
    , _mavlinkChannelsUsedBitMask(0)
    , m_mavlinkProtocol(NULL)
    , _autoconnectPixhawk(true)
    , _autoconnect3DRRadio(true)
    , _autoconnectPX4Flow(true)
    , _autoconnectRTKGPS(true)
    , _autoconnectLibrePilot(true)
    , _isConnectUAV(false)
    , _UAVRadioBaud(57600)
{
#ifndef __ios__
    _activeLinkCheckTimer.setInterval(ActiveLinkCheckTimeoutMSecs);
    _activeLinkCheckTimer.setSingleShot(false);
    connect(&_activeLinkCheckTimer, &QTimer::timeout, this, &LinkManager::_activeLinkCheck);
#endif

    m_mavlinkProtocol = new MAVLinkProtocol(this);
    connect(&_portListTimer, &QTimer::timeout, this, &LinkManager::_updateAutoConnectLinks);
    connect(this, &LinkManager::_shutdown, this, &LinkManager::onShutdown);
    _portListTimer.start(AutoconnectUpdateTimerMSecs);
    m_bOpenUdp = _readUdpLinkConfig();
}

LinkManager::~LinkManager()
{
    while (m_linkCmds.count() > 0)
    {
        delete m_linkCmds.takeFirst();
    }
    delete m_mavlinkProtocol;
}

bool LinkManager::_closeAllBlt()
{
#ifdef QGC_ENABLE_BLUETOOTH
    for (LinkCommand *itr: _getLinkCmds(LinkCommand::TypeBluetooth))
    {
        itr->deleteLater();
    }
    return true;
#else
    return false;
#endif //QGC_ENABLE_BLUETOOTH

}

void LinkManager::_checkUdpLink()
{
    if (!m_bOpenUdp)
        return;

    bool foundUDP = false;
    for (LinkCommand *cmd : m_linkCmds)
    {
        if (cmd->type() == LinkCommand::TypeUdp)
        {
            foundUDP = true;
            break;
        }
    }
    if (!foundUDP)
    {
        UDPCommand *udpConfig = new UDPCommand(this);
        udpConfig->setDynamic(true);
        m_linkCmds.append(udpConfig);
        udpConfig->connectLink();
        _addLink(udpConfig->link());
    }
}

bool LinkManager::_readUdpLinkConfig() const
{
    if (QSettings *st = Application::Instance()->GetSettings())
    {
        st->beginGroup("udpLinkConfig");
        bool bOpen = st->value("open", Application::Instance()->IsPcApp()).toBool();
        st->endGroup();
        return bOpen;
    }
    return Application::Instance()->IsPcApp();
}

void LinkManager::_writeUdpLinkConfig() const
{
    if (QSettings *st = Application::Instance()->GetSettings())
    {
        st->beginGroup("udpLinkConfig");
        st->setValue("open", m_bOpenUdp);
        st->endGroup();
    }
}

void LinkManager::_updateAutoSerialLinks(QStringList &currentPorts)
{
    QList<SerialPortInfo> portList = SerialPortInfo::availablePorts();
    // Iterate Comm Ports
    foreach(SerialPortInfo portInfo, portList)
    {
        currentPorts << portInfo.systemLocation();
        SerialPortInfo::BoardType_t boardType = portInfo.boardType();

        if (boardType != SerialPortInfo::BoardTypeUnknown)
        {
            if (portInfo.isBootloader())
                continue;
#ifndef __android__
            if (portInfo.boardTypeGPS())
                continue;
#endif
            if (_autoconnectCommandOfName(portInfo.systemLocation()))
                continue;

            if (!_autoconnectWaitList.contains(portInfo.systemLocation()))
            {
                _autoconnectWaitList[portInfo.systemLocation()] = 1;
                continue;
            }

            if (++_autoconnectWaitList[portInfo.systemLocation()] * AutoconnectUpdateTimerMSecs > AutoconnectConnectDelayMSecs)
            {
                SerialCommand *pSerialCmd = NULL;
                _autoconnectWaitList.remove(portInfo.systemLocation());
                switch (boardType)
                {
                case SerialPortInfo::BoardTypePX4FMUV1:
                case SerialPortInfo::BoardTypePX4FMUV2:
                case SerialPortInfo::BoardTypePX4FMUV4:
                case SerialPortInfo::BoardTypeAeroCore:
                    if (_autoconnectPixhawk)
                    {
                        pSerialCmd = new SerialCommand(this);
                        pSerialCmd->setUsbDirect(true);
                    }
                    break;
                case SerialPortInfo::BoardTypePX4Flow:
                    if (_autoconnectPX4Flow)
                        pSerialCmd = new SerialCommand();
                    break;
                case SerialPortInfo::BoardType3drRadio:
                    if (_autoconnect3DRRadio)
                    {
                        pSerialCmd = new SerialCommand();
                        pSerialCmd->setStopBits(1);
                        pSerialCmd->setFlowControl(0);
                        pSerialCmd->setDataBits(8);
                        pSerialCmd->setParity(0);
                    }
                    break;
                default:
                    qWarning() << "Internal error";
                    continue;
                }

                if (pSerialCmd)
                {
                    if (boardType == SerialPortInfo::BoardType3drRadio)
                        pSerialCmd->setBaud(_UAVRadioBaud);
                    else
                        pSerialCmd->setBaud(115200);

                    pSerialCmd->setDynamic(true);
                    pSerialCmd->setName(portInfo.systemLocation());
                    m_autolinkCmds.append(pSerialCmd);
                    pSerialCmd->connectLink();
                    _addLink(pSerialCmd->link());
                }
            }
        }
    }
}

void LinkManager::_addLink(LinkInterface *link)
{
    if (!link || !link->getLinkCommand())
        return;

    if (!containsLink(link))
    {
        bool channelSet = false;
        for (int i=1; i<32; i++)
        {
            if (!(_mavlinkChannelsUsedBitMask & 1 << i))
            {
                mavlink_reset_channel_status(i);
                link->_setMavlinkChannel(i);
                _mavlinkChannelsUsedBitMask |= i << i;
                channelSet = true;
                break;
            }
        }

        if (!channelSet)
            qWarning() << "Ran out of mavlink channels";
    }

    connect(link, &LinkInterface::communicationError, this, &LinkManager::communicationError, Qt::UniqueConnection);
    connect(link, &LinkInterface::bytesReceived,        m_mavlinkProtocol,   &MAVLinkProtocol::receiveBytes, Qt::UniqueConnection);
    connect(link, &LinkInterface::connected, this, &LinkManager::_linkConnect, Qt::UniqueConnection);
    connect(link->getLinkCommand(), &LinkCommand::linkDestroy, this, &LinkManager::onCommandDeleted, Qt::UniqueConnection);
    m_mavlinkProtocol->resetMetadataForLink(link);
}
 
void LinkManager::DeleteLink(LinkInterface *link)
{
    LinkCommand *cmd = link->getLinkCommand();
    if (!link || !cmd || !m_linkCmds.contains(cmd) || !cmd->link())
        return;

    m_linkCmds.removeAll(cmd);
    emit linkDeleted(cmd->link());
    if (m_autolinkCmds.contains((SerialCommand*)cmd))
        m_autolinkCmds.removeAll((SerialCommand*)cmd);

    _mavlinkChannelsUsedBitMask &= ~(1 << cmd->link()->getMavlinkChannel());
    cmd->deleteLater();
}

void LinkManager::_linkConnect(bool b)
{
    if (LinkInterface *link = dynamic_cast<LinkInterface*>(sender()))
    {
        if (b)
            emit linkConnected(link);
        else
            emit linkInactive(link);
    }
}

void LinkManager::onShutdown(const QList<LinkCommand*> &cmds)
{
    for (LinkCommand *itr : cmds)
    {
        DeleteLink(itr->link());
    }
}

void LinkManager::onCommandDeleted(LinkInterface *link)
{
    LinkCommand *cmd = link->getLinkCommand();
    if (!link || !cmd || !m_linkCmds.contains(cmd) || !cmd->link())
        return;

    _mavlinkChannelsUsedBitMask &= ~(1 << link->getMavlinkChannel());
    m_linkCmds.removeAll(cmd);
    emit linkDeleted(cmd->link());
    if (m_autolinkCmds.contains((SerialCommand*)cmd))
        m_autolinkCmds.removeAll((SerialCommand*)cmd);
}

void LinkManager::saveLinkCommandList()
{
    if (QSettings *settings = Application::Instance()->GetSettings())
    {
        settings->remove(LinkCommand::settingsRoot());
        int trueCount = 0;
        for (LinkCommand *cmd : m_linkCmds)
        {
            if (cmd && cmd->isDynamic())
            {
                QString root = LinkCommand::settingsRoot();
                root += QString("/Link%1").arg(trueCount++);
                settings->setValue(root + "/type", cmd->type());
                settings->setValue(root + "/auto", cmd->isAutoConnect());
                // Have the instance save its own values
                cmd->saveSettings(*settings, root);
            }
            else {
                qWarning() << "Internal error";
            }
        }
        QString root(LinkCommand::settingsRoot());
        settings->setValue(root + "/count", trueCount);
    }
}

void LinkManager::loadLinkCommandList()
{
    QSettings settings;
    // Is the group even there?
    if (settings.contains(LinkCommand::settingsRoot() + "/count"))
    {
        // Find out how many configurations we have
        int count = settings.value(LinkCommand::settingsRoot() + "/count").toInt();
        for (int i = 0; i < count; i++) {
            QString root(LinkCommand::settingsRoot());
            root += QString("/Link%1").arg(i);
            if (settings.contains(root + "/type")) {
                int type = settings.value(root + "/type").toInt();
                if ((LinkCommand::LinkType)type < LinkCommand::TypeLast)
                {
                    LinkCommand* pLink = NULL;
                    bool autoConnect = settings.value(root + "/auto").toBool();
                    switch ((LinkCommand::LinkType)type) {
#ifndef __ios__
                    case LinkCommand::TypeSerial:
                        pLink = (LinkCommand*)new SerialCommand();
                        break;
#endif
                    case LinkCommand::TypeUdp:
                        pLink = (LinkCommand*)new UDPCommand();
                        break;

#ifdef QGC_ENABLE_BLUETOOTH
                    case LinkCommand::TypeBluetooth:
                        pLink = (LinkCommand*)new BluetoothCommand();
                        break;
#endif
                    default:
                    case LinkCommand::TypeLast:
                        break;
                    }
                    if (pLink)
                    {
                        //-- Have the instance load its own values
                        pLink->setAutoConnect(autoConnect);
                        pLink->loadSettings(settings, root);
                        m_linkCmds.append(pLink);
                    }
                }
                else
                {
                    qWarning() << "Link Configuration" << root << "an invalid type: " << type;
                }
            }
        }
    }
    _configurationsLoaded = true;
}

#ifndef __ios__
SerialCommand* LinkManager::_autoconnectCommandOfName(const QString& portName)
{
    QString searchPort = portName.trimmed();
    for (SerialCommand *cmd: m_autolinkCmds)
    {
        if (cmd && cmd->getName() == searchPort)
        {
            return cmd;
        }
    }
    return NULL;
}
#endif

void LinkManager::_updateAutoConnectLinks(void)
{
    if (_isConnectUAV)
        return;

    _checkUdpLink();
#ifndef __ios__
    QStringList currentPorts;
    _updateAutoSerialLinks(currentPorts);
    for (SerialCommand* linkCmd : m_autolinkCmds)
    {
        if (linkCmd)
        {
            if (!currentPorts.contains(linkCmd->getName()))
            {
                if (linkCmd->isConnect() && linkCmd->link()->active())
                {
                    if (!m_linkCmds.contains(linkCmd))
                        m_linkCmds << linkCmd;

                    continue;
                }

                linkCmd->deleteLater();
            }
        }
    }
#endif // !__ios__
}

bool LinkManager::_setAutoconnectWorker(bool& currentAutoconnect, bool newAutoconnect, const char* autoconnectKey)
{
    if (currentAutoconnect != newAutoconnect) {
        QSettings settings;

        settings.beginGroup(_settingsGroup);
        settings.setValue(autoconnectKey, newAutoconnect);
        currentAutoconnect = newAutoconnect;
        return true;
    }

    return false;
}

void LinkManager::setAutoconnectPixhawk(bool autoconnect)
{
    if (_setAutoconnectWorker(_autoconnectPixhawk, autoconnect, _autoconnectPixhawkKey))
        emit autoconnectPixhawkChanged(autoconnect);
}

void LinkManager::setAutoconnect3DRRadio(bool autoconnect)
{
    if (_setAutoconnectWorker(_autoconnect3DRRadio, autoconnect, _autoconnect3DRRadioKey))
        emit autoconnect3DRRadioChanged(autoconnect);
}

void LinkManager::setAutoconnectPX4Flow(bool autoconnect)
{
    if (_setAutoconnectWorker(_autoconnectPX4Flow, autoconnect, _autoconnectPX4FlowKey)) {
        emit autoconnectPX4FlowChanged(autoconnect);
    }
}

void LinkManager::setAutoconnectRTKGPS(bool autoconnect)
{
    if (_setAutoconnectWorker(_autoconnectRTKGPS, autoconnect, _autoconnectRTKGPSKey)) {
        emit autoconnectRTKGPSChanged(autoconnect);
    }
}

void LinkManager::setAutoconnectLibrePilot(bool autoconnect)
{
    if (_setAutoconnectWorker(_autoconnectLibrePilot, autoconnect, _autoconnectLibrePilotKey)) {
        emit autoconnectLibrePilotChanged(autoconnect);
    }
}
QStringList LinkManager::linkTypeStrings(void) const
{
    //-- Must follow same order as enum LinkType in LinkCommand.h
    static QStringList list;
    if(!list.size())
    {
#ifndef __ios__
        list += tr("Serial");
#endif
        list += tr("UDP");
        //list += "TCP";
#ifdef QGC_ENABLE_BLUETOOTH
        list += tr("BlueTooth");
#endif
    }
    return list;
}

void LinkManager::_updateSerialPorts()
{
    m_commPortList.clear();
    m_commPortDisplayList.clear();
#ifndef __ios__
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &info, portList)
    {
        QString port = info.systemLocation().trimmed();
        m_commPortList += port;
        m_commPortDisplayList += SerialCommand::cleanPortDisplayname(port);
    }
#endif
}

QStringList LinkManager::serialPortStrings(void)
{
    if(!m_commPortDisplayList.size())
    {
        _updateSerialPorts();
    }
    return m_commPortDisplayList;
}

QStringList LinkManager::serialPorts(void)
{
    if(!m_commPortList.size())
    {
        _updateSerialPorts();
    }
    return m_commPortList;
}

bool LinkManager::containsLink(LinkInterface *link)const
{
    if (!link)
        return false;
    for (LinkCommand *itr : m_linkCmds)
    {
        if (itr->link() == link)
            return true;
    }
    for (LinkCommand *itr : m_autolinkCmds)
    {
        if (itr->link() == link)
            return true;
    }
    return false;
}

QStringList LinkManager::serialBaudRates(void)
{
#ifdef __ios__
    QStringList foo;
    return foo;
#else
    return SerialCommand::supportedBaudRates();
#endif
}

LinkCommand *LinkManager::createLinkCmd(int tp)
{
#ifndef __ios__
    if(tp == LinkCommand::TypeSerial)
        _updateSerialPorts();
#endif
    return LinkCommand::createSettings(tp);
}

void LinkManager::endCreateLinkCmd(LinkCommand *cmd, bool bLink)
{
    if (!cmd)
        return;

    if (!bLink || (isLinked(cmd->getName(), cmd->type()) && !m_linkCmds.contains(cmd)))
    {
        cmd->deleteLater();
        return;
    }

    if (!m_linkCmds.contains(cmd))
        m_linkCmds.append(cmd);

    cmd->connectLink();
    _addLink(cmd->link());
}

bool LinkManager::isLinked(const QString &name, int tp)
{
    for (LinkCommand *itr : m_linkCmds)
    {
        if (itr->type() == tp && itr->getName() == name)
            return itr->isConnect();
    }

    return false;
}

void LinkManager::shutdown(int tp)
{
    emit _shutdown(_getLinkCmds((LinkCommand::LinkType)tp));
}

QList<LinkCommand *> LinkManager::_getLinkCmds(LinkCommand::LinkType tp)
{
    if (tp == LinkCommand::TypeLast)
        return m_linkCmds;

    QList<LinkCommand *> ret;
    for (LinkCommand *itr : m_linkCmds)
    {
        if (itr->type() == tp)
            ret << itr;
    }
    return ret;
}

bool LinkManager::isAutoconnectLink(LinkCommand *cmd)const
{
    if (!cmd || cmd->type() != LinkCommand::TypeSerial)
        return false;

    return m_autolinkCmds.contains((SerialCommand*)cmd);
}

MAVLinkProtocol * LinkManager::getMavlinkProtocol() const
{
    return m_mavlinkProtocol;
}

void LinkManager::setConnectUAVStatus(bool isConnect)
{
    _isConnectUAV = isConnect;
}

void LinkManager::setUAVRadioBaud(long baud)
{
    _UAVRadioBaud = baud;
}

bool LinkManager::IsOpenUdp() const
{
    return m_bOpenUdp;
}

void LinkManager::SetOpenUdp(bool b)
{
    if (m_bOpenUdp == b)
        return;

    m_bOpenUdp = b;
    emit openUdpChanged(b);
    _writeUdpLinkConfig();
}

QList<LinkCommand *> LinkManager::linkCmds() const
{
    return m_linkCmds;
}

bool LinkManager::isBluetoothAvailable(void)
{
#ifdef QGC_ENABLE_BLUETOOTH
    return BluetoothCommand::IsBluetoothAvailable();
#else
    return false;
#endif
}

#ifndef __ios__
void LinkManager::_activeLinkCheck(void)
{
    SerialCommand* cmd = NULL;
    if (m_activeLinkCheckList.count() != 0)
    {
        cmd = m_activeLinkCheckList.takeFirst();
        if (!m_linkCmds.contains(cmd) || !cmd->isConnect())
            cmd = NULL;
    }

    if (m_activeLinkCheckList.count() == 0)
        _activeLinkCheckTimer.stop();

    if (cmd)
        cmd->write("\r", 1);
}
#endif

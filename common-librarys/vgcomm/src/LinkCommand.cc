#include "LinkCommand.h"
#ifndef __ios__
#include "SerialLink.h"
#endif
#include "UDPLink.h"
#ifdef QGC_ENABLE_BLUETOOTH
#include "BluetoothLink.h"
#endif

#define LINK_SETTING_ROOT "LinkCommand"

LinkCommand::LinkCommand(QObject *p):QObject(p), m_link(NULL)
, m_dynamic(false), m_autoConnect(false)
{
}

LinkCommand::~LinkCommand()
{
    if (m_link)
	{
		emit linkDestroy(m_link);
		m_link->deleteLater();
	}
}

LinkInterface* LinkCommand::link(void)
{
    return m_link;
}

void LinkCommand::updateSettings()
{
}

void LinkCommand::disconnectLink()
{
    if (m_link)
    {
        LinkInterface *link = m_link;
        m_link = NULL;
        emit linkDestroy(link);
        link->_disconnect();
        link->deleteLater();
    }
}

QString LinkCommand::settingsRoot()
{
    return QString(LINK_SETTING_ROOT);
}

LinkCommand *LinkCommand::createSettings(int type)
{
    LinkCommand *ret = NULL;
    switch(type) {
#ifndef __ios__
        case LinkCommand::TypeSerial:
            ret = new SerialCommand();
            break;
#endif
        case LinkCommand::TypeUdp:
            ret = new UDPCommand();
            break;
#ifdef QGC_ENABLE_BLUETOOTH
    case LinkCommand::TypeBluetooth:
        ret = new BluetoothCommand();
        break;
#endif
    }
    return ret;
}

bool LinkCommand::isDynamic() const
{
    return m_dynamic;
}

void LinkCommand::setDynamic(bool dynamic /*= true*/)
{
    m_dynamic = dynamic;
    emit dynamicChanged();
}

bool LinkCommand::isAutoConnect() const
{
    return m_autoConnect;
}

void LinkCommand::setAutoConnect(bool autoc /*= true*/)
{
    m_autoConnect = autoc;
    emit autoConnectChanged();
}

bool LinkCommand::isConnect() const
{
    return m_link && m_link->isConnected();
}

void LinkCommand::write(const char *buff, unsigned len)
{
    if (m_link && m_link->isConnected())
        m_link->writeBytes(QByteArray(buff, len));
}

bool LinkCommand::isAutoConnectAllowed() const
{
    return true;
}

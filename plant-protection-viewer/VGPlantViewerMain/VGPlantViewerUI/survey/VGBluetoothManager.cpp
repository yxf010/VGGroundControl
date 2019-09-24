#include "VGBluetoothManager.h"
#include <QList>
#include <QDebug>

#include "VGApplication.h"
#include "VGBluetoothManager.h"
#include "VGBDLocation.h"
#include "LinkManager.h"
#include "BluetoothLink.h"

VGBluetoothManager::VGBluetoothManager(QObject *parent)
    : QObject(parent), m_bltCmd(NULL)
{
}

VGBluetoothManager::~VGBluetoothManager()
{
    delete m_bltCmd;
    m_bltCmd = NULL;
}

void VGBluetoothManager::linkBlt(BluetoothCommand *cmd)
{
    if (m_bltCmd)
        m_bltCmd->deleteLater();
    m_bltCmd = cmd;
    if (cmd)
        cmd->connectLink();

    if (cmd)
    {
        connect(cmd, &QObject::destroyed, this, &VGBluetoothManager::onBltlinkDestroied);
        _prcsLink(cmd->link());
    }
}

bool VGBluetoothManager::isConnected(const QString &devName)
{
    if (m_bltCmd && m_bltCmd->getName() == devName)
        return m_bltCmd->isConnect();

    return false;
}

void VGBluetoothManager::_prcsLink(LinkInterface *link)
{
#ifdef QGC_ENABLE_BLUETOOTH
    if (link)
    {
        qvgApp->GetBDLocation()->Linked(link);
        connect(link, &BluetoothLink::connected, this, &VGBluetoothManager::_linkConnected);
    }
#endif
}

void VGBluetoothManager::_linkConnected(bool b)
{
    if (!b && m_bltCmd && m_bltCmd->link() == sender())
    {
        m_bltCmd->deleteLater();
        m_bltCmd = NULL;
    }
}

void VGBluetoothManager::onBltlinkDestroied(QObject *obj)
{
    if (obj == m_bltCmd)
        m_bltCmd = NULL;
}

void VGBluetoothManager::shutdown(void)
{
    if (m_bltCmd)
    {
        m_bltCmd->deleteLater();
        m_bltCmd = NULL;
    }
}

#include "VGToolBox.h"
#include <QTimer>
#include <QThread>
#include <QSettings>
#include <QDir>
#include "VGApplication.h"
#include "MAVLinkProtocol.h"
#include "VGVehicleManager.h"
#include "LinkManager.h"
#include "VGVehicleLogManager.h"
#include "VGBluetoothManager.h"
#include "audio/AudioWorker.h"

VGToolBox::VGToolBox(QObject *parent) : QObject(parent) , m_linkManager(new LinkManager(this))
, m_mavlinkProtocol(NULL), m_vgVehicleManager(new VGVehicleManager(this))
, m_vgAudioWorker(new AudioWorker), m_vgLogManager(new VGVehicleLogManager(NULL))
, m_logThread(new QThread(m_vgLogManager)), m_bltManager(new VGBluetoothManager(this))
{
	if (m_linkManager)
		m_mavlinkProtocol = m_linkManager->getMavlinkProtocol();

    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<LinkInterface*>("LinkInterface *");

	if (m_mavlinkProtocol)
	{
		connect(m_mavlinkProtocol, &MAVLinkProtocol::vehicleHeartbeatInfo, m_vgVehicleManager, &VGVehicleManager::onHeartbeat);
        connect(m_mavlinkProtocol, &MAVLinkProtocol::messageReceived, m_vgVehicleManager, &VGVehicleManager::sigMavlinkMessageReceived);
	}

	if (m_vgVehicleManager && m_vgLogManager)
	{
        connect(m_vgVehicleManager, &VGVehicleManager::sigActiveVehicleChanged, this, &VGToolBox::sigSetActiveVehicle);
        connect(m_vgVehicleManager, &VGVehicleManager::sigUpdateLogList, this, &VGToolBox::sigUpdateLogList);
		connect(m_vgLogManager, &VGVehicleLogManager::sigRequestLogData, m_vgVehicleManager, &VGVehicleManager::sltRequestLogData);
		connect(m_vgVehicleManager, &VGVehicleManager::sigReceiveLogData, m_vgLogManager, &VGVehicleLogManager::sltReceiveLogData);

		connect(m_vgLogManager, &VGVehicleLogManager::sigUpdateDownloadStatus, this, &VGToolBox::sigUpdateDownloadStatus);
		connect(m_vgLogManager, &VGVehicleLogManager::sigUpdateDownloadedLog, this, &VGToolBox::sigUpdateLogList);
		connect(m_vgLogManager, &VGVehicleLogManager::sigAllDownloaded, this, &VGToolBox::sigAllLogDownloaded);
		connect(this, &VGToolBox::sigRequestLogDataThread, m_vgLogManager, &VGVehicleLogManager::sltRequestLogData);
		connect(this, &VGToolBox::sigLoadDownloadedLogThread, m_vgLogManager, &VGVehicleLogManager::loadDownloadedLog);
		connect(this, &VGToolBox::sigSetActiveVehicle, m_vgLogManager, &VGVehicleLogManager::setVehicle);
		connect(this, &VGToolBox::sigSetLogDownloadedDir, m_vgLogManager, &VGVehicleLogManager::setLogDownloadDir);

		m_vgLogManager->moveToThread(m_logThread);
	}
	if (m_linkManager)
		connect(m_linkManager, &LinkManager::communicationError, this, &VGToolBox::sltLinkManagerError);

    qRegisterMetaType<VGVehicle*>("VGVehicle*");

    m_logThread->start();
    QString qStLogDir = qvgApp->GetDefualtPath() + QString("/vgUAVLog/");
    if (QDir::root().exists(qStLogDir))
        QDir::root().mkpath(qStLogDir);

    emit sigSetLogDownloadedDir(qStLogDir);
}

VGToolBox::~VGToolBox()
{
    m_logThread->terminate();
    delete m_vgLogManager;

    delete m_bltManager;
    delete m_vgAudioWorker;
    delete m_vgVehicleManager;
    delete m_linkManager;
}

void VGToolBox::connectLink()
{
    if(m_linkManager)
    {
        m_linkManager->setAutoconnect3DRRadio(true);

       if (QSettings *settings = qvgApp->GetSettings())
       {
            settings->beginGroup("serialPort");
            int baud = settings->value("baud", 57600).toInt();
            settings->endGroup();
            m_linkManager->setUAVRadioBaud(baud);
       }
    }

    if (m_vgLogManager)
        emit sigLoadDownloadedLogThread();
}

void VGToolBox::sltLinkManagerError(const QString &title, const QString &error)
{
    if (m_linkManager)
        emit sigShowNote(title + QString::fromLocal8Bit(":") + error);
}

MAVLinkProtocol* VGToolBox::mavlinkProtocol()const
{
    return m_mavlinkProtocol;
}

VGVehicleManager* VGToolBox::vehicleManager() const
{
    return m_vgVehicleManager;
}

LinkManager* VGToolBox::linkManager() const
{
    return m_linkManager;
}

AudioWorker* VGToolBox::audioOutput() const
{
    return m_vgAudioWorker;
}

VGVehicleLogManager* VGToolBox::vgLogManager() const
{
    return m_vgLogManager;
}

VGVehicle* VGToolBox::activeVehicle() const
{
    if (m_vgVehicleManager)
        return m_vgVehicleManager->activeVehicle();

    return NULL;
}

VGVehicle* VGToolBox::surveyVehicle() const
{
    if (m_vgVehicleManager)
        return m_vgVehicleManager->surveyVehicle();

    return NULL;
}

VGBluetoothManager* VGToolBox::bluetoothManager() const
{
    return m_bltManager;
}

void VGToolBox::SpeakNotice(const QString &value, const QStringList &param)
{
    m_vgAudioWorker->say(value, param);
}

int VGToolBox::countVoiceQue()
{
    return m_vgAudioWorker->countVoiceQue();
}


void VGToolBox::sltRequestLogData(int logId, long size, long time_utc)
{
    if(m_linkManager && m_vgLogManager)
    {
        qDebug() << "downLoadLog ......" << "VGToolBox::sltRequestLogData";
        //m_vgLogManager->sltRequestLogData(logId, size, time_utc);
        emit sigRequestLogDataThread(logId, size, time_utc);
    }
}

void VGToolBox::sltActiveVehicleChanged(VGVehicle *activeVehicle)
{
    if(m_vgLogManager)
    {
        //m_vgLogManager->setVehicle(activeVehicle);
        emit sigSetActiveVehicle(activeVehicle);
    }
}

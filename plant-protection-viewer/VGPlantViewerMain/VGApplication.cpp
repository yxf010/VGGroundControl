#include "VGApplication.h"
#include <QQmlApplicationEngine>
#include <QScreen>
#include <QGCMapEngine.h>
#include <VGLandInformation.h>
#include <VGNetManager.h>
#include <VGLandManager.h>
#include <QDir>
#include <QDomDocument>
#include <QFont>
#include <QThread>
#include "VGToolBox.h"
#include "VGMacro.h"

#include "math.h"
#include "LinkManager.h"
#include "VGDbManager.h"
#include "VGUIInstance.h"
#include "VGBluetoothManager.h"
#include "VGQXManager.h"
#include "ParametersLimit.h"
#include "VGPlantEvents.h"
#include "VGMainPage.h"

#ifdef __mobile__
#include <QStandardPaths>
#endif

#define CATCHFOLD "VGPlantViewerMain"

VGApplication::VGApplication(int &argc, char* argv[])
    : QApplication(argc, argv), m_pMainUIInterface(NULL)
    , m_toolBox(NULL), m_setting(NULL), m_qxMgr(NULL)
    , m_paramsLimit(NULL), m_plantEvents(NULL)
{
    setFont(QFont(QString(), 12));
    setApplicationName(VG_APPLICATION_NAME);
    setOrganizationName(VG_ORG_NAME);
    setOrganizationDomain(VG_ORG_DOMAIN);

    _initSettings();
    _initMember();
}

VGApplication::~VGApplication()
{
    delete m_toolBox;
    m_toolBox = NULL;
    delete m_pMainUIInterface;
    m_pMainUIInterface = NULL;
	delete m_setting;
    m_setting = NULL;
    delete m_plantEvents;
    m_plantEvents = NULL;
}

void VGApplication::initApp()
{
    QQmlContext *engineCtx = m_pMainUIInterface ? m_pMainUIInterface->getQQmlApplicationEngine().rootContext() : NULL;
    if (engineCtx && m_toolBox)
    {
        engineCtx->setContextProperty("vgToolbox", m_toolBox);
        if (LinkManager *mgr = m_toolBox->linkManager())
            engineCtx->setContextProperty("linkManager", mgr);
        if (VGBluetoothManager *mgr = m_toolBox->bluetoothManager())
            engineCtx->setContextProperty("bltManager", mgr);
    }

    getQGCMapEngine()->init();

    qRegisterMetaType<DescribeMap>("DescribeMap");
    m_dbManager = new VGDbManager;
    if (VGLandManager *landMgr = m_pMainUIInterface->landManager())
        connect(m_dbManager, &VGDbManager::sigQueryItem, landMgr, &VGLandManager::onQueryItem);

    m_threadDb = new QThread(m_dbManager);
    m_dbManager->moveToThread(m_threadDb);
    m_threadDb->start();

    QString cacheDir = GetDefualtPath() + QLatin1String("VGDBCache/");
    if((QDir::root().exists(cacheDir) || !QDir::root().mkpath(cacheDir)) && m_dbManager)
    {
        QString strFile = cacheDir + "vgdbCache.db";
        m_dbManager->CreateDb(strFile);
    }

    m_toolBox->connectLink();
    _initParamsAndEvents();
}

VGToolBox *VGApplication::toolbox() const
{
    return m_toolBox;
}

LinkManager * VGApplication::linkManager() const
{
    if (m_toolBox)
        return m_toolBox->linkManager();

    return NULL;
}

MAVLinkProtocol * VGApplication::mavLink() const
{
    return m_toolBox ? m_toolBox->mavlinkProtocol() : NULL;
}

VGMapManager *VGApplication::mapManager() const
{
    return m_pMainUIInterface ? m_pMainUIInterface->mapManager() : NULL;
}

VGDbManager *VGApplication::dbManager() const
{
    return m_dbManager;
}

VGNetManager * VGApplication::netManager() const
{
    return m_pMainUIInterface ? m_pMainUIInterface->netManager() : NULL;
}

VGLandManager *VGApplication::landManager() const
{
    if (m_pMainUIInterface)
        return m_pMainUIInterface->landManager();

    return NULL;
}

QThread *VGApplication::qmlThread() const
{
    if (m_pMainUIInterface)
        m_pMainUIInterface->getQQmlApplicationEngine().thread();

    return NULL;
}

QQmlApplicationEngine *VGApplication::getQmlEngine() const
{
    if (m_pMainUIInterface)
        m_pMainUIInterface->getQQmlApplicationEngine();

    return NULL;
}

VGPlantManager *VGApplication::plantManager() const
{
    if (m_pMainUIInterface)
        return m_pMainUIInterface->plantManager();

    return NULL;
}

VGSurveyMonitor *VGApplication::surveyMonitor() const
{
    if (m_pMainUIInterface)
        return m_pMainUIInterface->surveyMonitor();

    return NULL;
}

VGVehicleManager *VGApplication::vehicleManager() const
{
    if (m_toolBox)
        return m_toolBox->vehicleManager();

    return NULL;
}

VGQXManager *VGApplication::qxManager() const
{
    return m_qxMgr;
}

VGBluetoothManager *VGApplication::bluetoothManager() const
{
    if (m_toolBox)
        return m_toolBox->bluetoothManager();

    return NULL;
}

ParametersLimit * VGApplication::parameters() const
{
    return m_paramsLimit;
}

VGPlantEvents *VGApplication::plantEvents() const
{
    return m_plantEvents;
}

int VGApplication::getNetConnectStat() const
{
    if (m_pMainUIInterface && m_pMainUIInterface->netManager())
        return m_pMainUIInterface->netManager()->GetConnectState();

    return 2;
}

QString VGApplication::GetUserId() const
{
	if (m_pMainUIInterface)
	{
		if (VGNetManager *mgr = m_pMainUIInterface->netManager())
			return mgr->GetUserId();
	}

	return QString();
}

QSettings *VGApplication::GetSettings() const
{
    return m_setting;
}

QString VGApplication::GetDefualtPath()
{
	if (m_defualtPath.isEmpty())
	{
		m_defualtPath = applicationDirPath() + QString::fromLocal8Bit("/" CATCHFOLD);
		if (!QDir::root().exists(m_defualtPath) && !QDir::root().mkdir(m_defualtPath))
        {
#ifdef __mobile__
            m_defualtPath =  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QString::fromLocal8Bit("/" CATCHFOLD);
#else
			m_defualtPath = QDir::homePath() + QString::fromLocal8Bit("/" CATCHFOLD);
#endif
            if (!QDir::root().exists(m_defualtPath))
                QDir::root().mkdir(m_defualtPath);
        }
        if (!m_defualtPath.endsWith("/"))
            m_defualtPath += "/";
	}
	return m_defualtPath;
}

void VGApplication::speakNotice(const QString &value, const QStringList &param)
{
    if (m_toolBox)
        m_toolBox->SpeakNotice(value, param);
}

int VGApplication::countVoiceQue() const
{
    if (m_toolBox)
        return m_toolBox->countVoiceQue();

    return 0;
}

void VGApplication::SetQmlTip(const QString &tip, bool b)
{
    if (m_pMainUIInterface && m_pMainUIInterface->vgMainPage())
        m_pMainUIInterface->vgMainPage()->SetQmlTip(tip, b);
    if (b)
        speakNotice(tip);
}

void VGApplication::SetMessageBox(const QString & tip, bool bWarn)
{
	if (m_pMainUIInterface && m_pMainUIInterface->vgMainPage())
		m_pMainUIInterface->vgMainPage()->SetMessageBox(tip);
	if (bWarn)
		speakNotice(tip);
}

VGBDLocation *VGApplication::GetBDLocation() const
{
    if (VGLandManager *mgr = landManager())
        return mgr->GetBDLocation();

    return NULL;
}

bool VGApplication::IsPcApp() const
{
    VGMainPage  *mp = m_pMainUIInterface ? m_pMainUIInterface->vgMainPage() : NULL;
    if (mp)
        return mp->IsShowSrSt();

    return false;
}

void VGApplication::initUI()
{
    if(m_pMainUIInterface)
    {
        QFont f = qApp->font();
        m_pMainUIInterface->setUIFontSize(f.pointSize());
        m_pMainUIInterface->createQml();
    }
}

bool VGApplication::notify(QObject *receiver, QEvent *event)
{
#ifdef Q_OS_ANDROID
    if(event->type() == QEvent::Close)
    {
        emit sigBack();
        return false;
    }
    else if(event->type() == QEvent::Quit)
    {
        qDebug() << "quit...............";
        return false;
    }
    else
    {
        return QGuiApplication::notify(receiver, event);
    }
#else
    if(event->type() == QEvent::Close)
    {
        emit sigBack();
        event->accept();
        return true;
    }

    return QGuiApplication::notify(receiver, event);
#endif
}

void VGApplication::_initMember()
{
    m_qxMgr = new VGQXManager(this);
    m_pMainUIInterface = new VGUIInstance;

    m_paramsLimit = new ParametersLimit(this);
    m_plantEvents = new VGPlantEvents(this);
    m_toolBox = new VGToolBox(this);
}

void VGApplication::_initSettings()
{
    QString strConfigPath = GetDefualtPath() + SETTING_FILE_NAME;
    QFile file(strConfigPath);
    if (!file.exists())
    {
        file.open(QIODevice::WriteOnly);
        file.close();
        m_setting = new QSettings(strConfigPath, QSettings::IniFormat);

        m_setting->beginGroup("config");
        m_setting->setValue("enableLog", true);
        m_setting->setValue("batteryAlarm", 30.0);
        m_setting->endGroup();

        m_setting->beginGroup("serialPort");
        m_setting->setValue("baud", QVariant("57600"));
        m_setting->endGroup();

        m_setting->beginGroup("udpLinkConfig");
        m_setting->setValue("open", IsPcApp());
        m_setting->setValue("ip", "127.0.0.1");
        m_setting->setValue("localPort", 14550);
        m_setting->endGroup();
    }

    if (!m_setting)
        m_setting = new QSettings(strConfigPath, QSettings::IniFormat);
}

void VGApplication::_initParamsAndEvents()
{
#ifdef __mobile__
    QString path = "assets:/res/UavParamsAndEvents.xml";
#else
    QString path = "UavParamsAndEvents.xml";
#endif
    QFile f(path);
    if (f.open(QIODevice::ReadOnly))
    {
        QDomDocument doc;
        doc.setContent(&f);
        QDomElement ele = doc.firstChildElement();

        if (m_paramsLimit)
            m_paramsLimit->LoadParameters(ele);
        
        if (m_plantEvents)
            m_plantEvents->Parse(ele);

        f.close();
    }
}

void VGApplication::sltUpdateLogList(long time_utc, long size, int id, int num_logs, bool downloaded)
{
    if (m_pMainUIInterface)
        m_pMainUIInterface->sltReceiveLogList(time_utc, size, id, num_logs, downloaded);
}

void VGApplication::sltUpdateLogDownloadStatus(int logId, ulong time_utc, const QString &status)
{
    if (m_pMainUIInterface)
    {
        m_pMainUIInterface->sltUpdateLogDownloadStatus(logId, time_utc, status);
    }
}

void VGApplication::sltAllLogDownloaded()
{
    if (m_pMainUIInterface)
        m_pMainUIInterface->sltAllLogDownload();
}

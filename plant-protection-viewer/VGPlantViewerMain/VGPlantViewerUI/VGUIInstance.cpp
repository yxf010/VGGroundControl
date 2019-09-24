#include "VGUIInstance.h"
#include <QDebug>
#include <QQmlApplicationEngine>

#include "VGApplication.h"
#include "VGToolBox.h"
#include "VGLandInformation.h"
#include "VGLandBoundary.h"
#include "VGFlyRoute.h"
#include "VGOutline.h"
#include "VGCoordinate.h"
#include "VGPlantManager.h"
#include "VGSelectEdge.h"
#include "VGMapManager.h"
#include "VGLandManager.h"
#include "VGNetManager.h"
#include "VGPlantManager.h"
#include "VGSurveyMonitor.h"
#include "VGBDLocation.h"
#include "VGBluetoothManager.h"
#include "VGLandPolygon.h"
#include "VGVehicleMission.h"
#include "VGPolylineMapItem.h"
#include "VGQXManager.h"
#include "BluetoothLink.h"
#include "SerialLink.h"
#include "UdpLink.h"
#include "ParametersLimit.h"
#include "srcload/VGImageProvider.h"
#include "VGPlantEvents.h"
#include "VGMainPage.h"
#include "VGLogDataObject.h"
#include "LinkManager.h"
#include "VGVehicle.h"

VGUIInstance::VGUIInstance(QObject* parent) : QObject(parent)
, m_vgMainPage(new VGMainPage(this)), m_plantManager(new VGPlantManager(this))
, m_logDataObject(new VGLogDataObject(this)), m_mapManager(new VGMapManager(this))
, m_landManager(new VGLandManager(this)), m_vgNetManager(new VGNetManager)
, m_qmlEngine(new QQmlApplicationEngine(this)), m_surveyMonitor(new VGSurveyMonitor(this))
{
    qmlRegisterType<VGPolylineMapItem>("VGGroundControl", 1, 0, "VGPolylineMapItem");

    qmlRegisterUncreatableType<VGMapLineProperties>("VGGroundControl", 1, 0, "VGMapLineProperties", "Reference only");
    qmlRegisterUncreatableType<UrlFactory>("VGGroundControl", 1, 0, "UrlFactory", "Reference only");
    qmlRegisterUncreatableType<LinkManager>("VGGroundControl", 1, 0, "LinkManager", "Reference only");
    qmlRegisterUncreatableType<VGBDLocation>("VGGroundControl", 1, 0, "VGBDLocation", "Reference only");
    qmlRegisterUncreatableType<LinkCommand>("VGGroundControl", 1, 0, "LinkCommand", "Reference only");
    qmlRegisterUncreatableType<LinkInterface>("VGGroundControl", 1, 0, "LinkInterface", "Reference only");
    qmlRegisterUncreatableType<BluetoothCommand>("VGGroundControl", 1, 0, "BluetoothCommand", "Reference only");
    qmlRegisterUncreatableType<SerialCommand>("VGGroundControl", 1, 0, "SerialCommand", "Reference only");
    qmlRegisterUncreatableType<UDPCommand>("VGGroundControl", 1, 0, "UDPCommand", "Reference only");

    qmlRegisterUncreatableType<VGLandInformation>("VGGroundControl", 1, 0, "VGLandInformation", "Reference only");
    qmlRegisterUncreatableType<VGVehicle>("VGGroundControl", 1, 0, "VGVehicle", "Reference only");
    qmlRegisterUncreatableType<QmlObjectListModel>("VGGroundControl", 1, 0, "QmlObjectListModel", "Reference only");
    qmlRegisterUncreatableType<MapAbstractItem>("VGGroundControl", 1, 0, "MapAbstractItem", "Reference only");
    qmlRegisterUncreatableType<VGCoordinate>("VGGroundControl", 1, 0, "VGCoordinate", "Reference only");
    qmlRegisterUncreatableType<VGOutline>("VGGroundControl", 1, 0, "VGOutline", "Reference only");
    qmlRegisterUncreatableType<VGLandPolygon>("VGGroundControl", 1, 0, "VGLandPolygon", "Reference only");
    qmlRegisterUncreatableType<VGLandBoundary>("VGGroundControl", 1, 0, "VGLandBoundary", "Reference only");
    qmlRegisterUncreatableType<VGFlyRoute>("VGGroundControl", 1, 0, "VGFlyRoute", "Reference only");
    qmlRegisterUncreatableType<VGSelectEdge>("VGGroundControl", 1, 0, "VGSelectEdge", "Reference only");

    qmlRegisterUncreatableType<VGPlantInformation>("VGGroundControl", 1, 0, "VGPlantInformation", "Reference only");
    qmlRegisterUncreatableType<VGBluetoothManager>("VGGroundControl", 1, 0, "VGBluetoothManager", "Reference only");
    qmlRegisterUncreatableType<VGMapManager>("VGGroundControl", 1, 0, "VGMapManager", "Reference only");
    qmlRegisterUncreatableType<VGVehicleMission>("VGGroundControl", 1, 0, "VGVehicleMission", "Reference only");
    qmlRegisterUncreatableType<VGQXManager>("VGGroundControl", 1, 0, "VGQXManager", "Reference only");
    qmlRegisterUncreatableType<ParametersLimit>("VGGroundControl", 1, 0, "ParametersLimit", "Reference only");
    qmlRegisterUncreatableType<VGPlantEvent>("VGGroundControl", 1, 0, "VGPlantEvent", "Reference only");
    qmlRegisterUncreatableType<VGPlantEvents>("VGGroundControl", 1, 0, "VGPlantEvents", "Reference only");

    qRegisterMetaType<QList<double>>("QList<double>");

    connect(m_logDataObject, &VGLogDataObject::sigUpdateLogList, this, &VGUIInstance::sltUpdateLogList);
    connect(m_logDataObject, &VGLogDataObject::sigRequestLogData, this, &VGUIInstance::sigRequestLogData);
    if(m_qmlEngine)
        m_qmlEngine->addImageProvider("rscLoader", new VGImageProvider);
}

VGUIInstance::~VGUIInstance()
{
    m_surveyMonitor->deleteLater();
    m_surveyMonitor = NULL;
    m_landManager->deleteLater();
    m_landManager = NULL;
    m_vgNetManager->deleteLater();
    m_vgNetManager = NULL;
    m_qmlEngine->deleteLater();
    m_qmlEngine = NULL;
    m_vgMainPage->deleteLater();
    m_vgMainPage = NULL;
    m_vgNetManager->deleteLater();
    m_vgNetManager = NULL;

    m_logDataObject->deleteLater();
    m_logDataObject = NULL;
    m_plantManager->deleteLater();
    m_plantManager = NULL;
}

void VGUIInstance::createQml()
{
    connect(m_qmlEngine, &QQmlApplicationEngine::objectCreated, m_mapManager, &VGMapManager::sltQmlCreated);

    m_qmlEngine->rootContext()->setContextProperty("vgMainPage", m_vgMainPage);
    m_qmlEngine->rootContext()->setContextProperty("vgLogDataObject", m_logDataObject);
    m_qmlEngine->rootContext()->setContextProperty("mapManager", m_mapManager);
    m_qmlEngine->rootContext()->setContextProperty("landManager", m_landManager);
    m_qmlEngine->rootContext()->setContextProperty("netManager", m_vgNetManager);
    m_qmlEngine->rootContext()->setContextProperty("plantManager", m_plantManager);
    m_qmlEngine->rootContext()->setContextProperty("surveyMonitor", m_surveyMonitor);
    m_qmlEngine->rootContext()->setContextProperty("qxManager", qvgApp->qxManager());
    m_qmlEngine->rootContext()->setContextProperty("parameters", qvgApp->parameters());
    m_qmlEngine->rootContext()->setContextProperty("plantEvents", qvgApp->plantEvents());

	m_qmlEngine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    m_plantManager->InitialNetWork(m_vgNetManager);
    m_landManager->InitialNetWork(m_vgNetManager);
    if (m_landManager)
    {
        m_landManager->uploadLandInfo(MapAbstractItem::Type_LandInfo);
        m_landManager->uploadLandInfo(MapAbstractItem::Type_LandBoundary);
        m_landManager->uploadLandInfo(MapAbstractItem::Type_FlyRoute);
    }
}

void VGUIInstance::setUIFixedSize(const QSize &size)
{
    if(m_vgMainPage)
    {
        m_vgMainPage->setQmlRootWidth(size.width());
        m_vgMainPage->setQmlRootHeight(size.height());
    }
}

void VGUIInstance::setUIFontSize(int fontPixelSize)
{
    if(m_vgMainPage)
    {
        QFont ft = m_vgMainPage->font();
        ft.setPointSize(fontPixelSize);
        m_vgMainPage->setFont(ft);
    }
}

void VGUIInstance::sltReceiveLogList(long time_utc, long size, int id, int num_logs, bool downloaded)
{
    if(m_logDataObject)
        m_logDataObject->sltReceiveLogEntry(time_utc, size, id, num_logs, downloaded);
}

void VGUIInstance::sltUpdateLogDownloadStatus(int logId, ulong time_utc, const QString &status)
{
    if(m_logDataObject)
        m_logDataObject->sltUpdateLogDownloadStatus(logId, time_utc, status);
}

void VGUIInstance::sltAllLogDownload()
{
    if(m_logDataObject)
        m_logDataObject->setDownStatus(0);
}

QQmlApplicationEngine &VGUIInstance::getQQmlApplicationEngine()
{
	return *m_qmlEngine;
}


VGLandManager * VGUIInstance::landManager() const
{
    return m_landManager;
}


VGMapManager *VGUIInstance::mapManager() const
{
    return m_mapManager;
}

VGNetManager *VGUIInstance::netManager() const
{
    return m_vgNetManager;
}

VGPlantManager * VGUIInstance::plantManager() const
{
    return m_plantManager;
}

VGMainPage * VGUIInstance::vgMainPage() const
{
    return m_vgMainPage;
}

VGSurveyMonitor *VGUIInstance::surveyMonitor() const
{
    return m_surveyMonitor;
}

void VGUIInstance::sltSetUIErrorInfo(const QString &title, const QString &notes)
{
    if(m_vgMainPage)
        m_vgMainPage->setErrorInfo(title, notes);
}

void VGUIInstance::sltUpdateLogList()
{
    emit sigUpdateLogList();
}

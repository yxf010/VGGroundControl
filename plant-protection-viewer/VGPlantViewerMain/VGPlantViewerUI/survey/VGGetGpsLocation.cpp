#include "VGGetGpsLocation.h"
#include <QDebug>
#include "VGGlobalFun.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

VGGetGpsLocation::VGGetGpsLocation(QObject *parent) : QObject(parent)
, m_PosInfoSource(NULL), m_updateInterval(1), m_bOpenGps(false)
, m_countSatellites(0), m_SatelliteSource(NULL)
, m_bStart(false)
{
    connect(this, &VGGetGpsLocation::sigStartGetGps, this, &VGGetGpsLocation::onStartGetGps);
    connect(this, &VGGetGpsLocation::sigStopGetGps, this, &VGGetGpsLocation::onStopGetGps);
}

VGGetGpsLocation::~VGGetGpsLocation()
{
    if(m_PosInfoSource)
        delete m_PosInfoSource;
}

void VGGetGpsLocation::sltPositionUpdate(const QGeoPositionInfo &info)
{
    if(!info.isValid())
    {
        setResult(QString::fromUtf8("无效位置信息"));
        return;
    }

    if(!m_bOpenGps)
    {
        m_bOpenGps = true;
        setErrorInfo(1003);
    }

    QGeoCoordinate coordinate = info.coordinate();
    double longitude = coordinate.longitude();
    double latitude = coordinate.latitude();
    double alt = coordinate.altitude();
    ShareFunction::gpsCorrect(latitude, longitude);

    emit sigGetGpsCoordinate(latitude, longitude, alt, m_countSatellites);
}

void VGGetGpsLocation::sltUpdateTimeout()
{
    qDebug() << QString::fromStdWString(L"超时！！");
    setResult(QString::fromStdWString(L"超时！！"));
    setErrorInfo(1004);
}

void VGGetGpsLocation::sltGetError(QGeoPositionInfoSource::Error error)
{
    /////错误代码：1000：无权限
    /// 1001：定位服务关闭
    /// 1002：未知错误
    /// 1003：无错误
    if (error != QGeoPositionInfoSource::NoError)
    {
        setErrorInfo(error + 1000);
        stopGetGps();
        return;
    }
    
    setErrorInfo(1003); //成功
}


void VGGetGpsLocation::sltSatellitesChanged(const QList<QGeoSatelliteInfo> &infos)
{
    m_countSatellites = infos.count();
    if (m_PosInfoSource)
        sltPositionUpdate(m_PosInfoSource->lastKnownPosition());
}


void VGGetGpsLocation::sltSatellitesTimeout()
{
    qDebug() << QString::fromStdWString(L"超时！！");
    setResult(QString::fromStdWString(L"超时！！"));
    setErrorInfo(1007);
}


void VGGetGpsLocation::sltSatellitesError(QGeoSatelliteInfoSource::Error error)
{
    if (error != QGeoSatelliteInfoSource::UnknownSourceError)
    {
        setErrorInfo(10008);
    }
    else if (error!=QGeoSatelliteInfoSource::NoError)
    {
        setErrorInfo(10008 + error);
    }  
}

void VGGetGpsLocation::onStartGetGps(int timeOut)
{
#ifdef Q_OS_WIN
    HRESULT r = OleInitialize(0);
    if (r != S_OK && r != S_FALSE)
    {
        qWarning("Qt:初始化Ole失败(error %x)", (unsigned int)r);
    }
#endif
    if (!m_SatelliteSource)
    {
        m_SatelliteSource = QGeoSatelliteInfoSource::createDefaultSource(this);
        connect(m_SatelliteSource, &QGeoSatelliteInfoSource::satellitesInViewUpdated, this, &VGGetGpsLocation::sltSatellitesChanged);
        connect(m_SatelliteSource, &QGeoSatelliteInfoSource::requestTimeout, this, &VGGetGpsLocation::sltSatellitesTimeout);
        connect(m_SatelliteSource, SIGNAL(error(QGeoSatelliteInfoSource::Error)), this, SLOT(sltSatellitesError(QGeoSatelliteInfoSource::Error)));
    }
    if (NULL == m_PosInfoSource)
    {
        QStringList sourceList = QGeoPositionInfoSource::availableSources();
        if (sourceList.size() <= 0)
        {
            setErrorInfo(1005); //无可用gps设备
            return;
        }

        m_PosInfoSource = QGeoPositionInfoSource::createDefaultSource(this);
        if (m_PosInfoSource)
        {
            connect(m_PosInfoSource, &QGeoPositionInfoSource::positionUpdated, this, &VGGetGpsLocation::sltPositionUpdate);
            connect(m_PosInfoSource, &QGeoPositionInfoSource::updateTimeout, this, &VGGetGpsLocation::sltUpdateTimeout);
            connect(m_PosInfoSource, SIGNAL(error(QGeoPositionInfoSource::Error)), this, SLOT(sltGetError(QGeoPositionInfoSource::Error)));
        }
        else
        {
            setErrorInfo(1006); //GPS设备打开失败
            return;
        }
    }

    if (m_PosInfoSource)
    {
        if (!m_bStart)
        {
            m_bStart = true;
            m_PosInfoSource->setPreferredPositioningMethods(QGeoPositionInfoSource::AllPositioningMethods);
            m_PosInfoSource->setUpdateInterval(m_updateInterval * 1000);
            m_PosInfoSource->requestUpdate(timeOut * 1000);
            m_PosInfoSource->startUpdates();
        }
    }
    if (m_SatelliteSource)
    {
        m_SatelliteSource->setUpdateInterval(m_updateInterval * 1000);
        m_SatelliteSource->requestUpdate(timeOut * 1000);
        m_SatelliteSource->startUpdates();
    }
}

void VGGetGpsLocation::onStopGetGps()
{
    if (m_PosInfoSource)
    {
        m_PosInfoSource->stopUpdates();
        m_bOpenGps = false;
        m_bStart = false;
    }
    if (m_SatelliteSource)
        m_SatelliteSource->stopUpdates();
#ifdef Q_OS_WIN
    OleUninitialize();
#endif
}

void VGGetGpsLocation::startGetGps(int timeOut)
{
    //qDebug() << " @@@@@@@ startGetGps ";
    emit sigStartGetGps(timeOut);
}

void VGGetGpsLocation::stopGetGps()
{
    emit sigStopGetGps();
}

void VGGetGpsLocation::setErrorInfo(int error)
{
    if (!m_bStart)
        return;

     m_errorInfo = error;
     emit errorInfoChanged(error);
     qDebug() << "gps is error:......" << error;
}

bool VGGetGpsLocation::IsOpen() const
{
    return m_bOpenGps;
}


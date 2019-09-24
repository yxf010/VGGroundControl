#include "VGSurveyMonitor.h"
#include <QTimerEvent>
#include "VGGlobalFun.h"
#include "VGApplication.h"
#include "VGLandManager.h"
#include "VGLandInformation.h"
#include "VGQXManager.h"
#include "VGBDLocation.h"
#include "VGVehicleManager.h"

VGSurveyMonitor::VGSurveyMonitor(QObject *parent) :QObject(parent)
, m_idTimer(-1), m_countTimeout(0), m_isConnected(0), m_lat(1000)
, m_lon(1000), m_compassAngle(0), m_rollAngle(0), m_pitchAngle(0)
, m_satNum(0), m_isPositionValid(false), m_relativeAltitude(0)
, m_altitude(0), m_distance(0), m_fPrecision(0), m_powerPercent(0)
, m_posMode(0), m_errRange(-1)
{
    connect(qvgApp->qxManager(), &VGQXManager::rtcmRead, this, &VGSurveyMonitor::onReceiveRtcm);
}

VGSurveyMonitor::~VGSurveyMonitor()
{
}

bool VGSurveyMonitor::IsPositionValid() const
{
    return m_isPositionValid;
}

void VGSurveyMonitor::SetPositionValid(bool bValid)
{
    if (m_isPositionValid == bValid)
        return;

    m_isPositionValid = bValid;
    emit positionValidChanged(bValid);
    emit posTipChanged(GetPositionTip());
}

void VGSurveyMonitor::SetLatAndLon(double lat, double lon, bool bHasGGa)
{
    double dis = QGeoCoordinate(lat, lon, m_altitude).distanceTo(GetCoordinate());
    setErrRange(dis);
    SetPositionValid(true);
    m_countTimeout = 0;

    if (!bHasGGa)
        qvgApp->qxManager()->SendGGA(VGQXManager::GenerateGGa(QGeoCoordinate(lat, lon, m_altitude), m_relativeAltitude, m_satNum, m_fPrecision));

    if (lat == m_lat && lon == m_lon)
        return;

    ShareFunction::gpsCorrect(lat, lon);
    m_lat = lat;
    m_lon = lon;
    emit coordinateChanged(GetCoordinate());
}

void VGSurveyMonitor::SetRelativeAltitude(double altitude)
{
    if (m_relativeAltitude == altitude)
        return;

    m_relativeAltitude = altitude;
    emit relativeAltitudeChanged(altitude);
}

void VGSurveyMonitor::SetAltitude(double altitude)
{
    if (m_altitude == altitude)
        return;
    m_altitude = altitude;
    emit altitudeChanged(altitude);
}

void VGSurveyMonitor::SetSatNum(int n)
{
    if (m_satNum == n)
        return;
    m_satNum = n;
    emit satNumChanged(n);
}

void VGSurveyMonitor::SetPowerPercent(double percent)
{
    if (m_powerPercent == percent)
        return;

    m_powerPercent = percent;
    emit powerPercentChanged(percent);
}

void VGSurveyMonitor::SetDistance(double distance)
{
    if (m_distance == distance)
        return;
    m_distance = distance;
    emit distanceChanged(distance);
}

void VGSurveyMonitor::SetCompassAngle(double angle)
{
    if (m_compassAngle == angle)
        return;
    m_compassAngle = angle;
    emit compassAngleChanged(angle);
}

void VGSurveyMonitor::SetPitchAngle(double angle)
{
    if (m_pitchAngle == angle)
        return;
    m_pitchAngle = angle;
    emit pitchAngleChanged(angle);
}

void VGSurveyMonitor::SetRollAngle(double angle)
{
    if (m_rollAngle == angle)
        return;
    m_rollAngle = angle;
    emit rollAngleChanged(angle);
}

void VGSurveyMonitor::SetPrecision(double f)
{
    if (m_fPrecision == f)
        return;

    m_fPrecision = f;
    emit precisionChanged(f);
}

void VGSurveyMonitor::setErrRange(double errRange)
{
    if (m_errRange != errRange)
    {
        m_errRange = errRange;
        emit errRangeChanged(m_errRange);
    }
}

void VGSurveyMonitor::SetConnected(bool connected)
{
    if (m_isConnected == connected)
        return;

    m_isConnected = connected;
    emit connectedChanged(connected);
    if (connected && m_idTimer <= 0)
    {
        _initial();
        m_idTimer = startTimer(2000);
        emit posTipChanged(GetPositionTip());
    }
    else if (!connected)
    {
        if (m_idTimer>0)
        {
            killTimer(m_idTimer);
            m_idTimer = -1;
        }
        SetCheckCoord(QGeoCoordinate());
    }
}

void VGSurveyMonitor::SetPosMode(int mode)
{
    if (m_posMode == mode)
        return;

    m_posMode = mode;
    emit posTipChanged(GetPositionTip());
}

void VGSurveyMonitor::SetFixType(int type)
{
    int acc = 0;
    int gps = 0;
    int tmp = VGGlobalFunc::GetPosTypeByMavFix(type, acc, gps);
    if (tmp > 7 || tmp < 2)
        tmp = gps;
    SetPosMode(tmp);
}

QGeoCoordinate VGSurveyMonitor::GetCheckCoord() const
{
    return m_coorCheck;
}

double VGSurveyMonitor::correctValue(double v, const QString flag)
{
    double lat = (int(v*0.01) + (v*0.01 - int(v*0.01))*100.0 / 60.0);
    if (flag == "S" || flag == "W")
        lat = -lat;

    return lat;
}

double VGSurveyMonitor::toGGAValue(double v)
{
    int degree = v;
    return degree*100 + (v-degree)*60;
}

QString VGSurveyMonitor::GetPositionTip() const
{
    if (IsConnected())
    {
        return VGGlobalFunc::GetPosTypeTip(m_posMode);
    }
    return tr("N/A");
}

QGeoCoordinate VGSurveyMonitor::GetCoordinate() const
{
    return QGeoCoordinate(m_lat, m_lon, m_altitude);
}

bool VGSurveyMonitor::IsConnected() const
{
    return m_isConnected;
}

double VGSurveyMonitor::RelativeAltitude() const
{
    return m_relativeAltitude;
}

double VGSurveyMonitor::Altitude() const
{
    return m_altitude;
}

int VGSurveyMonitor::GetSatNum() const
{
    return m_satNum;
}

double VGSurveyMonitor::PowerPercent() const
{
    return m_powerPercent;
}

double VGSurveyMonitor::Distance() const
{
    return m_distance;
}

double VGSurveyMonitor::CompassAngle() const
{
    return m_compassAngle;
}

double VGSurveyMonitor::RollAngle() const
{
    return m_rollAngle;
}

double VGSurveyMonitor::PitchAngle()
{
    return m_pitchAngle;
}

double VGSurveyMonitor::Precision() const
{
    return m_fPrecision;
}

void VGSurveyMonitor::SetCheckCoord(const QGeoCoordinate &c)
{
    if (c == m_coorCheck)
        return;

    m_coorCheck = c;
    qvgApp->SetQmlTip(QString::fromStdWString(L"校准点设置成功！"));
    emit checkCoordChanged(c);
    if (VGLandInformation *land = qvgApp->landManager()->GetSurveyLand())
        land->SetCheckCoord(c);
}

double VGSurveyMonitor::ErrRange() const
{
    return m_errRange;
}

double VGSurveyMonitor::GetLatitude() const
{
    return m_lat;
}

double VGSurveyMonitor::GetLongitude() const
{
    return m_lon;
}

void VGSurveyMonitor::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_idTimer)
    {
        if (++m_countTimeout >= 3)
            SetPositionValid(false);
        return;
    }
    QObject::timerEvent(event);
}

void VGSurveyMonitor::onReceiveRtcm(const QList<uint16_t> idxs, const QList<QByteArray> &rtcms)
{
    if (VGVehicle *v = qvgApp->vehicleManager()->surveyVehicle())
        v->sendRTCM(idxs, rtcms);

    if (VGBDLocation *bd = qvgApp->GetBDLocation())
        bd->SendRTCM(idxs, rtcms);
}

void VGSurveyMonitor::_initial()
{
    SetPositionValid(false);
    SetPrecision(0);
    SetSatNum(0);
    SetPowerPercent(0);
}

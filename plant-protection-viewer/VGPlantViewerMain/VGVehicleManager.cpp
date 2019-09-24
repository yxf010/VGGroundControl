#include "VGVehicleManager.h"
#include <QDebug>
#include <QtMath>
#include <math.h>
#include "VGApplication.h"
#include "MissionManager.h"
#include "VGLandManager.h"
#include "VGSurveyMonitor.h"
#include "VGPlantManager.h"
#include "VGGlobalFun.h"
#include "VGMacro.h"
#include "VGPlantEvents.h"
#include "MAVLinkProtocol.h"
#include <QtCore/private/qnumeric_p.h>

VGVehicleManager::VGVehicleManager(QObject *parent) : QObject(parent)
  , _activeVehicle(NULL)
  , _activeVehicleAvailable(false)
  , _vehicleBeingSetActive(NULL)
  , _vgsHeartbeatEnabled(true)
  , m_nBatteryAlarm(0)
  , m_bAlarmFirst(true)
{
    if (QSettings *settings = qvgApp->GetSettings())
    {
        settings->beginGroup("config");
        m_nBatteryAlarm = settings->value("batteryAlarm", 30.0).toDouble();
        settings->endGroup();
    }
    _vgsHeartbeatTimer.setInterval(VGSHeartbeatRateMSecs);
    _vgsHeartbeatTimer.setSingleShot(false);
    connect(&_vgsHeartbeatTimer, &QTimer::timeout, this, &VGVehicleManager::sltSendVGHeartbeat);
    if (_vgsHeartbeatEnabled) {
        _vgsHeartbeatTimer.start();
    }
}

void VGVehicleManager::onHeartbeat(LinkInterface *link, int vehicleId, int, int vehicleFirmwareType, int vehicleType)
{
    if(getVehicleById(vehicleId))
        return;

    bool bVehivle = vehicleType != MAVTYPESURVEY;
    VGVehicle* vehicle = new VGVehicle(link, vehicleId, (MAV_AUTOPILOT)vehicleFirmwareType, (MAV_TYPE)vehicleType);
    vehicle->setAutoDisconnect(true);
    connect(this, &VGVehicleManager::sigMavlinkMessageReceived, vehicle, &VGVehicle::_mavlinkMessageReceived);
    qRegisterMetaType<VGVehicle*>("VGVehicle");
    connect(vehicle, &VGVehicle::attitudeChanged, this, &VGVehicleManager::sltVehicleAttitudeChanged);
    connect(vehicle, &VGVehicle::altitudeChanged, this, &VGVehicleManager::sltVehicelAltitudeChanged);
    connect(vehicle, &VGVehicle::gndHeightChanged, this, &VGVehicleManager::sltGndHeightChanged);
    connect(vehicle, &VGVehicle::speedChanged, this, &VGVehicleManager::sltVehicelSpeedChanged);
    connect(vehicle, &VGVehicle::postionChanged, this, &VGVehicleManager::sltVehicelGpsChanged);
    connect(vehicle, &VGVehicle::posTypeChanged, this, &VGVehicleManager::onPosTypeChanged);
    connect(vehicle, &VGVehicle::precisionChanged, this, &VGVehicleManager::onPrecisionChanged);
    connect(vehicle, &VGVehicle::batterryStatus, this, &VGVehicleManager::sltVehicelBatteryStatus);
    connect(vehicle, &VGVehicle::distanceChanged, this, &VGVehicleManager::onVehicelDistanceChanged);
    connect(vehicle, &VGVehicle::homePositionChanged, this, &VGVehicleManager::onHomeChanged);

    connect(vehicle, &VGVehicle::connectionLostChanged, this, &VGVehicleManager::sltConnectionLostChanged);
    connect(vehicle, &VGVehicle::sigUpdateLogList, this, &VGVehicleManager::sltUpdateLogList);
    connect(vehicle, &VGVehicle::sigReceiveLogData, this, &VGVehicleManager::sltReceiveLogData);
    connect(vehicle, &VGVehicle::vehicleUavIdChanged, this, &VGVehicleManager::onVehicleUavIdChanged);
    connect(vehicle, &VGVehicle::sysAvalibleChanged, this, &VGVehicleManager::onSysAvalibleChange);
    connect(vehicle, &VGVehicle::flightModeChanged, this, &VGVehicleManager::onModeChanged);
    connect(vehicle, &VGVehicle::mavCommandResult, this, &VGVehicleManager::onMavCommandResult);
    connect(vehicle, &VGVehicle::paramResult, this, &VGVehicleManager::onParamResult);
	

    if (bVehivle)
    {
        MissionManager *mm = vehicle->missionManager();
        connect(mm, &MissionManager::newMissionItemsAvailable, this, &VGVehicleManager::onNewMissionItemsAvailable);
        connect(mm, &MissionManager::progressPct, this, &VGVehicleManager::onMissionPrcs);
        connect(mm, &MissionManager::currentIndexChanged, this, &VGVehicleManager::onPrcCurrentIndex);
        connect(mm, &MissionManager::lastCurrentIndexChanged, this, &VGVehicleManager::onPrcLastIndex);
        connect(mm, &MissionManager::sendComplete, this, &VGVehicleManager::onSyschFinish);
        connect(vehicle, &VGVehicle::sprayGot, this, &VGVehicleManager::onSprayGot);
        connect(vehicle, &VGVehicle::vehicleCog, this, &VGVehicleManager::onVehicleCog);
        connect(vehicle, &VGVehicle::sysStatus, this, &VGVehicleManager::onSysStatus);
        connect(vehicle, &VGVehicle::assistChanged, this, &VGVehicleManager::onAssistChanged);
        connect(vehicle, &VGVehicle::vehicleEvent, this, &VGVehicleManager::onEventChanged);
        setActiveVehicle(vehicle);
    }
    else if (VGSurveyMonitor *mnt = qvgApp->surveyMonitor())
    {
        mnt->SetConnected(true);
    }

    _vehicles.append(vehicle);
    sltSendVGHeartbeat();
    // Mark link as active
    link->setActive(true);
}

VGVehicle* VGVehicleManager::getVehicleById(int vehicleId)
{
    foreach(VGVehicle* vehicle, _vehicles)
    {
        if (vehicle->id() == vehicleId)
            return vehicle;
    }

    return NULL;
}

VGVehicle *VGVehicleManager::activeVehicle() const
{
    return _activeVehicle;
}

VGVehicle *VGVehicleManager::surveyVehicle() const
{
    foreach(VGVehicle* vehicle, _vehicles)
    {
        if (vehicle->vehicleType() == MAVTYPESURVEY)//测绘
            return vehicle;
    }

    return NULL;
}

void VGVehicleManager::setActiveVehicle(VGVehicle* vehicle)
{
    if (vehicle != _activeVehicle)
    {
        m_bAlarmFirst = true;
        if (_activeVehicle) {
            //_activeVehicle->setActive(false);

            _activeVehicleAvailable = false;
            emit sigActiveVehicleAvailableChanged(false);
        }

        // See explanation in _deleteVehiclePhase1
        _vehicleBeingSetActive = vehicle;
        QTimer::singleShot(20, this, &VGVehicleManager::sltSetActiveVehiclePhase2);
    }
}

void VGVehicleManager::sltSetActiveVehiclePhase2(void)
{
    _activeVehicle = _vehicleBeingSetActive;
    emit sigActiveVehicleChanged(_activeVehicle);

    // And finally vehicle availability
    if (_activeVehicle->vehicleType() != MAVTYPESURVEY)
    {
        if (VGPlantManager *mgr = qvgApp->plantManager())
            mgr->ViaDrectPlantCreate(_activeVehicle->flightMode());

        _activeVehicleAvailable = true;
        emit sigActiveVehicleAvailableChanged(true);
        onVehicleUavIdChanged(_activeVehicle, _activeVehicle->uavid());
    } 
    else
    {
        if (VGSurveyMonitor *mgr = qvgApp->surveyMonitor())
            mgr->SetConnected(true);
    }
}

void VGVehicleManager::sltSendVGHeartbeat()
{
    foreach (VGVehicle* vehicle, _vehicles)
    {
        mavlink_message_t message;
        mavlink_msg_heartbeat_pack(qvgApp->mavLink()->getSystemId(),
                                   qvgApp->mavLink()->getComponentId(),
                                   &message,
                                   MAV_TYPE_GCS,            // MAV_TYPE
                                   MAV_AUTOPILOT_INVALID,   // MAV_AUTOPILOT
                                   MAV_MODE_MANUAL_ARMED,   // MAV_MODE
                                   0,                       // custom mode
                                   MAV_STATE_ACTIVE);       // MAV_STATE
        vehicle->sendMessageOnLink(vehicle->priorityLink(), message);
    }
}

void VGVehicleManager::sltConnectionLostChanged(bool b)
{
    VGVehicle *v = qobject_cast<VGVehicle *>(sender());
    if (_activeVehicle == v)
    {
        if (b)
            qvgApp->SetQmlTip(QString::fromStdWString(L"链接超时，断开通信"), true);
        else
            qvgApp->SetQmlTip(QString::fromStdWString(L"设备已连接"));

        if (VGPlantInformation *plant = qvgApp->plantManager()->GetPlantViaDrect())
            plant->SetStatus(!b? VGPlantInformation::Connected : VGPlantInformation::UnConnect);
    }
    else if (v == surveyVehicle())
    {
        if (VGSurveyMonitor *mgr = qvgApp->surveyMonitor())
            mgr->SetConnected(!b);
    }
}

void VGVehicleManager::onNewMissionItemsAvailable(bool, MAV_MISSION_TYPE tp)
{
    MissionManager *mm = qobject_cast<MissionManager*>(sender());

    if (!mm || _activeVehicle != mm->vehicle())
        return;

    if (VGPlantInformation *plant = qvgApp->plantManager()->GetPlantViaDrect())
        plant->SetMissionValible(true, tp);
}

void VGVehicleManager::onSysAvalibleChange(bool)
{
}

void VGVehicleManager::onModeChanged(const QString &mode)
{
    if (_activeVehicle != qobject_cast<VGVehicle*>(sender()))
        return;

    if (VGPlantInformation *plant = qvgApp->plantManager()->GetPlantViaDrect())
        plant->ChangeFlightMode(mode);
}

void VGVehicleManager::onMavCommandResult(MAV_CMD cmd, bool res)
{
    if (_activeVehicle == sender())
    {
        if (VGPlantInformation *plant = qvgApp->plantManager()->GetPlantViaDrect())
            plant->CommandRes(cmd, res);
    }
}

void VGVehicleManager::onMissionPrcs(double par, bool bs)
{
    MissionManager *mm = qobject_cast<MissionManager*>(sender());
    if (mm && bs)
    {
        if (mm->IsMissonDown())
            qvgApp->SetQmlTip(QString::fromStdWString(L"下载设备航线(%1%)").arg(QString::number(par * 100, 10, 1)));
        else
            qvgApp->SetQmlTip(QString::fromStdWString(L"同步航线到设备(%1%)").arg(QString::number(par * 100, 10, 1)));
    }
}

void VGVehicleManager::onPrcCurrentIndex(int curIndex)
{
    if (MissionManager *mm = qobject_cast<MissionManager*>(sender()))
    {
        VGVehicle *v = mm->vehicle();
        if (!v || v != _activeVehicle)
            return;

        if (VGPlantInformation *plant = qvgApp->plantManager()->GetPlantViaDrect())
            plant->SetCurrentExecuteItem(curIndex);
    }
}

void VGVehicleManager::onPrcLastIndex(int lastIndex)
{
    if (MissionManager *mm = qobject_cast<MissionManager*>(sender()))
    {
        if (mm->vehicle() != _activeVehicle)
            return;

        if (VGPlantInformation *plant = qvgApp->plantManager()->GetPlantViaDrect())
            plant->SetLastExecuteItem(lastIndex);
    }
}

void VGVehicleManager::onVehicleUavIdChanged(VGVehicle *v, const QString &uavid)
{
    if (v == activeVehicle() && !uavid.isEmpty())
    {
        if (VGPlantInformation *plant = qvgApp->plantManager()->GetPlantViaDrect())
            plant->setPlaneId(uavid);
    }
}

void VGVehicleManager::onHomeChanged(const QGeoCoordinate &coor)
{
    if (activeVehicle() != sender())
        return;
    
    if (VGPlantManager *mgr = qvgApp->plantManager())
    {
        if (VGPlantInformation *plant = mgr->GetPlantViaDrect())
            plant->SetHome(VGGlobalFunc::gpsCorrect(coor));
    }
}

void VGVehicleManager::onSyschFinish(bool error)
{
    if (VGPlantManager *mgr = qvgApp->plantManager())
    {
        if (VGPlantInformation *plant = mgr->GetPlantViaDrect())
            plant->SychFinish(!error);
    }
}

void VGVehicleManager::onParamResult(const QString &id, const QVariant &var)
{
    if (VGPlantManager *mgr = qvgApp->plantManager())
    {
        VGPlantInformation *plant = mgr->GetPlantViaDrect();
        if (plant)
            mgr->DealParam(plant, id, var);
    }
}

void VGVehicleManager::onSprayGot(VGVehicle *v, double speed, double vol, uint8_t stat, uint8_t)
{
    if (!v || v != _activeVehicle)
        return;

    if (VGPlantInformation *plant = qvgApp->plantManager()->GetPlantViaDrect())
    {
        plant->SetMedicineSpeed(speed);
        plant->SetMedicineVol(vol);
        plant->SetMedicineGrade(stat);
    }
}

void VGVehicleManager::onVehicleCog(int cog)
{
    if(3 == (cog&7))
	    qvgApp->SetQmlTip(QString::fromStdWString(L"磁异常,需要校磁"), true);

    if (cog & 8)
        qvgApp->SetQmlTip(QString::fromStdWString(L"GPS干扰,请尽快降落!"), true);
}

void VGVehicleManager::onSysStatus(uint8_t st)
{
    if (sender() != _activeVehicle)
        return;

    if (VGPlantInformation *plant = qvgApp->plantManager()->GetPlantViaDrect())
    {
        plant->SetSysStatus(st);
        plant->setLastConnected();
    }
}

void VGVehicleManager::onAssistChanged(const QGeoCoordinate &cE, const QGeoCoordinate &cR, const QGeoCoordinate &cCtn, int res)
{
    if (sender() != _activeVehicle)
        return;

    if (VGPlantInformation *plant = qvgApp->plantManager()->GetPlantViaDrect())
        plant->ReceiveSupports(cE, cR, cCtn, res);
}

void VGVehicleManager::onEventChanged(const QString &id, int evt)
{
    qvgApp->plantEvents()->AddEvent(id, evt);
}

void VGVehicleManager::sltVehicleAttitudeChanged(VGVehicle* vehicle, double roll, double pitch, double yaw)
{
    if (!vehicle)
        return;

    if(vehicle != _activeVehicle && vehicle->vehicleType() != MAVTYPESURVEY)
        return;

    double activeCompass = yaw;
    double activeRoll;
    double activePitch;

    if (qt_is_inf(roll))
        activeRoll = 0;
    else
        activeRoll = roll * (180.0 / M_PI);

    if (qt_is_inf(pitch))
        activePitch = 0;
    else
        activePitch = pitch * (180.0 / M_PI);

    if (qt_is_inf(yaw))
    {
        activeCompass = 0;
    }
    else
    {
        yaw = yaw * (180.0 / M_PI);
        if (yaw < 0)
            yaw += 360;
        activeCompass = yaw;
    }

    if (vehicle->vehicleType() == MAVTYPESURVEY)
    {
        if (VGSurveyMonitor *mnt = qvgApp->surveyMonitor())
        {
            mnt->SetCompassAngle(activeCompass);
            mnt->SetRollAngle(roll);
            mnt->SetPitchAngle(activePitch);
        }
    }
    else
    {
        VGPlantManager *mgr = qvgApp->plantManager();
        if (VGPlantInformation *plant = mgr ? mgr->GetPlantViaDrect() : NULL)
        { 
            plant->SetCompassAngle(activeCompass);
            plant->SetRollAngle(activeRoll);
            plant->SetPitchAngle(activePitch);
        }
    }
}

void VGVehicleManager::sltVehicelAltitudeChanged(VGVehicle *vehicle, double altitude)
{
    if (!vehicle)
        return;

    if (vehicle != _activeVehicle && vehicle->vehicleType() != MAVTYPESURVEY)
        return;

    if (altitude + 1 == altitude)
        altitude = 0.0;

    if (vehicle->vehicleType() == MAVTYPESURVEY)
    {
        if (VGSurveyMonitor *mnt = qvgApp->surveyMonitor())
            mnt->SetAltitude(altitude);
    }
    else
    {
        VGPlantManager *mgr = qvgApp->plantManager();
        if (VGPlantInformation *plant = mgr ? mgr->GetPlantViaDrect() : NULL)
            plant->SetAltitude(altitude);
    }
}

void VGVehicleManager::sltGndHeightChanged(VGVehicle* vehicle, double height)
{
    if (!vehicle || vehicle != _activeVehicle)
        return;

    VGPlantManager *mgr = qvgApp->plantManager();
    if (VGPlantInformation *plant = mgr ? mgr->GetPlantViaDrect() : NULL)
        plant->SetGndHeight(height);
}

void VGVehicleManager::sltVehicelGpsChanged(double lat, double lon, double alt, int nSatNum)
{
    VGVehicle *vehicle = qobject_cast<VGVehicle*>(sender());
    if (!vehicle)
        return;

    if (vehicle != _activeVehicle && vehicle->vehicleType() != MAVTYPESURVEY)
        return;

    //判断是否有效
    if (lat+1 ==lat)
        return;
    if (lon + 1 == lon)
        return;
    if (alt + 1 == alt)
        return;

    if (vehicle->vehicleType() == MAVTYPESURVEY)
    {
        if (VGSurveyMonitor *mnt = qvgApp->surveyMonitor())
        {
            mnt->SetSatNum(nSatNum);
            mnt->SetRelativeAltitude(alt);
            mnt->SetLatAndLon(lat, lon);
        }
    }
    else
    {
        VGPlantManager *mgr = qvgApp->plantManager();
        if (VGPlantInformation *plant = mgr ? mgr->GetPlantViaDrect() : NULL)
        {
            plant->SetSatlateNumb(nSatNum);
            ShareFunction::gpsCorrect(lat, lon);
            plant->setCoordinate(QGeoCoordinate(lat, lon));
            plant->SetRelativeAltitude(alt);
        }
    }
}

void VGVehicleManager::onPosTypeChanged(VGVehicle *vehicle, int fix)
{
    if (vehicle->vehicleType() == MAVTYPESURVEY)
    {
        if (VGSurveyMonitor *mnt = qvgApp->surveyMonitor())
            mnt->SetFixType(fix);
    }
    else if(VGPlantManager *mgr = qvgApp->plantManager())
    {
        if (VGPlantInformation *plant = mgr->GetPlantViaDrect())
            plant->SetFixType(fix);
    }
}

void VGVehicleManager::sltVehicelSpeedChanged(VGVehicle *vehicle, double speedX, double speedY)
{
    if(vehicle != _activeVehicle)
        return;

    VGPlantManager *mgr = qvgApp->plantManager();
    if (VGPlantInformation *plant = mgr ? mgr->GetPlantViaDrect() : NULL)
    {
        plant->SetSpeedHor(speedX);
        plant->SetSpeedVer(speedY);
    }
}

void VGVehicleManager::sltVehicelBatteryStatus(VGVehicle *vehicle, int batterry, uint16_t mv, uint8_t warn)
{
    if (!vehicle)
        return;

    if (vehicle != _activeVehicle && vehicle->vehicleType() != MAVTYPESURVEY)
        return;

    if (vehicle->vehicleType() == MAVTYPESURVEY)
    {
        if (VGSurveyMonitor *mnt = qvgApp->surveyMonitor())
            mnt->SetPowerPercent(batterry);
    }
    else if (VGPlantManager *mgr = qvgApp->plantManager())
    {
        if (VGPlantInformation *plant = mgr->GetPlantViaDrect())
        {
            plant->SetPowerPercent(batterry, batterry<=m_nBatteryAlarm && m_bAlarmFirst);
            if (batterry <= m_nBatteryAlarm)
                m_bAlarmFirst = false;
            plant->SetPowerGrade(warn);
            plant->SetVoltage(mv);
        }
    }
}

void VGVehicleManager::onPrecisionChanged(VGVehicle* vehicle, double pre)
{
    if (vehicle->vehicleType() == MAVTYPESURVEY)
    {
        if (VGSurveyMonitor *mnt = qvgApp->surveyMonitor())
            mnt->SetPrecision(pre);
    }
    else if (VGPlantManager *mgr = qvgApp->plantManager())
    {
        if (VGPlantInformation *plant = mgr->GetPlantViaDrect())
            plant->SetPrecision(pre);
    }
}

void VGVehicleManager::onVehicelDistanceChanged(VGVehicle *vehicle, double distance)
{
    if (!vehicle || vehicle->vehicleType() != MAVTYPESURVEY)
        return;
    if (vehicle->vehicleType() == MAVTYPESURVEY)
    {
        if (VGSurveyMonitor *mnt = qvgApp->surveyMonitor())
            mnt->SetDistance(distance);
    }
    else if (VGPlantManager *mgr = qvgApp->plantManager())
    {
        if (VGPlantInformation *plant = mgr->GetPlantViaDrect())
            plant->setDistance(distance);
    }
}

void VGVehicleManager::sltUpdateLogList(VGVehicle *vehicle, uint32_t time_utc, uint32_t size, uint16_t id, uint16_t num_logs)
{
    if(vehicle != _activeVehicle)
        return;

    emit sigUpdateLogList(time_utc, size, id, num_logs, false);

    if(id == 49 && id < num_logs - 1)
    {
        _activeVehicle->sltRequestLogList(50, num_logs - 1);
    }
}

void VGVehicleManager::sltRequestLogData(int vehicleId, int logId, ulong offset, ulong count)
{
    if(_activeVehicle->id() == vehicleId)
    {
        _activeVehicle->setConnectionLostEnabled(false);
        _activeVehicle->sltRequestLogData(logId, offset, count);
    }
}

void VGVehicleManager::sltReceiveLogData(VGVehicle *vehicle, uint32_t ofs, uint16_t id, uint8_t count, const uint8_t *data)
{
    if(_activeVehicle != vehicle)
    {
        return;
    }

    emit sigReceiveLogData(ofs, id, count, data);
}

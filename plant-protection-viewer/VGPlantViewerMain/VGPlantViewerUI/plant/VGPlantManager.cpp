#include "VGPlantManager.h"
#include <QSettings>
#include <QDateTime>
#include <QtCore/private/qnumeric_p.h>

#include "VGApplication.h"
#include "VGNetManager.h"
#include "VGFlyRoute.h"
#include "das.pb.h"
#include "VGMavLinkCode.h"
#include "VGGlobalFun.h"
#include "ParametersLimit.h"
#include "MissionItem.h"
#include "VGPlantEvents.h"
#include "VGMapManager.h"

using namespace das::proto;
#define MYPLANE_STATUS_TIMEOUT  10		//我的飞机状态查询定时器

typedef union {
    float tmp[7];
    MAVPACKED(struct {
        float velocity[3];
        uint16_t precision; //航线精度
        uint16_t gndHeight; //地面高度
        uint16_t gpsVSpeed; //垂直速度
        uint16_t curMs;     //当前任务点
        uint8_t fixType;    //定位模式及模块状态
        uint8_t baseMode;   //飞控模块状态
        uint8_t satellites; //卫星数
        uint8_t sysStat;        //飞控状态
        uint8_t missionRes : 4; //任务状态
        uint8_t voltageErr : 4; //电压报警
        uint8_t sprayState : 4; //喷洒报警
        uint8_t magneErr : 2;   //校磁报警
        uint8_t gpsJam : 1;     //GPS干扰
        uint8_t stDown : 1;     //下载状态 0:没有或者完成，1:正下载
        uint8_t sysType;        //飞控类型
    });
} GpsAdtionValue;

enum VoiceFlag
{
    Voice_Height = 1,
    Voice_Speed = Voice_Height << 1,
    Voice_VolumnS = Voice_Speed << 1,
    Voice_Sat = Voice_VolumnS << 1,

    Voice_ALL = Voice_Height|Voice_Speed|Voice_VolumnS|Voice_Sat,
};

VGPlantManager::VGPlantManager(QObject *parent /*= 0*/) : QObject(parent)
, m_plantCur(NULL), m_plantBound(NULL), m_bExistBound(false)
, m_idTimerQureryPlant(-1), m_bInitNet(false), m_bOneKeyArm(false)
, m_voice(0), m_bMission(false)
{
    _readConfig();
}

VGPlantManager::~VGPlantManager()
{
    qDeleteAll(m_palnts);
}

VGPlantInformation *VGPlantManager::getExistPlaneBound(const QString &userId)
{
    m_bExistBound = false;
    m_plantBound = NULL;
    foreach(VGPlantInformation* planeInfo, m_palnts)
    {
        if (planeInfo->master() == userId && planeInfo->IsBinded())
        {
            m_bExistBound = true;
            m_plantBound = planeInfo;
            break;
        }
    }
    return m_plantBound;
}

bool VGPlantManager::IsConnected() const
{
    if (VGPlantInformation *p = GetCurrentPlant())
        return p->GetStatus() == VGPlantInformation::Connected;

    return false;
}

void VGPlantManager::syncPlanFlight(VGVehicleMission *vm)
{
    if (VGPlantInformation *p = GetCurrentPlant())
        p->syncPlanFlight(vm);
}

void VGPlantManager::addNewPlant(const QString &planeId, bool bMnt)
{
    if (bMnt && !m_mntPants.contains(planeId))
    {
        _addMonitorPant(planeId);
        _writeConfig();
    }

    if (VGPlantInformation *info = GetPlantById(planeId))
    {
        if (bMnt)
            info->SetMonitor(bMnt);
        return;
    }

    VGNetManager *nm = qvgApp->netManager();
    if (nm)
        nm->requestUavInfo(QStringList(planeId));
}

void VGPlantManager::clearTmpPlant()
{
    foreach(VGPlantInformation *planeInfo, m_palnts)
    {
        if (planeInfo->isDirectLink() || m_mntPants.contains(planeInfo->planeId()))
            continue;

        planeInfo->deleteLater();
    }
}

void VGPlantManager::searchPlant(QGeoCoordinate &coor, double dis/*=200*/)
{
    VGNetManager *nm = qvgApp->netManager();
    if (nm)
        nm->requestUavInfo(coor, dis);
}

void VGPlantManager::remove(VGPlantInformation *pl)
{
    if (!pl)
        return;

    if (!pl->isDirectLink())
    {
        m_palnts.removeAll(pl);
        m_mntPants.removeAll(pl->planeId());

        _writeConfig();
        if (m_strCur == pl->planeId())
            m_strCur.clear();
            
        if (VGNetManager *nm = qvgApp->netManager())
            nm->requestBindUav(pl->planeId(), 0);
    }

    pl->deleteLater();
}

void VGPlantManager::testPump()
{
    if (VGPlantInformation *info = GetCurrentPlant())
        info->TestPump();
}

void VGPlantManager::testMotor(int idx, int time, int speed)
{
	if (VGPlantInformation *info = GetCurrentPlant())
		info->TestMotor(idx, time, speed);
}

void VGPlantManager::shutdown()
{
    if (VGPlantInformation *info = GetCurrentPlant())
        info->Shutdown();
}

void VGPlantManager::disarm()
{
    if (VGPlantInformation *info = GetCurrentPlant())
        info->Disarm();
}

void VGPlantManager::magnetic()
{
    if (IsImitateGround())
    {
        qvgApp->SetQmlTip(QString::fromStdWString(L"空中校磁，必须先关闭仿地!"), true);
        return;
    }

	if (VGPlantInformation *info = GetCurrentPlant())
        info->SetFlightMode(MagMission);
}

bool VGPlantManager::setParameter(const QString &id, const QString &val)
{
    if (VGPlantInformation *info = GetCurrentPlant())
    {
        QVariant v = qvgApp->parameters()->stringToVariant(id, val);
        if (v.isValid())
        {
            info->SetPlantParammeter(id, v);
            return true;
        }
    }
    return false;
}

void VGPlantManager::setParameterIndex(const QString &id, int idx)
{
    if (id.isEmpty() || idx < 0)
        return;

    if (VGPlantInformation *info = GetCurrentPlant())
    {
        QVariant v = qvgApp->parameters()->getValue(id, idx);
        info->SetPlantParammeter(id, v);
    }
}

QString VGPlantManager::getParamValue(const QString &key) const
{
    QString ret;
    if (VGPlantInformation *info = GetCurrentPlant())
        ret = qvgApp->parameters()->getStrValue(key, info->GetParamValue(key));
	
    return ret;
}

int VGPlantManager::getParamIndex(const QString &key) const
{
    int ret = -1;
    if (VGPlantInformation *info = GetCurrentPlant())
        ret = qvgApp->parameters()->getVarIndex(key, info->GetParamValue(key));

    return ret;
}

void VGPlantManager::writeConfig()
{
     _writeConfig();
}

void VGPlantManager::setQxAccount(const QString &acc, const QString &pswd)
{
    if (VGPlantInformation *info = GetCurrentPlant())
        info->SetQxAccount(acc, pswd);
}

void VGPlantManager::getQxAccount()
{
    if (VGPlantInformation *info = GetCurrentPlant())
        info->GetQxAccount();
}

void VGPlantManager::clearQxAccount()
{
    if (VGPlantInformation *info = GetCurrentPlant())
        info->ClearQxAccount();
}

void VGPlantManager::startQueryMyPlaneStatus()
{
    if (m_idTimerQureryPlant > 0)
        return;

    m_idTimerQureryPlant = startTimer(1000);
    QTimerEvent t(m_idTimerQureryPlant);
    timerEvent(&t);
    VGNetManager *nm = qvgApp->netManager();
    if (nm && m_mntPants.size() > 0)
        nm->requestUavInfo(m_mntPants);
}

 void VGPlantManager::stopQueryMyPlaneStatus()
 {
     if (m_idTimerQureryPlant > 0)
     {
         killTimer(m_idTimerQureryPlant);
         m_idTimerQureryPlant = -1;
         foreach(VGPlantInformation *planeInfo, m_palnts)
         {
             if (!planeInfo->isDirectLink())
                 planeInfo->SetStatus(VGPlantInformation::UnConnect);
         }
     }
 }

 VGPlantInformation *VGPlantManager::GetPlantViaDrect() const
 {
     if (m_palnts.count() && m_palnts.first()->isDirectLink())
         return m_palnts.first();

     return NULL;
 }

 void VGPlantManager::ViaDrectPlantCreate(const QString &mode)
 {
     if (GetPlantViaDrect())
         return;

     if (VGPlantInformation *pl = new VGPlantInformation(this))
     {
         pl->SetStatus(VGPlantInformation::Connected);
         pl->setDirectLink(true);
         pl->ChangeFlightMode(mode);
         Insert(pl, 0);
         if (!m_plantCur)
             SetCurrentPlant(pl);
     }
 }

 void VGPlantManager::RemoveViaCloudPlant(const QString &planeId)
 {
     foreach(VGPlantInformation *itr, m_palnts)
     {
         if (itr->isDirectLink())
             continue;

         if (itr->planeId() == planeId)
         {
             m_palnts.removeAll(itr);
             itr->deleteLater();
         }
     }
 }

 VGPlantInformation *VGPlantManager::GetPlantById(const QString &id)
 {
     if (id.isEmpty())
         return NULL;

     foreach(VGPlantInformation *itr, m_palnts)
     {
         if (id.compare(itr->planeId(), Qt::CaseInsensitive) == 0)
             return itr;
     }
     return NULL;
 }

 void VGPlantManager::Insert(VGPlantInformation *info, int i)
 {
     if (!info || m_palnts.contains(info))
         return;

     info->setParent(this);
     if (i > m_palnts.count() || i < 0)
         m_palnts.append(info);
     else
         m_palnts.insert(i, info);
     
     info->LoadFromVehicle();

     connect(info, &QObject::destroyed, this, &VGPlantManager::onChildDestroyed);
     connect(info, &VGPlantInformation::statusChanged, this, &VGPlantManager::onPlantConnectChanged);
     connect(qvgApp->mapManager(), &VGMapManager::mgrObjChanged, info, &VGPlantInformation::onMissionChange);
     info->Show(true);

     QString str = info->planeId();
     if (m_strCur == str && !str.isEmpty())
         SetCurPlant(m_strCur);
 }

 void VGPlantManager::InitialNetWork(VGNetManager *nm)
 {
     if (m_bInitNet)
         return;

     if (nm)
     {
         m_bInitNet = true;
         qRegisterMetaType<OperationStatus>("OperationStatus");
         qRegisterMetaType<UavAttitude>("UavAttitude");
         qRegisterMetaType<GpsInformation>("GpsInformation");
         qRegisterMetaType<OperationParams>("OperationParams");

         connect(nm, &VGNetManager::sigQueryUavStatusSuccess, this, &VGPlantManager::sltPlaneStsInformation);
         connect(nm, &VGNetManager::sigGpsInformation, this, &VGPlantManager::sltVGGpsInformation);
         connect(nm, &VGNetManager::sigVGOperationStatus, this, &VGPlantManager::sltVGOperationStatus);
         connect(nm, &VGNetManager::sigUavAttitude, this, &VGPlantManager::sltVGUavAttitude);
         connect(nm, &VGNetManager::connectStateChanged, this, &VGPlantManager::onNetStatchanged);
         connect(nm, &VGNetManager::sycRouteProcess, this, &VGPlantManager::onSycRouteProcess);
         connect(nm, &VGNetManager::sendUavErro, this, &VGPlantManager::onSendUavErro);
         connect(nm, &VGNetManager::sendFromUav, this, &VGPlantManager::onSendFromUav);
     }
 }

void VGPlantManager::sltDeleteMyPlaneResult(int result, const QString &planeId, const QString &)
{
    if (result != 1)
        return;

    foreach(VGPlantInformation *planeInfo, m_palnts)
    {
        if (planeInfo->planeId() == planeId)
        {
            delete planeInfo;
            m_palnts.removeAll(planeInfo);
            break;
        }
    }
}

void VGPlantManager::sltPlaneStsInformation(int st, const VGPlantInformation &planeInfo)
{
    if (st != 1)
        return;

    VGPlantInformation *info = GetPlantById(planeInfo.planeId());
    if (info)
    {
        info->setPlaneId(planeInfo.planeId());
        info->setContact(planeInfo.contact());
        info->setProductType(planeInfo.productType());
        info->setOrganization(planeInfo.organization()); 
        info->setManager(planeInfo.manager());
        info->SetBinded(planeInfo.IsBinded());
        info->setMaster(planeInfo.master());
        info->setLastTime(planeInfo.lastTime());//5s没有更新视为离线
        info->setBindTime(planeInfo.bindTime());
        info->setUnBindTime(planeInfo.unBindTime());
        info->setCoordinate(planeInfo.lastCoordinate());
    }
    else if (info = new VGPlantInformation(planeInfo))
    {     
        Insert(info);
        if (m_mntPants.contains(planeInfo.planeId()))
            info->SetMonitor(true);
    }
}

void VGPlantManager::sltVGOperationStatus(const QString &id, const OperationStatus &st)
{
    if (VGPlantInformation *info = GetPlantById(id))
    {
        if (st.has_surplusenergy())
            info->SetPowerPercent(st.surplusenergy());
        if (st.has_jetvelocity())
            info->SetMedicineSpeed(st.jetvelocity());
        if (st.has_sprayeddose())
            info->SetMedicineVol(st.sprayeddose());
        if (st.has_operationmode())
            info->ChangeFlightMode(st.operationmode().c_str());
        if (st.has_voltage())
            info->SetVoltage(st.voltage());
    }
}

void VGPlantManager::sltVGGpsInformation(const QString &id, const GpsInformation &gps)
{
    if (VGPlantInformation *info = GetPlantById(id))
    {
        double lat = gps.latitude() / 1e7;
        double lon = gps.longitude() / 1e7;
        double altitude = gps.altitude() / 1e3;
        info->setCoordinate(VGGlobalFunc::gpsCorrect(QGeoCoordinate(lat, lon, altitude)));
        GpsAdtionValue gpsAdt = { 0 };
        int count = (sizeof(GpsAdtionValue) + sizeof(float) - 1) / sizeof(float);
        for (int i = 0; i < gps.velocity_size() && i < count; ++i)
        {
            gpsAdt.tmp[i] = gps.velocity(i);
        }

        info->SetFixType(gpsAdt.fixType);
        info->SetSatlateNumb(gpsAdt.satellites);
        info->SetBaseMode(gpsAdt.baseMode);
        info->SetSpeedVer(gpsAdt.gpsVSpeed/100.0);
        info->SetPrecision(gpsAdt.precision/100.0);
        info->SetGndHeight(gpsAdt.gndHeight/100.0);
        info->SetPowerGrade(gpsAdt.voltageErr);
        info->SetSysStatus(gpsAdt.sysStat);
        info->SetMedicineGrade(gpsAdt.sprayState);
        info->SetDownMisson(gpsAdt.stDown);
        if (gpsAdt.gpsJam)
            qvgApp->SetQmlTip(QString::fromStdWString(L"飞机%1 GPS干扰,请尽快降落!"), true);

        if (gpsAdt.missionRes < MAV_MISSION_RESULT_ENUM_END)
            info->SychFinish(gpsAdt.missionRes == MAV_MISSION_ACCEPTED);
        
        info->SetCurrentExecuteItem(gpsAdt.curMs);
        info->setLastConnected();
    }
    else if(VGNetManager *nm = qvgApp->netManager())
    {
        _addMonitorPant(id);
        if (nm && m_mntPants.size() > 0)
            nm->requestUavInfo(QStringList(id));
    }
}

void VGPlantManager::sltVGUavAttitude(const QString &id, const UavAttitude &attInfo)
{
    if (VGPlantInformation *info = GetPlantById(id))
    {
        if (attInfo.has_roll())
        {
            double f = BASE::toMirrorAngle(attInfo.roll());
            if (qt_is_inf(f))
                f = 0;
            else
                f = f * (180.0 / M_PI);

            info->SetRollAngle(f);
        }
        if (attInfo.has_pitch())
        {
            double f = BASE::toMirrorAngle(attInfo.pitch());
            if (qt_is_inf(f))
                f = 0;
            else
                f = f * (180.0 / M_PI);
            info->SetPitchAngle(f);
        }
        if (attInfo.has_yaw())
        {
            double f = BASE::toMirrorAngle(attInfo.yaw());
            if (qt_is_inf(f))
                f = 0;
            else
                f = f * (180.0 / M_PI);
            info->SetCompassAngle(f);
        }

        if (attInfo.has_relative_alt())
            info->SetRelativeAltitude(attInfo.relative_alt()/1e3);
        if (attInfo.has_groundspeed())
            info->SetSpeedHor(attInfo.groundspeed());
    }
}

void VGPlantManager::onNetStatchanged(int stat)
{
    if (stat == 1)
        startQueryMyPlaneStatus();
    else
        stopQueryMyPlaneStatus();
}

void VGPlantManager::onPlantConnectChanged()
{
    VGPlantInformation *p = GetCurrentPlant();
    if (p && sender() == p)
        emit connectedChanged(IsConnected());
}

void VGPlantManager::onSendUavErro(const QString &id, int res)
{
    QString strErr;
    if (res == -3)
        strErr = QString::fromStdWString(L"无权控制飞机 %1").arg(id);
    else if (res == -2)
        strErr = QString::fromStdWString(L"飞机%1 无效").arg(id);
    else if (res == -1)
        strErr = QString::fromStdWString(L"飞机%1 被禁用").arg(id);

    if (!strErr.isEmpty())
        qvgApp->SetQmlTip(strErr, true);
}

void VGPlantManager::onSendFromUav(const QString &id, const mavlink_message_t &msg, bool finish)
{
    VGPlantInformation *info = GetPlantById(id);
    if (!info)
        return;

    switch (msg.msgid)
    {
    case MAVLINK_MSG_ID_COMMAND_ACK:
        _prcsCmdAck(*info, msg);
        break;
    case MAVLINK_MSG_ID_MISSION_CURRENT:
        _prcsMissionCur(*info, msg);
        break;
    case MAVLINK_MSG_ID_PARAM_VALUE:
        _prcsParamVal(*info, msg);
        break;
    case MAVLINK_MSG_ID_MISSION_REQUEST:
    case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
        info->SendBoundaryItem(VGMavLinkCode::DecodeMissionRequest(msg));
        break;
    case MAVLINK_MSG_ID_MISSION_COUNT:  
        _prcsMissionCount(*info, msg, finish);
        break;
    case MAVLINK_MSG_ID_MISSION_ITEM_INT:
        _prcsMissionItem(*info, msg, finish);
        break;
    case MAVLINK_MSG_ID_HOME_POSITION:
        _prcsHome(*info, msg);
        break;
    case MAVLINK_MSG_ID_ASSIST_POSITION:
        _prcsAssist(*info, msg);
        break;
    case MAVLINK_MSG_ID_VIGA_EVENT:
        _prcsUavEvent(id, msg);
        break;
    case MAVLINK_MSG_ID_QX_ACCOUNT:
        _prcsQXAccount(*info, msg);
        break;
    case MAVLINK_MSG_ID_QX_ACC_CMD:
        _prcsQXCmd(*info, msg);
        break;
    default:
        break;
    }
}

void VGPlantManager::onSycRouteProcess(const QString &id, int count, int index)
{
    bool showProcess = false;
    VGPlantInformation *info = GetPlantById(id);
    if (info)
    {
        if (count < 0)
            info->SychFinish(false);
        else
            showProcess = true;
    }

    if (showProcess)
    {
        double par = double(index+1) / count;
        qvgApp->SetQmlTip(QString::fromStdWString(L"同步航线到飞机(%1%)").arg(QString::number(par * 100, 10, 1)));
        info->SetSychBegin();
    }
}

void VGPlantManager::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != m_idTimerQureryPlant)
        return QObject::timerEvent(event);

    qint64 curMs = QDateTime::currentMSecsSinceEpoch();
    foreach(VGPlantInformation *planeInfo, m_palnts)
    {
        if (!planeInfo->isDirectLink())
            planeInfo->CheckConnect(curMs);
    }
}

void VGPlantManager::SetCurPlant(const QString &id)
{
    VGPlantInformation *pl = GetPlantById(id);
    if (!pl)
        m_strCur = id;
    else if (m_plantCur == pl)
        return;

    if (pl)
        SetCurrentPlant(pl);
}

void VGPlantManager::SetCurrentPlant(VGPlantInformation *pl)
{
    if (m_plantCur == pl)
        return;

    if (m_plantCur)
        m_plantCur->SetMonitor(true, false);

    QString strLast = m_plantCur ? m_plantCur->planeId() : QString();
    QString strTmp = pl ? pl->planeId() : QString();
    m_plantCur = pl;
    _addMonitorPant(strTmp);

    emit currentPlantChanged(pl);
    emit connectedChanged(IsConnected());
    emit imGndChanged();
    if (pl)
        pl->SetMonitor(true, true);

    if (strTmp != strLast && !strTmp.isEmpty())
    {
        VGNetManager *nm = qvgApp->netManager();
        if (!nm)
            return;

        if (!strLast.isEmpty())
            nm->requestBindUav(strLast, 0);
        nm->requestBindUav(strTmp, 1);
        m_strCur = strTmp;
    }
}

VGPlantInformation * VGPlantManager::GetCurrentPlant() const
{
    return m_plantCur;
}

VGPlantInformation *VGPlantManager::GetBoundPlant() const
{
    return m_plantBound;
}

VGPlantInformation *VGPlantManager::_getPlantByIdAndUser(const QString &id, const QString &user)
{
    foreach(VGPlantInformation *itr, m_palnts)
    {
        if (itr->master() == user && itr->planeId() == id)
            return itr;
    }

    return NULL;
}

bool VGPlantManager::IsOneKeyArm() const
{
    return m_bOneKeyArm;
}

void VGPlantManager::DealParam(VGPlantInformation *info, const QString &id, const QVariant &var)
{
    if (!info)
        return;

    if (info->DealPlantParameter(id, var) && GetCurrentPlant()==info)
    {
        emit paramChanged(id);
        if ("MPC_ALT_MODE" == id)
            emit imGndChanged();
    }
}

bool VGPlantManager::IsImitateGround() const
{
    if (VGPlantInformation *info = GetCurrentPlant())
        return info->IsImitateGround();

    return false;
}

void VGPlantManager::SetOneKeyArm(bool b)
{
    if (m_bOneKeyArm == b)
        return;
    m_bOneKeyArm = b;
    emit oneKeyArmChanged(b);
}

void VGPlantManager::SetVoiceHeight(bool b)
{
    if (b == IsVoiceHeight())
        return;

    if (b)
        m_voice |= Voice_Height;
    else
        m_voice &= ~Voice_Height;

    emit voiceHeightChanged(b);
}

bool VGPlantManager::IsVoiceHeight() const
{
    return m_voice & Voice_Height;
}

void VGPlantManager::SetVoiceSpeed(bool b)
{
    if (b == IsVoiceSpeed())
        return;

    if (b)
        m_voice |= Voice_Speed;
    else
        m_voice &= ~Voice_Speed;

    emit voiceSpeedChanged(b);
}

bool VGPlantManager::IsVoiceSpeed() const
{
    return m_voice & Voice_Speed;
}

void VGPlantManager::SetVoiceVS(bool b)
{
    if (b == IsVoiceVS())
        return;

    if (b)
        m_voice |= Voice_VolumnS;
    else
        m_voice &= ~Voice_VolumnS;

    emit voiceVSChanged(b);
}

bool VGPlantManager::IsVoiceVS() const
{
    return m_voice & Voice_VolumnS;
}

void VGPlantManager::SetVoiceSat(bool b)
{
    if (b == IsVoiceSat())
        return;

    if (b)
        m_voice |= Voice_Sat;
    else
        m_voice &= ~Voice_Sat;

    emit voiceSatChanged(b);
}

bool VGPlantManager::IsVoiceSat() const
{
    return m_voice & Voice_Sat;
}

void VGPlantManager::onChildDestroyed(QObject *obj)
{
    int idx = m_palnts.indexOf((VGPlantInformation *)obj);
    if (idx >= 0)
        m_palnts.removeAt(idx);

    if (m_plantCur == obj)
    {
        m_plantCur = NULL;
        emit currentPlantChanged(NULL);
        emit connectedChanged(IsConnected());
    }
}

void VGPlantManager::_readConfig()
{
    if (QSettings *st = qvgApp->GetSettings())
    {
        st->beginGroup("PlantManager");
        m_strCur = st->value("CurPlant", "").toString();
        m_bOneKeyArm = st->value("OneKeyArm", false).toBool();
        m_voice = st->value("voice", Voice_ALL).toUInt();

        int sz = st->beginReadArray("Vehicles");
        for (int i = 0; i < sz; ++i)
        {
            st->setArrayIndex(i);
            _addMonitorPant(st->value("name").toString());
        }
        st->endArray();
        st->endGroup();
    }
}

void VGPlantManager::_writeConfig()
{
    if (QSettings *st = qvgApp->GetSettings())
    {
        st->beginGroup("PlantManager");
        st->beginWriteArray("Vehicles");
        int i = 0;
        for (const QString &inf : m_mntPants)
        {
            st->setArrayIndex(i++);
            st->setValue("name", inf);
        }
        st->endArray();
        st->setValue("CurPlant", m_strCur);
        st->setValue("OneKeyArm", m_bOneKeyArm);
        st->setValue("voice", m_voice);
        st->endGroup();
    }
}

void VGPlantManager::_addMonitorPant(const QString &id)
{
    if (m_mntPants.contains(id) || id.isEmpty())
        return;

    m_mntPants << id;
}

void VGPlantManager::_prcsCmdAck(VGPlantInformation &p, const mavlink_message_t &m)
{
    uint16_t cmd;
    MAV_RESULT result;
    if (VGMavLinkCode::DecodeCommandAck(m, cmd, result))
        p.CommandRes(cmd, result == MAV_RESULT_ACCEPTED);
}

void VGPlantManager::_prcsMissionCur(VGPlantInformation &p, const mavlink_message_t &m)
{
    mavlink_mission_current_t missionCurrent;
    mavlink_msg_mission_current_decode(&m, &missionCurrent);
    p.SetCurrentExecuteItem(missionCurrent.seq);
}

void VGPlantManager::_prcsParamVal(VGPlantInformation &p, const mavlink_message_t &m)
{
    QVariant v;
    QString key = VGMavLinkCode::DecodeParameter(m, v);
    DealParam(&p, key, v);
}

void VGPlantManager::_prcsMissionCount(VGPlantInformation &p, const mavlink_message_t &msg, bool b)
{
    mavlink_mission_count_t msCount;
    mavlink_msg_mission_count_decode(&msg, &msCount);
    p.SetMissionCount(msCount.count, b);
}

void VGPlantManager::_prcsMissionItem(VGPlantInformation &p, const mavlink_message_t &m, bool b)
{
    MissionItem item;
    if (VGMavLinkCode::DecodeMissionItem(m, item))
        p.SetMissionItem(item, b);
}

void VGPlantManager::_prcsHome(VGPlantInformation &p, const mavlink_message_t &msg)
{
    QGeoCoordinate c;
    if (VGMavLinkCode::DecodeHomePos(msg, c))
        p.SetHome(VGGlobalFunc::gpsCorrect(c), true);
}

void VGPlantManager::_prcsAssist(VGPlantInformation &plant, const mavlink_message_t &msg)
{
    QGeoCoordinate et;
    QGeoCoordinate ret;
    QGeoCoordinate ctn;
    int tp = VGMavLinkCode::DecodeSupport(msg, et, ret, ctn);
    if (tp >= 0)
        plant.ReceiveSupports(et, ret, ctn, tp);
}

void VGPlantManager::_prcsUavEvent(const QString &id, const mavlink_message_t &msg)
{
    mavlink_viga_event_t e;
    mavlink_msg_viga_event_decode(&msg, &e);
    qvgApp->plantEvents()->AddEvent(id, e.viga_event);
}

void VGPlantManager::_prcsQXCmd(VGPlantInformation &plant, const mavlink_message_t &msg)
{
    mavlink_qx_acc_cmd_t qxcmd = { 0 };
    mavlink_msg_qx_acc_cmd_decode(&msg, &qxcmd);
    if (qxcmd.cmd == 2 || qxcmd.cmd == 3)
    {
        QString str = QString::fromStdWString(qxcmd.cmd == 2 ? L"设置千寻账号成功！" : L"清除千寻账号成功！");
        qvgApp->SetQmlTip(str);
        qvgApp->speakNotice(str);
        if (qxcmd.cmd == 3)
            plant.ClearQxAccount(false);
    }
}

void VGPlantManager::_prcsQXAccount(VGPlantInformation &plant, const mavlink_message_t &msg)
{
    QString pswd;
    QString acc = VGMavLinkCode::DecodeQXAccount(msg, pswd);
    plant.SetQxAccount(acc, pswd, false);
}

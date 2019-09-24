#include "VGPlantInformation.h"
#include "VGApplication.h"
#include "VGPlantManager.h"
#include "VGNetManager.h"
#include "VGFlyRoute.h"
#include "VGVehicleMission.h"
#include "VGGlobalFun.h"
#include "VGCoordinate.h"
#include "VGOutline.h"
#include "VGMavLinkCode.h"
#include "VGLandInformation.h"
#include "VGLandPolygon.h"
#include "ParametersLimit.h"
#include "MissionManager.h"
#include "VGSupportPolyline.h"
#include "VGMapManager.h"
#include "VGToolBox.h"
#include "VGVehicle.h"
#include "VGVehicleManager.h"
#include "MAVLinkProtocol.h"
#include <stdio.h>

#define MAXHEIGHT 1000
#define DOUBLELIIM 1000000
#define ISEQDouble(f1, f2) (int)((f1)*DOUBLELIIM)==(int)((f2)*DOUBLELIIM)
#define TESTMOTORPALSEW 1150
#define TMOUT_CONNECT   5000
#define MINVTGFORMS   3.7

enum SupportStat
{
    NoOperator = 0,
    SetSupport = 1,
    ClearSupport,
    GetSupport,
    GetAllSupport = GetSupport | (GetSupport<<8),
    SetHomePos = 0x10000,
    UAVSetHome = SetHomePos<<1,
};

static QString getStringFormBool(bool b)
{
    return QString::fromStdWString(b ? L"成功" : L"失败");
}

static QString getStatDescribe(int stat, MAV_CMD cmd)
{
    if (cmd == MAV_CMD_SET_START_POINT)
    {
        SupportStat stTmp = SupportStat(stat & 0xff);
        if (SetSupport == stTmp)
            return QString::fromStdWString(L"设置起飞辅助点");
        else if (ClearSupport == stTmp)
            return QString::fromStdWString(L"清除起飞辅助点");
    }
    else if (cmd == MAV_CMD_SET_END_POINT)
    {
        stat >>= 8;
        SupportStat stTmp = SupportStat(stat & 0xff);
        if (SetSupport == stTmp)
            return QString::fromStdWString(L"设置返航辅助点");
        else if (ClearSupport == stTmp)
            return QString::fromStdWString(L"清除返航辅助点");
    }
    return QString();
}

QString getDecString(double f)
{
    char str[256];
    sprintf(str, "%.2lf", f);
    return str;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//VGPlantInformation
//////////////////////////////////////////////////////////////////////////////////////////////
VGPlantInformation::VGPlantInformation(QObject *parent) : MapAbstractItem(parent)
, m_status(UnConnect), m_bBind(0), m_distance(0), m_bDirectLink(false)
, m_bLanded(true), m_userId("None"), m_satNum(0), m_horSpeed(0)
, m_verSpeed(0), m_relativeAltitude(0), m_compassAngle(0)
, m_rollAngle(0), m_pitchAngle(0), m_powerPercent(0)
, m_medicineSpeed(0), m_medicineVol(0), m_precision(0)
, m_uploadVm(NULL), m_bMonitor(false), m_lastTime(0)
, m_lastLacalTime(QDateTime::currentMSecsSinceEpoch())
, m_bMissionValible(false), m_tpPos(0), m_homeCoor(NULL), m_cntnCoor(NULL)
, m_planMissionLine(NULL), m_acc(0), m_baseMode(0), m_bSetArm(false)
, m_bSychUav(false), m_gndHeight(0), m_bSysBoudary(false), m_bSys(false)
, m_powerGrade(0),m_medicineGrade(0), m_voltage(0), m_bWlcLose(true)
, m_bArmOp(false), m_missionCount(0), m_bDownMs(false), m_rtl(NULL)
, m_statSupport(GetAllSupport)
{
    connect(this, &VGPlantInformation::missionRun, this, &VGPlantInformation::onMissionRun);
}

VGPlantInformation::VGPlantInformation(const VGPlantInformation &oth) : MapAbstractItem(oth)
, m_productType(oth.m_productType), m_organization(oth.m_organization), m_medicineVol(oth.m_medicineVol)
, m_manager(oth.m_manager), m_uploadVm(NULL), m_contact(oth.m_contact)
, m_bBind(oth.m_bBind), m_distance(oth.m_distance), m_lastTime(oth.m_lastTime), m_master(oth.m_master)
, m_lastPosition(oth.m_lastPosition), m_userId(oth.m_userId), m_bindTime(oth.m_bindTime)
, m_unBindTime(oth.m_unBindTime), m_bDirectLink(oth.m_bDirectLink), m_bLanded(oth.m_bLanded)
, m_satNum(oth.m_satNum), m_horSpeed(oth.m_horSpeed), m_verSpeed(oth.m_verSpeed)
, m_relativeAltitude(oth.m_relativeAltitude), m_compassAngle(oth.m_compassAngle)
, m_rollAngle(oth.m_rollAngle), m_pitchAngle(oth.m_pitchAngle)
, m_powerPercent(oth.m_powerPercent), m_medicineSpeed(oth.m_medicineSpeed)
, m_precision(0), m_status(oth.m_status), m_bMonitor(oth.m_bMonitor)
, m_lastLacalTime(oth.m_lastLacalTime), m_homeCoor(NULL), m_cntnCoor(NULL)
, m_bMissionValible(false), m_tpPos(oth.m_tpPos), m_bSychUav(false)
, m_planMissionLine(NULL), m_acc(oth.m_acc), m_bSetArm(false)
, m_baseMode(oth.m_baseMode), m_gndHeight(oth.m_gndHeight)
, m_bSysBoudary(false), m_bSys(false), m_powerGrade(0)
, m_medicineGrade(0), m_voltage(oth.m_voltage), m_bWlcLose(true)
, m_bArmOp(false), m_missionCount(0), m_bDownMs(false), m_rtl(NULL)
, m_statSupport(GetAllSupport)
{
    if (oth.m_homeCoor)
        SetHome(oth.m_homeCoor->GetCoordinate());
    if (oth.m_cntnCoor)
        SetContinueCoor(oth.m_cntnCoor->GetCoordinate());

    connect(this, &VGPlantInformation::missionRun, this, &VGPlantInformation::onMissionRun);
    setPlaneId(oth.planeId());
}

VGPlantInformation::~VGPlantInformation()
{
    if (m_planMissionLine)
        m_planMissionLine->deleteLater();
    if (m_uploadVm)
        m_uploadVm->deleteLater();
}

QString VGPlantInformation::planeId() const
{
    return m_planeId;
}

void VGPlantInformation::setPlaneId(const QString &id)
{
    if (id == m_planeId)
        return;

    if(m_bDirectLink)
    {
        if (VGPlantManager *mgr = qvgApp->plantManager())
            mgr->RemoveViaCloudPlant(id);
    }
    m_planeId = id;
    emit planeIdChanged(id);
}

QString VGPlantInformation::productType()const
{
    return m_productType;
}

void VGPlantInformation::setProductType(const QString &id)
{
    if (m_productType == id)
        return;
    m_productType = id;
    emit productTypeChanged(id);
}


QString VGPlantInformation::organization()const
{
    return m_organization;
}

void VGPlantInformation::setOrganization(const QString &id)
{
    if (id == m_organization)
        return;
    m_organization = id;
    emit organizationChanged(id);
}

void VGPlantInformation::setManager(const QString &id)
{
    if (id == m_manager)
        return;

    m_manager = id;
    emit managerChanged(id);
}

void VGPlantInformation::setContact(const QString &id)
{
    if (id == m_contact)
        return;
    m_contact = id;
    emit contactChanged(id);
}

VGPlantInformation::PlantStatus VGPlantInformation::GetStatus() const
{
    return m_status;
}

void VGPlantInformation::SetStatus(PlantStatus status)
{
    if (status == m_status)
        return;

    m_status = status;
    emit statusChanged();
    if (status == UnConnect)
    {
        SetPosType(0);
        SetAccType(0);
        SetGpsType(0);
    }
    else
    {
        GetParamValue("BAT_N_CELLS");
    }

    emit posTipChanged();
    emit accTipChanged();
    emit missionValibleChanged(IsMissionValible());
}

bool VGPlantInformation::IsBinded() const
{
    return m_bBind;
}

void VGPlantInformation::SetBinded(bool status)
{
    if (status == m_bBind)
        return;

    m_bBind = status;
    emit bindedChanged(status);
}

void VGPlantInformation::setMaster(const QString &master)
{
    if (master == m_master)
        return;

    m_master = master;
    emit masterChanged(master);
}

qint64 VGPlantInformation::lastTime() const
{
    return m_lastTime;
}

void VGPlantInformation::setLastTime(qint64 time)
{
    if (m_lastTime != time)
        m_lastTime = time;
}

void VGPlantInformation::setLastConnected()
{
    m_lastLacalTime = QDateTime::currentMSecsSinceEpoch();
    SetStatus(Connected);
    VGFlyRoute *rt = m_uploadVm ? m_uploadVm->GetFlyRoute() : NULL;
    if (rt && m_flightMode == GetFlightModeDscb(MissionMod))
    {
        float tmp = rt->GetSprinkleWidth();
        if (GetParamValue("SPRAY_SWAT") != tmp)
            SetPlantParammeter("SPRAY_SWAT", tmp);

        tmp = rt->GetMedPerAcre() * 1000;
        if (GetParamValue("SPRAY_QUAN") != tmp)
            SetPlantParammeter("SPRAY_QUAN", tmp);

        tmp = m_uploadVm->GetSpeed();
        if (GetParamValue("MPC_XY_CRUISE") != tmp)
            SetPlantParammeter("MPC_XY_CRUISE", tmp);
    }
    _sysFlightMode();
    prcsSupportAndHome();
}

void VGPlantInformation::SetLanded(bool isLanded)
{
    if (m_bLanded == isLanded)
        return;

    m_bLanded = isLanded;
    qvgApp->speakNotice(QString::fromStdWString(m_bLanded ? L"已加锁": L"已解锁"));
    emit landedChanged();
}

void VGPlantInformation::setCoordinate(const QGeoCoordinate &coor)
{
    bool bValidLast = m_lastPosition.isValid();
    bool bValid = coor.isValid();
    double dis = (bValid&&bValidLast) ? m_lastPosition.distanceTo(coor) : 0;

    m_lastPosition = bValid ? coor : QGeoCoordinate();
    emit coordinateChanged(coor);
    if (bValid != bValidLast)
    {
        emit posTipChanged();
        emit accTipChanged();
        emit gpsTipChanged();
    }

    if (bValid && m_planMissionLine && m_uploadVm && !IsLanded() && dis < 1e3)
        m_planMissionLine->addCoordinate(coor);
}

QGeoCoordinate VGPlantInformation::lastCoordinate() const
{
    return m_lastPosition;
}

double VGPlantInformation::distance() const
{
    return m_distance;
}

void VGPlantInformation::setDistance(double distance)
{
    if (distance == m_distance)
        return;

    m_distance = distance;
}

double VGPlantInformation::GetHeigth()
{
    return !IsLanded()&&IsImitateGround() ? m_gndHeight : m_relativeAltitude;
}

void VGPlantInformation::SetRelativeAltitude(double f)
{
    if (!IsImitateGround() && GetSelected() && qvgApp->plantManager()->IsVoiceHeight())
        qvgApp->speakNotice(QString::fromStdWString(L"高度:%1"), QStringList(getDecString(f)));

    if (m_gndHeight < -MAXHEIGHT || m_gndHeight > MAXHEIGHT)
        return;

    m_relativeAltitude = f;
    if (!IsImitateGround())
        emit heigthChanged(f);
}

void VGPlantInformation::SetAltitude(double f)
{
    m_lastPosition.setAltitude(f);
}

int VGPlantInformation::GetSatlateNumb() const
{
    return m_satNum;
}

void VGPlantInformation::SetSatlateNumb(int n)
{
    if (GetSelected() && qvgApp->plantManager()->IsVoiceSat())
        qvgApp->speakNotice(QString::fromStdWString(L"星数:%1"), QStringList(QString::number(n)));
    
    if (n == m_satNum)
        return;

    m_satNum = n;
    emit satlateNumbChanged(n);
}

double VGPlantInformation::GetSpeedHor() const
{
    return m_horSpeed;
}

void VGPlantInformation::SetSpeedHor(double f)
{
    if (GetSelected() && qvgApp->plantManager()->IsVoiceSpeed())
        qvgApp->speakNotice(QString::fromStdWString(L"速度:%1"), QStringList(getDecString(f)));
    
    if (f == m_horSpeed)
        return;
    m_horSpeed = f;
    emit horSpeedChanged(f);
}

double VGPlantInformation::GetSpeedVer() const
{
    return m_verSpeed;
}

void VGPlantInformation::SetSpeedVer(double f)
{
    if (f == m_verSpeed)
        return;

    m_verSpeed = f;
    emit verSpeedChanged(f);
}

double VGPlantInformation::GetCompassAngle() const
{
    return m_compassAngle;
}

void VGPlantInformation::SetCompassAngle(double f)
{
    if (f == m_compassAngle)
        return;

    m_compassAngle = f;
    emit compassAngleChanged(f);
}

double VGPlantInformation::GetRollAngle() const
{
    return m_rollAngle;
}

void VGPlantInformation::SetRollAngle(double f)
{
    if (f == m_rollAngle || f > 180 || f < -180)
        return;

    m_rollAngle = f;
    emit rollAngleChanged(f);
}

double VGPlantInformation::GetPitchAngle() const
{
    return m_pitchAngle;
}

void VGPlantInformation::SetPitchAngle(double f)
{
    if (f == m_pitchAngle || f > 180 || f < -180)
        return;

    m_pitchAngle = f;
    emit pitchAngleChanged(f);
}

double VGPlantInformation::GetPowerPercent() const
{
    return m_powerPercent;
}

void VGPlantInformation::SetPowerPercent(double f, bool alarm)
{
    if (f > 100 || f < 0)
        f = 100;

    if (alarm)
        qvgApp->speakNotice(QString::fromStdWString(L"可用电量过低，请及时返航"));
    if (f == m_powerPercent)
        return;

    m_powerPercent = f;
    emit powPercentChanged(f);
}

double VGPlantInformation::GetMedicineSpeed() const
{
    return m_medicineSpeed;
}

void VGPlantInformation::SetMedicineSpeed(double f)
{
    if (GetSelected() && qvgApp->plantManager()->IsVoiceSpeed())
        qvgApp->speakNotice(QString::fromStdWString(L"流速:%1"), QStringList(getDecString(f)));

    if (f == m_medicineSpeed)
        return;

    m_medicineSpeed = f;
    emit medSpeedChanged(f);
}

double VGPlantInformation::GetMedicineVol() const
{
    return m_medicineVol;
}

void VGPlantInformation::SetMedicineVol(double f)
{
    if (f == m_medicineVol)
        return;

    m_medicineVol = f;
    emit medVolChanged(f);
}

double VGPlantInformation::GetPrecision() const
{
    return m_precision;
}

void VGPlantInformation::SetPrecision(double f)
{
    if (f == m_precision)
        return;
    m_precision = f;
    emit precisionChanged(f);
}

void VGPlantInformation::SetMonitor(bool b, bool bCur)
{
    if (bCur)
        SetSelected(true);

    if (m_bMonitor == b)
        return;

    m_bMonitor = b;
    emit monitorChanged(b);
}

void VGPlantInformation::setUserId(const QString &id)
{
    if (id.isEmpty() || m_userId == id)
        return;

    m_userId = id;
    emit userIdChanged(id);
}

bool VGPlantInformation::isDirectLink() const
{
    return m_bDirectLink;
}

void VGPlantInformation::setDirectLink(bool b)
{
    if (m_bDirectLink == b)
        return;

    m_bDirectLink = b;
    emit directLinkChanged(b);
    emit posTipChanged();
}

bool VGPlantInformation::IsLanded() const
{
    return m_bLanded;
}

bool VGPlantInformation::operator==(const MapAbstractItem &item)const
{
    if (item.ItemType() != ItemType())
        return false;

    return true;
}

void VGPlantInformation::CheckConnect(qint64 curMs)
{
    if (!isDirectLink() && curMs - m_lastLacalTime > TMOUT_CONNECT)
    {
        SetStatus(UnConnect);
        return;
    }
}

void VGPlantInformation::syncPlanFlight(VGVehicleMission *vm)
{
    if (m_uploadVm && vm != m_uploadVm)
        m_uploadVm->deleteLater();

    m_uploadVm = vm;
    if (!m_uploadVm)
        return;

    if (m_uploadVm->MissionItems().size() > 100)
    {
        qvgApp->SetQmlTip(QString::fromStdWString(L"飞机每次任务最多支持100航路点！"), true);
        return;
    }

    if (m_uploadVm->CountBlock() > 0 && m_uploadVm->BoundaryItems().size() > 100)
    {
        qvgApp->SetQmlTip(QString::fromStdWString(L"边界点加障碍物点不能超过100！"), true);
        return;
    }

    m_uploadVm->AtachByPlant();
    connect(m_uploadVm, &QObject::destroyed, this, &VGPlantInformation::onContentDestroied);

    emit missionChanged(vm);
    if (GetStatus() != Connected || !vm)
        return;

    m_bMissionValible = false;
    SetFlightMode(QString());
    SetArm(false);
    if (m_bDirectLink)
    {
        VGVehicle *vk = qvgApp->vehicleManager()->activeVehicle();
        if (vk && vk->isMissionIdle())
        {
            vk->syncPlanFlight(vm);
            SetSychBegin();
        }
    }
    else if (VGNetManager *nm = qvgApp->netManager())
    {
        nm->syncPlanning(vm, m_planeId);
        connect(nm, &VGNetManager::missionUploaded, this, &VGPlantInformation::onSysFinish);
    }
}

MapAbstractItem::MapItemType VGPlantInformation::ItemType() const
{
    return Type_PlantInfo;
}

void VGPlantInformation::SetHome(const QGeoCoordinate &c, bool bRcv)
{
    if (!c.isValid())
        return;

    if (!m_homeCoor)
        m_homeCoor = new VGCoordinate(c, VGCoordinate::Home, this);

    if (!m_homeCoor)
        return;

    m_homeCoor->SetId(bRcv ? VGCoordinate::Home : VGCoordinate::HomeSet);
    m_homeCoor->SetCoordinate(c);

    if (m_homeCoor && GetSelected())
        m_homeCoor->Show(true);
}

void VGPlantInformation::CommandRes(unsigned short cmd, bool res)
{
    switch (cmd)
    {
    case MAV_CMD_COMPONENT_ARM_DISARM:
        _prcsArmOrDisarm(res);
        break;
    case MAV_CMD_DO_SET_MODE:
        _prcsFligtMode(res);
        break;
    case MAV_CMD_SET_START_POINT:
    case MAV_CMD_SET_END_POINT:
        _prcsSetAssist(cmd, res);
        break;
    case MAV_CMD_GET_START_POINT:
    case MAV_CMD_GET_END_POINT:
        _prcsGetAssist(cmd);
        break;
    case MAV_CMD_DO_SET_HOME:
        _prcsSetHome(res);
        break;
    default:
        break;
    }
}

QString VGPlantInformation::GetFlightMode() const
{
    return m_flightMode;
}

void VGPlantInformation::ChangeFlightMode(const QString &mod)
{
    QString str = GetFlightModeDscb(mod);
    if (m_flightMode == str)
		return;

	m_flightMode = str;
	emit flightModeChanged();
}

void VGPlantInformation::SetFlightMode(const QString &mod)
{
    if (m_flightModeSet == mod)
        return;

    m_flightModeSet = mod;
/*    if (MagMission == mod)
        SendSetHome();
    else*/ if (IsMissionValible() || m_flightMode != GetFlightModeDscb(MissionMod))
        _sysFlightMode();
}

void VGPlantInformation::SetMissionValible(bool b, int tp)
{
    if (m_bMissionValible == b)
        return;

    m_bMissionValible = b;
    emit missionValibleChanged(IsMissionValible());
    if (IsMissionValible())
        _sysFlightMode();

    MissionManager *mm = dynamic_cast<MissionManager *>(sender());
    if (mm && MAV_MISSION_TYPE_AOBPATH == tp)
    {
        SetDownMisson(true);
        SetMissionCount(mm->missionItems().count(), false);
        for (MissionItem *itr : mm->missionItems())
        {
            SetMissionItem(*itr, false);
        }
        SetDownMisson(false);
    }
}

void VGPlantInformation::SetCurrentExecuteItem(int idx)
{
    if (m_uploadVm)
        m_uploadVm->SetCurrentExecuteItem(idx);
}

void VGPlantInformation::SetLastExecuteItem(int idx)
{
    if (m_uploadVm)
        m_uploadVm->SetLastExecuteItem(idx);
}

void VGPlantInformation::SetFixType(unsigned char fixType)
{
    int acc, gps;
    int rtk = VGGlobalFunc::GetPosTypeByMavFix(fixType, acc, gps);
    SetPosType(rtk);
    SetAccType(acc);
    SetGpsType(gps);
}

void VGPlantInformation::SetPosType(int t)
{
    if (m_tpPos == t)
        return;

    if (m_tpPos == 4 && m_uploadVm && GetStatus()==Connected)
        qvgApp->SetQmlTip(QString::fromStdWString(L"导航非差分，请尽快就近降落"), true);

    m_tpPos = t;
    emit posTipChanged();
}

void VGPlantInformation::SetAccType(int t)
{
    if (m_acc == t)
        return;

    m_acc = t;
    emit accTipChanged();
}

void VGPlantInformation::SetGpsType(int t)
{
    if (m_gps == t)
        return;

    m_gps = t;
    emit gpsTipChanged();
}

void VGPlantInformation::SetMedicineGrade(int t)
{
	if (t == m_medicineGrade)
		return;

    m_medicineGrade = t;
}

void VGPlantInformation::SetPowerGrade(int t)
{
	if (t == m_powerGrade)
		return;

	m_powerGrade = t;
}

VGVehicleMission * VGPlantInformation::GetVehicleMission() const
{
    return m_uploadVm;
}

void VGPlantInformation::Disarm()
{
    _arm(false);
}

void VGPlantInformation::SetBaseMode(unsigned short bm)
{
    m_baseMode = bm;
}

void VGPlantInformation::SetSelected(bool b)
{
    MapAbstractItem::SetSelected(b);
    if (b && m_uploadVm)
        m_uploadVm->SetSelected(true);
    else if (b && !m_uploadVm)
        loadMission();

    if (m_rtl)
        m_rtl->Show(b);

    if (m_homeCoor)
        m_homeCoor->Show(b);

    if (m_cntnCoor)
        m_cntnCoor->Show(b);

    showContent(b);
}

void VGPlantInformation::SetSychBegin()
{
    m_bSychUav = true;
    m_bSys = true;
    m_bSysBoudary = false;
}

bool VGPlantInformation::IsImitateGround()
{
    QVariant var = GetParamValue("MPC_ALT_MODE");
    return var.toInt()!=0;
}

void VGPlantInformation::UploadBoundary()
{
    if (VGFlyRoute *rt = m_uploadVm ? m_uploadVm->GetFlyRoute() : NULL)
    {
        uint32_t count = rt->CountBlock();
        m_bSysBoudary = count > 0 ? false : true;
        if (m_bDirectLink)
        {
            if (VGVehicle *v = qvgApp->toolbox()->activeVehicle())
                v->syncPlanFlight(m_uploadVm, true);
        }
        else if (VGNetManager *nm = qvgApp->netManager())
        {
            mavlink_message_t message;
            VGMavLinkCode::EncodeMissionCount(message, count>0 ? rt->GetSafeBoudaryItems().count() : 0, MAV_MISSION_TYPE_OBSTACLE);
            nm->SendControlUav(m_planeId, message);
        }
    }
}

void VGPlantInformation::SendBoundaryItem(int seq)
{
    VGNetManager *nm = qvgApp->netManager();

    if (m_uploadVm && nm && seq>=0)
    {
        const QList<MissionItem*> &items = m_uploadVm->BoundaryItems();
        if (seq >= items.count())
            return;

        double par = double(seq + 1) / items.count();
        qvgApp->SetQmlTip(QString::fromStdWString(L"同步飞行边界到飞机(%1%)").arg(QString::number(par * 100, 10, 1)));
        int len = items.count() - seq;
        if (len > 5)
            len = 5;

        nm->SendControlUav(m_planeId, items.mid(seq, len));
        m_bSysBoudary = true;
    }
}

void VGPlantInformation::SetVoltage(uint16_t mv)
{
    if (int(m_voltage * 1000) == mv)
        return;
    m_voltage = float(mv) / 1000;
    emit voltageChanged();
}

float VGPlantInformation::GetVoltage() const
{
    return m_voltage;
}

void VGPlantInformation::SendSupport(bool bEnter, VGCoordinate *spt)
{
    int id = spt ? spt->GetId() : 0;
    if (id == VGCoordinate::SupportEnter || id == VGCoordinate::SupportReturn)
        return;

    MAV_CMD cmd = bEnter ? MAV_CMD_SET_START_POINT : MAV_CMD_SET_END_POINT;
    int lat = 0;
    int lon = 0;
    float alt = 0;
    if (spt)
    {
        QGeoCoordinate coor = VGGlobalFunc::toGps(spt->GetCoordinate());
        lat = coor.latitude()*1e7;
        lon = coor.longitude()*1e7;
        alt = coor.altitude();
    }
    float cl = spt ? 1 : 0;
    if (m_bDirectLink)
    {
        if (VGVehicle *v = qvgApp->toolbox()->activeVehicle())
            v->sendMavCommandInt(cmd, 0, 0, 0, cl, lat, lon, alt);
    }
    else if (VGNetManager *nm = qvgApp->netManager())
    {
        mavlink_message_t msg;
        VGMavLinkCode::EncodeCommandInt(msg, 0, cmd, NULL, 0, 0, 0, 0, 0, cl, lat, lon, alt);
        nm->SendControlUav(m_planeId, msg);
    }

    int stat = spt ? SetSupport : ClearSupport;
    if (!bEnter)
        stat = (stat << 8) | (m_statSupport & 0xff);
    else
        stat |= (m_statSupport & 0xff00);
    m_statSupport = stat;
}

void VGPlantInformation::SetSysStatus(uint8_t st)
{
    bool bLand = 0;
    switch (st & 7)
    {
    case MAV_STATE_STANDBY:
        bLand = true;
        break;
    case MAV_STATE_ACTIVE:
        bLand = false;
        break;
    }
    if (bLand != m_bLanded)
    {
        SetLanded(bLand);
        if (m_uploadVm && bLand && m_rtl)
        {
            m_rtl->Show(false);
            m_rtl->Clear();
        }
    }
    bool bLose = 0 != (st & 0x10);
    if (m_bWlcLose != bLose)
    {
        m_bWlcLose = !m_bWlcLose;
        emit wlcLoseChanged();
    }
}

bool VGPlantInformation::IsWlcLose() const
{
    return m_bWlcLose;
}

void VGPlantInformation::SetMissionCount(int c, bool finish)
{
    m_missionCount = c;
    if (finish)
    {
        if (!m_rtl)
        {
            m_rtl = new VGSupportPolyline(this);
            m_rtl->SetBorderColor(Qt::green);
        }
        m_rtl->Show(false);
        m_rtl->Clear();
        reqMissionFromNet(0);
    }
}

void VGPlantInformation::SetMissionItem(const MissionItem &item, bool finish)
{
    QGeoCoordinate coor = VGGlobalFunc::gpsCorrect(item.coordinate());
    if(m_rtl->CountPoint() == item.sequenceNumber() && m_missionCount>0)
    {
        m_rtl->AddPoint(coor);
        if (finish)
            reqMissionFromNet(m_rtl->CountPoint());
    }

    if (item.sequenceNumber() + 1 == m_missionCount)
        m_rtl->Show(GetSelected());
}

void VGPlantInformation::SetDownMisson(bool b)
{
    if(!b && m_bDownMs!=b && m_missionCount>0)
    {
        m_missionCount = 0;
        if(m_missionCount = m_rtl->CountPoint())
        {
            QString str = QString::fromStdWString(L"下载飞机避障路线完成！");
            qvgApp->SetQmlTip(str);
            qvgApp->speakNotice(str);
        }
    }
    m_bDownMs = b;
}

void VGPlantInformation::SendSetHome(const QGeoCoordinate &c)
{
    bool bValid = c.isValid();
    if(bValid)
    {
        SetHome(VGGlobalFunc::gpsCorrect(c));
        if (m_homeCoor)
            m_homeCoor->SetId(VGCoordinate::HomeSet);
    }
    m_statSupport &= ~(SetHomePos | UAVSetHome);
    m_statSupport |= bValid ? SetHomePos : UAVSetHome;
    sendSetHome(!bValid, c);
}

void VGPlantInformation::ReceiveSupports(const QGeoCoordinate &beg, const QGeoCoordinate &end, const QGeoCoordinate &cut, int st)
{
    if (st < 0)
        return;

    if (m_uploadVm)
    {
        int tmp = (m_statSupport & 0xff);
        if(tmp!=SetSupport || tmp!=ClearSupport)
        {
            if (st & 1)
                m_uploadVm->addSupport(VGGlobalFunc::gpsCorrect(beg), true, true);
            else
                m_uploadVm->clearSupport(true, true);
        }

        tmp = ((m_statSupport >> 8) & 0xff);
        if (tmp != SetSupport || tmp != ClearSupport)
        {
            if (st & 2)
                m_uploadVm->addSupport(VGGlobalFunc::gpsCorrect(end), false, true);
            else
                m_uploadVm->clearSupport(false, true);
        }
    }
    SetContinueCoor(VGGlobalFunc::gpsCorrect(cut), 0==(st&4));
}

void VGPlantInformation::SetQxAccount(const QString &acc, const QString &pswd, bool bSend)
{
    if (bSend)
    {
        if (m_bDirectLink)
        {
            if (VGVehicle *v = qvgApp->toolbox()->activeVehicle())
                v->setQxAccount(acc, pswd);
        }
        else if (VGNetManager *nm = qvgApp->netManager())
        {
            mavlink_message_t message;
            VGMavLinkCode::EncodeQXAccount(message, acc, pswd);
            nm->SendControlUav(m_planeId, message);
        }
    }

    if (acc == m_strQxAcc && pswd == m_strQxPswd)
        return;

    m_strQxAcc = acc;
    m_strQxPswd = pswd;
    emit qxaccChanged();
    emit qxpswdChanged();
}

void VGPlantInformation::GetQxAccount()
{
    if (m_bDirectLink)
    {
        VGVehicle *v = qvgApp->toolbox()->activeVehicle();
        MAVLinkProtocol *p = qvgApp->mavLink();
        if (v && p)
        {
            mavlink_message_t message;
            mavlink_msg_qx_acc_cmd_pack(p->getSystemId(), p->getComponentId(), &message, MAV_QX_ACC_CMD_READ);
            v->sendMessage(message);
        }
    }
    else if (VGNetManager *nm = qvgApp->netManager())
    {
        mavlink_message_t message;
        mavlink_msg_qx_acc_cmd_pack(0, 0, &message, MAV_QX_ACC_CMD_READ);
        nm->SendControlUav(m_planeId, message);
    }
}

void VGPlantInformation::ClearQxAccount(bool bSend)
{
    if (bSend)
    {
        if (m_bDirectLink)
        {
            VGVehicle *v = qvgApp->toolbox()->activeVehicle();
            MAVLinkProtocol *p = qvgApp->mavLink();
            if (v && p)
            {
                mavlink_message_t message;
                mavlink_msg_qx_acc_cmd_pack(p->getSystemId(), p->getComponentId(), &message, MAV_QX_ACC_CMD_CLEAR);
                v->sendMessage(message);
            }
        }
        else if (VGNetManager *nm = qvgApp->netManager())
        {
            mavlink_message_t message;
            mavlink_msg_qx_acc_cmd_pack(0, 0, &message, MAV_QX_ACC_CMD_CLEAR);
            nm->SendControlUav(m_planeId, message);
        }
    }
    else
    {
        m_strQxAcc.clear();
        m_strQxPswd.clear();
        emit qxaccChanged();
        emit qxpswdChanged();
    }
}

void VGPlantInformation::reqMissionFromNet(int idx)
{
    if (idx >= 0 && !m_bDirectLink && m_missionCount>0)
    {
        mavlink_message_t msg = {0};
        if (idx == m_missionCount)
            mavlink_msg_mission_ack_pack_chan(0, 0, 0, &msg, 0, 0, 4, 0);
        else
            mavlink_msg_mission_request_pack_chan(0, 0, 0, &msg, 0, 0, idx, 4);
        
        qvgApp->netManager()->SendControlUav(m_planeId, msg);
    }
}

void VGPlantInformation::SetContinueCoor(const QGeoCoordinate &c, bool clear)
{
    if (!c.isValid() || clear)
    {
        if (m_cntnCoor)
            m_cntnCoor->SetCoordinate(QGeoCoordinate());
        return;
    }

    if (!m_cntnCoor)
        m_cntnCoor = new VGCoordinate(c, VGCoordinate::ContinueFly, this);
    else
        m_cntnCoor->SetCoordinate(c);

    if (m_cntnCoor && GetSelected())
        m_cntnCoor->Show(!clear);
}

QString VGPlantInformation::GetQXAcc() const
{
    return m_strQxAcc;
}

QString VGPlantInformation::GetQXPswd() const
{
    return m_strQxPswd;
}

QString VGPlantInformation::GetFlightModeDscb(const QString &str)
{
    QString strMode = QString::fromStdWString(L"未知");
    if (str == DenyMod)
        strMode = QString::fromStdWString(L"禁飞");
    else if (str == MissionMod)
        strMode = QString::fromStdWString(L"任务");
    else if (str == ManualMod)
        strMode = QString::fromStdWString(L"手动");
    else if (str == HoldMod)
        strMode = QString::fromStdWString(L"悬停");
    else if (str == ReturnMod)
        strMode = QString::fromStdWString(L"返航");
    else if (str == MagMission)
        strMode = QString::fromStdWString(L"校磁");
    else if (str == Landing)
        strMode = QString::fromStdWString(L"着陆");

    return strMode;
}

bool VGPlantInformation::DealPlantParameter(const QString &key, const QVariant &v)
{
    if (qvgApp->parameters()->hasParam(key))
    {
        QMap<QString, QVariant>::iterator itr = m_params.find(key);
        if (itr == m_params.end() || itr.value() != v)
        {
            m_params[key] = v;
            return true;
        }
        m_strLsReqParams.remove(key);
    }

    return false;
}

void VGPlantInformation::SetGndHeight(double height)
{
    if (IsImitateGround() && GetSelected() && qvgApp->plantManager()->IsVoiceHeight())
        qvgApp->speakNotice(QString::fromStdWString(L"高度:%1"), QStringList(getDecString(height)));

    if (m_gndHeight < -MAXHEIGHT || m_gndHeight > MAXHEIGHT)
        return;

    m_gndHeight = height;
    if (IsImitateGround())
        emit  heigthChanged(height);
}

QVariant VGPlantInformation::GetParamValue(const QString &key)
{
    QMap<QString, QVariant>::const_iterator itr = m_params.find(key);
    if (itr != m_params.end())
        return itr.value();

    if(qvgApp->parameters()->hasParam(key))
        GetPlantParammeter(key);

    return QVariant();
}

void VGPlantInformation::onMissionChange()
{
    bool b = GetSelected() && qvgApp->mapManager()->IsMissionPage();
    if (m_uploadVm)
        m_uploadVm->Show(b);

    if (m_planMissionLine)
        m_planMissionLine->Show(b);
}

void VGPlantInformation::LoadFromVehicle()
{
    if (m_bDirectLink)
    {
        if (VGVehicle *v = qvgApp->toolbox()->activeVehicle())
            v->loadFromVehicle();
    }
    else
    {
    }
}

void VGPlantInformation::TestPump()
{
    sendCommand(MAV_CMD_VAGA_TEST, 1);
}

void VGPlantInformation::TestMotor(int idx, int ms, int speed)
{
	if (idx >= 8)
		return;

    sendCommand(MAV_CMD_VAGA_TEST, 0, 1<<idx, speed, ms);
}

void VGPlantInformation::Shutdown()
{
    sendCommand(MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN, 1);
}

void VGPlantInformation::SetPlantParammeter(const QString &key, const QVariant &var)
{
    if (GetParamValue(key) == var)
        return;

    if (m_bDirectLink)
    {
        if (VGVehicle *v = qvgApp->toolbox()->activeVehicle())
            v->setVehicleParameter(key, var);
    }
    else if (VGNetManager *nm = qvgApp->netManager())
    {
        setParameter(key, var);
    }
}

void VGPlantInformation::GetPlantParammeter(const QString &key)
{
    if (m_bDirectLink)
    {
        if (VGVehicle *v = qvgApp->toolbox()->activeVehicle())
            v->getVehicleParameter(key);
    }
    else if (VGNetManager *nm = qvgApp->netManager())
    {
        readParameter(key);
    }
}

void VGPlantInformation::SychFinish(bool bsuc)
{
    if (!m_bSys)
        return;

    if (m_bSychUav)
    { 
        m_bSychUav = false;
        QString suc = bsuc ? QString::fromStdWString(L"上传航线到飞机成功！") : QString::fromStdWString(L"上传航线到飞机失败！");
        qvgApp->SetQmlTip(suc, !bsuc);
        if (bsuc)
            qvgApp->speakNotice(suc);

        if (bsuc)
            saveMission();
    }
 
    if (bsuc)
    {
        if (m_planMissionLine)
            m_planMissionLine->clear();

        if(qvgApp->plantManager()->IsOneKeyArm())
            SetArm(true);
    }
}

bool VGPlantInformation::IsMonitor() const
{
    return m_bMonitor;
}

bool VGPlantInformation::IsSetArm() const
{
    return m_bSetArm;
}

void VGPlantInformation::SetArm(bool b)
{
    QVariant var = GetParamValue("BAT_N_CELLS");
    if (b && var.isValid() && GetVoltage()/var.toInt()<MINVTGFORMS)
    {
        qvgApp->SetQmlTip(QString::fromStdWString(L"电池电压低，请更换电池再作业!"),true);
        return;
    }

    if(b)
    {
        m_bSetArm = b;
        if (m_flightMode == GetFlightModeDscb(MissionMod))
            _arm(b);
        else
            SetFlightMode(MissionMod);
    }

    emit armChanged();
}

bool VGPlantInformation::IsMissionValible() const
{
    return m_status==Connected && m_bMissionValible;
}

QString VGPlantInformation::GetPosTip() const
{
    
    if (GetStatus() == Connected && lastCoordinate().isValid())
        return VGGlobalFunc::GetPosTypeTip(m_tpPos);

    return tr("N/A");
}

QString VGPlantInformation::GetAccTip() const
{
    if (GetStatus() == UnConnect || !lastCoordinate().isValid())
        return tr("N/A");

    switch (m_acc)
    {
    case 3:
        return QString::fromStdWString(L"不可用");
    case 5:
        return QString::fromStdWString(L"可用");
    default:
        break;
    }

    return QString::fromStdWString(L"未工作");
}

QString VGPlantInformation::GetGpsTip() const
{
    if (!lastCoordinate().isValid())
        return tr("N/A");

    switch (m_gps)
    {
    case 0:
        return QString::fromStdWString(L"未工作");
    case 1:
        return QString::fromStdWString(L"未定位");
    case 2:
        return QString::fromStdWString(L"2D定位");
    case 3:
        return QString::fromStdWString(L"单点定位");
    default:
        break;
    }
    return QString::fromStdWString(L"N/A");
}

void VGPlantInformation::showContent(bool b)
{
    bool bMisson = qvgApp->mapManager()->IsMissionPage();
    if (m_homeCoor)
        m_homeCoor->Show(b);
    if (m_uploadVm)
        m_uploadVm->Show(b && bMisson);
    if (m_planMissionLine)
        m_planMissionLine->Show(b && bMisson);
}

void VGPlantInformation::_arm(bool bArm)
{
    if (m_bDirectLink)
    {
        if (VGVehicle *v = qvgApp->toolbox()->activeVehicle())
            v->setArmed(bArm);
    }
    else if (VGNetManager *nm = qvgApp->netManager())
    {
        mavlink_message_t msg;
        VGMavLinkCode::EncodeCommands(msg, 0, MAV_CMD_COMPONENT_ARM_DISARM, NULL, 0, 0, bArm ? 1.0 : 0);
        nm->SendControlUav(m_planeId, msg);
    }
    m_bArmOp = !bArm;
    if (bArm)
        CheckConnect(m_lastLacalTime);
}

void VGPlantInformation::_sysFlightMode()
{
    if ((m_flightModeSet!=MissionMod || IsMissionValible()) && !m_flightModeSet.isEmpty())
    {
        if (m_bDirectLink)
        {
            if (VGVehicle *v = qvgApp->toolbox()->activeVehicle())
                v->SetFlightMode(m_flightModeSet);
        }
        else if (VGNetManager *nm = qvgApp->netManager())
        {
            mavlink_message_t msg;
            VGMavLinkCode::EncodeSetMode(msg, m_flightModeSet, m_baseMode);
            nm->SendControlUav(m_planeId, msg);
        }
    }
}

void VGPlantInformation::onSysFinish(const QString &id)
{
    if(id == m_planeId)
        SetMissionValible(true, MAV_MISSION_TYPE_MISSION);
}

void VGPlantInformation::onContentDestroied(QObject *o)
{
    if (m_uploadVm == o)
        m_uploadVm = NULL;
}

void VGPlantInformation::onMissionRun()
{
    if (m_planMissionLine != NULL)
    {
        m_planMissionLine->Show(false);
        m_planMissionLine->deleteLater();
    }

    m_planMissionLine = new VGOutline(this, VGLandPolygon::NoPoint);
    m_planMissionLine->SetBorderColor(QColor("#F01010"));
    m_planMissionLine->SetShowType(Show_Line);
    m_planMissionLine->Show(true);
}

void VGPlantInformation::readParameter(const QString &str)
{
    bool bSend = m_strLsReqParams.size()>0;
    QMap<QString, QVariant>::iterator itr = m_strLsReqParams.find(str);
    if (itr==m_strLsReqParams.end())
        m_strLsReqParams[str] = QVariant();

    if (!bSend)
        qvgApp->netManager()->SendControlUav(m_planeId, m_strLsReqParams);
}

void VGPlantInformation::setParameter(const QString &str, const QVariant &var)
{
    bool bSend = m_strLsReqParams.size()>0;
    m_strLsReqParams[str] = var;

    if (str == "MPC_ALT_MODE" && var.toInt() == 0)
        setParameter("MPC_Z_P", QVariant::fromValue<float>(1.2f));

    if (!bSend)
        qvgApp->netManager()->SendControlUav(m_planeId, m_strLsReqParams);
}

void VGPlantInformation::saveMission()
{
    if (m_uploadVm && !m_planeId.isEmpty())
    {
        if (QSettings *st = qvgApp->GetSettings())
        {
            QString str = QString("Mission%1").arg(m_planeId);
            st->beginGroup(str);
            st->setValue("info", m_uploadVm->GetInfo());
            st->endGroup();
        }
    }
}

void VGPlantInformation::loadMission()
{
    if (m_uploadVm || m_planeId.isEmpty())
        return;

    if (QSettings *st = qvgApp->GetSettings())
    {
        QString str = QString("Mission%1").arg(m_planeId);
        st->beginGroup(str);
        m_uploadVm = VGVehicleMission::fromInfo(st->value("info").toString());
        m_bMissionValible = true;
        emit missionValibleChanged(IsMissionValible());
        st->endGroup();
    }
}

void VGPlantInformation::sendCommand(int cmd, float f1, float f2, float f3, float f4, float f5, float f6, float f7)
{
    if (m_bDirectLink)
    {
        if (VGVehicle *v = qvgApp->toolbox()->activeVehicle())
            v->sendMavCommand((MAV_CMD)cmd, f1, f2, f3, f4, f5, f6, f7);
    }
    else if (VGNetManager *nm = qvgApp->netManager())
    {
        mavlink_message_t msg;
        VGMavLinkCode::EncodeCommands(msg, 0, (MAV_CMD)cmd, NULL, 0, 0, f1, f2, f3, f4, f5, f6, f7);
        nm->SendControlUav(m_planeId, msg);
    }
}

void VGPlantInformation::sendSetHome(bool uav, const QGeoCoordinate &c)
{
    int lat = uav ? 0 : c.latitude()*1e7;
    int lon = uav ? 0 : c.longitude()*1e7;
    float alt = uav ? 0 : c.altitude();
    if (m_bDirectLink)
    {
        if (VGVehicle *v = qvgApp->toolbox()->activeVehicle())
            v->sendMavCommandInt(MAV_CMD_DO_SET_HOME, uav?1:0, 0, 0, 0, lat, lon, alt);
    }
    else if (VGNetManager *nm = qvgApp->netManager())
    {
        mavlink_message_t msg;
        VGMavLinkCode::EncodeCommandInt(msg, 0, MAV_CMD_DO_SET_HOME, NULL, 0, 0, uav ? 1 : 0, 0, 0, 0, lat, lon, alt);
        nm->SendControlUav(m_planeId, msg);
    }
}

void VGPlantInformation::prcsSupportAndHome()
{
    if (!m_homeCoor || !m_homeCoor->IsValid())
        sendCommand(MAV_CMD_GET_HOME_POSITION);
    if ((m_statSupport&SetHomePos) && m_homeCoor && m_homeCoor->GetId() == VGCoordinate::HomeSet)
        sendSetHome(false, VGGlobalFunc::toGps(m_homeCoor->GetCoordinate()));

    if (m_strLsReqParams.size() > 0)
        qvgApp->netManager()->SendControlUav(m_planeId, m_strLsReqParams);
    SupportStat stTmp = SupportStat(m_statSupport & 0xff);
    if (SetSupport == stTmp && m_uploadVm && m_uploadVm->GetSupportEnter())
        SendSupport(true, m_uploadVm->GetSupportEnter());
    else if (ClearSupport == stTmp)
        SendSupport(true, NULL);
    else if (GetSupport == stTmp)
        sendCommand(MAV_CMD_GET_START_POINT);

    stTmp = SupportStat((m_statSupport >> 8) & 0xff);
    if (SetSupport == stTmp && m_uploadVm && m_uploadVm->GetSupportReturn())
        SendSupport(false, m_uploadVm->GetSupportReturn());
    else if (ClearSupport == stTmp)
        SendSupport(false, NULL);
    else if (GetSupport == stTmp)
        sendCommand(MAV_CMD_GET_END_POINT);
}

void VGPlantInformation::_prcsArmOrDisarm(bool res)
{
    QString str = QString::fromStdWString(L"飞机%1%2").arg(QString::fromStdWString(m_bArmOp ? L"加锁" : L"解锁")).arg(getStringFormBool(res));
    qvgApp->SetQmlTip(str, !res);

    if (IsMissionValible() && res && !m_bArmOp)
    {
        emit missionRun();
        m_uploadVm->showSquences(true);
    }
    m_bSetArm = false;
}

void VGPlantInformation::_prcsFligtMode(bool res)
{
    if (res)
    {
        bool bSend = m_strLsReqParams.count() < 1;
        if (m_flightModeSet == MissionMod && m_uploadVm)
        {
            QVariant tmp = QVariant::fromValue<float>(m_uploadVm->GetOpHeight());
            m_strLsReqParams["MPC_WORK_MIN_ALT"] = tmp;
            m_strLsReqParams["RTL_DESCEND_ALT"] = tmp;
            m_strLsReqParams["MIS_TAKEOFF_ALT"] = tmp;
            m_strLsReqParams["EKF2_ALT1_SAFETY"] = QVariant::fromValue(m_uploadVm->MaxWorkAlt() + 3);
        }
        else if (m_flightModeSet == MagMission)
        {
            m_strLsReqParams["MPC_WORK_MIN_ALT"] = QVariant::fromValue<float>(3);
        }
        if (!bSend)
            qvgApp->netManager()->SendControlUav(m_planeId, m_strLsReqParams);
    }

    if (!res)
        qvgApp->SetQmlTip(QString::fromStdWString(L"设置飞行模式失败"), true);
    else if (m_bSetArm || m_flightModeSet == MagMission)
        _arm(true);

    m_flightModeSet = QString();
}

void VGPlantInformation::_prcsSetAssist(int cmd, bool res)
{
    QString str = getStatDescribe(m_statSupport, (MAV_CMD)cmd);
    if (!str.isEmpty())
        qvgApp->SetQmlTip(str + getStringFormBool(res) + "!", !res);

    if (cmd == MAV_CMD_SET_START_POINT)
        m_statSupport &= ~0xff;
    else
        m_statSupport &= ~0xff00;
}

void VGPlantInformation::_prcsGetAssist(int cmd)
{
    if (cmd == MAV_CMD_GET_START_POINT)
        m_statSupport &= ~0xff;
    else
        m_statSupport &= ~0xff00;
}

void VGPlantInformation::_prcsSetHome(bool res)
{
    qvgApp->SetQmlTip(QString::fromStdWString(L"%1设置Home点%2!").arg(m_bSetArm ?"UAV":"").arg(getStringFormBool(res)), !res);
    m_statSupport &= ~(SetHomePos | UAVSetHome);
    if(res && m_bSetArm)
    {
        if (m_flightMode == GetFlightModeDscb(MissionMod) && m_bMissionValible)
            _arm(true);
        else
            _sysFlightMode();
    }
}

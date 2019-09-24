#include "VGFlyRoute.h"
#include "MapItemFactory.h"
#include "VGLandBoundary.h"
#include "VGLandInformation.h"
#include "VGCoordinate.h"
#include "RouteStruct.h"
#include "VGLandPolygon.h"
#include "VGOutline.h"
#include "VGApplication.h"
#include "VGMapManager.h"
#include "VGPlantManager.h"
#include "VGLandManager.h"
#include "VGVehicleMission.h"
#include "VGSelectEdge.h"
#include "VGGlobalFun.h"
#include "VGPlanningWorker.h"
#include "MissionItem.h"

static bool ComparePolyLineList(const QList<VGOutline*> &ls1, const QList<VGOutline*> &ls2)
{
    if (ls1.count() != ls2.count())
        return false;

    QList<VGOutline*>::const_iterator itr = ls1.begin();
    QList<VGOutline*>::const_iterator itr2 = ls2.begin();
    for (; itr != ls1.end(); ++itr, ++itr2)
    {
        if (**itr != **itr2)
            return false;
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//VGFlyRoute
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VGFlyRoute::VGFlyRoute(QObject *parent) : SingleTriggerItem<MapAbstractItem>(qobject_cast<VGLandBoundary*>(parent))
, m_sprinkleWidth(6), m_maxRoute(5), m_sprinkleCover(0)
, m_totalVoyage(0), m_transVoyage(0), m_operationHeight(3)
, m_operateVoyage(0), m_bErrorPlan(false), m_bSaveLocal(false)
, m_bPlan(false), m_homePoint(new VGCoordinate()), m_time(0)
, m_bUploaded(false), m_route(NULL), m_fBlockSafeDis(.5)
, m_fAngle(0), m_fPrise(10), m_olMission(NULL)
, m_medPerAcre(1), m_bAnti(false), m_nCurEdge(-1)
, m_bSingleShrink(false), m_fSafeDis(.5)
{
    setBelongBoundary(qobject_cast<VGLandBoundary*>(parent));
    connect(this, &VGFlyRoute::singleShrinkChanged, this, &VGFlyRoute::onSingleShrinkChanged);
}

VGFlyRoute::VGFlyRoute(const VGFlyRoute &oth) : SingleTriggerItem<MapAbstractItem>(oth)
, m_sprinkleWidth(oth.m_sprinkleWidth), m_maxRoute(oth.m_maxRoute)
, m_operationHeight(oth.m_operationHeight), m_sprinkleCover(oth.m_sprinkleCover)
, m_totalVoyage(oth.m_totalVoyage), m_homePoint(new VGCoordinate(*oth.m_homePoint))
, m_transVoyage(oth.m_transVoyage), m_operateVoyage(oth.m_operateVoyage)
, m_bUploaded(oth.m_bUploaded), m_strDescribe(oth.m_strDescribe)
, m_bPlan(oth.m_bPlan), m_bSaveLocal(oth.m_bSaveLocal), m_bAnti(oth.m_bAnti)
, m_time(oth.m_time), m_fBlockSafeDis(oth.m_fBlockSafeDis), m_route(NULL)
, m_strCropper(oth.m_strCropper), m_bErrorPlan(false), m_fPrise(oth.m_fPrise)
, m_strPesticide(oth.m_strPesticide), m_fAngle(oth.m_fAngle)
, m_strActId(oth.m_strActId), m_olMission(NULL), m_medPerAcre(1)
, m_nCurEdge(oth.m_nCurEdge), m_lsOutlineSafe(oth.m_lsOutlineSafe)
, m_fSafeDis(oth.m_fSafeDis), m_bSingleShrink(oth.m_bSingleShrink)
{
    if (oth.m_route)
    {
        m_route = new VGOutline(*oth.m_route);
        m_route->SetId(VGLandPolygon::FlyRoute);
        m_route->SetWidth(1);
        m_route->SetParentItem(this);
        m_route->SetShowType(Show_LineAndPoint);
        m_route->SetVisible(GetVisible());
    }
}

VGFlyRoute::~VGFlyRoute()
{
    qDeleteAll(m_safeBdrs);
    delete m_homePoint;
}

VGLandBoundary *VGFlyRoute::GetBelongedBoundary() const
{
    return (VGLandBoundary *)m_parentItem;
}

VGLandInformation *VGFlyRoute::GetBelongedLand() const
{
    if (VGLandBoundary *bd = GetBelongedBoundary())
        return bd->GetBelongedLand();

    return NULL;
}

MapAbstractItem::MapItemType VGFlyRoute::ItemType() const
{
    return MapAbstractItem::Type_FlyRoute;
}

void VGFlyRoute::onChildDestroied(QObject *obj)
{
    if (obj == m_olMission)
        m_olMission = NULL;
}

void VGFlyRoute::onSingleShrinkChanged(bool b)
{
    if (b && m_lsSelectEdge.size() < 1)
    {
        VGLandBoundary *bdrCur = GetBelongedBoundary();
        VGLandPolygon *plg = bdrCur ? bdrCur->GetBoundaryPolygon() : NULL;
        if (plg)
        {
            QList<VGCoordinate *> ls = plg->GetCoordinates();
            for (int i = 0; i < ls.size(); ++i)
            {
                VGSelectEdge *edge = new VGSelectEdge(this);
                edge->SetBeg(ls.at(i)->GetCoordinate());
                edge->SetEnd(ls.at((i+1) % ls.size())->GetCoordinate());
                m_lsSelectEdge << edge;
                connect(edge, &VGSelectEdge::selected, this, &VGFlyRoute::onSelectedEdge);
            }
            m_nCurEdge = 0;
        }
    }
    else if(!b)
    {
        m_lsOutlineSafe.clear();
        emit outlineSafeChanged(m_fSafeDis);
    }
}

void VGFlyRoute::onSelectedEdge(VGSelectEdge *edge)
{
    int nS = m_lsSelectEdge.indexOf(edge);
    if (m_nCurEdge != nS)
    {
        m_nCurEdge = nS;
        emit curEdgeChanged(nS);
        emit outlineSafeChanged(GetOutlineSafe());
    }
}

void VGFlyRoute::planRoute(bool chgBdr)
{
    if (canPlanRoute())
    {
        emit sigPlanRoute(this);
        if (chgBdr && !m_safeBdrs.isEmpty())
        {
            qDeleteAll(m_safeBdrs);
            m_safeBdrs.clear();
        }
    }
}

bool VGFlyRoute::isPlanning() const
{
    return m_bPlan;
}

bool VGFlyRoute::IsValide() const
{
    return !m_bErrorPlan && m_route!=NULL;
}

VGOutline *VGFlyRoute::allMissionRoute() const
{
    return m_route;
}

QString VGFlyRoute::GetBoundaryID() const
{
    if (VGLandBoundary *bdr = GetBelongedBoundary())
        return bdr->GetBoundaryId();

    return QString();
}

QString VGFlyRoute::GetActId() const
{
    return m_strActId.isEmpty() ? QString::number(m_time) : m_strActId;
}

void VGFlyRoute::SetActId(const QString &id)
{
    m_strActId = id;
    emit actIdchanged(id);
}

QString VGFlyRoute::GetCropper() const
{
    return m_strCropper;
}

void VGFlyRoute::SetCropper(const QString &c)
{
    if (m_strCropper == c)
        return;

    m_strCropper = c;
    emit cropperChanged(c);
}

QString VGFlyRoute::GetPesticide() const
{
    return m_strPesticide;
}

void VGFlyRoute::SetPesticide(const QString &c)
{
    if (m_strPesticide == c)
        return;

    m_strPesticide = c;
    emit pesticideChanged(c);
}

double VGFlyRoute::GetPrice() const
{
    return m_fPrise;
}

void VGFlyRoute::SetPrice(double f)
{
    if (m_fPrise == f)
        return;

    m_fPrise = f;
    emit priceChanged(f);
}

double VGFlyRoute::GetBlockSafeDis() const
{
    return m_fBlockSafeDis;
}

void VGFlyRoute::SetBlockSafeDis(double f)
{
    if (m_fBlockSafeDis == f)
        return;

    m_fBlockSafeDis = f;
    emit blockSafeChanged(f);
    if (CountBdrBlocks()>0)
        planRoute(true);
}

double VGFlyRoute::GetOutlineSafe() const
{
    return GetEdgeShink(m_nCurEdge);
}

void VGFlyRoute::releaseSafe()
{
    MapAbstractItem::releaseSafe();
}

void VGFlyRoute::SetOutlineSafe(double f)
{
    if (!m_bSingleShrink)
    {
        if (m_fSafeDis == f)
            return;

        m_fSafeDis = f;
    }
    else
    {
        if (m_nCurEdge < 0 && f==GetEdgeShink(m_nCurEdge))
            return;

        QList<EdgeShrinkPair>::iterator itr = m_lsOutlineSafe.begin();
        for (; itr != m_lsOutlineSafe.end(); ++itr)
        {
            if (itr->first == m_nCurEdge)
            {
                m_lsOutlineSafe.erase(itr);
                break;
            }
        }
        m_lsOutlineSafe.push_back(EdgeShrinkPair(m_nCurEdge, f));
    }
    emit outlineSafeChanged(f);
    if (canPlanRoute())
        planRoute(true);
}

float VGFlyRoute::GetAngle() const
{
    return m_fAngle;
}

void VGFlyRoute::SetAngle(float f)
{
    if (m_fAngle == f)
        return;

    if (f > 180)
        f -= 360;
    m_fAngle = f > 180 ? f-360 : f;
    emit angleChanged(f);
    if (canPlanRoute())
        planRoute(false);
}

double VGFlyRoute::GetSupportHeight() const
{
    return 0;
}

void VGFlyRoute::SetSupportHeight(double f)
{
    emit supportHeightChanged(f);
    _adjustSupportAttitude(f);
}

bool VGFlyRoute::save(bool bCloud)
{
    bool ret = MapAbstractItem::save(bCloud);
    VGLandManager::PlanRouteFinish(this);
    if (!bCloud && m_time==0)
    {
        m_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
        emit planTimeChanged(m_time);
        if (m_strActId.isEmpty())
            emit actIdchanged(GetActId());
    }
    return ret;
}

void VGFlyRoute::processSaveReslt(const DescribeMap &result)
{
    DescribeMap::const_iterator itr = result.find("routeId");
    if (itr != result.end())
    {
        QString actId = itr.value().toString();
        SetActId(actId);
        SetUploaded(true);
        UpdateDB();
    }
}

bool VGFlyRoute::GetAnti() const
{
    return m_bAnti;
}

MapAbstractItem *VGFlyRoute::FindParentByProperty(const DescribeMap &dsc) const
{
    DescribeMap dscParent = VGGlobalFunc::ParentDescribe(dsc, *this);
    QString strId = dsc.contains("boundaryId") ? dsc["boundaryId"].toString() : QString();
    for (VGLandBoundary *bdr : qvgApp->landManager()->Boundaries())
    {
        if (!strId.isEmpty() && bdr->GetBoundaryId() == strId)
            return bdr;

        DescribeMap tmp;
        VGGlobalFunc::initialItemDescribe(tmp, *bdr, false);
        if (MapAbstractItem::IsContain(dscParent, tmp))
            return bdr;
    }
    return NULL;
}

void VGFlyRoute::SetAnti(bool b)
{
    if (m_bAnti == b)
        return;
    m_bAnti = b;
    emit antiChanged(b);

    if (canPlanRoute())
        planRoute(false);
}

void VGFlyRoute::SetSingleShrink(bool b)
{
    if (m_bSingleShrink == b)
        return;

    m_bSingleShrink = b;
    emit singleShrinkChanged(b);
}

int VGFlyRoute::GetCurEdge() const
{
    return m_nCurEdge;
}

MapAbstractItem *VGFlyRoute::Clone(QObject *parent) const
{
    if (VGFlyRoute *ret = new VGFlyRoute(*this))
    {
        ret->setBelongBoundary(qobject_cast<VGLandBoundary*>(parent));
        ret->SetActId(QString());
        return ret;
    }
    return NULL;
}

QGeoCoordinate VGFlyRoute::SupportPoint() const
{
    if (m_homePoint)
        return m_homePoint->GetCoordinate();

    return QGeoCoordinate();
}

qint64 VGFlyRoute::GetTime() const
{
    return m_time;
}

void VGFlyRoute::showContent(bool b)
{
    if (m_parentItem)
    {
        if (b)
            m_parentItem->SetShowType(Show_Line);

        m_parentItem->showContent(m_bShowBelonged && b);
    }

    if (m_route)
        m_route->Show(b);
}

bool VGFlyRoute::canPlanRoute()const
{
    if (VGLandBoundary *tmp = GetBelongedBoundary())
        return tmp->IsValid() && !m_bPlan;

    return false;
}

VGVehicleMission *VGFlyRoute::beginOperation()
{
    if (!IsValide())
        return NULL;

    if (!m_olMission)
    {
        m_olMission = new VGVehicleMission(this);
        connect(m_olMission, &QObject::destroyed, this, &VGFlyRoute::onChildDestroied);
    }

    if (m_olMission)
        m_olMission->SetSelected(true);

    return m_olMission;
}

void VGFlyRoute::showSelectEdge(bool b)
{
    for (VGSelectEdge *edge : m_lsSelectEdge)
    {
        edge->Show(b);
    }
    if (b && m_nCurEdge>=0 && m_nCurEdge<m_lsSelectEdge.count())
    {
        VGSelectEdge *edge = m_lsSelectEdge.at(m_nCurEdge);
        edge->SetSelected(true);
    }
}

void VGFlyRoute::setBelongBoundary(VGLandBoundary *bdr)
{
    VGLandBoundary *bdrCur = GetBelongedBoundary();

    if (bdrCur == bdr)
        return;

    qDeleteAll(m_lsSelectEdge);
    m_lsSelectEdge.clear();
    m_lsOutlineSafe.clear();
    if (bdrCur)
        bdrCur->showContent(false);
    
    if (!m_bSaveLocal)
        _genarateDefualtHome(bdr);
    else if(bdr)
        m_homePoint->SetCoordinate(bdr->GetCenter());

    SetParentItem(bdr);
    if (bdr)
        showContent(GetVisible());

    onSingleShrinkChanged(m_bSingleShrink);
    emit landNameChanged(GetLandName());
    planRoute(true);
}

VGVehicleMission *VGFlyRoute::GetVehicleMission() const
{
    return m_olMission;
}

QList<MissionItem*> VGFlyRoute::GetMissionItems() const
{
    if (m_olMission)
        return m_olMission->MissionItems();

    return QList<MissionItem*>();
}

const QList<MissionItem*> &VGFlyRoute::GetSafeBoudaryItems()
{
    if (m_safeBdrs.isEmpty())
        _calculateSafeBoundary();

    return m_safeBdrs;
}

void VGFlyRoute::SetSelected(bool b)
{
    MapAbstractItem::SetSelected(b);
    if (!b)
        return;

    if (VGLandManager *mgr = qvgApp->landManager())
        mgr->BoundaryChanged(GetBoundarys());

    if (!m_route)
    {
        VGLandManager::PlanRoute(this);
        planRoute(true);
    }
}

QList<double> VGFlyRoute::GetBoundarys() const
{
    if (VGLandBoundary *bdr = GetBelongedBoundary())
        return bdr->GetBoundarys();

    return QList<double>();
}

void VGFlyRoute::_genarateDefualtHome(VGLandBoundary *bdr)
{
    VGLandBoundary *curBdr = GetBelongedBoundary();
    if (!bdr || bdr==curBdr || (!curBdr && m_homePoint->IsValid()))
        return;

    VGLandPolygon *ol = bdr->GetBoundaryPolygon();
    if (!ol)
        return;

    double dis = 0;
    double angle = 0;
    QList<VGCoordinate *>coors = ol->GetCoordinates();
    QList<VGCoordinate *>::iterator itr = coors.begin();
    if (itr != coors.end())
        m_homePoint->SetCoordinate((*itr)->GetCoordinate());

    if (m_bUploaded)
        return;
    for (; itr != coors.end(); ++itr)
    {
        QList<VGCoordinate *>::iterator itrNext = itr + 1;
        if (itrNext == coors.end())
            break;

        double tmp = (*itr)->DistanceTo(**itrNext);
        if (tmp > dis)
        {
            dis = tmp;
            angle = (*itr)->AzimuthTo(**itrNext);
        }
    }
    SetAngle(angle);
}

void VGFlyRoute::_adjustSupportAttitude(double alt)
{
    if (!m_route)
        return;

    foreach(VGCoordinate * itr, m_route->GetCoordinates())
    {
        if (itr->GetId() == VGCoordinate::RouteBegin || itr->GetId() == VGCoordinate::RouteBack)
            itr->SetAltitude(alt);
    }
}

void VGFlyRoute::_adjustOperateAttitude(double alt)
{
    if (!m_route)
        return;

    for (VGCoordinate * itr : m_route->GetCoordinates())
    {
        if (itr->GetId() != VGCoordinate::RouteBegin && itr->GetId() != VGCoordinate::RouteBack)
            itr->SetAltitude(alt);
    }
}

void VGFlyRoute::_calculateAnggle(VGOutline *rt)
{
    if (!rt)
        return;

    QList<VGCoordinate *>coors = rt->GetCoordinates();
    QList<VGCoordinate *>::iterator itr = coors.begin();
    for (; itr != coors.end(); ++itr)
    {
        int id = (*itr)->GetId();
        if (id == VGCoordinate::RouteBegin || id == VGCoordinate::RouteOperate)
        {
            QList<VGCoordinate *>::iterator itrNext = itr + 1;
            if (itrNext != coors.end())
                m_fAngle = (*itr)->AzimuthTo(**itrNext);

            break;
        }
    }
}

void VGFlyRoute::_calculateSafeBoundary()
{
    qDeleteAll(m_safeBdrs);
    m_safeBdrs.clear();
    int seq = 0;
    int id = 0;
    for (const CoordinateList &itrLs : VGLandManager::PlanningWorker()->GenShrinkBoudary(this))
    {
        int i = 0;
        for (const QGeoCoordinate &coor : itrLs)
        {
            m_safeBdrs << new MissionItem(seq++, MAV_CMD_NAV_WAYPOINT, MAV_FRAME_GLOBAL
                , 0, id, i++, 0, coor.latitude(), coor.longitude(), 0, false, false, MAV_MISSION_TYPE_OBSTACLE);
        }
        id++;
    }
}

double VGFlyRoute::GetEdgeShink(int idx) const
{
    if (m_bSingleShrink)
    {
        for (const EdgeShrinkPair &itr : m_lsOutlineSafe)
        {
            if (idx == itr.first)
                return itr.second;
        }
    }

    return m_fSafeDis;
}

int VGFlyRoute::CountBlock() const
{
    if (VGLandBoundary *bdr = GetBelongedBoundary())
        return bdr->BlocksCount();

    return 0;
}

QList<int> VGFlyRoute::GetEdgeShinkLs() const
{
    QList<int> ret;
    if (!m_bSingleShrink)
    {
        ret << -1;
        return ret;
    }

    for (const EdgeShrinkPair &itr : m_lsOutlineSafe)
    {
        if (itr.first>=0)
            ret << itr.first;
    }
    if (ret.size() == 0)
        ret << -1;

    for (int i = m_lsSelectEdge.count()-1; i >=0; --i)
    {
        if (!ret.contains(i))
            ret.push_front(i);
    }
    return ret;
}

void VGFlyRoute::showPlanningResult(const VGRoute &result)
{
    m_bPlan = false;
    emit planningChanged(m_bPlan);

    m_totalVoyage = result.TotalVoyage() / 1000;
    m_transVoyage = result.TransVoyage() / 1000;
    m_operateVoyage = result.GetOperateVoyage() / 1000;

    QGeoCoordinate homeCoor = SupportPoint();

    if (!m_route)
    {
        if (m_route = new VGOutline(this, VGLandPolygon::FlyRoute))
        {
            m_route->SetShowType(Show_LineAndPoint);
            m_route->Show(GetVisible());
            m_route->SetWidth(1);
        }
    }
    if (!m_route)
        return;

    m_bErrorPlan = false;
    QList<VGCoordinate *>ls;
    foreach(const VGRoutePoint &point, result.GetRoutePoints())
    {
        QGeoCoordinate coor;
        VGCoordinate::transXY2Coor(homeCoor, point.GetX(), point.GetY(), coor);
        VGCoordinate::CoordinateType tp = VGCoordinate::CoordinateType(point.GetType() + VGCoordinate::RouteBegin);
        VGCoordinate *c = new VGCoordinate(coor, point.GetType()+VGCoordinate::RouteBegin, m_route);
        if (!m_bErrorPlan && VGCoordinate::RouteEnd==tp)
            m_bErrorPlan = true;
        ls << c;
    }
    m_route->ChangeAll(ls);

    emit valideChanged(IsValide());
    if (VGLandBoundary *bdr = GetBelongedBoundary())
    {
        QList<double> bds = bdr->GetBoundarys();
        VGLandManager *mgr = qvgApp->landManager();
        if (mgr && bds.count())
            mgr->BoundaryChanged(bds);
    }
    emit planRouteFinished();
    emit canPlanChanged(canPlanRoute());
    if (m_bErrorPlan)
        qvgApp->SetQmlTip(QString::fromStdWString(L"地块无法规划，请尝试修改参数!"), true);
}

float VGFlyRoute::GetSprinkleWidth() const
{
    return m_sprinkleWidth;
}

double VGFlyRoute::GetLandArea() const
{
    if (VGLandBoundary *bdr = GetBelongedBoundary())
        return bdr->GetLandArea();

    return 0;
}

QString VGFlyRoute::GetUser() const
{
    if (VGLandBoundary *bdr = GetBelongedBoundary())
        return bdr->GetUser();

    return qvgApp->GetUserId();
}

QString VGFlyRoute::_bdrSafeDis2Str() const
{
    QString str = QString::number(m_fSafeDis);
    for (const EdgeShrinkPair &itr : m_lsOutlineSafe)
    {
        str += QString(":%1,%2").arg(itr.first).arg(itr.second);
    }

    return str;
}

void VGFlyRoute::_parseSafeDis(const QString &str)
{
    QStringList strLs = str.split(":", QString::SkipEmptyParts);
    if (strLs.isEmpty())
        return;

    m_lsOutlineSafe.clear();
    m_fSafeDis = strLs.takeFirst().toDouble();
    for (const QString &itr : strLs)
    {
        QStringList lsTmp = itr.split(",", QString::SkipEmptyParts);
        if (lsTmp.size() == 2)
            m_lsOutlineSafe << EdgeShrinkPair(lsTmp.first().toInt(), lsTmp.last().toDouble());
    }
}

QString VGFlyRoute::GetInfoString() const
{
    return QString("%1;%2;%3;;%4;%5;%6;%7;%8").arg(m_sprinkleWidth).arg(m_strCropper).arg(m_strPesticide)
        .arg(m_fBlockSafeDis).arg(m_fAngle).arg(m_fPrise).arg(m_strActId).arg(_bdrSafeDis2Str());
}

void VGFlyRoute::ParseInfoString(const QString &str)
{
    QStringList strLs = str.split(";");
    if (strLs.count() < 6)
        return;
    m_sprinkleWidth = strLs.at(0).toDouble();
    m_strCropper = strLs.at(1);
    m_strPesticide = strLs.at(2);
    m_fBlockSafeDis = strLs.at(4).toDouble();
    m_fAngle = strLs.at(5).toDouble();

    if (strLs.count()>6)
        m_fPrise = strLs.at(6).toDouble();
    if (strLs.count() > 7)
        m_strActId = strLs.at(7);
    if (strLs.count() > 8)
        _parseSafeDis(strLs.at(8));
}

void VGFlyRoute::SetSprinkleWidth(float f)
{
    if (m_sprinkleWidth == f || !canPlanRoute())
        return;

    m_sprinkleWidth = f;
    emit sprinkleWidthChanged(f);
    if (canPlanRoute())
        planRoute(false);
}

float VGFlyRoute::GetSprinkleCover() const
{
    return m_sprinkleCover;
}

void VGFlyRoute::SetSprinkleCover(double f)
{
    if (f == m_sprinkleCover)
        return;

    m_sprinkleCover = f;
    emit sprinkleCoverChanged(f);
}

float VGFlyRoute::GetOperationHeight() const
{
    return m_operationHeight;
}

void VGFlyRoute::SetOperationHeight(float f)
{
    if (f == m_operationHeight)
        return;

    m_operationHeight = f;
    emit operationHeightChanged(f);
    _adjustOperateAttitude(f);
}

float VGFlyRoute::GetMedPerAcre() const
{
    return m_medPerAcre;
}

void VGFlyRoute::SetMedPerAcre(float f)
{
    m_medPerAcre = f;
}

double VGFlyRoute::GetMaxRoute() const
{
    return m_maxRoute;
}

void VGFlyRoute::SetMaxRoute(double f)
{
    if (f == m_maxRoute)
        return;

    m_maxRoute = f;
    emit maxRouteChanged(f);
}

QString VGFlyRoute::GetDescribe() const
{
    return m_strDescribe;
}

void VGFlyRoute::SetDescribe(const QString &des)
{
    if (des.isEmpty() || m_strDescribe == des)
        return;

    m_strDescribe = des;
    emit describeChanged(des);
}

QString VGFlyRoute::GetLandName() const
{
    if (VGLandInformation *land = GetBelongedLand())
        return land->GetAddress();

    return QString();
}

double VGFlyRoute::GetTotalVoyage() const
{
    return m_totalVoyage;
}

double VGFlyRoute::GetTransVoyage() const
{
    return m_transVoyage;
}

double VGFlyRoute::GetOperateVoyage() const
{
    return m_operateVoyage;
}

void VGFlyRoute::DetachVm(VGVehicleMission *vm)
{
    if (vm == m_olMission)
        m_olMission = NULL;
}

int VGFlyRoute::GetVoyageNum() const
{
    return CountChildren();
}

int VGFlyRoute::CountBdrBlocks() const
{
    if (VGLandBoundary *bdr = GetBelongedBoundary())
        return bdr->BlocksCount();

    return 0;
}

bool VGFlyRoute::IsSaveLocal() const
{
	return m_bSaveLocal;
}

bool VGFlyRoute::GetSingleShrink() const
{
    return m_bSingleShrink;
}

void VGFlyRoute::SetSaveLocal(bool bSl)
{
	if (m_bSaveLocal == bSl)
		return;
	m_bSaveLocal = bSl;
	emit saveLocalChanged(bSl);
}

bool VGFlyRoute::IsUpLoaded() const
{
	return m_bUploaded;
}

void VGFlyRoute::SetUploaded(bool bUpload)
{
	if (m_bUploaded == bUpload)
		return;
	m_bUploaded = bUpload;
	emit uplaodedChanged(bUpload);
	SetSaveLocal(false);
}

bool VGFlyRoute::operator==(const MapAbstractItem &item) const
{
    if (item.ItemType() != ItemType())
        return false;

    const VGFlyRoute &fr = *(VGFlyRoute*)&item;

    if (m_sprinkleWidth != fr.m_sprinkleWidth)
        return false;
    if (m_sprinkleCover != fr.m_sprinkleCover)
        return false;
    if (m_operationHeight != fr.m_operationHeight)
        return false;
    if (m_maxRoute != fr.m_maxRoute)
        return false;
    if (m_totalVoyage != fr.m_totalVoyage)
        return false;
    if (m_transVoyage != fr.m_transVoyage)
        return false;
    if (m_operateVoyage != fr.m_operateVoyage)
        return false;
    return *m_route == *fr.m_route;
}

DECLARE_ITEM_FACTORY(VGFlyRoute)


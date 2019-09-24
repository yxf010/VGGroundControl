#include "VGLandManager.h"
#include <QtMath>
#include <QDebug>
#include <QQmlEngine>
#include <QThread>
#include <QDateTime>
#include"VGApplication.h"
#include "VGMapManager.h"
#include "VGLandBoundary.h"
#include "VGFlyRoute.h"
#include "VGGetGpsLocation.h"
#include "VGLandPolygon.h"
#include "VGPlanningWorker.h"
#include "VGGlobalFun.h"
#include "VGBDLocation.h"
#include "VGDbManager.h"
#include "VGNetManager.h"
#include "VGToolBox.h"
#include "VGVehicle.h"

#define GPS_ValideTm 5000

static VGGetGpsLocation *s_getGpsObject = NULL;
static QThread *s_gpsThread = NULL;
static VGPlanningWorker* s_planningWorker = NULL;
static QThread*          s_planningThread = NULL;    //规划线程
static void initPlanWork()
{
    qRegisterMetaType<VGRoute>("VGRoute");
    if (s_planningThread && s_planningWorker)
        return;

    if (!s_planningThread)
        s_planningThread = new QThread;

    if (!s_planningWorker)
        s_planningWorker = new VGPlanningWorker;

    if (s_planningThread && s_planningWorker)
    {
        s_planningWorker->moveToThread(s_planningThread);
        s_planningThread->start();
    }
}
//////////////////////////////////////////////////////////////
VGLandManager::VGLandManager(QObject *parent)
: QObject(parent), m_gptTimeout(10)
, m_landCur(NULL), m_connectState(-1)
, m_landSurvey(NULL), m_routeCur(NULL)
, m_boundaryEdit(NULL), m_flyRoutePlan(NULL)
, m_tmLastGps(QDateTime::currentMSecsSinceEpoch())
, m_location(new VGBDLocation)
{
    initPlanWork();
    startGetGps(30);
    m_tmId = startTimer(2500);
}

VGLandManager::~VGLandManager()
{
    killTimer(m_tmId);
}

void VGLandManager::SetCurCoordinat(const QGeoCoordinate &coor)
{
    if (coor == m_coorCur)
        return;

    m_coorCur = coor;
    emit curCoordinateChanged(coor);
}

VGBDLocation *VGLandManager::GetBDLocation() const
{
    return m_location;
}

void VGLandManager::startGetGps(int t)
{
    initialGPS(this);
    GetGps()->startGetGps(t);
}

void VGLandManager::stopGetGps()
{
    if (GetGps())
        GetGps()->stopGetGps();
}

void VGLandManager::sltSurveyError(int errorInfo)
{
    QString strError = "";
    switch(errorInfo)
    {
    case 1000:
        strError = QString::fromStdWString(L"无打开GPS的权限") ;
        break;
    case 1001:
        strError = QString::fromStdWString(L"GPS定位服务未开启");
        break;
    case 1002:
        strError = QString::fromStdWString(L"打开GPS设备失败");
        break;
    case 1003:
        strError = QString::fromStdWString(L"打开GPS成功");
        break;
    case 1004:
        strError = QString::fromStdWString(L"打开GPS设备超时");
        break;
    case 1005:
        strError = QString::fromStdWString(L"无可用GPS设备");
        break;
    case 1006:
        strError = QString::fromStdWString(L"打开GPS设备失败，被占用或者其它错误");
        break;
    case 1007:
        strError = QString::fromStdWString(L"打开卫星设备超时");
        break;
    case 1008:
        strError = QString::fromStdWString(L"打开卫星设备通道错误");
        break;
    case 1009:
        strError = QString::fromStdWString(L"关闭卫星设备错误");
        break;
    case 1010:
        strError = QString::fromStdWString(L"没有连接测绘设备");
        break;
    default:
        return;
    }

    setError(strError);
}

bool VGLandManager::isGPSOpened() const
{
    return GetGps() && GetGps()->IsOpen();
}

void VGLandManager::setError(const QString &error)
{
    qvgApp->SetQmlTip(error);
    emit errorChanged(error);
}

QGeoCoordinate VGLandManager::GetCurCoordinate() const
{
    return m_coorCur;
}

int VGLandManager::GetNumbSatlate() const
{
    return m_numbSatlate;
}

const QList<VGLandInformation*> & VGLandManager::Lands() const
{
    return m_lstLand;
}

const QList<VGLandBoundary*> & VGLandManager::Boundaries() const
{
    return m_lstBoundary;
}

const QList<VGFlyRoute*> & VGLandManager::FlyRoutes() const
{
    return m_lstRoute;
}

bool VGLandManager::IsCurCoordValid() const
{
    return QDateTime::currentMSecsSinceEpoch() - m_tmLastGps < GPS_ValideTm;
}

void VGLandManager::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_tmId)
    {
        qint64 tm = QDateTime::currentMSecsSinceEpoch() - m_tmLastGps;
        if (tm > GPS_ValideTm && tm < GPS_ValideTm * 2)
            emit curCoordValidChanged(false);
    }
    else
    {
        QObject::timerEvent(e);
    }
}

int VGLandManager::landCount() const
{
    return m_lstLand.count();
}

VGFlyRoute *VGLandManager::preparePlanRoute(VGLandBoundary *bdy)
{
    if (!m_flyRoutePlan)
    {
        if (m_flyRoutePlan = _getPlanFlyRoute(bdy))
        {
            connect(m_flyRoutePlan, &VGFlyRoute::sigPlanRoute, s_planningWorker, &VGPlanningWorker::sltPlanningPlant);
            connect(s_planningWorker, &VGPlanningWorker::sigPlanningFinished, m_flyRoutePlan, &VGFlyRoute::showPlanningResult);
        }
    }
    if (m_flyRoutePlan)
        m_flyRoutePlan->Show(true);

    if (m_routeCur)
    {
        if (Indexof(m_routeCur) >= 0)
            m_routeCur->SetSelected(false);

        SetCurFlyRoute(NULL);
    }
    return m_flyRoutePlan;
}

VGLandInformation *VGLandManager::surveyLand()
{
    if (!m_landSurvey)
    {
        VGLandInformation *tmp = _getLandInformation();
        if (!tmp)
            return tmp;

        m_landSurvey = tmp;
        tmp->Show(true);
    }
    return m_landSurvey;
}

VGLandBoundary *VGLandManager::editBoundary(MapAbstractItem *item)
{
    if (m_boundaryEdit)
        RELEASEPOINTER(m_boundaryEdit);

    if (item && item->ItemType() == MapAbstractItem::Type_LandBoundary)
        m_boundaryEdit = (VGLandBoundary*)item->Clone();
    else
        m_boundaryEdit = _getBoundary();

    if (m_boundaryEdit)
    {
        if (item && item->ItemType() == MapAbstractItem::Type_LandInfo)
            m_boundaryEdit->SetBelongedLand((VGLandInformation*)item);
        connect(m_boundaryEdit, &VGLandBoundary::editFinished, this, &VGLandManager::onEditFinished);
        connect(m_boundaryEdit, &VGLandBoundary::destroyed, this, &VGLandManager::onChildDestroyed);
        connect(m_boundaryEdit, &VGLandBoundary::selectedChanged, this, &VGLandManager::onSelectedChanged);
        m_boundaryEdit->SetVisible(true);
        m_boundaryEdit->SetShowType(MapAbstractItem::Show_LineAndPoint);
        m_boundaryEdit->SetSelected(true);
    }

    return m_boundaryEdit;
}

void VGLandManager::quitOperation()
{
    VGMapManager *mm = qvgApp->mapManager();
    MapAbstractItem *item = mm ? mm->GetSelecedItem(MapAbstractItem::Type_FlyRoute) : NULL;
    if (VGFlyRoute *rt = dynamic_cast<VGFlyRoute*>(item))
        rt->SetSelected(false);
}

void VGLandManager::onEditFinished(VGLandBoundary *bd)
{
    if (!bd)
        return;

    if (bd == m_boundaryEdit)
    {
        m_boundaryEdit = NULL;
        bd->Show(true);
        bd->SetSelected(true);
    }
}

void VGLandManager::onGpsPosition(double lat, double lon, double alt, int sat)
{
    SetCurCoordinat(QGeoCoordinate(lat, lon, alt));
    qint64 tm = QDateTime::currentMSecsSinceEpoch();
    bool bValidChaged = tm - m_tmLastGps > GPS_ValideTm;
    m_tmLastGps = tm;
    if (sat != m_numbSatlate)
    {
        m_numbSatlate = sat;
        emit numbSatlateChanged(sat);
    }
    if (bValidChaged)
        emit curCoordValidChanged(true);
}

void VGLandManager::onPlanRouteFinished()
{
    if (m_flyRoutePlan != sender())
        return;

    m_lstRoute << m_flyRoutePlan;
    m_flyRoutePlan->Show(true);
    m_flyRoutePlan->SetSelected(true);
    m_flyRoutePlan = NULL;
}

VGLandInformation * VGLandManager::GetCurLand() const
{
    return m_landCur;
}

QString VGLandManager::GetUserId() const
{
	return qvgApp->GetUserId();
}

void VGLandManager::uploadLandInfo(MapAbstractItem::MapItemType tp, const MapListItem *parent)
{
    if (VGDbManager *db = qvgApp->dbManager())
        db->QueryItems(tp, parent);
}

VGFlyRoute *VGLandManager::GetCurFlyRoute() const
{
    return m_routeCur;
}

void VGLandManager::SetCurFlyRoute(VGFlyRoute *rt)
{
    if (m_routeCur == rt)
        return;

    if (m_routeCur)
        m_routeCur->disconnect(SIGNAL(destroyed(QObject*)), this);

    m_routeCur = rt;
    emit curFlyRouteChanged(rt);
}

QList<VGFlyRoute*> VGLandManager::GetRoutesOfBoundary(VGLandBoundary *bdr)
{
    QList<VGFlyRoute*> ret;
    if (bdr)
    {
        foreach(VGFlyRoute *bdy, m_lstRoute)
        {
            if (bdy->GetBelongedBoundary() == bdr)
                ret << bdy;
        }
    }
    return ret;
}

void VGLandManager::BoundaryChanged(const QList<double> &bdr)
{
    if (bdr.count() != 4)
        return;

    emit boundaryCoordinateChanged(bdr.at(0), bdr.at(1), bdr.at(2), bdr.at(3));
}

int VGLandManager::Indexof(VGFlyRoute *rt)
{
    return m_lstRoute.indexOf(rt);
}

void VGLandManager::AddChild(MapAbstractItem *item)
{
    if (!item)
        return;

    if (item->ItemType() == MapAbstractItem::Type_LandInfo)
        m_lstLand << (VGLandInformation*)item;
    else if (item->ItemType() == MapAbstractItem::Type_LandBoundary)
        m_lstBoundary << (VGLandBoundary*)item;
    else if (item->ItemType() == MapAbstractItem::Type_FlyRoute)
        m_lstRoute << (VGFlyRoute*)item;
    else
        return;

    qvgApp->mapManager()->InsertItem(item);
    connect(item, &MapAbstractItem::selectedChanged, this, &VGLandManager::onSelectedChanged);
    connect(item, SIGNAL(destroyed(QObject*)), this, SLOT(onChildDestroyed()));
}

void VGLandManager::InitialNetWork(VGNetManager *nm)
{
    connect(nm, &VGNetManager::sigQueryItem, this, &VGLandManager::onQueryItem);
}

VGLandInformation * VGLandManager::Clone(VGLandInformation *item, bool bNew)
{
    if (!item)
        return NULL;

    if (VGLandInformation *land = new VGLandInformation(*item))
    {
        if (bNew)
        {
            land->setActualSurveyId(QString());
            land->SetUploaded(false);
            land->SetSaveLocal(false);
            land->SetSurveyTime(0);
        }

        connect(land, &VGLandInformation::selectedChanged, this, &VGLandManager::onSelectedChanged);
        connect(land, SIGNAL(destroyed(QObject*)), this, SLOT(onChildDestroyed()));
        return appedSurveyLand(land, bNew) ? land : NULL;
    }
    return NULL;
}

VGLandBoundary *VGLandManager::Clone(VGLandBoundary *item, VGLandInformation *parent)
{
    if (!item)
        return NULL;

    if (VGLandBoundary *bdy = new VGLandBoundary(*(VGLandBoundary*)item))
    { 
        connect(bdy, &VGLandBoundary::selectedChanged, this, &VGLandManager::onSelectedChanged);
        connect(bdy, SIGNAL(destroyed(QObject*)), this, SLOT(onChildDestroyed()));
        bdy->SetBelongedLand(parent);

        return appendBoundary(bdy) ? bdy : NULL;
    }
    return NULL;
}

VGFlyRoute *VGLandManager::Clone(VGFlyRoute *item, VGLandBoundary *p)
{
    if (!item)
        return NULL;

    if (VGFlyRoute *ft = new VGFlyRoute(*(VGFlyRoute*)item))
    {
        ft->setBelongBoundary(p);
        connect(ft, &VGFlyRoute::selectedChanged, this, &VGLandManager::onSelectedChanged);
        connect(ft, &VGFlyRoute::destroyed, this, &VGLandManager::onChildDestroyed);       
        return appendRoute(ft) ? ft : NULL;
    }
    return NULL;
}

VGLandInformation *VGLandManager::GetSurveyLand() const
{
    return m_landSurvey;
}

void VGLandManager::SetCurLand(VGLandInformation *land)
{
    if (land == m_landCur)
        return;
    m_landCur = land;
    emit curLandChanged(land);
}

void VGLandManager::onChildDestroyed()
{
    QObject *obj = sender();
    if (m_landSurvey == obj)
        m_landSurvey = NULL;
    if (m_landCur == obj)
        m_landCur = NULL;

    if (m_boundaryEdit == obj)
        m_boundaryEdit = NULL;

    if (m_routeCur == obj)
        SetCurFlyRoute(NULL);
    if (m_flyRoutePlan == obj)
        m_flyRoutePlan = NULL;

	m_lstLand.removeAll((VGLandInformation*)obj);
    m_lstBoundary.removeAll((VGLandBoundary*)obj);
    m_lstRoute.removeAll((VGFlyRoute*)obj);
}

void VGLandManager::onLandSurveyed(VGLandInformation *land)
{
    if (land && !m_lstLand.contains(land))
        m_lstLand.append(land);

    if (m_landSurvey == land)
    {
        m_landSurvey = NULL;
        land->Show(true);
        land->SetSelected(true);
    }
}

void VGLandManager::onSelectedChanged(bool b)
{
    if (!b)
        return;

    if (MapAbstractItem *item = dynamic_cast<MapAbstractItem*>(sender()))
    {
        if (item->ItemType() == MapAbstractItem::Type_LandInfo)
            m_landCur = (VGLandInformation *)item;
        else if (item->ItemType() == MapAbstractItem::Type_FlyRoute)
            m_routeCur = (VGFlyRoute*)item;
    }
}

VGLandInformation *VGLandManager::_getLandInformation()
{
    VGLandInformation *ret = new VGLandInformation(this);
    if (ret)
    {
        connect(ret, &VGLandInformation::selectedChanged, this, &VGLandManager::onSelectedChanged);
        connect(ret, &VGLandInformation::destroyed, this, &VGLandManager::onChildDestroyed);
    }

    return ret;
}

VGLandBoundary * VGLandManager::_getBoundary()
{
    if (m_boundaryEdit && !m_lstBoundary.contains(m_boundaryEdit))
    {
        delete m_boundaryEdit;
        m_boundaryEdit = NULL;
    }

    return new VGLandBoundary(this);
}

MapAbstractItem *VGLandManager::findItem(const DescribeMap &dsc) const
{
    foreach (VGFlyRoute *rt, m_lstRoute)
    {
        DescribeMap tmp;
        VGGlobalFunc::initialItemDescribe(tmp, *rt);
        if (tmp == dsc)
            return rt;
    }
    foreach(VGLandBoundary *bdy, m_lstBoundary)
    {
        DescribeMap tmp;
        VGGlobalFunc::initialItemDescribe(tmp, *bdy);
        if (tmp == dsc)
            return bdy;
    }
    foreach(VGLandInformation *land, m_lstLand)
    {
        DescribeMap tmp;
        VGGlobalFunc::initialItemDescribe(tmp, *land);
        if (tmp == dsc)
            return land;
    }
    return NULL;
}

VGFlyRoute * VGLandManager::_getPlanFlyRoute(VGLandBoundary *bdy)
{
    if (VGFlyRoute *tmp = new VGFlyRoute(bdy))
    {
        connect(tmp, &VGFlyRoute::planRouteFinished, this, &VGLandManager::onPlanRouteFinished);
        connect(tmp, &VGFlyRoute::destroyed, this, &VGLandManager::onChildDestroyed);
        connect(tmp, &VGFlyRoute::selectedChanged, this, &VGLandManager::onSelectedChanged);
        return tmp;
    }

    return NULL;
}

void VGLandManager::onQueryItem(MapAbstractItem *item, const DescribeMap &dsc)
{
    if (!item)
        return;

    MapAbstractItem::MapItemType tp = item->ItemType();
    if (tp != MapAbstractItem::Type_LandInfo && tp != MapAbstractItem::Type_LandBoundary && tp != MapAbstractItem::Type_FlyRoute)
        return;

    DescribeMap tmp = dsc;
    VGGlobalFunc::initialItemDescribe(tmp, *item);
    if (!findItem(tmp))
    {
        if (tp == MapAbstractItem::Type_LandInfo)
            Clone((VGLandInformation*)item);
        else if (tp == MapAbstractItem::Type_LandBoundary)
            Clone((VGLandBoundary*)item, qobject_cast<VGLandInformation*>(item->FindParentByProperty(dsc)));
        else if (tp == MapAbstractItem::Type_FlyRoute)
            Clone((VGFlyRoute*)item, qobject_cast<VGLandBoundary*>(item->FindParentByProperty(dsc)));
    }

    item->deleteLater();
}

void VGLandManager::initialGPS(VGLandManager *mgr)
{
    if (!s_getGpsObject || !s_getGpsObject)
    {
        if (!s_getGpsObject)
            s_getGpsObject = new VGGetGpsLocation;
        if (!s_gpsThread)
            s_gpsThread = new QThread;

        if (s_getGpsObject && s_gpsThread)
            s_getGpsObject->moveToThread(s_gpsThread);
        if (m_location)
            m_location->moveToThread(s_gpsThread);
    }

    if (!s_gpsThread->isRunning())
    {
        connect(s_getGpsObject, &VGGetGpsLocation::errorInfoChanged, mgr, &VGLandManager::sltSurveyError);
        connect(s_getGpsObject, &VGGetGpsLocation::sigGetGpsCoordinate, mgr, &VGLandManager::onGpsPosition);
        s_gpsThread->start();
    }
}

bool VGLandManager::appedSurveyLand(VGLandInformation *land, bool b)
{
    if (!land)
        return false;

    QString strId = land->actualSurveyId();
    if (!strId.isEmpty())
    {
        for (VGLandInformation* item : m_lstLand)
        {
            if (item->actualSurveyId() == strId)
            {
                land->deleteLater();
                return false;
            }
        }
    }

    m_lstLand.append(land);
    qvgApp->mapManager()->InsertItem(land);
    if (!land->IsSaveLocal() && !b)
        land->save(false);
    return true;
}


bool VGLandManager::appendBoundary(VGLandBoundary *bdy)
{
    if (!bdy)
        return false;

    QString strId = bdy->GetBoundaryId();
    if (!strId.isEmpty())
    {
        for (VGLandBoundary* item : m_lstBoundary)
        {
            if (item->GetBoundaryId() == strId)
            {
                bdy->deleteLater();
                return false;
            }
        }
    }

    m_lstBoundary.append(bdy);
    qvgApp->mapManager()->InsertItem(bdy);
    if (!bdy->IsSaveLocal())
        bdy->save(false);
	return true;
}

bool VGLandManager::appendRoute(VGFlyRoute *fr)
{
    if (!fr)
        return false;

    QString strId = fr->GetActId();
    if(!strId.isEmpty())
    {
        for (VGFlyRoute* item : m_lstRoute)
        {
            if (item->GetActId() == strId)
            {
                fr->deleteLater();
                return false;
            }
        }
    }

    m_lstRoute.append(fr);
    qvgApp->mapManager()->InsertItem(fr);
    if (!fr->IsSaveLocal())
        fr->save(false);

    return true;
}

void VGLandManager::CloseGPS()
{
    if (s_getGpsObject)
	{
		s_getGpsObject->deleteLater();
		s_gpsThread->deleteLater();
		s_getGpsObject = NULL;
		s_gpsThread = NULL;
    }
}

void VGLandManager::PlanRouteFinish(VGFlyRoute *fr)
{
    if (fr && s_planningWorker)
        disconnect(s_planningWorker, &VGPlanningWorker::sigPlanningFinished, fr, &VGFlyRoute::showPlanningResult);
}

void VGLandManager::PlanRoute(VGFlyRoute *fr)
{
    if (fr)
    {
        connect(fr, &VGFlyRoute::sigPlanRoute, s_planningWorker, &VGPlanningWorker::sltPlanningPlant);
        connect(s_planningWorker, &VGPlanningWorker::sigPlanningFinished, fr, &VGFlyRoute::showPlanningResult);
    }
}

VGPlanningWorker *VGLandManager::PlanningWorker()
{
    return s_planningWorker;
}

VGGetGpsLocation *VGLandManager::GetGps()
{
    return s_getGpsObject;
}
